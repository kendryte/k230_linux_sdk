#include "sliding_puzzle.h"

/**
 * @brief SlidingPuzzle类的构造函数，用于初始化拼图游戏的相关参数
 * @param level 拼图游戏的难度级别，决定了拼图块的数量（level * level）
 */
SlidingPuzzle::SlidingPuzzle(int level,int osd_width,int osd_height) : level_(level),osd_width_(osd_width),osd_height_(osd_height) {
    // 根据显示模式设置拼图区域和缩略图区域的尺寸
    puzzle_width_ = osd_height_ - 2;  // 拼图区域的宽度
    puzzle_height_ = osd_height_ - 2; // 拼图区域的高度
    // 缩略图区域的宽度，为OSD宽度减去拼图高度再减去7
    puzzle_ori_width_ = osd_width_ - puzzle_height_ - 7;
    puzzle_ori_height_ = puzzle_ori_width_; // 缩略图区域的高度与宽度相同

    // 计算每个拼图块的宽度和高度
    block_w_ = puzzle_width_ / level_;
    block_h_ = puzzle_height_ / level_;
    // 初始化空白块的位置
    blank_x_ = 0;
    blank_y_ = 0;

    // 创建空白块图像，设置其颜色为灰色
    blank_block_ = cv::Mat(block_h_, block_w_, CV_8UC4, cv::Scalar(114, 114, 114, 220));
}

/**
 * @brief 在拼图块上绘制编号
 * @param img 要绘制编号的图像
 * @param ratio 字体大小和线条粗细的缩放比例
 */
void SlidingPuzzle::draw_numbered_blocks(cv::Mat &img, float ratio) {
    // 遍历所有拼图块
    for (int i = 0; i < level_ * level_; ++i) {
        // 计算当前拼图块在网格中的列坐标
        int x = i % level_;
        // 计算当前拼图块在网格中的行坐标
        int y = i / level_;
        // 获取当前拼图块对应的矩形区域
        auto rect = cv::Rect(x * block_w_, y * block_h_, block_w_, block_h_);;
        // 在图像上绘制拼图块的边框
        cv::rectangle(img, rect, cv::Scalar(0, 0, 0, 255), 5);

        // 将当前拼图块的编号转换为字符串
        std::string label = std::to_string(i);
        // 计算编号文本的尺寸
        auto text_size = cv::getTextSize(label, cv::FONT_HERSHEY_DUPLEX, 7 * ratio, 8 * ratio, 0);
        // 计算编号文本的起始绘制位置，使其居中显示
        cv::Point text_org(rect.x + (block_w_ - text_size.width) / 2,
                           rect.y + (block_h_ + text_size.height) / 2);
        // 在图像上绘制编号文本
        cv::putText(img, label, text_org, cv::FONT_HERSHEY_COMPLEX, 7 * ratio,
                    cv::Scalar(255, 0, 0, 255), 8 * ratio, 0);
    }
}

/**
 * @brief 初始化拼图游戏界面，包括绘制编号拼图块、显示缩略图并打乱拼图
 * @param draw_frame 用于绘制游戏界面的图像
 */
void SlidingPuzzle::initialize(cv::Mat &draw_frame) {
    // 计算字体大小和线条粗细的缩放比例，基于拼图块宽度与360的比例
    float ratio = block_w_ / 360.0f;

    // 创建一个用于显示拼图的图像，设置背景颜色
    cv::Mat puzzle_img(puzzle_height_, puzzle_width_, CV_8UC4, cv::Scalar(130, 150, 100, 200));
    // 在拼图图像上绘制带编号的拼图块
    draw_numbered_blocks(puzzle_img, ratio);
    auto to_rect = cv::Rect(blank_x_ * block_w_, blank_y_ * block_h_, block_w_, block_h_);
    // 将空白块复制到拼图图像中空白块所在的位置
    blank_block_.copyTo(puzzle_img(to_rect));
    // 将拼图图像复制到绘制帧的左上角位置
    puzzle_img.copyTo(draw_frame(cv::Rect(0, 0, puzzle_width_, puzzle_height_)));

    // 缩略图显示部分
    // 创建一个用于存储缩略图的图像
    cv::Mat puzzle_thumb;
    // 调整拼图图像的大小以生成缩略图
    cv::resize(puzzle_img, puzzle_thumb, cv::Size(puzzle_ori_width_, puzzle_ori_height_));

    // // 根据显示模式将缩略图复制到绘制帧的不同位置
    // if (DISPLAY_MODE == 0) {
    // 显示模式0下，将缩略图放置在拼图右侧居中位置
    puzzle_thumb.copyTo(draw_frame(cv::Rect(puzzle_width_ + 2, (osd_height_ - puzzle_ori_height_) / 2,
                                            puzzle_ori_width_, puzzle_ori_height_)));
    // 打乱拼图块的位置，使游戏开始时拼图处于无序状态
    shuffle_puzzle(draw_frame);
}

