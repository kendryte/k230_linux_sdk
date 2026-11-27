#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import json
import hashlib
import re
import sys
import io
import urllib.request
import urllib.error
from datetime import datetime,timedelta
from collections import OrderedDict

# --- 兼容性处理 ---
# Python 3 不再需要 reload(sys) 且 sys 模块没有 setdefaultencoding
if sys.version_info[0] < 3:
    reload(sys)
    sys.setdefaultencoding('utf-8')

def get_china_time():
    if sys.version_info[0] >= 3 and sys.version_info[1] >= 12:
        # Python 3.12+：推荐使用带时区的方式（兼容新版）
        utc_now = datetime.now(datetime.UTC)
    else:
        # Python 2.7 + Python 3.11及以下：使用utcnow()获取UTC时间（无时区标识，跨版本兼容）
        utc_now = datetime.utcnow()

    china_now = utc_now + timedelta(hours=8)

    # 格式化返回，忽略时区标识（仅保留标准时间字符串，符合CSV/JSON存储需求）
    return china_now.strftime('%Y-%m-%d %H:%M:%S')

def get_md5_from_file(file_path):
    md5_file = file_path + ".md5"
    if os.path.exists(md5_file):
        # 使用 io.open 并指定编码，确保双版本下读取行为一致
        with io.open(md5_file, 'r', encoding='utf-8') as f:
            content = f.read().split()
            return content[0] if content else "N/A"
    return "N/A"

# --- 版本摘要 / 更新日志链接 sidecar 文件 ---
# 和 .md5 文件的约定一致：维护者在发布目录里放一个和镜像文件同名、
# 后缀为 .summary / .changelog_url 的文本文件，发布流程不强制要求，缺失时
# 前端就不展示摘要/更新日志区域。
#   <image>.img.gz.summary       一段版本摘要文字（可多行，原样展示）
#   <image>.img.gz.changelog_url SDK 仓库 CHANGELOG.md 对应版本小节的链接（单行），
#                                 例如 https://github.com/kendryte/k230_linux_sdk/blob/main/CHANGELOG.md#xxx
def get_summary_from_file(file_path):
    summary_file = file_path + ".summary"
    if os.path.exists(summary_file):
        with io.open(summary_file, 'r', encoding='utf-8') as f:
            return f.read().strip()
    return ""

def get_changelog_url_from_file(file_path):
    changelog_url_file = file_path + ".changelog_url"
    if os.path.exists(changelog_url_file):
        with io.open(changelog_url_file, 'r', encoding='utf-8') as f:
            return f.read().strip()
    return ""

def get_dir_creation_time(dir_path):
    if not os.path.exists(dir_path):
        return "dir '{0}' not exist".format(dir_path)

    dir_stat = os.stat(dir_path)
    try:
        # 优先获取出生时间（macOS/FreeBSD），否则获取 ctime
        create_timestamp = getattr(dir_stat, 'st_birthtime', dir_stat.st_ctime)
    except AttributeError:
        create_timestamp = dir_stat.st_ctime

    create_datetime = datetime.fromtimestamp(create_timestamp)
    return create_datetime.strftime('%Y-%m-%d %H:%M:%S')

def get_version_from_filename(filename):
    if "micropython" in filename:
        pattern = r'v\d+\.\d+(?:\.\d+)?'
    else:
        pattern = r'v\d+\.\d+(?:\.\d+)?'

    m = re.search(pattern, filename)
    return m.group(0) if m else "unknown"

