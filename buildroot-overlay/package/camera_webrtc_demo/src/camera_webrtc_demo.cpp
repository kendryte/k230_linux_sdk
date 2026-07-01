#include "camera_webrtc_demo.h"
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "osd.h"
#include "OSD1_40x40_argb.c"
#include "web_page.h"

static unsigned int g_small_osd_data[40 * 40] __attribute__((aligned(0x1000)));
static int g_frame_count = 0;

static void GenerateSmallRainbow() {
    g_frame_count++;
    int time_offset = (g_frame_count / 2) % 360;
    for (int y = 0; y < 40; y++) {
        for (int x = 0; x < 40; x++) {
            int hue = (time_offset + x * 10 + y * 10) % 360;
            int red, green, blue;
            if (hue < 60) {
                red = 255; green = hue * 255 / 60; blue = 0;
            } else if (hue < 120) {
                red = 255 - (hue - 60) * 255 / 60; green = 255; blue = 0;
            } else if (hue < 180) {
                red = 0; green = 255; blue = (hue - 120) * 255 / 60;
            } else if (hue < 240) {
                red = 0; green = 255 - (hue - 180) * 255 / 60; blue = 255;
            } else if (hue < 300) {
                red = (hue - 240) * 255 / 60; green = 0; blue = 255;
            } else {
                red = 255; green = 0; blue = 255 - (hue - 300) * 255 / 60;
            }
            int alpha = 200;
            g_small_osd_data[y * 40 + x] = (alpha << 24) | (red << 16) | (green << 8) | blue;
        }
    }
}

MyCameraWebRtcDemo *MyCameraWebRtcDemo::instance_ = nullptr;

MyCameraWebRtcDemo::MyCameraWebRtcDemo() {
    instance_ = this;
}

MyCameraWebRtcDemo::~MyCameraWebRtcDemo() {
    DeInit();
    instance_ = nullptr;
}

int MyCameraWebRtcDemo::Init(const KdMediaInputConfig &config, int port) {
    input_config_ = config;
    port_ = port;

    feature_config_.on_venc_data = this;

    if (input_config_.osd_region > 0) {
        input_config_.osd_callback = OnUpdateOsdRegions;
        input_config_.osd_user_data = this;
    }

    if (media_.configure_media_features(input_config_, feature_config_) < 0) {
        printf("[WebRTC] Media configure failed\n");
        return -1;
    }

    peer_init();
    PeerConfiguration pc_config = {
        .ice_servers = {{0}},
        .audio_codec = CODEC_NONE,
        .video_codec = CODEC_H264,
        .datachannel = DATA_CHANNEL_NONE,
        .onaudiotrack = NULL,
        .onvideotrack = NULL,
        .on_request_keyframe = NULL,
        .user_data = this,
    };
    pc_ = peer_connection_create(&pc_config);
    if (!pc_) {
        printf("[WebRTC] peer_connection_create failed\n");
        return -1;
    }

    peer_connection_oniceconnectionstatechange(pc_, OnIceConnectionStateChange);
    peer_connection_onicecandidate(pc_, OnIceCandidate);

    return 0;
}

int MyCameraWebRtcDemo::DeInit() {
    Stop();

    if (pc_) {
        peer_connection_destroy(pc_);
        pc_ = nullptr;
    }
    peer_deinit();

    if (sps_pps_buf_) {
        free(sps_pps_buf_);
        sps_pps_buf_ = nullptr;
        sps_pps_size_ = 0;
    }

    media_.destroy_media_features();
    return 0;
}

