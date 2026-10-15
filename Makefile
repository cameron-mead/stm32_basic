NAME   := basic
CFG    ?= $(NAME).cfg
LDSCRIPT := STM32L476RGTX_FLASH.ld
BUILD  := build

# Sources: your code, startup assembly, and the HAL driver.
# The HAL folder ships *_template.c files that are examples, not meant to be compiled.
C_SRCS := $(wildcard Core/Src/*.c) \
          $(filter-out %_template.c,$(wildcard Drivers/STM32L4xx_HAL_Driver/Src/*.c))
S_SRCS := $(wildcard Core/Startup/*.s)

OBJS := $(patsubst %.c,$(BUILD)/%.o,$(C_SRCS)) \
        $(patsubst %.s,$(BUILD)/%.o,$(S_SRCS))

CPU    := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
DEFS   := -DDEBUG -DUSE_HAL_DRIVER -DSTM32L476xx
INCS   := -ICore/Inc \
          -IDrivers/STM32L4xx_HAL_Driver/Inc \
          -IDrivers/STM32L4xx_HAL_Driver/Inc/Legacy \
          -IDrivers/CMSIS/Device/ST/STM32L4xx/Include \
          -IDrivers/CMSIS/Include

CFLAGS  := $(CPU) -std=gnu11 -g3 -O0 -Wall $(DEFS) $(INCS) \
           -ffunction-sections -fdata-sections --specs=nano.specs -MMD -MP
ASFLAGS := $(CPU) -g3 -x assembler-with-cpp -MMD -MP
LDFLAGS := $(CPU) -T$(LDSCRIPT) --specs=nosys.specs --specs=nano.specs \
           -Wl,-Map=$(BUILD)/$(NAME).map -Wl,--gc-sections -static \
           -Wl,--start-group -lc -lm -Wl,--end-group

CC := arm-none-eabi-gcc

.PHONY: all flash clean

all: $(BUILD)/$(NAME).elf

$(BUILD)/$(NAME).elf: $(OBJS) $(LDSCRIPT)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
	arm-none-eabi-size $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD)/$(NAME).hex: $(BUILD)/$(NAME).elf
	arm-none-eabi-objcopy -O ihex $< $@

flash: $(BUILD)/$(NAME).hex
	openocd -f $(CFG) -c "program $< verify reset exit"

clean:
	rm -rf $(BUILD)

-include $(OBJS:.o=.d)
