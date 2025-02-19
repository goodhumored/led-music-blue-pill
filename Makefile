#          ╭──────────────────────────────────────────────────────────╮
#          │                   Переменные для путей                   │
#          ╰──────────────────────────────────────────────────────────╯
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include
BINARY = ${BUILD_DIR}/main
OBJ_DIR = ${BUILD_DIR}/obj
LIBS_DIR = lib

TARGET_OBJ_DIR  = $(BUILD_DIR)/target-obj
HOST_OBJ_DIR    = $(BUILD_DIR)/host-obj

#   ────────────────────────────── Библиотеки ──────────────────────────────
LIBPATHS := ${LIBS_DIR}/libopencm3

#   ──────────────────────────────── тесты ──────────────────────────────
TEST_DIR       = tests
TEST_FILES := $(wildcard $(TEST_DIR)/*.cpp)
TEST_BINS  := $(patsubst $(TEST_DIR)/%.cpp, $(BUILD_DIR)/tests/%, $(TEST_FILES))

OPENCM3_DIR := $(wildcard $(LIBPATHS:=/locm3.sublime-project))
OPENCM3_DIR := $(firstword $(dir $(OPENCM3_DIR)))

CC = arm-none-eabi-gcc
CFLAGS = -Wall -Wextra -O3 -DSTM32F1
LDFLAGS = -Llib/libopencm3/lib -lopencm3_stm32f1
LIBNAME		= opencm3_stm32f1
DEFS		+= -DSTM32F1
LDSCRIPT = stm32f103.ld

FP_FLAGS	?= -msoft-float
ARCH_FLAGS	= -mthumb -mcpu=cortex-m3 $(FP_FLAGS) -mfix-cortex-m3-ldrd

################################################################################
# OpenOCD specific variables

OOCD		?= openocd
OOCD_INTERFACE	?= flossjtag
OOCD_TARGET	?= stm32f1x

################################################################################
# Black Magic Probe specific variables
# Set the BMP_PORT to a serial port and then BMP is used for flashing
BMP_PORT	?=


# Имя исполнимого файла
TARGET = $(BUILD_DIR)/main


ifneq ($(V),1)
Q		:= @
NULL		:= 2>/dev/null
endif

###############################################################################
# Executables

PREFIX		?= arm-none-eabi-

HOST_CXX        := g++
HOST_CC         := gcc
CC		:= $(PREFIX)gcc
CXX		:= $(PREFIX)g++
LD		:= $(PREFIX)gcc
AS		:= $(PREFIX)as
OBJCOPY		:= $(PREFIX)objcopy
GDB		:= $(PREFIX)gdb
STFLASH		= $(shell which st-flash)
STYLECHECK	:= /checkpatch.pl
STYLECHECKFLAGS	:= --no-tree -f --terse --mailback
STYLECHECKFILES	:= $(shell find . -name '*.[ch]')
OPT		:= -O3
DEBUG		:= -ggdb3
CSTD		?= -std=c99

# Функция для красивого вывода заголовка
define header
	@echo ""
	@echo "  =========================== $(1) ========================== "
	@echo ""
endef

#          ╭──────────────────────────────────────────────────────────╮
#          │                       Source files                       │
#          ╰──────────────────────────────────────────────────────────╯
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
#   ───────────────────────────────── objs ─────────────────────────────────
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))
#   ────────────────────────────── obj tests ────────────────────────────
HOST_OBJS       = $(patsubst $(SRC_DIR)/%.c, $(HOST_OBJ_DIR)/%.o, $(SRC_FILES))
KISSFFT_OBJ     = $(patsubst $(KISSFFT_DIR)/%.c, $(HOST_OBJ_DIR)/kissfft/%.o, $(KISSFFT_SRC))


ifeq ($(strip $(OPENCM3_DIR)),)
# user has not specified the library path, so we try to detect it

ifeq ($(strip $(OPENCM3_DIR)),)
$(warning Cannot find libopencm3 library in the standard search paths.)
$(error Please specify it through OPENCM3_DIR variable!)
endif
endif

ifeq ($(V),1)
$(info Using $(OPENCM3_DIR) path to library)
endif

define ERR_DEVICE_LDSCRIPT_CONFLICT
You can either specify DEVICE=blah, and have the LDSCRIPT generated,
or you can provide LDSCRIPT, and ensure CPPFLAGS, LDFLAGS and LDLIBS
all contain the correct values for the target you wish to use.
You cannot provide both!
endef

ifeq ($(strip $(DEVICE)),)
# Old style, assume LDSCRIPT exists
DEFS		+= -I$(OPENCM3_DIR)/include
DEFS		+= -I$(INCLUDE_DIR)
LDFLAGS		+= -L$(OPENCM3_DIR)/lib
LDLIBS		+= -l$(LIBNAME)
LDSCRIPT	?= $(BINARY).ld
else
# New style, assume device is provided, and we're generating the rest.
ifneq ($(strip $(LDSCRIPT)),)
$(error $(ERR_DEVICE_LDSCRIPT_CONFLICT))
endif
include $(OPENCM3_DIR)/mk/genlink-config.mk
endif

OPENCM3_SCRIPT_DIR = $(OPENCM3_DIR)/scripts
EXAMPLES_SCRIPT_DIR	= $(OPENCM3_DIR)/../scripts

#          ╭──────────────────────────────────────────────────────────╮
#          │                            flags                         │
#          ╰──────────────────────────────────────────────────────────╯
#   ──────────────────────────────── embed ──────────────────────────────
LDLIBS		+= -Wl,--start-group -lc -lgcc -lnosys -lm -Wl,--end-group

#   ──────────────────────────────── tests ──────────────────────────────
GTEST_LDFLAGS = -lgtest -lgtest_main -lpthread

HOST_CXXFLAGS = -std=c++14 \
                -I$(INCLUDE_DIR) \
                -I$(KISSFFT_DIR) \
                -DFIXED_POINT=16 \
                -DKISSFFT_DATATYPE=int16_t \
                -DKISSFFT_TEST=OFF \
                -DUNIT_TEST \
		-DFFT_SIZE=1024
		# -DPLOTS

#   ─────────────────────────────── C flags ─────────────────────────────
TGT_CFLAGS	+= $(OPT) $(CSTD) $(DEBUG)
TGT_CFLAGS	+= $(ARCH_FLAGS)
TGT_CFLAGS	+= -Wextra -Wshadow -Wimplicit-function-declaration
TGT_CFLAGS	+= -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes
TGT_CFLAGS	+= -fno-common -ffunction-sections -fdata-sections
DEP_DIR := $(OBJ_DIR)/dep
TGT_CFLAGS += -MMD -MP -MF $(DEP_DIR)/$(@F:.o=.d)

#   ────────────────────────────── C++ flags ────────────────────────────
TGT_CXXFLAGS	+= $(OPT) $(CXXSTD) $(DEBUG)
TGT_CXXFLAGS	+= $(ARCH_FLAGS)
TGT_CXXFLAGS	+= -Wextra -Wshadow -Wredundant-decls  -Weffc++
TGT_CXXFLAGS	+= -fno-common -ffunction-sections -fdata-sections

#   ────────────────── C & C++ preprocessor common flags ────────────────
TGT_CPPFLAGS	+= -MD
TGT_CPPFLAGS	+= -Wall -Wundef
TGT_CPPFLAGS	+= $(DEFS)

#   ───────────────────────────── Linker flags ─────────────────────────────
TGT_LDFLAGS		+= --static -nostartfiles
TGT_LDFLAGS		+= -T$(LDSCRIPT)
TGT_LDFLAGS		+= $(ARCH_FLAGS) $(DEBUG)
TGT_LDFLAGS		+= -Wl,-Map=$(*).map -Wl,--cref
TGT_LDFLAGS		+= -Wl,--gc-sections
ifeq ($(V),99)
TGT_LDFLAGS		+= -Wl,--print-gc-sections
endif

#          ╭──────────────────────────────────────────────────────────╮
#          │                         kissfft                          │
#          ╰──────────────────────────────────────────────────────────╯
KISSFFT_DIR = lib/kissfft
TGT_CPPFLAGS += -I$(KISSFFT_DIR)
KISSFFT_SRC := $(KISSFFT_DIR)/kiss_fft.c $(KISSFFT_DIR)/kiss_fftr.c
SRC_FILES += $(KISSFFT_SRC)

KISSFFT_OBJS := $(patsubst $(KISSFFT_DIR)/%.c, $(OBJ_DIR)/%.o, $(KISSFFT_SRC))
OBJS += $(KISSFFT_OBJS)

DEFS += -DFIXED_POINT=16
DEFS += -DKISSFFT_DATATYPE=int16_t
DEFS += -DKISSFFT_TEST=OFF

#          ╭──────────────────────────────────────────────────────────╮
#          │                         targets                          │
#          ╰──────────────────────────────────────────────────────────╯
.SUFFIXES: .elf .bin .hex .srec .list .map .image-static
.SECONDEXPANSION:
.SECONDARY:

all: ${BUILD_DIR} elf

${BUILD_DIR}:
	@mkdir -p $(BUILD_DIR) ${OBJ_DIR} ${DEP_DIR}

elf: $(BINARY).elf
bin: $(BINARY).bin
watch:
	@find src -type f \( -name "*.c" -o -name "*.h" \) | entr -r $(MAKE) flash
flash: $(BINARY).stlink-flash
erase: 
	$(call header,ERASING FLASH)
	$(STFLASH) erase

clean:
	$(call header, CLEAN)
	$(Q)$(RM) -r build
GENERATED_BINARIES=$(BINARY).elf $(BINARY).bin $(BINARY).hex $(BINARY).srec $(BINARY).list $(BINARY).map

# Either verify the user provided LDSCRIPT exists, or generate it.
ifeq ($(strip $(DEVICE)),)
$(LDSCRIPT):
    ifeq (,$(wildcard $(LDSCRIPT)))
        $(error Unable to find specified linker script: $(LDSCRIPT))
    endif
else
include $(OPENCM3_DIR)/mk/genlink-rules.mk
endif

#   ──────────────────────────────── OPENCM ────────────────────────────────
$(OPENCM3_DIR)/lib/lib$(LIBNAME).a:
ifeq (,$(wildcard $@))
	$(warning $(LIBNAME).a not found, attempting to rebuild in $(OPENCM3_DIR))
	$(MAKE) -C $(OPENCM3_DIR)
endif

$(OBJ_DIR)/%.o: $(KISSFFT_DIR)/%.c
	$(Q)$(CC) $(TGT_CFLAGS) $(CFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $@ -c $<


${BUILD_DIR}/%.bin: ${BUILD_DIR}/%.elf
	@#printf "  OBJCOPY $(*).bin\n"
	$(Q)$(OBJCOPY) -Obinary ${BUILD_DIR}/$(*).elf ${BUILD_DIR}/$(*).bin

${BUILD_DIR}/%.elf: $(OBJS) $(LDSCRIPT) $(OPENCM3_DIR)/lib/lib$(LIBNAME).a
	@#printf "  LD      $(*).elf\n"
	$(Q)$(LD) $(TGT_LDFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o ${BUILD_DIR}/$(*).elf

#   ─────────────────────────── объектные файлы ─────────────────────────
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(Q)$(CC) $(TGT_CFLAGS) $(CFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $@  -c $<


#          ╭──────────────────────────────────────────────────────────╮
#          │                         стайлчек                         │
#          ╰──────────────────────────────────────────────────────────╯
stylecheck: $(STYLECHECKFILES:=.stylecheck)
styleclean: $(STYLECHECKFILES:=.styleclean)

# the cat is due to multithreaded nature - we like to have consistent chunks of text on the output
%.stylecheck: %
	$(Q)$(OPENCM3_SCRIPT_DIR)$(STYLECHECK) $(STYLECHECKFLAGS) $* > $*.stylecheck; \
		if [ -s $*.stylecheck ]; then \
			cat $*.stylecheck; \
		else \
			rm -f $*.stylecheck; \
		fi;

%.styleclean:
	$(Q)rm -f $*.stylecheck;

#          ╭──────────────────────────────────────────────────────────╮
#          │                          flash                           │
#          ╰──────────────────────────────────────────────────────────╯
%.stlink-flash: %.bin
	$(call header, FLASH)
	$(STFLASH) write $(*).bin 0x8000000

ifeq ($(BMP_PORT),)
ifeq ($(OOCD_FILE),)
%.flash: %.elf
	@printf "  FLASH   $<\n"
	(echo "halt; program $(realpath $(*).elf) verify reset" | nc -4 localhost 4444 2>/dev/null) || \
		$(OOCD) -f interface/$(OOCD_INTERFACE).cfg \
		-f target/$(OOCD_TARGET).cfg \
		-c "program $(*).elf verify reset exit" \
		$(NULL)
else
%.flash: %.elf
	@printf "  FLASH   $<\n"
	(echo "halt; program $(realpath $(*).elf) verify reset" | nc -4 localhost 4444 2>/dev/null) || \
		$(OOCD) -f $(OOCD_FILE) \
		-c "program $(*).elf verify reset exit" \
		$(NULL)
endif
else
%.flash: %.elf
	@printf "  GDB   $(*).elf (flash)\n"
	$(GDB) --batch \
		   -ex 'target extended-remote $(BMP_PORT)' \
		   -x $(EXAMPLES_SCRIPT_DIR)/black_magic_probe_flash.scr \
		   $(*).elf
endif

#          ╭──────────────────────────────────────────────────────────╮
#          │                          тесты                           │
#          ╰──────────────────────────────────────────────────────────╯
#   ────────────────────────────── объектники ──────────────────────────────
$(HOST_OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(HOST_CC) $(HOST_CXXFLAGS) -c $< -o $@

$(HOST_OBJ_DIR)/kissfft/%.o: $(KISSFFT_DIR)/%.c
	@mkdir -p $(@D)
	$(HOST_CXX) $(HOST_CXXFLAGS) -c $< -o $@


$(TARGET_OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@

test: ${BUILD_DIR} $(TEST_BINS)
	@echo "Запуск всех тестов:"
	@for test_bin in $(TEST_BINS); do \
		echo "▶ $$test_bin"; \
		$$test_bin || exit 1; \
	done

$(BUILD_DIR)/tests/%: $(TEST_DIR)/%.cpp $(HOST_OBJS) $(KISSFFT_OBJ)
	@mkdir -p $(@D)
	$(HOST_CXX) $(HOST_CXXFLAGS) \
		$< \
		$(HOST_OBJS) \
		-o $@ \
		$(GTEST_LDFLAGS)

test-watch:
	find tests src include -name "*.cpp" -o -name "*.c" -o -name "*.h" | entr -c make test


.PHONY: images clean stylecheck styleclean elf bin hex srec list ${BUILD_DIR} test $(TEST_BINARY)

-include $(OBJS:.o=.d)
