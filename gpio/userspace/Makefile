#
# Makefile for rpi gpio
#
#

TOP=../..
INSTALL_BIN=/usr/bin/install -c -m 555
INSTALL_DIR=rpi_build

SUBDIRS=lib
INC_DIRS=$(include)
INCLUDE=$(foreach d, $(INC_DIRS), -I$d)

PKG_NO=$(shell cat $(TOP)/buildnum.txt)
PKG_NAME=$(shell cat $(TOP)/package.txt)

.PHONY: all
all: main.o
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir all; \
	done
	$(CC) -o rpi-gpio-console main.o lib/librpigpio.a -lpthread -Wall

.PHONY: install
install: all
# Make the package structure and copy the binaries
	mkdir -p $(INSTALL_DIR)/usr/local/bin
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir install; \
	done
	$(INSTALL_BIN) rpi-gpio-console $(INSTALL_DIR)/usr/local/bin
	
.PHONY: deploy
deploy: install	
# Make the control file
	@echo "Package: $(PKG_NAME)" > control
	@echo "Version: 1.0.$(PKG_NO)" >> control
	@echo "Architecture: all" >> control
	@echo "Maintainer: fudgebread" >> control
	@echo "Description: Basic ipk for rpi gpio." >> control

# And the mandatory debian-binary file for ipks
	@echo "2.0" > debian-binary
	
# Tar everything up
	@tar czf control.tar.gz control
	@tar czf data.tar.gz -C $(INSTALL_DIR) .
	@ar -r $(PKG_NAME)-$(PKG_NO).ipk debian-binary control.tar.gz data.tar.gz 
	
# Housekeeping
	@rm control
	@rm debian-binary
	@rm *.gz
	@echo `expr $(PKG_NO) + 1` > $(TOP)/buildnum.txt
	
.PHONY: clean
clean:
	-@rm -rf $(INSTALL_DIR)
	-@rm main.o rpi-gpio-console
	-@rm *.ipk
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir clean; \
	done
