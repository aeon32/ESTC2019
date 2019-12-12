BUILD_ROOT=../..
OBJS = main.o

BINARY_NAME = main

INCLUDE_DIRS += -I$(LIBEOM_PATH)
LIB_DIRS  += $(LIBEOM_PATH)
LDLIBS += -l$(LIBEOM)

include $(BUILD_ROOT)/common_defs.mk

.PHONY: $(BINARY_NAME)

$(BINARY_NAME): $(BINARY_NAME).bin

clean:
	@rm -f $(OBJS) main.elf main.bin main.elf.map 

flash: $(BINARY_NAME)
	$(FLASH_BINARY)
