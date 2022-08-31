##############################################################################
BUILD = build
BIN = simpleBlePeripheral_low_power
SDK_PATH = ../
CMSIS_PATH = ../../CMSIS_5/
##############################################################################
.PHONY: all directory clean size

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

CFLAGS += -W -Wall --std=gnu99
CFLAGS += -fno-diagnostics-show-caret
CFLAGS += -fdata-sections -ffunction-sections
CFLAGS += -funsigned-char -funsigned-bitfields
CFLAGS += -mcpu=cortex-m0 -mthumb -mthumb-interwork
CFLAGS += -MD -MP -MT $(BUILD)/$(*F).o -MF $(BUILD)/$(@F).d

LDFLAGS += -mcpu=cortex-m0 -mthumb -mthumb-interwork
LDFLAGS += -Wl,--gc-sections

INCLUDES += \
  -I./source \
  -I$(SDK_PATH)/misc \
  -I$(SDK_PATH)/components/arch/cm0 \
  -I$(SDK_PATH)/components/ble/controller \
  -I$(SDK_PATH)/components/ble/include \
  -I$(SDK_PATH)/components/ble/hci \
  -I$(SDK_PATH)/components/ble/host \
  -I$(SDK_PATH)/components/inc \
  -I$(SDK_PATH)/components/osal/include \
  -I$(SDK_PATH)/components/common \
  -I$(SDK_PATH)/components/profiles/ota_app \
  -I$(SDK_PATH)/components/profiles/DevInfo \
  -I$(SDK_PATH)/components/profiles/SimpleProfile \
  -I$(SDK_PATH)/components/profiles/Roles \
  -I$(SDK_PATH)/components/driver/clock \
  -I$(SDK_PATH)/components/driver/pwrmgr \
  -I$(SDK_PATH)/components/driver/uart \
  -I$(SDK_PATH)/components/driver/gpio \
  -I$(SDK_PATH)/components/driver/timer \
  -I$(SDK_PATH)/components/driver/log \
  -I$(SDK_PATH)/components/driver/key \
  -I$(SDK_PATH)/components/driver/pwm \
  -I$(SDK_PATH)/components/driver/flash \
  -I$(SDK_PATH)/components/libraries/crc16 \
  -I$(SDK_PATH)/components/libraries/cliface \
  -I$(SDK_PATH)/components/libraries/fs \
  -I$(SDK_PATH)/components/driver/watchdog \
  -I$(CMSIS_PATH)/Device/ARM/ARMCM0/Include \
  -I$(CMSIS_PATH)/CMSIS/Core/Include

SRCS += \
  ./main.c \
  $(SDK_PATH)/components/profiles/Roles/central.c \
  $(SDK_PATH)/components/driver/clock/clock.c \
  $(SDK_PATH)/components/driver/flash/flash.c \
  $(SDK_PATH)/components/libraries/fs/fs.c \
  $(SDK_PATH)/components/profiles/Roles/gapbondmgr.c \
  $(SDK_PATH)/components/profiles/Roles/gapgattserver.c \
  $(SDK_PATH)/components/profiles/Roles/gap.c \
  $(SDK_PATH)/components/profiles/GATT/gattservapp.c \
  $(SDK_PATH)/components/driver/gpio/gpio.c \
  $(SDK_PATH)/components/driver/key/key.c \
  $(SDK_PATH)/components/driver/led_light/led_light.c \
  $(SDK_PATH)/components/driver/log/my_printf.c \
  $(SDK_PATH)/components/osal/snv/osal_snv.c \
  $(SDK_PATH)/components/profiles/ota_app/ota_app_service.c \
  $(SDK_PATH)/components/profiles/Roles/peripheral.c \
  $(SDK_PATH)/components/driver/pwm/pwm.c \
  $(SDK_PATH)/components/driver/pwrmgr/pwrmgr.c \
  $(SDK_PATH)/components/driver/timer/timer.c \
  $(SDK_PATH)/components/driver/uart/uart.c \
  $(SDK_PATH)/components/driver/watchdog/watchdog.c \
  $(SDK_PATH)/misc/jump_table.c \
  ./RTE/Device/ARMCM0/system_ARMCM0.c \
  ./source/OSAL_SimpleBLEPeripheral.c \
  ./source/sbpProfile_ota.c \
  ./source/FindMy.c \
  ./source/SimpleBLEPeripheral_Main.c


DEFINES += \
  -DADV_NCONN_CFG=0x01 \
  -DADV_CONN_CFG=0x02 \
  -DSCAN_CFG=0x04 \
  -DINIT_CFG=0x08 \
  -DBROADCASTER_CFG=0x01 \
  -DOBSERVER_CFG=0x02 \
  -DPERIPHERAL_CFG=0x04 \
  -DCENTRAL_CFG=0x08 \
  -DHOST_CONFIG=0x4 \
  -D__GCC \
  -DARMCM0 \
  -D_RTE_ \
  -DCFG_CP \
  -DOSAL_CBTIMER_NUM_TASKS="1" \
  -DMTU_SIZE="247" \
  -DHOST_CONFIG="4" \
  -DHCI_TL_NONE="1" \
  -DENABLE_LOG_ROM_="0" \
  -D_BUILD_FOR_DTM_="0" \
  -DDEBUG_INFO="1" \
  -DDBG_ROM_MAIN="0" \
  -DAPP_CFG="0" \
  -DOSALMEM_METRICS="0" \
  -DPHY_MCU_TYPE="MCU_BUMBEE_M0" \
  -DCFG_SLEEP_MODE="PWR_MODE_SLEEP" \
  -DDEF_GAPBOND_MGR_ENABLE="0" \
  -DUSE_FS="0" \
  -DMAX_NUM_LL_CONN="1" \


CFLAGS += $(INCLUDES) $(DEFINES)

OBJS = $(addprefix $(BUILD)/, $(notdir %/$(subst .c,.o, $(SRCS))))
# $(info List of files : [${OBJS}])

all: directory $(BUILD)/$(BIN).elf $(BUILD)/$(BIN).hex $(BUILD)/$(BIN).bin size

$(BUILD)/$(BIN).elf: $(OBJS)
	@echo LD $@
	@$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

$(BUILD)/$(BIN).hex: $(BUILD)/$(BIN).elf
	@echo OBJCOPY $@
	@$(OBJCOPY) -O ihex $^ $@

$(BUILD)/$(BIN).bin: $(BUILD)/$(BIN).elf
	@echo OBJCOPY $@
	@$(OBJCOPY) -O binary $^ $@

%.o:
	@echo CC $@
	@$(CC) $(CFLAGS) $(filter %/$(subst .o,.c,$(notdir $@)), $(SRCS)) -c -o $@

directory:
	@mkdir -p $(BUILD)

size: $(BUILD)/$(BIN).elf
	@echo size:
	@$(SIZE) -t $^

clean:
	@echo clean
	@-rm -rf $(BUILD)

-include $(wildcard $(BUILD)/*.d)
