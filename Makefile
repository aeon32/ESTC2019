BUILD_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))


SPL_PATH = $(BUILD_ROOT)/stm_spl/
COMMON_PATH = $(BUILD_ROOT)/common
LABS_DIR = $(BUILD_ROOT)/projects

.PHONY: labs spl common lab1 clean

all: labs

labs: lab1 lab2 lab2_it lab3

spl:
	make -C $(SPL_PATH)

common:
	make -C $(COMMON_PATH)

lab1: spl common
	make -C $(LABS_DIR)/lab1

lab2: spl common
	make -C $(LABS_DIR)/lab2

lab2_it: spl common
	make -C $(LABS_DIR)/lab2_it

lab3: spl common
	make -C $(LABS_DIR)/lab3

clean:
	make -C $(SPL_PATH) clean
	make -C $(COMMON_PATH) clean
	make -C $(LABS_DIR)/lab1 clean
	make -C $(LABS_DIR)/lab2 clean
	make -C $(LABS_DIR)/lab2_it clean
	make -C $(LABS_DIR)/lab3 clean