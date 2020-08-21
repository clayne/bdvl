WARNING_FLAGS+=-Wall
OPTION_FLAGS+=-fPIC
R_LFLAGS+=-lc -ldl -lcrypt -lpcap
PLATFORM+=$(shell uname -m)
SONAME+=bdvl.so
NEW_INC+=newinc

all: setup kit

setup:
	python setup.py $(NEW_INC)

kit: $(NEW_INC)/bedevil.c
	$(CC) -std=gnu99 -g $(OPTION_FLAGS) $(WARNING_FLAGS) -I$(NEW_INC) -shared -Wl,--build-id=none $(NEW_INC)/bedevil.c $(R_LFLAGS) -o build/$(SONAME).$(PLATFORM)
	-$(CC) -m32 -std=gnu99 -g $(OPTION_FLAGS) $(WARNING_FLAGS) -I$(NEW_INC) -shared -Wl,--build-id=none $(NEW_INC)/bedevil.c $(R_LFLAGS) -o build/$(SONAME).i686
	strip build/$(SONAME)*

clean:
	rm -rf build/$(SONAME)* $(NEW_INC)

cleanall:
	rm -rf build $(NEW_INC)