def get_product_name_from_filepath(file_path):
    if "linux_sdk_images" in file_path:
        return os.path.basename(os.path.dirname(file_path))
    elif "micropython" in file_path:
        filename = os.path.basename(file_path)
        product_ = filename.split("_micropython")[0]
        product_dict = {
            "CanMV_K230_01Studio": "k230_canmv_01studio_defconfig",
            "CanMV_K230_LCKFB": "k230_canmv_lckfb_defconfig",
            "CanMV_K230_V1P0_P1": "k230_canmv_defconfig",
            "CanMV-K230_DONGSHANPI": "k230_canmv_dongshanpi_defconfig",
            "CanMV_K230D_Zero": "BPI-CanMV-K230D-Zero_defconfig",
            "CanMV_K230_01Studio_Emmc": "k230_canmv_01studio_emmc_defconfig",
            "CanMV-K230_RTT_EVB_BOARD":"k230_canmv_rtt_evb_defconfig",
            "k230_canmv_rtt_evb_defconfig":"CanMV-K230_RTT_EVB_BOARD",
            "CanMV_K230D_ATK_DNK230D":"k230d_canmv_atk_dnk230d_defconfig",
            "CanMV_K230_Hiwonder":"k230_canmv_hiwonder_defconfig",
            "CanMV_K230_YAHBOOM":"k230_canmv_yahboom_defconfig",
            "CanMV_K230_GT6700" :"k230_canmv_gt6700_defconfig",
            "CanMV_K230D_LabPlusAiCamera":"k230d_canmv_labplus_ai_camera_defconfig",
            "CanMV_K230D_JUNROC_AI_CAM":"k230d_canmv_junroc_ai_cam_defconfig",
            "CanMV_K230_MRT":"k230_canmv_mrt_defconfig",
            "CanMV_K230_V3P0": "k230_canmv_v3_defconfig"
        }
        return product_dict.get(product_, product_)
    elif "images" in file_path:
        idx = file_path.index("/images/")
        return os.path.basename(file_path[:idx])
    return ""

def get_url(file_path):
    a = file_path.replace("/data/kendryte-download", "https://download.kendryte.com")
    return a.replace("/data1/k230/release/", "https://ai.b-bug.org/k230/release/")

def update_file_to_json(file_path, products):
    filename = os.path.basename(file_path)
    if not file_path.endswith(".img.gz"):
        return

    if "latest" in file_path:
        return

    product_name = get_product_name_from_filepath(file_path)
    if product_name not in products:
        return
    if "nand.img.gz" in filename:
        variant_key = "nand"
    elif "linux" in filename:
        variant_key = "linux"
    elif "debian" in filename:
        variant_key = "debian"
    elif "ubuntu" in filename:
        variant_key = "ubuntu"
    elif "micropython" in filename:
        variant_key = "micropython"
    else:
        variant_key = "unknown"

    variants = products[product_name].get("variants", {})
    if variant_key not in variants:
        return

    version_type = "latest" if ("daily_build" in file_path) else "history"

    ver_info = {
        "version": get_version_from_filename(filename),
        "date": get_dir_creation_time(os.path.dirname(file_path)),
        "url": get_url(file_path),
        "md5": get_md5_from_file(file_path),
        # 正式发布版本才有意义：summary / changelogUrl 由维护者手工在发布目录放置
        # <filename>.summary / <filename>.changelog_url 提供，daily build 通常没有，留空即可
        "summary": get_summary_from_file(file_path),
        "changelogUrl": get_changelog_url_from_file(file_path)
    }

    if version_type == "latest":
        variants[variant_key][version_type] = ver_info
    else:
        if not isinstance(variants[variant_key].get(version_type), list):
            variants[variant_key][version_type] = []
        variants[variant_key][version_type].append(ver_info)

    products[product_name]["variants"] = variants

def update_directory_to_json(directory, products):
    file_info_list = []
    for root, _, files in os.walk(directory):
        for f in files:
            if not f.endswith(".img.gz"):
                continue
            file_path = os.path.join(root, f)
            try:
                file_mtime = os.path.getmtime(file_path)
                file_info_list.append((file_mtime, file_path))
            except OSError:
                continue

    file_info_list.sort(key=lambda x: x[0])

    for _, file_path in file_info_list:
        update_file_to_json(file_path, products)

def sdk_release_dirs_2_json(base_dir, products, max_dirs=3):
    if not os.path.isdir(base_dir):
        return
    subdirs = [
        os.path.join(base_dir, d)
        for d in os.listdir(base_dir)
        if os.path.isdir(os.path.join(base_dir, d))
    ]
    subdirs.sort(key=os.path.getmtime, reverse=True)

    for d in subdirs[:max_dirs]:
        update_directory_to_json(d, products)

