#ifndef K230_OSD_HPP
#define K230_OSD_HPP

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

struct display;
struct display_plane;
struct display_buffer;

//=============================================================================
// k230_osd class - OSD overlay support
//=============================================================================
class k230_osd {
public:
    // 构造函数/析构函数
    k230_osd();
    ~k230_osd();

    // 公有方法
    int init(struct display* pdisplay);
    int update(py::array img);
    unsigned int fourcc_;

private:
    // 成员变量
    struct display_plane* plane_;
    struct display_buffer* buffer_[2];
    struct display_buffer* cur_buf_;
};

#endif // K230_OSD_HPP
