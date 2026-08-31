#!/usr/bin/env python3
"""
nncaseruntime API 测试 - 验证 python-k230 内编译的 _nncaseruntime_k230 扩展

覆盖 RuntimeTensor 往返、AI2D 硬件(resize/crop/pad/类型转换)以及可选的
kmodel 推理, 全部使用合成数据, 不依赖摄像头和显示器。

参考: https://github.com/kendryte/nncase_docs/blob/master/Linux_only_dir/nncaseruntime_k230_API.md

Usage:
    python nncaseruntime_test.py [kmodel_path]

Example:
    python nncaseruntime_test.py
    python nncaseruntime_test.py face_detection_320.kmodel
"""

import sys
import time
import numpy as np
import nncaseruntime as nn


class ScopedTiming:
    """耗时统计工具类"""

    def __init__(self, info="", enable_profile=True):
        self.info = info
        self.enable_profile = enable_profile

    def __enter__(self):
        if self.enable_profile:
            self.start_time = time.time_ns()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        if self.enable_profile:
            elapsed_time = time.time_ns() - self.start_time
            print(f"{self.info} : {elapsed_time / 1000000} ms")


def test_runtime_tensor():
    """RuntimeTensor.from_numpy / to_numpy / dtype / shape 往返校验"""
    for dtype in (np.uint8, np.float32):
        src = np.arange(1 * 3 * 16 * 16, dtype=dtype).reshape(1, 3, 16, 16)
        tensor = nn.RuntimeTensor.from_numpy(src)

        assert list(tensor.shape) == [1, 3, 16, 16], \
            f"shape 不匹配: {tensor.shape}"
        assert tensor.dtype == np.dtype(dtype), \
            f"dtype 不匹配: {tensor.dtype} != {np.dtype(dtype)}"

        dst = tensor.to_numpy()
        assert dst.shape == src.shape and dst.dtype == src.dtype, \
            f"to_numpy 形状/类型不匹配: {dst.shape} {dst.dtype}"
        assert np.array_equal(dst, src), "to_numpy 数据往返不一致"
        print(f"  {np.dtype(dtype)} 往返一致, shape={tensor.shape}")


def _ai2d_check(out_np, expect_shape, expect_val):
    """校验 AI2D 输出: 形状 + 常量值全等 (常量图经过采样/类型转换仍应是常量)"""
    assert list(out_np.shape) == expect_shape, \
        f"输出 shape 不匹配: {out_np.shape} != {expect_shape}"
    assert np.all(out_np == expect_val), \
        f"输出值 != {expect_val}, 实际 min={out_np.min()} max={out_np.max()}"


def test_ai2d_resize():
    """AI2D 硬件 resize: NCHW uint8 (1,3,240,320) -> (1,3,120,160)"""
    src = np.full((1, 3, 240, 320), 128, dtype=np.uint8)

    ai2d = nn.AI2D()
    ai2d.set_datatype(
        nn.AI2D_FORMAT.NCHW_FMT,
        nn.AI2D_FORMAT.NCHW_FMT,
        np.uint8, np.uint8
    )
    ai2d.set_resize_param(
        True,
        nn.AI2D_INTERP_METHOD.tf_bilinear,
        nn.AI2D_INTERP_MODE.half_pixel
    )
    ai2d.build([1, 3, 240, 320], [1, 3, 120, 160])

    in_tensor = nn.RuntimeTensor.from_numpy(src)
    out_tensor = nn.RuntimeTensor.from_numpy(
        np.zeros((1, 3, 120, 160), dtype=np.uint8)
    )
    ai2d.run(in_tensor, out_tensor)
    _ai2d_check(out_tensor.to_numpy(), [1, 3, 120, 160], 128)


def test_ai2d_crop_resize():
    """AI2D 硬件 crop + resize: 从 (240,320) 裁 (100,100) 再缩放到 (50,50)"""
    src = np.full((1, 3, 240, 320), 66, dtype=np.uint8)

    ai2d = nn.AI2D()
    ai2d.set_datatype(
        nn.AI2D_FORMAT.NCHW_FMT,
        nn.AI2D_FORMAT.NCHW_FMT,
        np.uint8, np.uint8
    )
    ai2d.set_crop_param(True, 110, 70, 100, 100)  # start_x, start_y, w, h
    ai2d.set_resize_param(
        True,
        nn.AI2D_INTERP_METHOD.tf_bilinear,
        nn.AI2D_INTERP_MODE.half_pixel
    )
    ai2d.build([1, 3, 240, 320], [1, 3, 50, 50])

    in_tensor = nn.RuntimeTensor.from_numpy(src)
    out_tensor = nn.RuntimeTensor.from_numpy(
        np.zeros((1, 3, 50, 50), dtype=np.uint8)
    )
    ai2d.run(in_tensor, out_tensor)
    _ai2d_check(out_tensor.to_numpy(), [1, 3, 50, 50], 66)


def test_ai2d_pad():
    """AI2D 硬件 pad: NCHW (1,3,120,160) 常量填充 -> (1,3,160,200)"""
    src = np.full((1, 3, 120, 160), 200, dtype=np.uint8)

    ai2d = nn.AI2D()
    ai2d.set_datatype(
        nn.AI2D_FORMAT.NCHW_FMT,
        nn.AI2D_FORMAT.NCHW_FMT,
        np.uint8, np.uint8
    )
    # paddings 按 (N0,N1, C0,C1, H0,H1, W0,W1) 前后填充, pad_mode: 0=constant
    ai2d.set_pad_param(True, [0, 0, 0, 0, 0, 40, 0, 40], 0, [77, 77, 77])
    ai2d.build([1, 3, 120, 160], [1, 3, 160, 200])

    in_tensor = nn.RuntimeTensor.from_numpy(src)
    out_tensor = nn.RuntimeTensor.from_numpy(
        np.zeros((1, 3, 160, 200), dtype=np.uint8)
    )
    ai2d.run(in_tensor, out_tensor)

    out = out_tensor.to_numpy()
    assert list(out.shape) == [1, 3, 160, 200], f"输出 shape 不匹配: {out.shape}"
    assert np.all(out[:, :, :120, :160] == 200), "pad 后原始区域被改动"
    assert np.all(out[:, :, 120:, :] == 77) and np.all(out[:, :, :, 160:] == 77), \
        "pad 区域填充值不正确"


