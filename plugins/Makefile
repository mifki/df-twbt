DFHACKVER ?= 0.40.16-r1

DFVERNUM = `echo $(DFHACKVER) | sed -e s/-r.*// -e s/\\\\.//g`

DF ?= /Users/vit/Downloads/df_40_16_osx
DH ?= /Users/vit/Downloads/dfhack-master

SRC = $(PLUGIN).cpp
DEP = ../renderer_twbt.h Makefile

ifeq ($(shell uname -s), Darwin)
	ifneq (,$(findstring 0.40,$(DFHACKVER)))
		EXT = dylib
	else
		EXT = so
	endif
else
	EXT = so
endif
OUT = ../dist/$(DFHACKVER)/$(PLUGIN).plug.$(EXT)

INC = -I"$(DH)/library/include" -I"$(DH)/library/proto" -I"$(DH)/depends/protobuf" -I"$(DH)/depends/lua/include" -I"$(DH)/plugins" -I..
LIB = -L"$(DH)/build/library" -ldfhack

CFLAGS = $(INC) -m32 -DLINUX_BUILD -O3
LDFLAGS = $(LIB) -shared 

ifeq ($(shell uname -s), Darwin)
	CXX = c++ -std=gnu++0x -stdlib=libstdc++
	CFLAGS += -Wno-tautological-compare
	LDFLAGS += -framework OpenGL -mmacosx-version-min=10.6 -undefined dynamic_lookup
else
	CXX = c++ -std=c++0x
endif


all: $(OUT)

$(OUT): $(SRC) $(DEP)
	-@mkdir -p `dirname $(OUT)`
	$(CXX) $(SRC) -o $(OUT) -DDFHACK_VERSION=\"$(DFHACKVER)\" -DDF_$(DFVERNUM) $(CFLAGS) $(LDFLAGS)

inst: $(OUT)
	cp $(OUT) "$(DF)/hack/plugins/"

clean:
	-rm $(OUT)