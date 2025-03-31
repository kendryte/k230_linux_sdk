# sample 编译准备
1. 更改`Makefile`中的`CROSS`宏为主控使用的工具链。  
2. 在`lib`文件夹中新增SLE静态库文件

# sample 编译
1. 在`applications/sample/bt/ble_uuid_client`目录下执行`make clean;make`即可编译出`sle_client_sample`二进制（server同理）。