# 内置初始 JSON 字符串
json_str = u'''
{
    "time": "2024-07-29 16:00:00",
    "products": {
        "k230_canmv_01studio_defconfig": {
            "name": "01studio",
            "description": "01studio canmv and xCAM EVT1",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=833",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_lckfb_defconfig": {
            "name": "嘉立创k230",
            "description": "k230_canmv_lckfb",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=832",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_v3_defconfig": {
            "name": "CanMV V3.0",
            "description": "CanMV K230 V3.0(创乐博)",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=834",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_dongshanpi_defconfig": {
            "name": "dongshanpai",
            "description": "百问网，东山派",
            "image_url": "https://eai.100ask.net/assets/images/image-20240729155648319-504e2c8f5bc5607d6e03148239c45087.png",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_defconfig": {
            "name": "CanMV 1.0/1.1",
            "description": "CanMV K230 1.0/1.1",
            "image_url": "https://www.kendryte.com/api/imagecdn/zh/sdk/canmv_k230_sdk/images/CanMV-K230_front.png",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_01studio_emmc_defconfig": {
            "name": "01studio emmc ",
            "description": "01studio canmv base emmc board",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=835",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "BPI-CanMV-K230D-Zero_defconfig": {
            "name": "BPI-CanMV-K230D",
            "description": "BPI-CanMV-K230D",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=482",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_rtt_evb_defconfig": {
            "name": "rtt evb",
            "description": "k230_canmv_rtt_evb_defconfig",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=577",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230d_canmv_atk_dnk230d_defconfig": {
            "name": "正点原子",
            "description": "k230d_canmv_atk_dnk230d_defconfig",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=577",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_hiwonder_defconfig": {
            "name": "hiwonder",
            "description": "k230_canmv_hiwonder_defconfig",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=577",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_yahboom_defconfig": {
            "name": "亚博",
            "description": "k230_canmv_yahboom_defconfig",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=577",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_gt6700_defconfig": {
            "name": "银杏",
            "description": "k230_canmv_gt6700_defconfig",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=577",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230d_canmv_junroc_ai_cam_defconfig": {
            "name": "隽鹏Junroc",
            "description": "隽鹏Junroc AI Cam",
            "image_url": "https://www.kendryte.com/img/junpeng.15e0c9ac.webp",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_mrt_defconfig": {
            "name": "韩端",
            "description": "韩端",
            "image_url": "https://www.kendryte.com/api/imagecdn/zh/sdk/k230_linux_sdk_docs/screenshot_20260730_182637.png",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_evb_defconfig": {
            "name": "k230 evb",
            "description": "k230 evb",
            "image_url": "https://www.kendryte.com/api/imagecdn/zh/sdk/k230_linux_sdk_docs/evb.png",
            "variants": { "linux" :{"latest": {}, "history": []}, "debian" : {"latest": {}, "history": [] }, "ubuntu" : {"latest": {}, "history": [] }, "nand" : {"latest": {}, "history": [] }, "nor" : {"latest": {}, "history": [] }, "micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_gt6700_defconfig": {
            "name": "银杏科技gt6700",
            "description": "银杏科技gt6700",
            "image_url": "https://img.alicdn.com/imgextra/i1/677513810/O1CN01L98hhw1e1243Y2AGL_!!677513810.jpg",
            "variants": { "linux" :{"latest": {}, "history": []}, "debian" : {"latest": {}, "history": [] }, "ubuntu" : {"latest": {}, "history": [] }, "nand" : {"latest": {}, "history": [] }, "nor" : {"latest": {}, "history": [] }, "micropython" : {"latest": {}, "history": []} }
        },
        "k230d_canmv_labplus_ai_camera_defconfig": {
            "name": "labplus",
            "description": "k230d_canmv_labplus_ai_camera_defconfig",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=577",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        }
    }
}
'''

