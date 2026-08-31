#include "ai2d_wrapper.h"
#include <fstream>
#include "pystreambuf.h"
#include "pytype_utils.h"
#include "type_casters.h"
#include <nncase/runtime/interpreter.h>
#include <nncase/runtime/runtime_op_utility.h>
#include <nncase/version.h>
#include <pybind11/iostream.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <sstream>


namespace py = pybind11;

using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::k230;
using namespace nncase::clr;
using namespace nncaseruntime_AI2D_api;

namespace pybind11::detail {
std::atomic_bool g_python_shutdown = false;
}

PYBIND11_MODULE(_nncaseruntime_k230, m)
{
    m.doc() = "nncase runtime Library";
    m.attr("__version__") = NNCASE_VERSION;

#include "runtime_tensor.inl"

    py::enum_<ai2d_format>(m, "AI2D_FORMAT", py::arithmetic(), "AI2D format types")
        .value("YUV420_NV12", (enum ai2d_format)ai2d_format::YUV420_NV12, "YUV420 NV12 format")
        .value("YUV420_NV21", (enum ai2d_format)ai2d_format::YUV420_NV21, "YUV420 NV21 format")
        .value("YUV420_I420", (enum ai2d_format)ai2d_format::YUV420_I420, "YUV420 I420 format")
        .value("NCHW_FMT", (enum ai2d_format)ai2d_format::NCHW_FMT, "NCHW format")
        .value("RGB_packed", (enum ai2d_format)ai2d_format::RGB_packed, "RGB packed format")
        .value("RAW16", (enum ai2d_format)ai2d_format::RAW16, "RAW16 format");

    py::enum_<ai2d_pad_mode>(m, "AI2D_PAD_MODE", py::arithmetic(), "AI2D padding modes")
        .value("constant", (enum ai2d_pad_mode)ai2d_pad_mode::constant, "Constant padding")
        .value("copy", (enum ai2d_pad_mode)ai2d_pad_mode::copy, "copy padding")
        .value("mirror", (enum ai2d_pad_mode)ai2d_pad_mode::mirror, "mirror padding");

    py::enum_<ai2d_interp_method>(m, "AI2D_INTERP_METHOD", py::arithmetic(), "AI2D interpolation methods")
        .value("tf_nearest", (enum ai2d_interp_method)ai2d_interp_method::tf_nearest, "TensorFlow nearest interpolation")
        .value("tf_bilinear", (enum ai2d_interp_method)ai2d_interp_method::tf_bilinear, "TensorFlow bilinear interpolation");

    py::enum_<ai2d_interp_mode>(m, "AI2D_INTERP_MODE", py::arithmetic(), "AI2D interpolation modes")
        .value("none", (enum ai2d_interp_mode)ai2d_interp_mode::none, "No interpolation")
        .value("align_corner", (enum ai2d_interp_mode)ai2d_interp_mode::align_corner, "align_corner interpolation")
        .value("half_pixel", (enum ai2d_interp_mode)ai2d_interp_mode::half_pixel, "half_pixel interpolation");

    py::class_<AI2D>(m, "AI2D", "AI2D class for image processing")
        .def(py::init(), "Default constructor")
        .def("set_datatype",
            [](AI2D &ai2d, ai2d_format src_format, ai2d_format dst_format, py::object src_type, py::object dst_type)
            { ai2d.set_datatype(src_format, dst_format, from_type(src_type), from_type(dst_type)); },
            "Set the data type for AI2D",
            py::arg("src_format"),
            py::arg("dst_format"),
            py::arg("src_type"),
            py::arg("dst_type"))
        .def("set_crop_param",
            [](AI2D &ai2d, bool crop_flag, int32_t start_x, int32_t start_y, int32_t width, int32_t height)
            { ai2d.set_crop_param(crop_flag, start_x, start_y, width, height); },
            "Set the crop parameters",
            py::arg("crop_flag"),
            py::arg("start_x"),
            py::arg("start_y"),
            py::arg("width"),
            py::arg("height"))
        .def("set_shift_param",
            [](AI2D &ai2d, bool shift_flag, int32_t shift_val)
            { ai2d.set_shift_param(shift_flag, shift_val); },
            "Set the shift parameters",
            py::arg("shift_flag"),
            py::arg("shift_val"))
        .def("set_pad_param",
            [](AI2D &ai2d, bool pad_flag, const std::vector<pybind11::ssize_t> paddings, int pad_mode, py::list pad_val)
            { ai2d.set_pad_param(pad_flag, to_rt_paddings(paddings), (enum ai2d_pad_mode)pad_mode, from_py_list<int32_t>(pad_val)); },
            "Set the padding parameters",
            py::arg("pad_flag"),
            py::arg("paddings"),
            py::arg("pad_mode"),
            py::arg("pad_val"))
        .def("set_resize_param",
            [](AI2D &ai2d, bool resize_flag, ai2d_interp_method interp_method, ai2d_interp_mode interp_mode)
            { ai2d.set_resize_param(resize_flag, interp_method, interp_mode); },
            "Set the resize parameters",
            py::arg("resize_flag"),
            py::arg("interp_method"),
            py::arg("interp_mode"))
        .def("set_affine_param",
            [](AI2D &ai2d, bool affine_flag, ai2d_interp_method interp_method, uint32_t cord_round, uint32_t bound_ind, int32_t bound_val, uint32_t bound_smooth, py::list M)
            { ai2d.set_affine_param(affine_flag, interp_method, cord_round, bound_ind, bound_val, bound_smooth, from_py_list<float>(M)); },
            "Set the affine transformation parameters",
            py::arg("affine_flag"),
            py::arg("interp_method"),
            py::arg("cord_round"),
            py::arg("bound_ind"),
            py::arg("bound_val"),
            py::arg("bound_smooth"),
            py::arg("M"))
        .def("build",
            [](AI2D &ai2d, const std::vector<pybind11::ssize_t> input_shape, const std::vector<pybind11::ssize_t> output_shape)
            { ai2d.build(to_rt_shape(input_shape), to_rt_shape(output_shape)).unwrap_or_throw(); },
            "Build the AI2D processing pipeline",
            py::arg("input_shape"),
            py::arg("output_shape"))
        .def("run",
            [](AI2D &ai2d, runtime_tensor &input_tensor, runtime_tensor &output_tensor)
            { ai2d.run(input_tensor, output_tensor).unwrap_or_throw(); },
            "Run the AI2D processing pipeline",
            py::arg("input_tensor"),
            py::arg("output_tensor"));

    py::class_<interpreter>(m, "Interpreter", "KPU function for K230")
        .def(py::init())
        .def("load_model",
            [](interpreter &interp, gsl::span<const gsl::byte> buffer)
            {
                interp.load_model(buffer, true).unwrap_or_throw();
            })
        .def("load_model",
            [](interpreter &interp, std::string kmodel_path)
            {
                std::ifstream ifs(kmodel_path, std::ios::binary);
                interp.load_model(ifs).unwrap_or_throw();
            })
        .def_property_readonly("inputs_size", &interpreter::inputs_size)
        .def_property_readonly("outputs_size", &interpreter::outputs_size)
        .def("get_input_shape",
            [](interpreter &interp, size_t index) {
                return to_py_shape(interp.input_shape(index));
            })
       .def("get_output_shape",
            [](interpreter &interp, size_t index) {
                return to_py_shape(interp.output_shape(index));
            })
        .def("get_input_desc", &interpreter::input_desc)
        .def("get_output_desc", &interpreter::output_desc)
        .def("get_input_tensor",
            [](interpreter &interp, size_t index)
            {
                return interp.input_tensor(index).unwrap_or_throw();
            })
        .def("set_input_tensor",
            [](interpreter &interp, size_t index, runtime_tensor tensor)
            {
                return interp.input_tensor(index, tensor).unwrap_or_throw();
            })
        .def("get_output_tensor",
            [](interpreter &interp, size_t index)
            {
                return interp.output_tensor(index).unwrap_or_throw();
            })
        .def("set_output_tensor",
            [](interpreter &interp, size_t index, runtime_tensor tensor)
            {
                return interp.output_tensor(index, tensor).unwrap_or_throw();
            })
        .def("set_profiling",
             [](interpreter &interp, uint8_t enabled) {
                 interp.set_profiling(enabled);
             })
        .def("run",
            [](interpreter &interp)
            { interp.run().unwrap_or_throw(); });

    py::module_ atexit = py::module_::import("atexit");
    atexit.attr("register")(py::cpp_function(&shrink_memory_pool));
}
