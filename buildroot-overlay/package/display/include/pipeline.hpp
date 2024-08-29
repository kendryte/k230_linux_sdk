#pragma once
#include <stdio.h>
#ifndef __DISPLAY_HPP__
#define __DISPLAY_HPP__

#include <algorithm>
#include <bits/types/struct_timeval.h>
#include <map>
#include <vector>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <atomic>
#include <cstddef>
#include <sys/select.h>
#include "display.h"
#include <optional>
#include <cerrno>
#include <tuple>

// base on DMA-BUF
namespace pipeline {
    class Pipeline;

    // FIXME: seperate source | sink
    class Endpoint {
    public:
        virtual bool set_buffer_num(unsigned channel, unsigned num) = 0;
        // return false if not support
        virtual bool import_buffer(unsigned channel, int fd, unsigned index, unsigned size) { return false; };
        // return -1 if not support export
        virtual int export_buffer(unsigned channel, unsigned index, unsigned& size) { return -1; };
        // buffer input callback
        virtual bool buffer_in(unsigned channel, unsigned index) = 0;
        // return -1 if no buffer to output
        virtual int buffer_out(unsigned channel) = 0;
        virtual bool start() { /* do nothing */ return true; }
        virtual void stop() {}
        virtual int fd_to_select() = 0;
    };

    class Pipeline {
        class SourceInfo {
        public:
            std::vector<std::tuple<Endpoint*, unsigned>> sinks;
            std::vector<unsigned> bufferHold;
        };
    public:
        std::map<Endpoint*, SourceInfo> forward;
        std::map<Endpoint*, std::vector<Endpoint*>> backword;
        std::map<int, Endpoint*> fdMap;
        int maxFds;
        fd_set fds;
        std::atomic<bool> running;

        Pipeline(): maxFds(0), running(false) {
            FD_ZERO(&fds);
        }
        ~Pipeline() {}

        void stop() {
            for (auto [_, ep]: fdMap) {
                ep->stop();
            }
            running.store(false);
        }

        int run() {
            running.store(true);
            for (auto [_, ep]: fdMap) {
                ep->start();
            }
            while (running.load()) {
                fd_set rfds = fds;
                struct timeval tv = {
                    .tv_sec = 0,
                    .tv_usec = 100
                };
                int ret = select(maxFds + 1, &rfds, NULL, NULL, &tv);
                if (ret > 0) {
                    // handle
                    for (auto [fd, ep]: fdMap) {
                        if (FD_ISSET(fd, &rfds)) {
                            // check source or sink
                            auto iter = backword.find(ep);
                            if (iter == backword.end()) {
                                // ep is source
                                auto info = forward[ep];
                                // dequeue buffer, source only support 1 channel
                                auto index = ep->buffer_out(0);
                                if (index < 0) {
                                    continue;
                                }
                                info.bufferHold[index] = 0;
                                for (auto [sink, channel]: info.sinks) {
                                    if (sink->buffer_in(index, channel)) {
                                        // drop
                                        info.bufferHold[index] += 1;
                                    }
                                }
                                if (info.bufferHold[index] == 0) {
                                    // no sink use, queue buffer back
                                    ep->buffer_in(index, 0);
                                }
                            } else {
                                // ep is sink
                                auto sources = iter->second;
                                auto channels = sources.size();
                                for (unsigned channel = 0; channel < channels; channel++) {
                                    auto index = ep->buffer_out(channel);
                                    if (index < 0) {
                                        continue;
                                    }
                                    auto source = sources[channel];
                                    auto info = forward[source];
                                    info.bufferHold[index] -= 1;
                                    if (info.bufferHold[index] == 0) {
                                        source->buffer_in(index, 0);
                                    }
                                }
                            }
                        }
                    }
                } else if (ret == 0) {
                    // timeout
                    continue;
                } else {
                    if (errno == EINTR) {
                        continue;
                    }
                    // error
                    return errno;
                }
            }
            return 0;
        }

