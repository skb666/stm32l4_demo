cmake_minimum_required(VERSION 3.10)

project(stm32l4_demo LANGUAGES C CXX)

# 更详细的编译信息
# set(CMAKE_VERBOSE_MAKEFILE on)

# 选择编译版本（可以通过 vscode 指定）
set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_BUILD_TYPE Debug)

set(TOOLCHAINS_PATH "")
set(TOOLCHAINS_PREFIX "arm-none-eabi-")

# 交叉编译器（可以通过 vscode 指定）
set(CMAKE_C_COMPILER "${TOOLCHAINS_PATH}${TOOLCHAINS_PREFIX}gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAINS_PATH}${TOOLCHAINS_PREFIX}g++")

# 生成目标的存放目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output)
# 默认存放静态库的文件夹位置
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output/archive)
# 默认存放动态库的文件夹位置
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output/library)

set(CPU "-mcpu=cortex-m4")
set(FPU "-mfpu=fpv4-sp-d16")
set(FLOAT-ABI "-mfloat-abi=hard")
set(MCU "${CPU} -mthumb ${FPU} ${FLOAT-ABI}")
MESSAGE(STATUS "MCU: " ${MCU})

# 添加编译参数
add_compile_options(-Wall -fdata-sections -ffunction-sections -Os)
add_link_options(-Wall -fdata-sections -ffunction-sections -Os)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${MCU}")
set(CMAKE_C_FLAGS_DEBUG "-g -gdwarf-2 -D DEBUG")
set(CMAKE_C_FLAGS_RELEASE "")

# 如果CMAKE_CXX_STANDARD_REQUIRED设置为ON则必须使用CMAKE_CXX_STANDARD指定的版本
# 如果CMAKE_CXX_STANDARD_REQUIRED设置为OFF则CMAKE_CXX_STANDARD指定版本的为首选版本如果没有会使用上一版本
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${MCU}")
set(CMAKE_CXX_FLAGS_DEBUG "-g -gdwarf-2 -D DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "")

# 递归调用子文件的 CMakeLists.txt
# add_subdirectory(lib)

set(LDSCRIPT "${PROJECT_SOURCE_DIR}/STM32L431RCTx_FLASH.ld")
set(MAP_FILE "${PROJECT_SOURCE_DIR}/output/${PROJECT_NAME}.map")

set(ASM_SOURCES "${PROJECT_SOURCE_DIR}/startup_stm32l431xx.s")
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

