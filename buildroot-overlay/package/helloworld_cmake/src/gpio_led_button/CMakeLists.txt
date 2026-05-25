# cmake_minimum_required(VERSION 3.10)
# project(helloWorld_cmake)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")


set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
add_executable(led  led.c)
target_link_libraries(led PRIVATE m gpiod)
install(TARGETS led  DESTINATION /root/app/helloworld/gpio_led_button//)




add_executable(button  button.c)
target_link_libraries(button PRIVATE m pthread gpiod)
install(TARGETS button  DESTINATION /root/app/helloworld/gpio_led_button/)


add_executable(btn_led  btn_led.c)
target_link_libraries(btn_led PRIVATE m pthread gpiod)
install(TARGETS btn_led  DESTINATION /root/app/helloworld/gpio_led_button/)