        bool link(Endpoint& source, Endpoint& sink, unsigned bufferNum=5) {
            auto sinkInfo = backword[&sink];
            auto channel = sinkInfo.size();
            sinkInfo.push_back(&source);
            auto info = forward[&source];
            info.sinks.push_back({&sink, channel});
            maxFds = std::max(source.fd_to_select(), std::max(sink.fd_to_select(), maxFds));
            FD_SET(source.fd_to_select(), &fds);
            FD_SET(sink.fd_to_select(), &fds);
            fdMap[source.fd_to_select()] = &source;
            fdMap[sink.fd_to_select()] = &sink;
            // setup buffer
            if (!source.set_buffer_num(0, bufferNum)) {
                return false;
            }
            if (!sink.set_buffer_num(channel, bufferNum)) {
                return false;
            }
            // export buffer from sink
            unsigned size;
            bool exportFromSink = true;
            for (unsigned i = 0; i < bufferNum; i++) {
                int fd = sink.export_buffer(channel, i, size);
                if (fd < 0) {
                    exportFromSink = false;
                    break;
                }
                if (!source.import_buffer(0, fd, i, size)) {
                    // error
                    return false;
                }
            }
            if (!exportFromSink) {
                // export from source
                for (unsigned i = 0; i < bufferNum; i++) {
                    if (!source.export_buffer(channel, i, size)) {
                        // no buffer, error
                        return false;
                    }
                }
            }
            return true;
        }
    };

    class Vicap: public Endpoint {
    public:
        int fd;
        unsigned size;
        Vicap(int fd): fd(fd), size(0) {}
        ~Vicap() {
            if (fd >= 0)
                close(fd);
        }

        static std::optional<Vicap> create(unsigned device, unsigned width, unsigned height, uint32_t fourcc) {
            char filepath[128];
            snprintf(filepath, sizeof(filepath), "/dev/video%u", device);
            struct v4l2_capability capbility;
            int fd = open(filepath, O_RDWR);
            if (fd < 0) {
                return {};
            };
            if (ioctl(fd, VIDIOC_QUERYCAP, &capbility)) {
                close(fd);
                return {};
            }
            struct v4l2_fmtdesc fmtdesc;
            memset(&fmtdesc, 0, sizeof(fmtdesc));
            fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
                printf(
                    "/dev/video%u support format %c%c%c%c",
                    device,
                    (fmtdesc.pixelformat >> 0) & 0xff,
                    (fmtdesc.pixelformat >> 8) & 0xff,
                    (fmtdesc.pixelformat >> 16) & 0xff,
                    (fmtdesc.pixelformat >> 24) & 0xff
                );
                fmtdesc.index += 1;
            }
            struct v4l2_format format;
            format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (ioctl(fd, VIDIOC_G_FMT, &format)) {
                close(fd);
                return {};
            }
            format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            format.fmt.pix.pixelformat = fourcc;
            format.fmt.pix.width = width;
            format.fmt.pix.height = height;
            if (ioctl(fd, VIDIOC_S_FMT, &format)) {
                close(fd);
                return {};
            }
            return Vicap(fd);
        }

        bool start() {
            int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (ioctl(fd, VIDIOC_STREAMON, &type)) {
                return false;
            }
            return true;
        }

        void stop() {
            int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            ioctl(fd, VIDIOC_STREAMOFF, &type);
        }

        bool import_buffer(unsigned channel, int fd, unsigned index, unsigned size) {
            // QBUF
            struct v4l2_buffer buffer;
            memset(&buffer, 0, sizeof(buffer));
            buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buffer.memory = V4L2_MEMORY_DMABUF;
            buffer.index = index;
            buffer.m.fd = fd;
            buffer.length = size;
            if (ioctl(fd, VIDIOC_QBUF, &buffer)) {
                return false;
            }
            return true;
        }

        bool set_buffer_num(unsigned channel, unsigned num) {
            struct v4l2_requestbuffers request_buffer;
            memset(&request_buffer, 0, sizeof(request_buffer));
            request_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            request_buffer.memory = V4L2_MEMORY_DMABUF;
            request_buffer.count = num;
            if (ioctl(fd, VIDIOC_REQBUFS, &request_buffer)) {
                return false;
            }
            return true;
        }