int MyCameraWebRtcDemo::Start() {
    if (started_) return 0;

    media_.enable_media_features();

    interrupted_ = 0;
    pthread_create(&peer_thread_, NULL, PeerConnectionTask, this);

    http_server_start(port_, OnHttpRequest);

    started_ = true;

    char local_ip[64] = "0.0.0.0";
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock >= 0) {
        static const char *ifnames[] = {"eth0", "en0", "wlan0", "u0", "e0", NULL};
        struct ifreq ifr;
        for (int i = 0; ifnames[i]; i++) {
            memset(&ifr, 0, sizeof(ifr));
            strncpy(ifr.ifr_name, ifnames[i], sizeof(ifr.ifr_name) - 1);
            if (ioctl(sock, SIOCGIFADDR, &ifr) == 0) {
                struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
                if (sin->sin_addr.s_addr != htonl(INADDR_ANY)) {
                    inet_ntop(AF_INET, &sin->sin_addr, local_ip, sizeof(local_ip));
                    break;
                }
            }
        }
        close(sock);
    }

    printf("\n========================================\n");
    printf("  Camera WebRTC Demo\n");
    printf("========================================\n");
    printf("  Encode: %dx%d @ %dkbps H264\n", input_config_.venc_width, input_config_.venc_height, input_config_.bitrate_kbps);
    printf("  Open in browser:\n");
    printf("  http://%s:%d\n", local_ip, port_);
    printf("========================================\n\n");

    return 0;
}

int MyCameraWebRtcDemo::Stop() {
    if (!started_) return 0;

    http_server_stop();
    interrupted_ = 1;
    pthread_join(peer_thread_, NULL);

    started_ = false;
    media_.disable_media_features();
    return 0;
}

// ── WebRTC callbacks ──

void MyCameraWebRtcDemo::OnIceConnectionStateChange(PeerConnectionState state, void *data) {
    (void)data;
    printf("[WebRTC] State: %s\n", peer_connection_state_to_string(state));
    if (instance_) instance_->state_ = state;
}

void MyCameraWebRtcDemo::OnIceCandidate(char *sdp, void *userdata) {
    MyCameraWebRtcDemo *self = static_cast<MyCameraWebRtcDemo *>(userdata);
    pthread_mutex_lock(&self->offer_mutex_);
    if (self->offer_sdp_) {
        free(self->offer_sdp_);
        self->offer_sdp_ = nullptr;
    }
    self->offer_sdp_ = strdup(sdp);
    self->offer_ready_ = 1;
    pthread_cond_signal(&self->offer_cond_);
    pthread_mutex_unlock(&self->offer_mutex_);
}

void MyCameraWebRtcDemo::OnUpdateOsdRegions(OsdRegion* regions, int region_count, void* user_data) {
    (void)user_data;
    if (!regions || region_count <= 0) return;

    GenerateSmallRainbow();
    int time_offset = (g_frame_count / 3) % 360;
    int radius = 250;
    int center_x = 640;
    int center_y = 360;

    for (int i = 0; i < region_count; i++) {
        int angle = (i * 360 / region_count + time_offset) % 360;
        float rad = angle * 3.14159f / 180.0f;
        int x = center_x + (int)(cos(rad) * radius) - 20;
        int y = center_y + (int)(sin(rad) * radius) - 20;
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x > 1240) x = 1240;
        if (y > 680) y = 680;
        regions[i].x = x;
        regions[i].y = y;
        regions[i].width = 40;
        regions[i].height = 40;
        regions[i].osd_image_data = g_small_osd_data;
        regions[i].osd_image_size = sizeof(g_small_osd_data);
        regions[i].enabled = 1;
    }
}

void *MyCameraWebRtcDemo::PeerConnectionTask(void *data) {
    MyCameraWebRtcDemo *self = static_cast<MyCameraWebRtcDemo *>(data);
    while (!self->interrupted_) {
        self->peer_in_pc_ = 1;
        peer_connection_loop(self->pc_);
        self->peer_in_pc_ = 0;
        usleep(1000);
    }
    return NULL;
}

// ── HTTP signaling ──