def open_json():
    # Python 3 中 json.loads 处理 unicode 字符串很稳健
    data = json.loads(json_str, object_pairs_hook=OrderedDict)
    return data.get('products', {})

def save_json(products, file_path):
    for product in list(products.keys()):
        variants = products[product].get("variants", {})
        variants_to_delete = []
        for v_name, v_info in variants.items():
            latest = v_info.get("latest", {})
            history = v_info.get("history", [])
            if not latest and not history:
                variants_to_delete.append(v_name)

        for v_name in variants_to_delete:
            variants.pop(v_name, None)
        if variants:
            products[product]["variants"] = variants
        else:
            products.pop(product, None)

    try:
        # 使用 io.open 并指定 utf-8，这是解决 Python 2/3 乱码问题的通用方法
        with io.open(file_path, 'w', encoding='utf-8') as f:
            output = json.dumps(
                { "time" : get_china_time(), "products": products},
                indent=4,
                ensure_ascii=False,
                sort_keys=False
            )
            # Python 2 中 dumps 返回的是 str(utf-8)，Python 3 返回的是 unicode
            # io.open 在 Python 2 中期望输入 unicode，所以需要转换
            if sys.version_info[0] < 3 and isinstance(output, str):
                output = output.decode('utf-8')
            f.write(output)
    except Exception as e:
        print("save error: {0}".format(str(e)))
        raise

# --- 新版本订阅通知 ---
# k230_linux_sdk 的每日构建/正式发布不经过 bbs-admin 填表单这条路径（不像
# MicroPython 走 downloadData 表新增记录时自动触发通知），而是由本脚本扫描
# 文件系统生成 products.json。为了让订阅了 Linux SDK 更新通知的用户也能及时
# 收到邮件，这里在生成新 products.json 前后做一次 diff：只要 linux/debian/ubuntu
# 这几个 variant 的 history 列表里出现了旧文件里没有的版本号，就认为是"新版本"，
# 调用 bbs-be 的 /internal/version-notify 触发通知。
#
# 注意：
#   - micropython variant 不在这里通知范围内，它走的是 bbs-admin/downloadData
#     表那条独立的通知路径，二者互不影响、不会重复发信。
#   - 一次脚本运行检测到的所有新版本合并成一次通知（version 字段传一段摘要文本），
#     避免同一次 CI 运行里产品/板卡较多时连续发送多封邮件。
#   - 网络请求失败只打印日志，不能影响本脚本生成 products.json 的主流程。

NOTIFY_VARIANTS = ("linux", "debian", "ubuntu")

# 内部通知接口地址与共享密钥：优先从环境变量读取，方便部署环境按需覆盖，
# 不强制要求配置——未配置时直接跳过通知（不影响 products.json 正常生成）。
INTERNAL_NOTIFY_URL = os.environ.get(
    "BBS_INTERNAL_NOTIFY_URL", "https://www.kendryte.com/api/internal/version-notify"
)
INTERNAL_NOTIFY_SECRET = os.environ.get("BBS_INTERNAL_NOTIFY_SECRET", "")
# Linux SDK 下载中心页面，通知邮件里的下载链接固定指向这里（不指向某个具体文件），
# 和 download_data 表里 osType='linux' 的 Daily Build 占位记录 link 字段保持一致。
LINUX_SDK_DOWNLOAD_URL = "https://download.kendryte.com/k230/release/linux_sdk_images/daily_build/?lang=zh"

