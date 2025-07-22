#include "parse_args.h"

/**
 * @brief 解析配置文件并填充 config_args 结构体，支持默认值和 JSON 配置文件两种模式
 * 
 * @param config 配置文件路径（为空字符串时使用默认参数）
 * @param args 输出参数结构体，解析后填充相关字段
 * @param debug_mode 调试等级，>0 时打印解析信息
 */
void parse_args(std::string config, config_args& args, int debug_mode)
{
    if (config == "")
    {
        // 无配置文件，设置默认参数
        args.input_height = 320;   ///< 输入图像高度默认320
        args.input_width = 320;    ///< 输入图像宽度默认320
        args.obj_thresh = 0.5;     ///< 目标置信度阈值默认0.5
        args.nms_thresh = 0.35;    ///< NMS阈值默认0.35
        args.num_class = 0;        ///< 类别数默认0
        args.model_type = "FreeDet";   ///< 模型类型默认FreeDet
        args.kmodel_path = "";          ///< 模型文件路径为空
        int strides[3] = {8, 16, 32};
        memcpy(args.strides, strides, 3);  ///< 步幅设置为8,16,32
        args.labels = {""};          ///< 类别标签默认
    }
    else
    {
        // 读取并解析JSON配置文件
        cout << "Deploy Json config file = " << config << std::endl;

        Json::Reader reader;
        Json::Value root;

        ifstream srcFile(config, ios::binary);
        if (!srcFile.is_open())
        {
            cout << "Fail to open : " << config << endl;
            return;
        }

        if (reader.parse(srcFile, root))
        {
            // 必填参数
            args.input_height = root["inference_height"].asInt();
            args.input_width = root["inference_width"].asInt();

            // 可选参数，存在则读取，否则赋默认值
            if (root.isMember("nms_threshold"))
                args.nms_thresh = root["nms_threshold"].asFloat();
            else
                args.nms_thresh = 0;

            if (root.isMember("confidence_threshold"))
                args.obj_thresh = root["confidence_threshold"].asFloat();
            else
                args.obj_thresh = 0;

            if (root.isMember("mask_threshold"))
                args.mask_thresh = root["mask_threshold"].asFloat();
            else
                args.mask_thresh = 0;

            if (root.isMember("box_threshold"))
                args.box_thresh = root["box_threshold"].asFloat();
            else
                args.box_thresh = 0;

            if (root.isMember("fixed_length"))
                args.fixed_length = root["fixed_length"].asBool();
            else
                args.fixed_length = false;

            if (root.isMember("dict_num"))
                args.dict_num = root["dict_num"].asInt();
            else
                args.dict_num = 0;

            if (root.isMember("nms_option"))
                args.nms_option = root["nms_option"].asBool();
            else
                args.nms_option = false;

            if (root.isMember("image_threshold"))
                args.image_threshold = root["image_threshold"].asFloat();
            else
                args.image_threshold = 1.5;

            // memory_bank_shape 默认大小，支持2个维度
            if (root.isMember("memory_bank_shape"))
            {
                args.memory_bank_shape.push_back(root["memory_bank_shape"][0].asInt());
                args.memory_bank_shape.push_back(root["memory_bank_shape"][1].asInt());
            }
            else
            {
                args.memory_bank_shape.push_back(2140);
                args.memory_bank_shape.push_back(40);
            }

            // 其他必填参数
            args.num_class = root["num_classes"].asInt();
            args.model_type = root["model_type"].asString();
            args.kmodel_path = root["kmodel_path"].asString();

            // 固定步幅赋值
            args.strides[0] = 8;
            args.strides[1] = 16;
            args.strides[2] = 32;

            // 读取类别标签
            args.labels.clear();
            for (int i = 0; i < root["categories"].size(); i++)
            {
                args.labels.push_back(root["categories"][i].asString());
            }

            // 校验类别数与标签数是否一致
            if (args.num_class != args.labels.size())
            {
                cout << "ERROR in " << __func__ << "  line:" << __LINE__ << ": class nums is not equals labels' nums" << endl;
                exit(1);
            }
            if (root.isMember("anchors"))
            {
                // 读取 anchors 三维数组：anchors[layer][anchor_index][xy]
                for (int i = 0; i < root["anchors"].size(); i++)
                {
                    for (int j = 0; j < root["anchors"][i].size(); j++)
                    {
                        args.anchors[i][j / 2][j % 2] = root["anchors"][i][j].asFloat();
                        std::cout << "anchors[" << i << "][" << j / 2 << "][" << j % 2 << "] = " << args.anchors[i][j / 2][j % 2] << std::endl;
                    }
                }
            }
        }
    }

    // 调试打印解析结果
    if (debug_mode > 0)
    {
        std::cout << "=== Parsed Args (Debug Mode) ===" << std::endl;
        std::cout << "Input Height: " << args.input_height << std::endl;
        std::cout << "Input Width: " << args.input_width << std::endl;
        std::cout << "Object Threshold: " << args.obj_thresh << std::endl;
        std::cout << "NMS Threshold: " << args.nms_thresh << std::endl;
        std::cout << "Mask Threshold: " << args.mask_thresh << std::endl;
        std::cout << "Box Threshold: " << args.box_thresh << std::endl;
        std::cout << "Num Classes: " << args.num_class << std::endl;
        std::cout << "Model Type: " << args.model_type << std::endl;
        std::cout << "KModel Path: " << args.kmodel_path << std::endl;
        std::cout << "Strides: [" << args.strides[0] << ", " << args.strides[1] << ", " << args.strides[2] << "]" << std::endl;
        std::cout << "Fixed Length: " << (args.fixed_length ? "true" : "false") << std::endl;
        std::cout << "Dict Num: " << args.dict_num << std::endl;
        std::cout << "NMS Option: " << (args.nms_option ? "true" : "false") << std::endl;
        std::cout << "Image Threshold: " << args.image_threshold << std::endl;
        std::cout << "Memory Bank Shape: [" << args.memory_bank_shape[0] << ", " << args.memory_bank_shape[1] << "]" << std::endl;

        std::cout << "Labels: ";
        for (const auto &label : args.labels)
        {
            std::cout << label << " ";
        }
        std::cout << std::endl;
        std::cout << "===============================" << std::endl;
    }
}
