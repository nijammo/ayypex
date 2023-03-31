LIBRARY = lib/libayypex.so

CXXFLAGS=-Wall -std=c++17 -fPIC -shared -I./include/ -I./src/interface-internal/vk_hook/imgui
LDFLAGS=-ldl

SOURCES = $(shell find src -type f -name "*.c*")

.PHONY: all $(PROGRAM) clean install

all:
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $(SOURCES) -o $(PROGRAM)

install:
	sudo mkdir -p /usr/lib/ayypex
	sudo cp $(LIBRARY) /usr/lib/ayypex/libayypex.so

debug:
	sudo gdb -ex "attach $(shell pgrep R5A)"

clean:
	rm -f $(LIBRARY)