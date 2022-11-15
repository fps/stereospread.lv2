.PHONY: clean install

PREFIX ?= /usr
INSTALL_DIR ?= ${PREFIX}/lib/lv2

OPTFLAGS ?= -O3 -march=native -mcpu=native

all: lv2/stereospread/stereospread.so

lv2/stereospread/stereospread.so: stereospread.cc
	g++ ${OPTFLAGS} -shared -o lv2/stereospread/stereospread.so stereospread.cc

install: all
	install -d ${INSTALL_DIR}/stereospread.lv2
	install lv2/stereospread/stereospread.so ${INSTALL_DIR}/stereospread.lv2
	install lv2/stereospread/*.ttl ${INSTALL_DIR}/stereospread.lv2

