cmake_minimum_required(VERSION 3.10)

project(stm32l4_demo LANGUAGES C CXX)

MESSAGE(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")
MESSAGE(STATUS "MCU: " ${MCU})
MESSAGE(STATUS "LDSCRIPT: " ${LDSCRIPT})
MESSAGE(STATUS "ASM_SOURCES: " ${ASM_SOURCES})
MESSAGE(STATUS "MAP_FILE: " ${MAP_FILE})

# 添加编译参数
add_compile_options(-Wall -fdata-sections -ffunction-sections -Os)
add_link_options(-Wall -fdata-sections -ffunction-sections -Os)

# 递归调用子文件的 CMakeLists.txt
# add_subdirectory(lib)

set_property(SOURCE ${ASM_SOURCES} PROPERTY LANGUAGE C)
set_source_files_properties(${ASM_SOURCES} PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")

file(GLOB_RECURSE SOURCE_FILES
  "Core/Src/*.c"
  "Drivers/STM32L4xx_HAL_Driver/Src/*.c"
  "Mylibs/RING_FIFO/src/*.c"
  "Mylibs/KEY/src/*.c"
)
MESSAGE(STATUS "SOURCE_FILES: " ${SOURCE_FILES})

# 目标所需的源文件
add_executable(stm32l4_demo.elf ${SOURCE_FILES} ${ASM_SOURCES})

# 目标所需宏定义
target_compile_definitions(stm32l4_demo.elf PUBLIC
  USE_FULL_LL_DRIVER
  USE_HAL_DRIVER
  STM32L431xx
)

# 目标所需的头文件路径
target_include_directories(stm32l4_demo.elf PUBLIC
  "Core/Inc"
  "Drivers/STM32L4xx_HAL_Driver/Inc"
  "Drivers/STM32L4xx_HAL_Driver/Inc/Legacy"
  "Drivers/CMSIS/Include"
  "Drivers/CMSIS/Device/ST/STM32L4xx/Include"
  "Mylibs/RING_FIFO/inc"
  "Mylibs/KEY/inc"
)

# 目标所需链接库
target_link_libraries(stm32l4_demo.elf -lc -lm -lnosys)

# 目标所需链接选项
target_link_options(stm32l4_demo.elf PUBLIC
  -specs=nano.specs
  -T${LDSCRIPT}
  -Wl,-Map=${MAP_FILE},--cref
  -Wl,--gc-sections
)

# 目标编译后自定义指令
add_custom_command(
  TARGET stm32l4_demo.elf POST_BUILD
  COMMAND pwd
  COMMAND ${TOOLCHAINS_PATH}${TOOLCHAINS_PREFIX}objcopy -O binary -S stm32l4_demo.elf stm32l4_demo.bin
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/output
)

