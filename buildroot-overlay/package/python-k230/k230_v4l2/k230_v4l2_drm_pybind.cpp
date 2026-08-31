#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <drm_fourcc.h>
#include <linux/videodev2.h>
#include <v4l2-drm.h>
#include <v4l2-drm++.hpp>

namespace py = pybind11;

//=============================================================================
// Python module binding - thin wrapper over C++ V4l2Drm class
//=============================================================================
PYBIND11_MODULE(k230_v4l2_drm, m) {
    m.doc() = "Python bindings for v4l2-drm library (pybind11)";

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

    // V4l2Drm class - thin pybind11 wrapper
    py::class_<V4l2Drm>(m, "V4l2Drm")
        .def(py::init<size_t, bool>(), py::arg("context_num") = 1,
             py::arg("osd") = false)
        // Display initialization - convert std::pair to py::tuple
        .def("drm_init",
             [](V4l2Drm& self, int drm_id) -> py::tuple {
                 auto result = self.drm_init(drm_id);
                 return py::make_tuple(result.first, result.second);
             },
             py::arg("drm_id") = 0,
             "Initialize display, returns (width, height) tuple on success, "
             "(-1, -1) on failure")
        // Configuration methods - pass through directly
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
        // Property getters - pass through directly
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
        // Buffer access - convert BufferInfo to py::array_t
        .def("get_buffer_data",
             [](V4l2Drm& self, size_t index) -> py::array_t<uint8_t> {
                 auto info = self.get_buffer_data(index);
                 return py::array_t<uint8_t>(
                     std::vector<ssize_t>{static_cast<ssize_t>(info.size)},
                     static_cast<uint8_t*>(info.ptr));
             },
             py::arg("index") = 0)
        // Buffer array access - convert ArrayInfo to py::array_t
        .def("get_buffer_array",
             [](V4l2Drm& self, size_t index) -> py::array_t<uint8_t> {
                 auto info = self.get_buffer_array(index);
                 return py::array_t<uint8_t>(
                     info.shape,
                     info.strides,
                     static_cast<uint8_t*>(info.ptr));
             },
             py::arg("index") = 0)
        // Control methods - pass through directly
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
        // Display loop
        .def("display_start", &V4l2Drm::display_start,
             "Start the display run loop in a background thread")
        .def("display_stop", &V4l2Drm::display_stop,
             "Stop the display run loop and wait for thread to finish")
        // Display frame count
        .def_property("display_frame_count",
             &V4l2Drm::get_display_frame_count,
             &V4l2Drm::set_display_frame_count,
             "Current display frame counter")
        // OSD methods - convert py::array_t to void*+size
        .def("set_osd_format", &V4l2Drm::set_osd_format, py::arg("fourcc"),
             "Set OSD format (fourcc) DRM_FORMAT_ARGB8888/DRM_FORMAT_RGB888")
        .def("osd_update",
             [](V4l2Drm& self, py::array_t<uint8_t> img) -> int {
                 py::buffer_info buf = img.request();
                 // array_t<uint8_t> guarantees itemsize == 1, so element
                 // count equals byte count
                 return self.osd_update(buf.ptr, static_cast<size_t>(buf.size));
             },
             py::arg("img"))
        // Helper: expose internal display pointer for LVGL integration
        // Returns the pointer as an integer so it can be passed across
        // pybind11 module boundaries to lvgl.k230_driver_init()
        .def("get_display_ptr",
             [](V4l2Drm& self) -> uintptr_t {
                 return reinterpret_cast<uintptr_t>(self.get_display());
             },
             "Get internal display pointer as integer (for LVGL k230_driver_init)");

    m.attr("__version__") = "1.0.0";
}
