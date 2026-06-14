#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <drm_fourcc.h>
#include <linux/videodev2.h>
#include <v4l2-drm.h>

#include "v4l2_drm.hpp"

namespace py = pybind11;

//=============================================================================
// Python module binding
//=============================================================================
PYBIND11_MODULE(k230_v4l2_drm, m) {
    m.doc() = "Python bindings for v4l2-drm library";

    // Rotation enum
    py::enum_<drm_rotation>(m, "Rotation")
        .value("ROTATION_0", rotation_0)
        .value("ROTATION_90", rotation_90)
        .value("ROTATION_180", rotation_180)
        .value("ROTATION_270", rotation_270)
        .value("REFLECT_X", rotation_reflect_x)
        .value("REFLECT_Y", rotation_reflect_y)
        .export_values();

    // Format constants
    m.attr("FORMAT_NV12") = py::bytes("NV12");
    m.attr("FORMAT_NV21") = py::bytes("NV21");
    m.attr("FORMAT_NV16") = py::bytes("NV16");
    m.attr("FORMAT_NV61") = py::bytes("NV61");
    m.attr("FORMAT_BGR24") = py::bytes("BGR3");
    m.attr("FORMAT_RGB24") = py::bytes("RGB3");
    m.attr("FORMAT_YUYV") = py::bytes("YUYV");

    m.attr("V4L2_PIX_FMT_NV12") = V4L2_PIX_FMT_NV12;
    m.attr("V4L2_PIX_FMT_NV21") = V4L2_PIX_FMT_NV21;
    m.attr("V4L2_PIX_FMT_NV16") = V4L2_PIX_FMT_NV16;
    m.attr("V4L2_PIX_FMT_NV61") = V4L2_PIX_FMT_NV61;
    m.attr("V4L2_PIX_FMT_BGR24") = V4L2_PIX_FMT_BGR24;
    m.attr("V4L2_PIX_FMT_RGB24") = V4L2_PIX_FMT_RGB24;
    m.attr("V4L2_PIX_FMT_YUYV") = V4L2_PIX_FMT_YUYV;

    m.attr("DRM_FORMAT_NV12") = DRM_FORMAT_NV12;
    m.attr("DRM_FORMAT_NV21") = DRM_FORMAT_NV21;
    m.attr("DRM_FORMAT_NV16") = DRM_FORMAT_NV16;
    m.attr("DRM_FORMAT_NV61") = DRM_FORMAT_NV61;
    m.attr("DRM_FORMAT_BGR888") = DRM_FORMAT_BGR888;
    m.attr("DRM_FORMAT_RGB888") = DRM_FORMAT_RGB888;
    m.attr("DRM_FORMAT_YUYV") = DRM_FORMAT_YUYV;
    m.attr("DRM_FORMAT_ARGB8888") = DRM_FORMAT_ARGB8888;
    m.attr("DRM_FORMAT_XRGB8888") = DRM_FORMAT_XRGB8888;

    // V4l2Drm class
    py::class_<V4l2Drm>(m, "V4l2Drm")
        .def(py::init<size_t, bool>(), py::arg("context_num") = 1,
             py::arg("osd") = false)
        // Display initialization
        .def("drm_init", &V4l2Drm::drm_init, py::arg("drm_id") = 0,
             "Initialize display, returns (width, height) tuple on success, "
             "(-1, "
             "-1) on failure")
        // Configuration methods
        .def("set_context", &V4l2Drm::set_context, py::arg("index") = 0,
             py::arg("device") = 1, py::arg("width") = 640,
             py::arg("height") = 480, py::arg("format") = "NV12",
             py::arg("display") = true)
        .def("set_offset", &V4l2Drm::set_offset, py::arg("index") = 0,
             py::arg("x") = 0, py::arg("y") = 0)
        .def("set_crop", &V4l2Drm::set_crop, py::arg("index") = 0,
             py::arg("w") = 0, py::arg("h") = 0, py::arg("ox") = 0,
             py::arg("oy") = 0)
        .def("set_rotation", &V4l2Drm::set_rotation, py::arg("index") = 0,
             py::arg("rotation") = 0)
        .def("set_flip", &V4l2Drm::set_flip, py::arg("index") = 0,
             py::arg("hflip") = 0, py::arg("vflip") = 0)
        .def("set_buffer_num", &V4l2Drm::set_buffer_num, py::arg("index") = 0,
             py::arg("num") = 4)
        // Property getters
        .def("get_context_count", &V4l2Drm::get_context_count)
        .def("get_width", &V4l2Drm::get_width, py::arg("index") = 0)
        .def("get_height", &V4l2Drm::get_height, py::arg("index") = 0)
        .def("get_device", &V4l2Drm::get_device, py::arg("index") = 0)
        .def("get_video_format", &V4l2Drm::get_video_format,
             py::arg("index") = 0)
        .def("get_video_format_str", &V4l2Drm::get_video_format_str,
             py::arg("index") = 0)
        .def("get_offset_x", &V4l2Drm::get_offset_x, py::arg("index") = 0)
        .def("get_offset_y", &V4l2Drm::get_offset_y, py::arg("index") = 0)
        .def("get_rotation", &V4l2Drm::get_rotation, py::arg("index") = 0)
        .def("get_hflip", &V4l2Drm::get_hflip, py::arg("index") = 0)
        .def("get_vflip", &V4l2Drm::get_vflip, py::arg("index") = 0)
        .def("get_frame_count", &V4l2Drm::get_frame_count, py::arg("index") = 0)
        .def("set_frame_count", &V4l2Drm::set_frame_count, py::arg("index") = 0,
             py::arg("count") = 0, "Set frame count for the specified context")
        .def("get_video_fd", &V4l2Drm::get_video_fd, py::arg("index") = 0)
        // Buffer access
        .def("get_buffer_data", &V4l2Drm::get_buffer_data, py::arg("index") = 0)
        .def("get_buffer_array", &V4l2Drm::get_buffer_array,
             py::arg("index") = 0)
        // Control methods
        .def("setup", &V4l2Drm::setup,
             "Setup and initialize the v4l2-drm pipeline")
        .def("dump_start", &V4l2Drm::dump_start, py::arg("index") = 0,
             "Start video capture for the specified context")
        .def("dump_stop", &V4l2Drm::dump_stop, py::arg("index") = 0,
             "Stop video capture for the specified context")
        .def("dump_frame", &V4l2Drm::dump_frame, py::arg("index") = 0,
             py::arg("timeout_ms") = 1000,
             "Capture a single frame, returns True on success")
        .def("dump_release", &V4l2Drm::dump_release, py::arg("index") = 0,
             "Release the captured frame buffer")
        // Display loop (non-blocking, background thread)
        .def("display_start", &V4l2Drm::display_start,
             "Start the display run loop in a background thread")
        .def("display_stop", &V4l2Drm::display_stop,
             "Stop the display run loop and wait for thread to finish")
        // Display frame count (read/write access from Python)
        .def_property("display_frame_count",
             &V4l2Drm::get_display_frame_count,
             &V4l2Drm::set_display_frame_count,
             "Current display frame counter")
        // OSD methods
        .def("osd_update", &V4l2Drm::osd_update, py::arg("img"));

    m.attr("__version__") = "1.0.0";
}
