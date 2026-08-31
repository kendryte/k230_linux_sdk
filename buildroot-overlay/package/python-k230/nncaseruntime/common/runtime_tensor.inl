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
py::class_<tensor_desc>(m, "TensorDesc")
    .def_property(
        "dtype",
        [](const tensor_desc &desc)
        { return to_dtype(desc.datatype); },
        [](tensor_desc &desc, py::object dtype)
        {
            desc.datatype = from_dtype(py::dtype::from_args(dtype));
        })
    .def_readwrite("start", &tensor_desc::start)
    .def_readwrite("size", &tensor_desc::size);

py::class_<runtime_tensor>(m, "RuntimeTensor")
    .def_static("from_numpy",
        [](py::array arr)
        {
            arr = py::array::ensure(arr, py::array::c_style);
            auto src_buffer = arr.request();
            auto datatype = from_dtype(arr);
            auto tensor = host_runtime_tensor::create(
                datatype,
                to_rt_shape(src_buffer.shape),
                to_rt_strides(src_buffer.itemsize,src_buffer.strides),
                gsl::make_span(reinterpret_cast<gsl::byte *>(src_buffer.ptr),src_buffer.size * src_buffer.itemsize),
                true, host_runtime_tensor::memory_pool_t::pool_shared, 0).unwrap_or_throw();
            return tensor;
        })
    // .def_static("from_dmabuf",
    //     [](py::object dtype, const std::vector<pybind11::ssize_t> shape, int device_fd, int vbuffer_fd, void *vaddr)
    //     {
    //         auto tensor_from_dmabuf = host_runtime_tensor::create_from_dmabuf(
    //             from_type(dtype),
    //             to_rt_shape(shape),
    //             device_fd, vbuffer_fd, vaddr,
    //             host_runtime_tensor::memory_pool_t::pool_shared)
    //                           .unwrap_or_throw();
    //         return tensor_from_dmabuf;
    //     })
    .def("copy_to",
        [](runtime_tensor &from, runtime_tensor &to)
        {
            from.copy_to(to).unwrap_or_throw();
        })
    .def("to_numpy",
        [](runtime_tensor &tensor)
        {
            auto host = tensor.to_host().unwrap_or_throw();
            auto src_map = std::move(hrt::map(host, runtime::map_read).unwrap_or_throw());
            auto src_buffer = src_map.buffer();
            return py::array(
                to_dtype(tensor.datatype()), tensor.shape(),
                to_py_strides(runtime::get_bytes(tensor.datatype()),
                    tensor.strides()),
                src_buffer.data());
        }, py::keep_alive<0, 1>())
    .def_property_readonly("dtype",
        [](runtime_tensor &tensor)
        {
            return to_dtype(tensor.datatype());
        })
    .def_property_readonly("shape", [](runtime_tensor &tensor)
        { return to_py_shape(tensor.shape()); });