        int buffer_out(unsigned channel) {
            // DQBUF
            struct v4l2_buffer buffer;
            memset(&buffer, 0, sizeof(buffer));
            buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buffer.memory = V4L2_MEMORY_DMABUF;
            if (ioctl(fd, VIDIOC_DQBUF, &buffer)) {
                // TODO: warning
                return -1;
            }
            return buffer.index;
        }

        bool buffer_in(unsigned channel, unsigned index) {
            // QBUF
            struct v4l2_buffer buffer;
            memset(&buffer, 0, sizeof(buffer));
            buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buffer.memory = V4L2_MEMORY_DMABUF;
            buffer.index = index;
            if (ioctl(fd, VIDIOC_QBUF, &buffer)) {
                // warning
                return false;
            }
            return true;
        }

        int fd_to_select() {
            return fd;
        }
    };

    class Display: public Endpoint {
        class Plane {
        public:
            struct display_plane* plane;
            std::vector<struct display_buffer*> buffers;
            unsigned width;
            unsigned height;
            unsigned x;
            unsigned y;
            int queue[3];
            unsigned wp;
            bool updated;

            Plane(struct display_plane* plane, unsigned width, unsigned height, unsigned x, unsigned y):
                wp(0), plane(plane), width(width), height(height), updated(false), x(x), y(y)
            {
                for (unsigned i = 0; i < 3; i++) {
                    queue[i] = -1;
                }
            }

            bool input(unsigned index) {
                if (updated) {
                    return false;
                }
                queue[wp] = index;
                return true;
            }

            int output() {
                if (updated) {
                    updated = false;
                    auto b = buffers[queue[wp]];
                    display_update_buffer(b, x, y);
                    wp = (wp + 1) % sizeof(queue);
                    return queue[wp];
                } else {
                    // no new frame, no output
                    return -1;
                }
            }
        };
    public:
        struct display* d;
        std::vector<Plane> planes;

        Display(struct display* d): d(d) {
            printf("construct display fd: %d\n", this->d->fd);
        }
        ~Display() {
            display_exit(d);
        }

        bool createChannel(unsigned width, unsigned height, uint32_t fourcc) {
            printf("fd: %d\n", d->fd);
            auto p = display_get_plane(d, fourcc);
            if (p == nullptr) {
                return false;
            }
            planes.emplace_back(p, width, height, 0, 0);
            return true;
        }

        static std::optional<Display> create(unsigned device=0) {
            auto d = display_init(device);
            if (d == nullptr) {
                return {};
            }
            printf("fd: %d\n", d->fd);
            return Display(d);
        }

        bool start() {
            // commit buffer trig event
            return display_commit_buffer(planes[0].buffers[0], planes[0].x, planes[0].y) == 0;
        }

        bool set_buffer_num(unsigned channel, unsigned num) {
            auto plane = planes[channel];
            for (unsigned i = 0; i < num; i++) {
                auto buffer = display_allocate_buffer(plane.plane, plane.width, plane.height);
                if (buffer) {
                    plane.buffers.push_back(buffer);
                } else {
                    for (auto b: plane.buffers) {
                        display_free_buffer(b);
                    }
                    plane.buffers.clear();
                    return false;
                }
            }
            return true;
        }

        int export_buffer(unsigned channel, unsigned index, unsigned& size) {
            auto plane = planes[channel];
            auto b = plane.buffers[index];
            size = b->size;
            return b->dmabuf_fd;
        }

        bool buffer_in(unsigned channel, unsigned index) {
            auto plane = planes[channel];
            return plane.input(index);
        }

        int buffer_out(unsigned channel) {
            if (channel == 0) {
                display_handle_vsync(d);
            }
            auto plane = planes[channel];
            int ret = plane.output();
            if (channel == planes.size() - 1) {
                // the last channel
                display_commit(d);
            }
            return ret;
        }

        int fd_to_select() {
            return this->d->fd;
        }
    };
}

#endif