#!/bin/bash

KMODEL_DIR="${PWD}/kmodel"
# 检查目录是否存在
if [ -d "$KMODEL_DIR" ]; then
    echo "kmodel directory exists."
else
    echo "kmodel directory does not exist."
    if curl --output /dev/null --silent --head --fail "https://ai.b-bug.org/k230/"; then 
        echo "URL is accessible"; 
        wget https://ai.b-bug.org/k230/downloads/kmodel/kmodel_v2.9.0.1.tgz;
        tar -xvf kmodel_v2.9.0.1.tgz;
        rm -rf kmodel_v2.9.0.1.tgz;
    else
        echo "URL is not accessible";
        wget https://kendryte-download.canaan-creative.com/k230/downloads/kmodel/kmodel_v2.9.0.1.tgz;
        tar -xvf kmodel_v2.9.0.1.tgz;
        rm -rf kmodel_v2.9.0.1.tgz;
    fi
fi

# clear
rm -rf out
mkdir out

subdirs=$(find . -mindepth 1 -maxdepth 1 -type d)

k230_bin=`pwd`/k230_bin
kmodel_root_dir=`pwd`/kmodel/ai_poc
rm -rf ${k230_bin}
mkdir -p ${k230_bin}

if [ -z "$1" ]; then
    curr_pro="all"
else
    curr_pro="$1"
fi

echo "$curr_pro"

