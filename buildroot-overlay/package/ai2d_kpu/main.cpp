#include <chrono>
#include <fstream>
#include <iostream>
#include <nncase/runtime/interpreter.h>
#include <nncase/runtime/runtime_op_utility.h>
#include <nncase/runtime/util.h>
#include <string>
#include <nncase/functional/ai2d/ai2d_builder.h>
#include "mmz.h"
#include <csignal>
#include <exception>
// #include "dbg.h"

using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::detail;
using namespace nncase::runtime::k230;
using namespace nncase::F::k230;

template <class T>
void print_one_line_data(std::string name, std::vector<T> tmp, size_t count = 0)
{
    std::cout << name << " data, size = " << tmp.size() << std::endl;
    if (count == 0)
        count = tmp.size();
    std::cout << "[ ";
    for (size_t i = 0; i < count; i++)
        std::cout << (int64_t)tmp[i] << " ";
    std::cout << " ]" << std::endl;
}

template <class T>
void print_one_line_data(std::string name, T tmp, size_t count = 0)
{
    std::cout << name << " data " << std::endl;
    std::cout << "[ ";
    for (size_t i = 0; i < count; i++)
        std::cout << (char)tmp[i] << " ";
    std::cout << " ]" << std::endl;
}

template <class T>
std::vector<std::vector<T>> txt_to_vector(const char *pathname)
{
    std::ifstream infile;
    infile.open(pathname);

    std::vector<std::vector<T>> res;
    std::vector<T> suanz;
    std::string s;

    while (getline(infile, s))
    {
        std::istringstream is(s);
        T d;
        while (!is.eof())
        {
            is >> d;
            suanz.push_back(d);
        }
        res.push_back(suanz);

        suanz.clear();
        s.clear();
    }

    infile.close();

    return res;
}

std::vector<unsigned char> read_binary_file(const char *file_name)
{
    std::ifstream ifs(file_name, std::ios::binary);
    ifs.seekg(0, ifs.end);
    size_t len = ifs.tellg();
    std::vector<unsigned char> vec(len / sizeof(unsigned char), 0);
    ifs.seekg(0, ifs.beg);
    ifs.read(reinterpret_cast<char *>(vec.data()), len);
    ifs.close();
    return vec;
}

size_t read_binary_file(const char *file_name, char *buffer)
{
    std::ifstream ifs(file_name, std::ios::binary);
    ifs.seekg(0, ifs.end);
    size_t len = ifs.tellg();
    ifs.seekg(0, ifs.beg);
    ifs.read(buffer, len);
    ifs.close();
    return len;
}

auto read_binary(const char *file_name, char *buffer, size_t begin, size_t count)
{
    std::ifstream ifs(file_name, std::ios::binary);
    ifs.seekg(begin, ifs.beg);
    ifs.read(buffer + begin, count);
    ifs.close();
}

size_t get_binary_file_size(const char *file_name)
{
    std::ifstream ifs(file_name, std::ios::binary);
    ifs.seekg(0, ifs.end);
    size_t len = ifs.tellg();
    ifs.close();
    return len;
}

template <typename T>
double dot(const T *v1, const T *v2, size_t size)
{
    double ret = 0.f;
    for (size_t i = 0; i < size; i++)
    {
        ret += v1[i] * v2[i];
    }

    return ret;
}

template <typename T>
double cosine(const T *v1, const T *v2, size_t size)
{
    return dot(v1, v2, size) / ((sqrt(dot(v1, v1, size)) * sqrt(dot(v2, v2, size))));
}

