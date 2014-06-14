DF=/Users/vit/Desktop/Macnewbie/Dwarf\ Fortress

INC=-I/Users/vit/Downloads/dfhack/library/include -I/Users/vit/Downloads/dfhack/library/proto -I/Users/vit/Downloads/dfhack/depends/protobuf -I/Users/vit/Downloads/dfhack/depends/lua/include
LIB=-L$(DF)/hack -ldfhack

all: twbt.plug.so

inst: twbt.plug.so
	cp twbt.plug.so $(DF)/hack/plugins

twbt.plug.so: twbt.cpp
	g++ twbt.cpp -o twbt.plug.so -m32 -shared -std=gnu++11 -stdlib=libstdc++ $(INC) $(LIB) -DDFHACK_VERSION=\"0.34.11-r3\" -Wno-ignored-attributes -Wno-tautological-compare