for subdir in $(ls -d */); do
      if [ ! -d "$subdir" ]; then
        echo "$subdir 不是一个目录，跳过..."
        continue
      fi
      
      subdir_name=$(basename $subdir)
      # 检查子目录是否为目录"A"，如果是，则跳过
      if [ "$subdir_name" = "common" ] || [[ "$subdir_name" == k230_bin* ]] || [ "$subdir_name" = "shell" ] || [ "$subdir_name" = "out" ] || [ "$subdir_name" = "kmodel" ]; then
            continue
      fi
      
      if [ "$subdir_name" = "llamac" ] && { [ "$curr_pro" = "llamac" ] || [ "$curr_pro" = "all" ]; }; then
            /opt/toolchain/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.2/bin/riscv64-unknown-linux-gnu-g++ -O3 llamac/llama_run.cc -o out/llama_run -lm
      elif { [ "$curr_pro" = "$subdir_name" ] || [ "$curr_pro" = "all" ]; }; then
            echo "******************$subdir_name 开始编译******************"
            if [ "$subdir_name" = "pose_det_rtsp_plug" ]; then
                  cd pose_det_rtsp_plug/little/
                  ./build.sh
                  cd ../../
            elif [ "$subdir_name" = "translate_en_ch" ]; then
                  cp -a ${kmodel_root_dir}/utils/*.a ${k230_bin}
            fi
            pushd out
            cmake -DCMAKE_BUILD_TYPE=Release                \
            -DCMAKE_INSTALL_PREFIX=`pwd`               \
            -D$subdir_name=ON                        \
            -DCMAKE_C_COMPILER=/opt/toolchain/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.2/bin/riscv64-unknown-linux-gnu-gcc \
            ..
            make -j && make install
            echo "******************$subdir_name 编译完成******************"
            popd
      else
          continue  
      fi

      mkdir -p ${k230_bin}/$subdir_name

      if [ "$subdir_name" = "face_detection" ]; then
            cp out/bin/face_detection.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_detection_640.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/1024x624.jpg ${k230_bin}/$subdir_name
            cp -a shell/face_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_landmark" ]; then
            cp out/bin/face_landmark.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_landmark.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/1024x1331.jpg ${k230_bin}/$subdir_name
            cp -a shell/face_landmark_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_glasses" ]; then
            cp out/bin/face_glasses.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_glasses.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/1024x768.jpg ${k230_bin}/$subdir_name
            cp -a shell/face_glasses_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_mask" ]; then
            cp out/bin/face_mask.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_mask.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/1024x768.jpg ${k230_bin}/$subdir_name
            cp -a shell/face_mask_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_emotion" ]; then
            cp out/bin/face_emotion.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_emotion.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/1024x768.jpg ${k230_bin}/$subdir_name
            cp -a shell/face_emotion_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_pose" ]; then
            cp out/bin/face_pose.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_pose.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/1024x768.jpg ${k230_bin}/$subdir_name
            cp -a shell/face_pose_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_gender" ]; then
            cp out/bin/face_gender.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_gender.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/1024x768.jpg ${k230_bin}/$subdir_name
            cp -a shell/face_gender_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_parse" ]; then
            cp out/bin/face_parse.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_parse.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/1024x768.jpg ${k230_bin}/$subdir_name
            cp -a shell/face_parse_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "head_detection" ]; then
            cp out/bin/head_detection.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/head_detection.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/640x340.jpg ${k230_bin}/$subdir_name
            cp -a shell/head_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_mesh" ]; then
            cp out/bin/face_mesh.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_alignment.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_alignment_post.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/1024x768.jpg ${k230_bin}/$subdir_name 
            cp -a ${kmodel_root_dir}/utils/bfm_tri.bin ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/ncc_code.bin ${k230_bin}/$subdir_name
            cp -a shell/face_mesh_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "face_alignment" ]; then
            cp out/bin/face_alignment.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_alignment.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_alignment_post.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/1024x768.jpg ${k230_bin}/$subdir_name 
            cp -a ${kmodel_root_dir}/utils/bfm_tri.bin ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/ncc_code.bin ${k230_bin}/$subdir_name
            cp -a shell/face_alignment_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "licence_det" ]; then
            cp out/bin/licence_det.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/LPD_640.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/licence.jpg ${k230_bin}/$subdir_name
            cp -a shell/licence_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "licence_det_rec" ]; then
            cp out/bin/licence_det_rec.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/LPD_640.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/licence_reco.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/licence.jpg ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/HZKf2424.hz ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/Asci0816.zf ${k230_bin}/$subdir_name
            cp -a shell/licence_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "llamac" ]; then
            cp out/llama_run ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/llama.bin ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/tokenizer.bin ${k230_bin}/$subdir_name
            cp -a shell/llama_build.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "object_detect_yolov8n" ]; then
            cp out/bin/ob_det.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/yolov8n_320.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/yolov8n_640.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/bus.jpg ${k230_bin}/$subdir_name            
            cp -a shell/ob_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "segment_yolov8n" ]; then
            cp out/bin/seg.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/yolov8n_seg_320.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/yolov8n_seg_640.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/bus.jpg ${k230_bin}/$subdir_name 
            cp -a shell/segment_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "person_detect" ]; then
            cp out/bin/person_detect.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/person_detect_yolov5n.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/bus.jpg ${k230_bin}/$subdir_name            
            cp -a shell/person_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "pose_detect" ]; then
            cp out/bin/pose_detect.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/yolov8n-pose.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/bus.jpg ${k230_bin}/$subdir_name            
            cp -a shell/pose_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "pphumanseg" ]; then
            cp out/bin/pphumanseg.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/human_seg_2023mar.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/1000.jpg ${k230_bin}/$subdir_name            
            cp -a shell/pphumanseg_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "smoke_detect" ]; then
            cp out/bin/smoke_detect.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/yolov5s_smoke_best.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/smoke1.jpg ${k230_bin}/$subdir_name 
            cp -a shell/smoke_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "traffic_light_detect" ]; then
            cp out/bin/traffic_light_detect.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/traffic.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/traffic.jpg ${k230_bin}/$subdir_name 
            cp -a shell/traffic_light_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "helmet_detect" ]; then
            cp out/bin/helmet_detect.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/helmet.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/helmet.jpg ${k230_bin}/$subdir_name 
            cp -a shell/helmet_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "yolop_lane_seg" ]; then
            cp out/bin/yolop.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/yolop.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/road.jpg ${k230_bin}/$subdir_name
            cp -a shell/yolop_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "crosswalk_detect" ]; then
            cp out/bin/crosswalk_detect.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/crosswalk.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/cw.jpg ${k230_bin}/$subdir_name 
            cp -a shell/crosswalk_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "bytetrack" ]; then
            cp out/bin/bytetrack.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/bytetrack_yolov5n.kmodel ${k230_bin}/$subdir_name
            cp -ar ${kmodel_root_dir}/images/bytetrack_data ${k230_bin}/$subdir_name
            cp -a shell/bytetrack_*.sh ${k230_bin}/$subdir_name
            mkdir -p ${k230_bin}/$subdir_name/bytetrack_data/output
      fi

      if [ "$subdir_name" = "falldown_detect" ]; then
            cp out/bin/falldown_detect.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/yolov5n-falldown.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/falldown_elder.jpg ${k230_bin}/$subdir_name            
            cp -a shell/falldown_detect_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "person_attr" ]; then
            cp out/bin/person_attr.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/person_attr_yolov5n.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/person_pulc.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/hrnet_demo.jpg ${k230_bin}/$subdir_name            
            cp -a shell/person_attr_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "vehicle_attr" ]; then
            cp out/bin/vehicle_attr.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/vehicle_attr_yolov5n.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/vehicle.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/car.jpg ${k230_bin}/$subdir_name            
            cp -a shell/vehicle_attr_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "puzzle_game" ]; then
            cp out/bin/puzzle_game.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/pintu.bin ${k230_bin}/$subdir_name
            cp -a shell/puzzle_game_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "finger_guessing" ]; then
            cp out/bin/finger_guessing.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/shitou.bin ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/bu.bin ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/jiandao.bin ${k230_bin}/$subdir_name
            cp -a shell/finger_guessing_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "fitness" ]; then
            cp out/bin/fitness.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/yolov8n-pose.kmodel ${k230_bin}/$subdir_name
            cp -a shell/fitness_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "eye_gaze" ]; then
            cp out/bin/eye_gaze.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/eye_gaze.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/1024x1111.jpg ${k230_bin}/$subdir_name 
            cp -a shell/eye_gaze_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "space_resize" ]; then
            cp out/bin/space_resize.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a shell/space_resize_*.sh ${k230_bin}/$subdir_name
      fi
      if [ "$subdir_name" = "face_verification" ]; then
            cp out/bin/face_verification.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_recognition.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/identification_card.png ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/person.png ${k230_bin}/$subdir_name
            cp -a shell/face_verification_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "sq_hand_det" ]; then
            cp out/bin/sq_hand_det.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/input_hd.jpg ${k230_bin}/$subdir_name
            cp -a shell/handdet_cpp_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "sq_handkp_det" ]; then
            cp out/bin/sq_handkp_det.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/input_hd.jpg ${k230_bin}/$subdir_name
            cp -a shell/handkpdet_cpp_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "sq_handkp_class" ]; then
            cp out/bin/sq_handkp_class.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/input_hd.jpg ${k230_bin}/$subdir_name
            cp -a shell/handkpclass_cpp_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "sq_handreco" ]; then
            cp out/bin/sq_handreco.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/hand_reco.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/input_hd.jpg ${k230_bin}/$subdir_name
            cp -a shell/handreco_cpp_*.sh ${k230_bin}/$subdir_name
      fi
      if [ "$subdir_name" = "sq_handkp_flower" ]; then
            cp out/bin/sq_handkp_flower.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/flower_rec.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/input_flower.jpg ${k230_bin}/$subdir_name
            cp -a shell/handkpflower_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "sq_handkp_ocr" ]; then
            cp out/bin/sq_handkp_ocr.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/ocr_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/ocr_rec.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/input_ocr.jpg ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/dict_6625.txt ${k230_bin}/$subdir_name
            cp -a shell/handkpocr_cpp_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "person_distance" ]; then
            cp out/bin/person_distance.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/person_detect_yolov5n.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/input_pd.jpg ${k230_bin}/$subdir_name
            cp -a shell/person_distance_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "dynamic_gesture" ]; then
            cp out/bin/dynamic_gesture.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/gesture.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/shang.bin ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/xia.bin ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/zuo.bin ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/you.bin ${k230_bin}/$subdir_name
            cp -a shell/gesture*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "virtual_keyboard" ]; then
            cp out/bin/virtual_keyboard.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a shell/virtual_keyboard.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "anomaly_det" ]; then
            cp out/bin/anomaly_det.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/anomaly_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/000.png ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/memory.bin ${k230_bin}/$subdir_name
            cp -a shell/anomaly_det_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "ocr" ]; then
            cp out/bin/ocr_reco.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/ocr_det_int16.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/ocr_rec_int16.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/images/333.jpg ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/dict_ocr.txt  ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/SourceHanSansSC-Normal-Min.ttf  ${k230_bin}/$subdir_name
            cp -a shell/ocr_*.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "demo_mix" ]; then
            cp out/bin/demo_mix.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/hand_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/handkp_det.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/gesture.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_detection_320.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/face_pose.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/cropped_test127.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/nanotrack_backbone_sim.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/nanotracker_head_calib_k230.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/shang.bin ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/xia.bin ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/zuo.bin ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/you.bin ${k230_bin}/$subdir_name
            cp -a shell/demo_mix.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "tts_zh" ]; then
            cp out/bin/tts_zh.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/zh_fastspeech_1.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/zh_fastspeech_2.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/hifigan.kmodel ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/utils/wav_play.elf ${k230_bin}/$subdir_name 
            cp -ar ${kmodel_root_dir}/utils/file ${k230_bin}/$subdir_name 
            cp -a shell/tts_zh.sh ${k230_bin}/$subdir_name
      fi

      if [ "$subdir_name" = "kws" ]; then
            cp out/bin/kws.elf ${k230_bin}/$subdir_name
            cp -a ${kmodel_root_dir}/kmodel/kws.kmodel ${k230_bin}/$subdir_name
            cp -ar ${kmodel_root_dir}/utils/reply_wav/ ${k230_bin}/$subdir_name
            cp -a shell/kws.sh ${k230_bin}/$subdir_name
      fi
      rm -rf out/*
done

rm -rf out