void MyCameraWebRtcDemo::OnHttpRequest(const char *method, const char *path,
                                        const char *body, int body_len,
                                        http_response_t *response) {
    if (!instance_) return;
    MyCameraWebRtcDemo *self = instance_;

    if (strcmp(method, "GET") == 0 && (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0)) {
        response->status = 200;
        response->content_type = "text/html; charset=utf-8";
        response->body = WEB_PAGE_HTML;
        response->body_len = strlen(WEB_PAGE_HTML);

    } else if (strcmp(method, "GET") == 0 && strcmp(path, "/offer") == 0) {
        if (self->state_ == PEER_CONNECTION_COMPLETED || self->state_ == PEER_CONNECTION_CONNECTED) {
            peer_connection_close(self->pc_);
            self->state_ = PEER_CONNECTION_CLOSED;
            /* Drain: wait for both worker threads to exit pc_ internals.
             * close() sets pc->state=CLOSED so no new loop/send_video calls
             * will enter the dtls_srtp code path, but a call already past the
             * state check may still be using srtp_out. Wait until both
             * in-use flags are 0 — guaranteed within ~1ms. */
            while (self->peer_in_pc_ || self->venc_in_pc_) {
                usleep(1000);
            }
        }

        self->offer_ready_ = 0;
        if (self->offer_sdp_) {
            free(self->offer_sdp_);
            self->offer_sdp_ = nullptr;
        }

        peer_connection_create_offer(self->pc_);

        pthread_mutex_lock(&self->offer_mutex_);
        while (!self->offer_ready_ && !self->interrupted_) {
            pthread_cond_wait(&self->offer_cond_, &self->offer_mutex_);
        }

        if (self->offer_sdp_) {
            response->status = 200;
            response->content_type = "application/sdp";
            response->body = self->offer_sdp_;
            response->body_len = strlen(self->offer_sdp_);
        } else {
            response->status = 500;
            response->content_type = "text/plain";
            response->body = "Failed to create offer";
            response->body_len = strlen(response->body);
        }
        pthread_mutex_unlock(&self->offer_mutex_);

    } else if (strcmp(method, "POST") == 0 && strcmp(path, "/answer") == 0) {
        if (!body || body_len <= 0) {
            response->status = 400;
            response->content_type = "text/plain";
            response->body = "Missing SDP body";
            response->body_len = strlen(response->body);
            return;
        }
        char *answer_copy = (char *)calloc(1, body_len + 1);
        memcpy(answer_copy, body, body_len);
        answer_copy[body_len] = '\0';

        peer_connection_set_remote_description(self->pc_, answer_copy, SDP_TYPE_ANSWER);

        response->status = 200;
        response->content_type = "text/plain";
        response->body = "OK";
        response->body_len = strlen(response->body);
        free(answer_copy);

    } else {
        response->status = 404;
        response->content_type = "text/plain";
        response->body = "Not Found";
        response->body_len = strlen(response->body);
    }
}

// ── Encoded data from KdMedia → WebRTC ──

void MyCameraWebRtcDemo::OnVEncData(unsigned char *data, size_t size, bool bKeyFrame, uint64_t timestamp) {
    if (!started_) return;
    if (size < 5 || data[0] != 0 || data[1] != 0) return;

    uint8_t nal_type = data[4] & 0x1f;

    if (nal_type == 7 || nal_type == 8) {
        if (sps_pps_buf_) free(sps_pps_buf_);
        sps_pps_buf_ = (uint8_t *)malloc(size);
        if (sps_pps_buf_) {
            memcpy(sps_pps_buf_, data, size);
            sps_pps_size_ = size;
        }
        return;
    }

    if (state_ != PEER_CONNECTION_COMPLETED) return;

    /* Mark in-use BEFORE the state check so the reconnect path (GET /offer)
     * sees us and waits. If we checked state first, close() could happen
     * between the check and setting the flag — the drain would miss us. */
    venc_in_pc_ = 1;
    if (state_ != PEER_CONNECTION_COMPLETED) {
        venc_in_pc_ = 0;
        return;
    }

    if (nal_type == 5 && sps_pps_buf_ && sps_pps_size_ > 0) {
        peer_connection_send_video(pc_, sps_pps_buf_, sps_pps_size_, timestamp);
    }
    peer_connection_send_video(pc_, data, size, timestamp);
    venc_in_pc_ = 0;
}
