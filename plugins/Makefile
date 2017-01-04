DFHACKVER ?= 0.43.05-alpha4

DFVERNUM = `echo $(DFHACKVER) | sed -e s/-r.*// -e s/\\\\.//g`

DF ?= /Users/vit/Downloads/df_43_05_osx
DH ?= /Users/vit/Downloads/buildagent-2/workspace/root/dfhack/0.43.05

SRC = $(PLUGIN).cpp
DEP = ../renderer_twbt.h Makefile

ifeq ($(shell uname -s), Darwin)
	ifneq (,$(findstring 0.34,$(DFHACKVER)))
		EXT = so
	else
		EXT = dylib
	endif
else
	EXT = so
endif
OUT = ../dist/$(DFHACKVER)/$(PLUGIN).plug.$(EXT)

INC = -I"$(DH)/library/include" -I"$(DH)/library/proto" -I"$(DH)/depends/protobuf" -I"$(DH)/depends/lua/include" -I"$(DH)/plugins" -I..
LIB = -L"$(DH)/build/library" -ldfhack -ldfhack-version

CXX ?= c++
CFLAGS = $(INC) -m64 -DLINUX_BUILD -O3
LDFLAGS = $(LIB) -shared 

ifeq ($(shell uname -s), Darwin)
	export MACOSX_DEPLOYMENT_TARGET=10.6
	CXX = g++-4.8
	CFLAGS += -std=gnu++0x #-stdlib=libstdc++
	CFLAGS += -Wno-tautological-compare
	LDFLAGS += -framework OpenGL -mmacosx-version-min=10.6 -undefined dynamic_lookup
else
	CFLAGS += -std=c++0x
endif


all: $(OUT)

$(OUT): $(SRC) $(DEP)
	-@mkdir -p `dirname $(OUT)`
	$(CXX) $(SRC) -o $(OUT) -DDFHACK_VERSION=\"$(DFHACKVER)\" -DDF_$(DFVERNUM) $(CFLAGS) $(LDFLAGS)

inst: $(OUT)
	cp $(OUT) "$(DF)/hack/plugins/"

clean:
	-rm $(OUT)