def test_ai2d_dtype_convert():
    """AI2D resize (uint8) + numpy 类型转换 uint8 -> float32

    K230 AI2D 硬件不支持 float 输出: ai2d_config 寄存器没有输出数据类型字段,
    set_datatype 的 dst_type 只用于计算输出 plane 步长, 设为 float32 时硬件
    仍按 8bit 写数据, 读回是垃圾 (check_config 不拦截)。硬件支持的"类型转换"
    仅限 RAW16+shift (uint16->uint8)、有/无符号、YUV->RGB CSC。
    需要 float 时先用 AI2D 输出 uint8, 再在 numpy 侧转换; kmodel 输入为
    uint8 时由运行时自动反量化。
    """
    src = np.full((1, 3, 240, 320), 100, dtype=np.uint8)

    ai2d = nn.AI2D()
    ai2d.set_datatype(
        nn.AI2D_FORMAT.NCHW_FMT,
        nn.AI2D_FORMAT.NCHW_FMT,
        np.uint8, np.uint8
    )
    ai2d.set_resize_param(
        True,
        nn.AI2D_INTERP_METHOD.tf_bilinear,
        nn.AI2D_INTERP_MODE.half_pixel
    )
    ai2d.build([1, 3, 240, 320], [1, 3, 120, 160])

    in_tensor = nn.RuntimeTensor.from_numpy(src)
    out_tensor = nn.RuntimeTensor.from_numpy(
        np.zeros((1, 3, 120, 160), dtype=np.uint8)
    )
    ai2d.run(in_tensor, out_tensor)

    out = out_tensor.to_numpy().astype(np.float32)
    assert out.dtype == np.float32, f"输出 dtype 不匹配: {out.dtype}"
    assert list(out.shape) == [1, 3, 120, 160], f"输出 shape 不匹配: {out.shape}"
    assert np.all(out == np.float32(100.0)), \
        f"转换后值 != 100.0, 实际 min={out.min()} max={out.max()}"
    print(f"  uint8 -> {out.dtype} 转换一致")


def test_kmodel(kmodel_path):
    """kmodel 推理: 加载模型 -> 打印输入/输出描述 -> 全 1 输入跑一次"""
    kpu = nn.Interpreter()
    kpu.load_model(kmodel_path)
    print(f"  模型: {kmodel_path}, 输入 {kpu.inputs_size} 个, 输出 {kpu.outputs_size} 个")

    for i in range(kpu.inputs_size):
        shape = kpu.get_input_shape(i)
        desc = kpu.get_input_desc(i)
        print(f"  input[{i}]: shape={list(shape)}, dtype={desc.dtype}, size={desc.size}")

        # 按模型描述构造全 1 输入 (文档建议在函数内完成, 便于及时释放)
        tmp_tensor = nn.RuntimeTensor.from_numpy(np.ones(shape, dtype=desc.dtype))
        kpu.set_input_tensor(i, tmp_tensor)

    for i in range(kpu.outputs_size):
        print(f"  output[{i}]: shape={list(kpu.get_output_shape(i))}")

    with ScopedTiming("  kpu.run"):
        kpu.run()

    out = kpu.get_output_tensor(0).to_numpy()
    print(f"  output[0] 概要: shape={out.shape}, dtype={out.dtype}, "
          f"min={out.min():.6f}, max={out.max():.6f}, mean={out.mean():.6f}")


TESTS = [
    ("RuntimeTensor 往返 (uint8/float32)", test_runtime_tensor),
    ("AI2D resize (240x320 -> 120x160)", test_ai2d_resize),
    ("AI2D crop + resize", test_ai2d_crop_resize),
    ("AI2D pad (constant)", test_ai2d_pad),
    ("AI2D resize + uint8->float32 转换", test_ai2d_dtype_convert),
]


def main():
    print(f"nncaseruntime 版本: {nn.__version__}")
    print(f"模块: {nn.__file__}\n")

    results = []
    for name, func in TESTS:
        try:
            with ScopedTiming(name):
                func()
            print(f"[PASS] {name}")
            results.append((name, True, ""))
        except Exception as e:
            print(f"[FAIL] {name}: {e}")
            results.append((name, False, str(e)))

    if len(sys.argv) > 1:
        kmodel_path = sys.argv[1]
        try:
            with ScopedTiming("kmodel 推理"):
                test_kmodel(kmodel_path)
            print(f"[PASS] kmodel 推理 ({kmodel_path})")
            results.append((f"kmodel 推理 ({kmodel_path})", True, ""))
        except Exception as e:
            print(f"[FAIL] kmodel 推理 ({kmodel_path}): {e}")
            results.append((f"kmodel 推理 ({kmodel_path})", False, str(e)))

    failed = [r for r in results if not r[1]]
    print(f"\n共 {len(results)} 项, 通过 {len(results) - len(failed)}, 失败 {len(failed)}")
    if failed:
        sys.exit(1)


if __name__ == "__main__":
    main()
