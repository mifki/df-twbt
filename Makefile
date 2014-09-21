DFHACKVER ?= 0.34.11-r5

DFVERNUM = `echo $(DFHACKVER) | sed -e s/-r.*// -e s/\\\\.//g`

TWBT_VER ?= "5.xx"

DF ?= /Users/vit/Desktop/df-r5
DH ?= /Users/vit/Downloads/dfhack-master

SRC = twbt.cpp
DEP = renderer.hpp config.hpp tradefix.hpp dungeonmode.hpp dwarfmode.hpp renderer_twbt.h commands.hpp plugin.hpp tileupdate_text.hpp tileupdate_map.hpp patches.hpp Makefile legacy/renderer_legacy.hpp legacy/twbt_legacy.hpp

ifeq ($(shell uname -s), Darwin)
	ifneq (,$(findstring 0.40,$(DFHACKVER)))
		EXT = dylib
	else
		EXT = so
	endif
else
	EXT = so
endif
OUT = dist/$(DFHACKVER)/twbt.plug.$(EXT)

INC = -I"$(DH)/library/include" -I"$(DH)/library/proto" -I"$(DH)/depends/protobuf" -I"$(DH)/depends/lua/include"
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
	$(CXX) $(SRC) -o $(OUT) -DDFHACK_VERSION=\"$(DFHACKVER)\" -DDF_$(DFVERNUM) -DTWBT_VER=\"$(TWBT_VER)\" $(CFLAGS) $(LDFLAGS)

inst: $(OUT)
	cp $(OUT) "$(DF)/hack/plugins/"

clean:
	-rm $(OUT)