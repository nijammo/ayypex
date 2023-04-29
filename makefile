LIBRARY = lib/libayypex.so

CXXFLAGS=-Wall -std=c++17 -fPIC -shared -I./include/ -I./src/interface-internal/vk_hook/imgui -masm=intel
LDFLAGS=-ldl

SOURCES = $(shell find src -type f -name "*.c*")

.PHONY: all clean install

all:
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $(SOURCES) -o $(LIBRARY)

install:
	sudo mkdir -p /usr/lib/ayypex
	sudo cp $(LIBRARY) /usr/lib/ayypex/libayypex.so

debug:
	sudo gdb -ex "attach $(shell pgrep R5A)"

clean:
	rm -f $(LIBRARY)

clear:
	rm /data/SteamLibrary/steamapps/compatdata/1172470/pfx/drive_c/users/steamuser/Saved\ Games/Respawn/Apex/assets/temp/netparams