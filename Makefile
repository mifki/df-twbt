DFHACKVER ?= 0.34.11
DFHACKREL ?= r3

DF ?= /Users/vit/Desktop/Macnewbie/Dwarf Fortress
DH ?= /Users/vit/Downloads/dfhack

OUT = twbt.plug.so
SRC = twbt.cpp

INC = -I"$(DH)/library/include" -I"$(DH)/library/proto" -I"$(DH)/depends/protobuf" -I"$(DH)/depends/lua/include"
LIB = -L"$(DF)/hack" -ldfhack

CFLAGS = $(INC) -m32 -std=gnu++11 -stdlib=libstdc++ -Wno-ignored-attributes -Wno-tautological-compare 
LDFLAGS = $(LIB) -shared -mmacosx-version-min=10.6


all: $(OUT)

inst: $(OUT)
	cp $(OUT) "$(DF)/hack/plugins/"

$(OUT): $(SRC)
	g++ $(SRC) -o $(OUT) -DDFHACK_VERSION=\"$(DFHACKVER)-$(DFHACKREL)\" $(CFLAGS) $(LDFLAGS)

clean:
	-rm $(OUT)


dist: clean $(SRC)
	-mkdir dist/dfhack-r3 dist/dfhack-r4 dist/dfhack-r5
	DFHACKREL=r3 $(MAKE) -e
	mv $(OUT) dist/dfhack-r3/
	DFHACKREL=r4 $(MAKE) -e
	mv $(OUT) dist/dfhack-r4/
	DFHACKREL=r5 $(MAKE) -e
	mv $(OUT) dist/dfhack-r5/