result<std::vector<value_t>> to_values(value_t v)
{
    if (v.is_a<tensor>())
    {
        return ok(std::vector { v });
    }
    else if (v.is_a<tuple>())
    {
        auto out_fields = v.as<tuple>().unwrap()->fields();
        return ok(std::vector(out_fields.begin(), out_fields.end()));
    }
    else
    {
        return err(std::errc::invalid_argument);
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}

static dims_t parse_digits(const std::string& s) {
    dims_t digits;
    auto strs = split(s, ' ');
    for (size_t i = 0; i < strs.size(); i++)
    {
        digits.push_back(std::stoi(strs[i]));
    }
    return digits;
}

static std::vector<dims_t> parse_multilines(const std::vector<std::string>& strs, size_t begin, size_t size) {
    std::vector<dims_t> result;
    for(auto i = strs.begin() + begin; i != strs.begin() + begin + size; ++i)
    {
        auto shape = parse_digits(*i);
        if(shape[0] == 0)
        {
            shape = dims_t{};
        }
        result.push_back(shape);
    }
    return result;
}


struct data_desc {
    std::vector<dims_t> input_shape;
    std::vector<dims_t> output_shape;
    bool is_empty() { return input_shape.empty() && output_shape.empty(); }
};

data_desc parse_desc(const unsigned char *kmodel_desc_raw) {
    auto kmode_desc = std::string(reinterpret_cast<const char*>(kmodel_desc_raw));
    auto descs = split(kmode_desc, '\n');
    auto nums = parse_digits(descs[0]);
    auto input_num = nums[0];
    auto output_num = nums[1];
    auto in_shapes = parse_multilines(descs, 1, input_num);
    auto out_shapes = parse_multilines(descs, 1 + input_num, output_num);
    return data_desc{in_shapes, out_shapes};
}

template<class T>
void print_data(void *a, void *b, size_t all_size)
{
    for (size_t i = 0; i < all_size/sizeof(T); i++)
    {
        std::cout << "[" << std::setw(5) << i << "] " // index width set to 2
                  << std::setw(10) << std::left << *(reinterpret_cast<const T *>(a) + i) << " " // value from a width set to 10
                  << std::setw(10) << std::left << *(reinterpret_cast<const T *>(b) + i) << std::flush; // value from b width set to 10
        if(*(reinterpret_cast<T *>(a) + i) != *(reinterpret_cast<T *>(b) + i))
        {
            std::cout << "  error" << std::flush;
        }
        std::cout << std::endl;
    }
}

template<class T>
void print_single_data(void *a, size_t all_size)
{
    std::cout << "----------------------------" << std::endl;
    for (size_t i = 0; i < all_size / sizeof(T); i++)
    {
        if(i %10 == 0)
        {
            std::cout << std::endl;
        }
        std::cout << std::setw(10) << std::left << *(reinterpret_cast<const T *>(a) + i) << " " << std::flush; // value from b width set to 10
    }
    std::cout << "----------------------------" << std::endl;
}

template<class T>
float compare_output(tensor t,std::vector<T> expect) {
    auto unmap_buf = t->to_host()
                            .expect("not host")
                            ->buffer()
                            .as_host()
                            .expect("not host buffer");
    auto mapped_buf = std::move(unmap_buf.map(map_access_t::map_read).unwrap());
    int ret = memcmp((void *)mapped_buf.buffer().data(), (void *)expect.data(), expect.size()*sizeof(T));
    if (!ret)
    {
        return 1;
    }
    else
    {

        print_data<float>(mapped_buf.buffer().data(), expect.data(), 32);
        float cos = cosine((float *)mapped_buf.buffer().data(), (float *)expect.data(), expect.size() * sizeof(T) / sizeof(float));
        return cos;
    }
}

void __attribute__((destructor)) cleanup() {
    std::cout << "Cleaning up memory..." << std::endl;
    shrink_memory_pool();
#ifdef LINUX_RUNTIME
    kd_mpi_mmz_deinit();
#endif
}

void signal_handler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    exit(signum);
}

int main(int argc, char *argv[])
{
    // 处理信号，在程序退出时清理内存
    signal(SIGINT, signal_handler);

    std::cout << "case " << argv[0] << " build " << __DATE__ << " " << __TIME__ << std::endl;
    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " <kmodel> <input_0.bin> <input_1.bin> ... <input_N.bin> <output_0.bin> <output_1.bin> ... <output_N.bin>" << std::endl;
        return -1;
    }

    interpreter interp;
    std::ifstream ifs(argv[1], std::ios::binary);
    interp.load_model(ifs).expect("Invalid kmodel");

    std::vector<runtime_tensor> ai2d_output;
    {
        runtime_tensor tensor = host_runtime_tensor::create(typecode_t::dt_uint8, dims_t {1,3,320,320}, hrt::pool_shared).expect("create ai2d input tensor failed");
        // auto tensor = interp.get_input_tensor(0);
        auto in_data = read_binary_file(argv[2]);
        std::cout << "data size: " << (size_t)in_data.size() << std::endl;
        auto ai2d_in_tensor = host_runtime_tensor::create(typecode_t::dt_uint8, dims_t { 1, 3, 624, 1024 }, { (gsl::byte *)in_data.data(), (size_t)in_data.size() }, true, hrt::pool_shared).expect("create ai2d input tensor failed");

        ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, ai2d_in_tensor.datatype(), tensor.datatype()};
        ai2d_crop_param_t crop_param{false, 0, 0, 0, 0};
        ai2d_shift_param_t shift_param{false, 0};
        ai2d_pad_param_t pad_param{true, {{0, 0}, {0, 0}, {0, 125}, {0, 0}}, ai2d_pad_mode::constant, {104,117,123}};
        ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
        ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};

        dims_t in_shape = ai2d_in_tensor.shape();
        dims_t out_shape = tensor.shape();
        ai2d_builder builder { in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param };
        builder.build_schedule().expect("build ai2d schedule error");
        builder.invoke(ai2d_in_tensor,tensor).expect("error occurred in ai2d running");

        auto cos = compare_output(tensor.impl(), read_binary_file(argv[3]));
        std::cout << "compare ai2d output with kmodel input cosine similarity = " << cos << std::endl;
        ai2d_output.emplace_back(tensor);
    }

    std::vector<value_t> inputs;
    for (size_t i = 0; i < interp.inputs_size(); i++)
    {
        hrt::sync(ai2d_output[i], sync_op_t::sync_write_back, true).unwrap();
        inputs.push_back(ai2d_output[i].impl());
    }

    auto start = std::chrono::steady_clock::now();
    auto return_value = interp.entry_function().expect("no entry_function")->invoke(inputs).expect("run entry_function failed");
    auto stop = std::chrono::steady_clock::now();
    double duration = std::chrono::duration<double, std::milli>(stop - start).count();
    std::cout << "interp run: " << duration << " ms, fps = " << 1000 / duration << std::endl;
    auto values = to_values(return_value).expect("unsupported value type");
    for (size_t i = 0; i < values.size(); ++i)
    {
        auto t = values[i].as<tensor>().expect("value is not a tensor");
        auto cos = compare_output(t, read_binary_file(argv[i + 3 + interp.inputs_size()]));
        std::cout << "compare output [" << i << "] cosine similarity = " << cos << std::endl;
        if (cos == 1) {
            std::cout << "ai2d kpu test pass" << std::endl;
        } else {
            std::cout << "ai2d kpu test fail" << std::endl;
        }
    }

    return 0;
}