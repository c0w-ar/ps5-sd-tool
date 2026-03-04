# 1. SDK Setup
ifdef PS5_PAYLOAD_SDK
    include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk
else
    $(error PS5_PAYLOAD_SDK is undefined)
endif

# 2. Paths and Variables
BIN_DIR := bin
CFLAGS  := -Werror -Iinclude -Iinclude/commands -Iexternal/sqlite
LDFLAGS :=
SRCS := $(wildcard source/*.c) $(wildcard source/commands/*.c) $(wildcard external/sqlite/sqlite3.c)
LIBS    := -lkernel_sys -lkernel -lSceSystemService -lSceUserService -lSceFsInternalForVsh -lSceRegMgr

# 3. Define the 3 Output Targets
ELF_ALL := $(BIN_DIR)/ps5-sd-tool.elf

# --- Target-Specific Flags ---

$(ELF_ALL): CFLAGS += -DDUMP_MAIN_SEGMENTS=1 -DDUMP_FILES=1

# --- Rules ---

# Build all by default
all: $(ELF_INFO) $(ELF_SEGMENTS) $(ELF_ALL) $(ELF_SAVEDATA)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Generic rule to build any ELF from the same sources
$(BIN_DIR)/%.elf: $(SRCS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRCS) $(LIBS)

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean