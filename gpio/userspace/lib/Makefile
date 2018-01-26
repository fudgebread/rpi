#
# Makefile for rpi gpio library
#
#

TOP=..
INSTALL_BIN=/usr/bin/install -c -m 555
INSTALL_DIR=rpi_build

.PHONY: all
all: librpigpio.a

librpigpio.a: lib_gpio.o
	ar rcs librpigpio.a lib_gpio.o
	
.PHONY: install
install: all
	mkdir -p $(TOP)/$(INSTALL_DIR)/usr/lib
	$(INSTALL_BIN) librpigpio.a $(TOP)/$(INSTALL_DIR)/usr/lib

.PHONY: clean
clean:
	rm librpigpio.a lib_gpio.o
