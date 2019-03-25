.PHONY: all common main module
#****************************************************************************
#
# Makefile
#
# This is a GNU make (gmake) makefile
all: main

INSTALL_DIR ?= ./package
COMPILE_TIME = $(shell date +"%Y-%M-%d-%H:%M:%S")
RELEASE_VISION = $(shell git show -s --pretty=format:%h)

common:
	$(MAKE) -C $@

module:
	$(MAKE) -C $@

main: common module
	$(MAKE) -C $@

clean:
	$(MAKE) -C module clean
	$(MAKE) -C common clean
	$(MAKE) -C main clean
	rm -rf .vscode/ipch

# install:
# 	test -d ${INSTALL_DIR} || mkdir -p ${INSTALL_DIR} $(INSTALL_DIR)/bin/ $(INSTALL_DIR)/etc/ $(INSTALL_DIR)/lib/
# 	install -m 770 main/yihao01-edge-service $(INSTALL_DIR)/bin/
# 	install -m 644 config/* $(INSTALL_DIR)/etc/	
# 	install -m 770 ../script/catchInfo.sh $(INSTALL_DIR)/bin/
# 	install -m 770 ../script/send_error_message.sh $(INSTALL_DIR)/bin/	
# 	install -m 770 ../script/packInfo.sh $(INSTALL_DIR)/bin/	
# 	-cp -d /usr/local/lib/*.so* $(INSTALL_DIR)/lib/
# 	-cp -d /usr/local/lib64/*.so* $(INSTALL_DIR)/lib/
# 	-cp -d /usr/lib64/mysql/*.so* $(INSTALL_DIR)/lib/

# uninstall:
# 	-rm -rf ${INSTALL_DIR}