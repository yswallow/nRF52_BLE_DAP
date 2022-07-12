OUTPUT_DIRECTORY := build

include ./Makefile.user

GNU_PREFIX ?= arm-none-eabi
UF2_OUTPUT_DIR ?= uf2

VERBOSE      ?= 0
PROJ_SRC_DIR := ../src
SDK_ROOT 	 ?= ../../../..
DEBUG 		 ?= 

include ./FileList.mk

LINKER_SCRIPT := nRF52_CMSISDAP_v1_s140_v6.ld
ALL_TYPES :=
SRC_FILES := 

ifeq ($(VERBOSE),1)
NO_ECHO :=
else
NO_ECHO := @
endif

# Optimization flags
OPT = -O3 -g3
# Uncomment the line below to enable link time optimization
#OPT += -flto

# GCC flags common to C and ASM
GCC_COMMON_FLAGS += \
	-DAPP_TIMER_V2 \
	-DAPP_TIMER_V2_RTC1_ENABLED \
	-DCONFIG_GPIO_AS_PINRESET \
	-DFLOAT_ABI_HARD \
	-DSOFTDEVICE_PRESENT \
	-D__HEAP_SIZE=8192 \
	-D__STACK_SIZE=8192 \
	-mcpu=cortex-m4 \
	-mthumb -mabi=aapcs \
	-mfloat-abi=hard -mfpu=fpv4-sp-d16 \
	-Wno-unused-function \


# C flags common to all targets
C_COMMON_FLAGS += $(OPT) $(GCC_COMMON_FLAGS)
# keep every function in a separate section, this allows linker to discard unused ones
C_COMMON_FLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
C_COMMON_FLAGS += -fno-builtin -fshort-enums
C_COMMON_FLAGS += -Wall -Werror
ifeq ($(USE_RECENT_ARMGCC),1)
C_COMMON_FLAGS += -Wno-maybe-uninitialized -Wno-array-bounds
endif
C_COMMON_FLAGS += $(if $(DEBUG),-DDEBUG)
C_COMMON_FLAGS += -DNRF52840_XXAA -DS140


# C++ flags common to all targets
CXXFLAGS += $(OPT) $(C_COMMON_FLAGS)

# Assembler flags common to all targets
ASMFLAGS += -g3
ASMFLAGS += $(GCC_COMMON_FLAGS)

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs


CC      := $(GNU_INSTALL_ROOT)/$(GNU_PREFIX)-gcc
CXX     := $(GNU_INSTALL_ROOT)/$(GNU_PREFIX)-c++
LD      := $(GNU_INSTALL_ROOT)/$(GNU_PREFIX)-ld
OBJDUMP := $(GNU_INSTALL_ROOT)/$(GNU_PREFIX)-objdump
OBJCOPY := $(GNU_INSTALL_ROOT)/$(GNU_PREFIX)-objcopy
SIZE    := $(GNU_INSTALL_ROOT)/$(GNU_PREFIX)-size
UF2CONV ?= python ../lib/uf2/utils/uf2conv.py 

CCACHE  := $(if $(wildcard /usr/bin/ccache),ccache)
CC      := $(CCACHE) $(CC)

# $1: file
# $2: files
# out: count of items in files after file
define progress
$(if $(filter-out $(1),$(firstword $(2))),$(call progress,$(1),$(wordlist 2,$(words $(2)),$(2))),$(words $(2)))
endef

# $1: target file
# out: remain files count
define remain_files
$(call progress,$(1),$(TARGET_FILES))
endef

# $1:  a compiled filename
# out: (count of remain files)/(count of all the target files)
define print_progress_light
@echo -ne "$(call remain_files,$(1))/$(words $(TARGET_FILES))\r"
endef

%.hex: %.out
	@echo "Making hex"
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_DIRECTORY)/$< $(OUTPUT_DIRECTORY)/$@

%.uf2: %.hex
	$(NO_ECHO)$(UF2CONV) $(OUTPUT_DIRECTORY)/$< -c -b $(if $(filter 7,SOFTDEVICE_VERSION),0x26000,0x27000) -o $(UF2_OUTPUT_DIR)/$@ -f 0xADA52840

%.c.o:
	@echo "Compiling $(basename $@)"
	$(call print_progress_light,$@)
	$(NO_ECHO)$(CC) $(CFLAGS) $(CFLAGS_PROJ) $(addprefix -I,$(INC_FOLDERS)) -MP -MD -c -o $(OUTPUT_DIRECTORY)/$(notdir $@) $(basename $@)
	
%.cpp.o:
	@echo "Compiling $(basename $@)"
	$(call print_progress_light,$@)
	$(NO_ECHO)$(CC) $(CXXFLAGS) $(addprefix -I,$(INC_FOLDERS)) -MP -MD -c -o $(OUTPUT_DIRECTORY)/$(notdir $@) $(basename $@)
	
%.S.o:
	@echo "Assembling $(basename $@)"
	$(call print_progress_light,$@)
	$(NO_ECHO)$(CC) -x assembler-with-cpp -MP -MD -c -o $(OUTPUT_DIRECTORY)/$(notdir $@) $(basename $@)

# $(1): path
#
# mkdir -p path
define make_dir_available
$(if $(wildcard $(1)),@echo DIRECTORY $(1) exist,mkdir -p $(1))
endef

.PHONY: check_outdir
check_outdir:
	$(call make_dir_available,$(OUTPUT_DIRECTORY))
	$(call make_dir_available,$(UF2_OUTPUT_DIR))


nRF52_BLE_DAP.out: $(addsuffix .o,$(COMMON_SRC))
	@echo "Linking $@"
	$(NO_ECHO)$(CC) $(LDFLAGS) -T$(LINKER_SCRIPT) $(addprefix $(OUTPUT_DIRECTORY)/,$(notdir $(addsuffix .o,$(COMMON_SRC)))) -W -o $(OUTPUT_DIRECTORY)/$@
	$(NO_ECHO)$(SIZE) $(OUTPUT_DIRECTORY)/$@

.PHONY: nRF52_BLE_DAP

nRF52_BLE_DAP: CFLAGS := $(C_COMMON_FLAGS) -std=c99 -DNRF_SD_BLE_API_VERSION=6
nRF52_BLE_DAP: TARGET_FILES := $(addsuffix .o,$(COMMON_SRC))
nRF52_BLE_DAP: SOFTDEVICE_VERSION := 6
nRF52_BLE_DAP: INC_FOLDERS += $(INC_FOLDERS_S140)

nRF52_BLE_DAP: check_outdir nRF52_BLE_DAP.uf2
	@echo done

.PHONY: clean
clean:
	rm -r $(OUTPUT_DIRECTORY_BASE)
	rm -r $(UF2_OUTPUT_DIR)

