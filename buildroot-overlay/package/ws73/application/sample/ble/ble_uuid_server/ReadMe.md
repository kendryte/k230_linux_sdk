# sample 编译准备
1. 更改`Makefile`中的`CROSS`宏为主控使用的工具链。  
2. 在`lib`文件夹中新增BTH静态库文件

# sample 编译
1. 在`applications/sample/ble/ble_uuid_server`目录下执行`make clean;make`即可编译出`ble_server_sample`二进制。  