/**
 * @brief 打乱拼图块的位置，使拼图处于无序状态
 * @param draw_frame 用于绘制拼图的图像
 */
void SlidingPuzzle::shuffle_puzzle(cv::Mat &draw_frame) {
    // 定义四个方向：上、下、左、右，用于移动空白块
    std::vector<std::pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    // 初始化随机数种子，使用当前时间作为种子
    srand(static_cast<unsigned>(time(nullptr)));

    // 进行 level_ * 10 次空白块移动操作，以打乱拼图
    for (int i = 0; i < level_ * 10; ++i) {
        // 随机打乱方向数组的顺序，确保每次移动方向随机
        std::random_shuffle(directions.begin(), directions.end());
        // 遍历打乱后的方向数组
        for (auto [dx, dy] : directions) {
            // 计算空白块移动后的新位置
            int nx = blank_x_ + dx;
            int ny = blank_y_ + dy;

            // 检查新位置是否在拼图网格范围内
            if (nx >= 0 && nx < level_ && ny >= 0 && ny < level_) {
                // 获取要移动的拼图块的矩形区域
                auto from_rect = cv::Rect(nx * block_w_, ny * block_h_, block_w_, block_h_);
                // 获取空白块的矩形区域
                auto to_rect =  cv::Rect(blank_x_ * block_w_, blank_y_ * block_h_, block_w_, block_h_);

                // 将选中的拼图块复制到空白块位置
                draw_frame(from_rect).copyTo(draw_frame(to_rect));
                // 将空白块复制到选中的拼图块原来的位置
                blank_block_.copyTo(draw_frame(from_rect));

                // 更新空白块的位置
                blank_x_ = nx;
                blank_y_ = ny;
                // 找到一个有效移动方向后，跳出内层循环
                break;
            }
        }
    }
}

/**
 * @brief 处理手部交互信息，根据手部两点位置尝试移动拼图块
 * @param two_point 包含两个点坐标的向量，格式为 [x1, y1, x2, y2]
 * @param draw_frame 用于绘制拼图的图像
 */
void SlidingPuzzle::process_hand(const std::vector<int> &two_point, cv::Mat &draw_frame) {
    // 检查输入的点坐标数量是否足够，若不足4个则直接返回
    if (two_point.size() < 4) return;

    // 获取绘制帧的宽度和高度
    int osd_width = draw_frame.cols;
    int osd_height = draw_frame.rows;

    // 计算两个点之间的欧几里得距离
    float distance = std::sqrt(std::pow(two_point[0] - two_point[2], 2) + std::pow(two_point[1] - two_point[3], 2));
    // 计算第一个点所在的拼图块的列坐标（浮点型）
    float x = (two_point[0]) / block_w_;
    // 计算第一个点所在的拼图块的行坐标（浮点型）
    float y = (two_point[1]) / block_h_;

    // 将浮点型的列坐标转换为整型
    int gx = static_cast<int>(x);
    // 将浮点型的行坐标转换为整型
    int gy = static_cast<int>(y);

    // 检查两点距离是否小于40，且选中的拼图块坐标在有效范围内
    if (distance < 50.0f && gx >= 0 && gx < level_ && gy >= 0 && gy < level_) {
        // 检查选中的拼图块是否与空白块相邻（水平或垂直方向）
        if ((std::abs(gx - blank_x_) == 1 && gy == blank_y_) || (std::abs(gy - blank_y_) == 1 && gx == blank_x_)) {
            // 获取选中的拼图块的矩形区域
            auto from_rect = cv::Rect(gx * block_w_, gy * block_h_, block_w_, block_h_);
            // 获取空白块的矩形区域
            auto to_rect = cv::Rect(blank_x_ * block_w_, blank_y_ * block_h_, block_w_, block_h_);

            // 将选中的拼图块复制到空白块位置
            draw_frame(from_rect).copyTo(draw_frame(to_rect));
            // 将空白块复制到选中的拼图块原来的位置
            blank_block_.copyTo(draw_frame(from_rect));

            // 更新空白块的位置
            blank_x_ = gx;
            blank_y_ = gy;
        }
    }

    // // 可选：在绘制帧上绘制第一个点击点
    // int cx = two_point[0];
    // int cy = two_point[1];
    // cv::circle(draw_frame, cv::Point(cx, cy), 10, cv::Scalar(0, 0, 255, 255), 5);
}