def load_history_versions(file_path):
    """
    读取已存在的 products.json（如果有），返回 {(product_name, variant): set(version)}，
    用于和新生成的数据 diff 出"新增版本"。文件不存在或解析失败时返回空字典，
    这种情况下不会误判出一堆"新版本"（因为下面的 diff 只看新增的 key，不存在
    的旧文件等价于"所有版本都是新的"，而这正是首次运行的预期行为——不应该对
    首次运行发一堆通知，所以首次运行时跳过通知，见 diff_new_versions 的调用处）。
    """
    if not os.path.exists(file_path):
        return None
    try:
        with io.open(file_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
    except Exception as e:
        print("[notify] failed to read old products.json: {0}".format(str(e)))
        return None

    result = {}
    for product_name, product_info in (data.get("products") or {}).items():
        variants = product_info.get("variants") or {}
        for variant_name, variant_info in variants.items():
            history = variant_info.get("history") or []
            versions = set(
                item.get("version") for item in history if item.get("version")
            )
            result[(product_name, variant_name)] = versions
    return result

def diff_new_versions(old_versions_map, products):
    """
    对比旧版本集合和新生成的 products 数据，返回新增版本列表，每项为
    (product_name, variant_name, version)。只关心 NOTIFY_VARIANTS 里的 variant。
    old_versions_map 为 None（首次运行、无旧文件）时不产生任何"新增"，
    避免首次生成就把全部历史版本当成新版本发一遍通知。
    """
    if old_versions_map is None:
        return []

    new_items = []
    for product_name, product_info in products.items():
        variants = product_info.get("variants") or {}
        for variant_name, variant_info in variants.items():
            if variant_name not in NOTIFY_VARIANTS:
                continue
            history = variant_info.get("history") or []
            old_versions = old_versions_map.get((product_name, variant_name), set())
            for item in history:
                version = item.get("version")
                if version and version not in old_versions:
                    new_items.append((product_name, variant_name, version))
    return new_items

def notify_new_versions(new_items):
    """
    把本次检测到的新版本合并成一次通知，调用 bbs-be 的 /internal/version-notify。
    任何异常都只打印日志，不向上抛出，避免影响 products.json 的正常生成。
    """
    if not new_items:
        return
    if not INTERNAL_NOTIFY_SECRET:
        print("[notify] BBS_INTERNAL_NOTIFY_SECRET not configured, skip notify")
        return

    # 摘要文本示例："k230_canmv_defconfig(linux v0.6.5), BPI-CanMV-K230D-Zero_defconfig(debian v0.6.4)"
    summary = ", ".join(
        "{0}({1} v{2})".format(product_name, variant_name, version)
        for product_name, variant_name, version in new_items
    )

    body = json.dumps({
        "chip": "k230,k230d",
        "osType": "linux",
        "version": summary,
        "downloadUrl": LINUX_SDK_DOWNLOAD_URL,
    }).encode("utf-8")

    req = urllib.request.Request(
        INTERNAL_NOTIFY_URL,
        data=body,
        headers={
            "Content-Type": "application/json",
            "X-Internal-Secret": INTERNAL_NOTIFY_SECRET,
        },
        method="POST",
    )
    try:
        with urllib.request.urlopen(req, timeout=10) as resp:
            print("[notify] version-notify responded: {0} {1}".format(
                resp.status, resp.read().decode("utf-8", "ignore")
            ))
    except urllib.error.URLError as e:
        print("[notify] failed to call version-notify: {0}".format(str(e)))
    except Exception as e:
        print("[notify] unexpected error calling version-notify: {0}".format(str(e)))

def update_products_json():
    old_versions_map = load_history_versions("products.json")
    products = open_json()
    # 路径根据实际环境可能需要调整
    sdk_release_dirs_2_json("/data/kendryte-download/k230/release/linux_sdk_images", products, 20)
    #sdk_release_dirs_2_json("/data/kendryte-download/developer/releases/canmv_k230_micropython", products, 10)
    sdk_release_dirs_2_json("/data1/k230/release/linux_sdk_images/", products, 20)
    #sdk_release_dirs_2_json("/home/wangjianxin/k230/k230_linux_sdk/output/", products, 20)

    try:
        new_items = diff_new_versions(old_versions_map, products)
        if new_items:
            print("[notify] detected {0} new version(s): {1}".format(
                len(new_items), new_items
            ))
            notify_new_versions(new_items)
    except Exception as e:
        # 通知流程本身不应该影响 products.json 的生成，任何异常都只记录不抛出
        print("[notify] notify flow failed unexpectedly: {0}".format(str(e)))

    save_json(products, "products.json")

if __name__ == "__main__":
    update_products_json()
