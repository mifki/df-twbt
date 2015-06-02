DFHACKVER ?= 0.40.24-r3

DFVERNUM = `echo $(DFHACKVER) | sed -e s/-r.*// -e s/\\\\.//g`

TWBT_VER ?= "6.xx"

DF ?= /Users/vit/Downloads/df_40_24_osx
DH ?= /Users/vit/Downloads/dfhack-4024

SRC = twbt.cpp
DEP = renderer.hpp config.hpp dungeonmode.hpp dwarfmode.hpp renderer_twbt.h commands.hpp plugin.hpp tileupdate_text.hpp tileupdate_map.hpp patches.hpp macros.hpp items.hpp buildings.hpp Makefile legacy/renderer_legacy.hpp legacy/twbt_legacy.hpp

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

INC = -I"$(DH)/library/include" -I"$(DH)/library/proto" -I"$(DH)/depends/protobuf" -I"$(DH)/depends/lua/include" -I"$(DF)/libs/SDL.framework/Headers"
LIB = -L"$(DH)/build/library" -F"$(DF)/libs" -framework SDL -framework SDL_image -ldfhack -ldfhack-version

CXX = c++
CFLAGS = $(INC) -m32 -DLINUX_BUILD -g #-O3
LDFLAGS = $(LIB) -shared 

ifeq ($(shell uname -s), Darwin)
	CFLAGS += -std=gnu++0x -stdlib=libstdc++
	CFLAGS += -Wno-tautological-compare
	LDFLAGS += -framework OpenGL -mmacosx-version-min=10.6 #-undefined dynamic_lookup
else
	CFLAGS += -std=c++0x
endif


all: $(OUT)

$(OUT): $(SRC) $(DEP)
	-@mkdir -p `dirname $(OUT)`
	$(CXX) $(SRC) -o $(OUT) -DDF_$(DFVERNUM) -DTWBT_VER=\"$(TWBT_VER)\" $(CFLAGS) $(LDFLAGS)

inst: $(OUT)
	cp $(OUT) "$(DF)/hack/plugins/"

inst_all:
	make inst
	cd plugins && DFHACKVER=$(DFHACKVER) DF=$(DF) DH=$(DH) PLUGIN=mousequery make inst
	cd plugins && DFHACKVER=$(DFHACKVER) DF=$(DF) DH=$(DH) PLUGIN=resume make inst
	cd plugins && DFHACKVER=$(DFHACKVER) DF=$(DF) DH=$(DH) PLUGIN=automaterial make inst

clean:
	-rm $(OUT)