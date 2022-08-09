# CMake minimum version
cmake_minimum_required (VERSION 3.1)

# Project Infomation
project( FindMy )
enable_language(ASM)
enable_language(C)

# Reset output path
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

# STDLIB
set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)

# Set include path
include_directories (../../CMSIS_5/CMSIS/Core/Include/)
include_directories (../../CMSIS_5/Device/ARM/ARMCM0/Include/)
include_directories (./source)
include_directories (../misc)
include_directories (../components/inc)
include_directories (../components/ble/controller)
include_directories (../components/osal/include)
include_directories (../components/common)
include_directories (../components/ble/include)
include_directories (../components/ble/hci)
include_directories (../components/ble/host)
include_directories (../components/profiles/ota_app)
include_directories (../components/profiles/DevInfo)
include_directories (../components/profiles/SimpleProfile)
include_directories (../components/profiles/Roles)
include_directories (../components/libraries/crc16)
include_directories (../components/driver/watchdog)
include_directories (../components/driver/clock)
include_directories (../components/arch/cm0)
include_directories (../components/driver/pwrmgr)
include_directories (../components/driver/uart)
include_directories (../components/driver/gpio)
include_directories (../components/driver/timer)
include_directories (../components/driver/log)
include_directories (../components/libraries/cliface)
include_directories (../components/driver/key)
include_directories (../components/driver/pwm)
include_directories (../components/driver/flash)
include_directories (../components/libraries/fs)
include_directories (../components/driver/led_light)

# The need build source path and build all files
set (SRC_FILE0 ./main.c)
set (SRC_FILE1 ../components/driver/uart/uart.c)
set (SRC_FILE2 ../components/driver/clock/clock.c)
set (SRC_FILE3 ../components/driver/gpio/gpio.c)
set (SRC_FILE4 ../components/driver/timer/timer.c)
set (SRC_FILE5 ../components/driver/watchdog/watchdog.c)
set (SRC_FILE6 ../components/driver/pwrmgr/pwrmgr.c)
set (SRC_FILE7 ../components/driver/log/my_printf.c)
set (SRC_FILE8 ../components/driver/key/key.c)
set (SRC_FILE9 ../components/driver/pwm/pwm.c)
set (SRC_FILE10 ../components/osal/snv/osal_snv.c)
set (SRC_FILE11 ../components/driver/flash/flash.c)
set (SRC_FILE12 ../components/driver/led_light/led_light.c)
set (SRC_FILE13 ../components/libraries/fs/fs.c)
set (SRC_FILE14 ../components/libraries/fs/fs.h)
set (SRC_FILE15 ../components/profiles/Roles/central.c)
set (SRC_FILE16 ../components/profiles/Roles/gap.c)
set (SRC_FILE17 ../components/profiles/Roles/gapbondmgr.c)
set (SRC_FILE18 ../components/profiles/Roles/gapgattserver.c)
set (SRC_FILE19 ../components/profiles/Roles/peripheral.c)
set (SRC_FILE20 ../components/profiles/GATT/gattservapp.c)
set (SRC_FILE21 ../components/profiles/ota_app/ota_app_service.c)
set (SRC_FILE22 ../misc/jump_table.c)
set (SRC_FILE23 ./source/sbpProfile_ota.c)
set (SRC_FILE24 ./source/OSAL_SimpleBLEPeripheral.c)
set (SRC_FILE25 ./source/FindMy.c)
set (SRC_FILE26 ./source/SimpleBLEPeripheral_Main.c)


set(CROSS_TARGET_TRIPLET "arm-none-eabi-")

# CC AR LD AS
set(CMAKE_C_COMPILER "${CROSS_TARGET_TRIPLET}gcc")
set(CMAKE_ASM_COMPILER "${CROSS_TARGET_TRIPLET}gcc")

# CFLAGS
set (CMAKE_C_FLAGS "-g -Wextra -Wshadow -Wimplicit-function-declaration -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes -fno-common -ffunction-sections -fdata-sections -MD -Wall -Wundef -mthumb -mcpu=cortex-m0  " CACHE INTERNAL "c compiler flags")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -DCFG_CP  -DOSAL_CBTIMER_NUM_TASKS=1 -DMTU_SIZE=247 -DHOST_CONFIG=4 -DHCI_TL_NONE=1 -DENABLE_LOG_ROM_=0 -D_BUILD_FOR_DTM_=0 -DDEBUG_INFO=1 -DDBG_ROM_MAIN=0 -DAPP_CFG=0 -DOSALMEM_METRICS=0 -DPHY_MCU_TYPE=MCU_BUMBEE_M0 -DCFG_SLEEP_MODE=PWR_MODE_SLEEP -DDEF_GAPBOND_MGR_ENABLE=0 -DUSE_FS=0 -DMAX_NUM_LL_CONN=1 ")

# CXXFLAGS
set (CMAKE_CXX_FLAGS "-Wextra -Wshadow -Wredundant-decls  -Weffc++ -fno-common -ffunction-sections -fdata-sections -MD -Wall -Wundef -mthumb  " CACHE INTERNAL "cxx compiler flags")

# ASMFLAGS
set (CMAKE_ASM_FLAGS "-g -mthumb  " CACHE INTERNAL "asm compiler flags")

# LDFLAGS
set (CMAKE_EXE_LINKER_FLAGS "-g -Wl,--gc-sections -Wl,-Map=FindMy.map -mthumb  " CACHE INTERNAL "executable linker flags")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${CMAKE_SOURCE_DIR}/STM32FLASH.ld -L ")


# Generate the target
add_executable (${CMAKE_PROJECT_NAME}.elf  ${SRC_FILE0}  ${SRC_FILE1}  ${SRC_FILE2}  ${SRC_FILE3}  ${SRC_FILE4}  ${SRC_FILE5}  ${SRC_FILE6}  ${SRC_FILE7}  ${SRC_FILE8}  ${SRC_FILE9}  ${SRC_FILE10}  ${SRC_FILE11}  ${SRC_FILE12}  ${SRC_FILE13}  ${SRC_FILE14}  ${SRC_FILE15}  ${SRC_FILE16}  ${SRC_FILE17}  ${SRC_FILE18}  ${SRC_FILE19}  ${SRC_FILE20}  ${SRC_FILE21}  ${SRC_FILE22}  ${SRC_FILE23}  ${SRC_FILE24}  ${SRC_FILE25}  ${SRC_FILE26} )

# Link the library to the target
target_link_libraries (${CMAKE_PROJECT_NAME}.elf )

# Generate the binary file
add_custom_target (${CMAKE_PROJECT_NAME}.bin ALL arm-none-eabi-objcopy -Obinary "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf" "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.bin" DEPENDS ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf)

# Generate the hex file
add_custom_target (${CMAKE_PROJECT_NAME}.hex ALL arm-none-eabi-objcopy -Oihex "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf" "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.hex" DEPENDS ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf)

# Echo the size Infomation
add_custom_target (size ALL arm-none-eabi-size "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf" DEPENDS ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf)

# Make flash to the board by st-link
add_custom_target (flash COMMAND st-flash write ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.bin 0x8000000)

# Make clean-all
add_custom_target (clean-all COMMAND rm -rf ${CMAKE_BINARY_DIR}/*)
