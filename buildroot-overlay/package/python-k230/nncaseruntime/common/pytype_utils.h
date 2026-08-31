/* Copyright 2019-2021 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include <nncase/runtime/datatypes.h>
#include <nncase/runtime/debug.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

namespace pybind11::detail {
// Similar to enums in `pybind11/numpy.h`. Determined by doing:
// python3 -c 'import numpy as np; print(np.dtype(np.float16).num)'
constexpr int NPY_BOOL = 0;
constexpr int NPY_INT8 = 1;
constexpr int NPY_INT16 = 3;
constexpr int NPY_INT32 = 5;
constexpr int NPY_INT64 = 7;
constexpr int NPY_UINT8 = 2;
constexpr int NPY_UINT16 = 4;
constexpr int NPY_UINT32 = 6;
constexpr int NPY_UINT64 = 8;
constexpr int NPY_FLOAT32 = 11;
constexpr int NPY_FLOAT64 = 12;
constexpr int NPY_FLOAT16 = 23;

// Kinda following:
// https://github.com/pybind/pybind11/blob/9bb3313162c0b856125e481ceece9d8faa567716/include/pybind11/numpy.h#L1000
template <> struct npy_format_descriptor<nncase::half> {
    static pybind11::dtype dtype() {
        handle ptr = npy_api::get().PyArray_DescrFromType_(NPY_FLOAT16);
        return reinterpret_borrow<pybind11::dtype>(ptr);
    }
    static std::string format() {
        // following:
        // https://docs.python.org/3/library/struct.html#format-characters
        return "e";
    }
    static constexpr auto name() { return _("float16"); }
};
} // namespace pybind11::detail

namespace nncase {
pybind11::dtype to_dtype(typecode_t type) {
    namespace py = pybind11;

    switch (type) {
    case dt_boolean:
        return py::dtype::of<bool>();
    case dt_uint8:
        return py::dtype::of<uint8_t>();
    case dt_uint16:
        return py::dtype::of<uint16_t>();
    case dt_uint32:
        return py::dtype::of<uint32_t>();
    case dt_uint64:
        return py::dtype::of<uint64_t>();
    case dt_int8:
        return py::dtype::of<int8_t>();
    case dt_int16:
        return py::dtype::of<int16_t>();
    case dt_int32:
        return py::dtype::of<int32_t>();
    case dt_int64:
        return py::dtype::of<int64_t>();
    case dt_float16:
        return py::dtype("float16");
    case dt_float32:
        return py::dtype::of<float>();
    case dt_float64:
        return py::dtype::of<double>();
    default:
        throw std::runtime_error("Unsupported dtype " + to_string(type));
    }
}

pybind11::dtype to_dtype(const datatype_t type) {
    auto primtype = type.as<prim_type_t>();
    if (primtype.is_err()) {
        throw std::runtime_error("Only support primtype.");
    }
    return to_dtype(primtype.unwrap()->typecode());
}

typecode_t from_dtype(pybind11::array array) {
    namespace py = pybind11;

    if (py::isinstance<py::array_t<bool>>(array))
        return dt_boolean;
    else if (py::isinstance<py::array_t<uint8_t>>(array))
        return dt_uint8;
    else if (py::isinstance<py::array_t<uint16_t>>(array))
        return dt_uint16;
    else if (py::isinstance<py::array_t<uint32_t>>(array))
        return dt_uint32;
    else if (py::isinstance<py::array_t<uint64_t>>(array))
        return dt_uint64;
    else if (py::isinstance<py::array_t<int8_t>>(array))
        return dt_int8;
    else if (py::isinstance<py::array_t<int16_t>>(array))
        return dt_int16;
    else if (py::isinstance<py::array_t<int32_t>>(array))
        return dt_int32;
    else if (py::isinstance<py::array_t<int64_t>>(array))
        return dt_int64;
    else if (py::isinstance<py::array_t<half>>(array))
        return dt_float16;
    else if (py::isinstance<py::array_t<float>>(array))
        return dt_float32;
    else if (py::isinstance<py::array_t<double>>(array))
        return dt_float64;
    throw std::runtime_error("Unsupported dtype " +
                             (std::string)py::str(array.dtype()));
}

paddings_t to_rt_paddings(const std::vector<pybind11::ssize_t> &value) {
    assert(value.size() == 8);
    paddings_t paddings {{(int32_t)value[0],(int32_t)value[1]},
                        {(int32_t)value[2], (int32_t)value[3]},
                        {(int32_t)value[4], (int32_t)value[5]},
                        {(int32_t)value[6], (int32_t)value[7]}};
    return paddings;
}

dims_t to_rt_shape(const std::vector<pybind11::ssize_t> &value) {
    dims_t shape(value.size());
    for (size_t i = 0; i < shape.size(); i++)
        shape[i] = (size_t)value[i];
    return shape;
}

strides_t to_rt_strides(size_t elemsize,
                        const std::vector<pybind11::ssize_t> &value) {
    strides_t strides(value.size());
    for (size_t i = 0; i < strides.size(); i++)
        strides[i] = (size_t)value[i] / elemsize;
    return strides;
}

std::vector<pybind11::ssize_t> to_py_shape(gsl::span<const size_t> value) {
    namespace py = pybind11;

    std::vector<py::ssize_t> shape(value.size());
    for (size_t i = 0; i < shape.size(); i++)
        shape[i] = (py::ssize_t)value[i];
    return shape;
}

std::vector<pybind11::ssize_t> to_py_strides(size_t elemsize,
                                             gsl::span<const size_t> value) {
    namespace py = pybind11;

    std::vector<py::ssize_t> strides(value.size());
    for (size_t i = 0; i < strides.size(); i++)
        strides[i] = (py::ssize_t)value[i] * elemsize;
    return strides;
}

// 添加类型检查和转换函数
typecode_t from_type(const pybind11::object obj) {
    namespace py = pybind11;
    auto d = py::dtype::from_args(obj);
    auto num = d.num();
    switch (num) {
        case pybind11::detail::NPY_BOOL: return typecode_t::dt_boolean;
        case pybind11::detail::NPY_INT8: return typecode_t::dt_int8;
        case pybind11::detail::NPY_INT16: return typecode_t::dt_int16;
        case pybind11::detail::NPY_INT32: return typecode_t::dt_int32;
        case pybind11::detail::NPY_INT64: return typecode_t::dt_int64;
        case pybind11::detail::NPY_UINT8: return typecode_t::dt_uint8;
        case pybind11::detail::NPY_UINT16: return typecode_t::dt_uint16;
        case pybind11::detail::NPY_UINT32: return typecode_t::dt_uint32;
        case pybind11::detail::NPY_UINT64: return typecode_t::dt_uint64;
        case pybind11::detail::NPY_FLOAT32: return typecode_t::dt_float32;
        case pybind11::detail::NPY_FLOAT64: return typecode_t::dt_float64;
        default:
            throw std::runtime_error("Unsupported numpy dtype");
    }
}

template <typename T>
std::vector<T> from_py_list(const pybind11::list &list) {
    return list.cast<std::vector<T>>();
}
} // namespace nncase
