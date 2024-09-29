CC=clang

CFLAGS= -Wall -Wextra 
#-ggdb

$(info ---------------------)
$(info CFLAGS => $(CFLAGS))
$(info ---------------------)

XFCE_FLAGS= \
	`pkg-config --cflags --libs libxfce4ui-2` \
	`pkg-config --cflags --libs libxfce4panel-2.0` \

$(info XFCE_FLAGS => $(XFCE_FLAGS))
$(info ---------------------)

INCLUDE_PATH=./include
BUILD_PATH=./build
SRC_PATH=./src

SOURCES=$(wildcard $(SRC_PATH)/*)

$(info SOURCES => $(SOURCES))
$(info HEADERS => $(wildcard $(INCLUDE_PATH)/*))
$(info ---------------------)

$(shell test -d $(BUILD_PATH) || mkdir -p $(BUILD_PATH))

default: libprayer-times-plugin.so

libprayer-times-plugin.so: $(SOURCES)
	$(CC) -o $(BUILD_PATH)/$@ $^ -shared -fPIC $(CFLAGS) $(XFCE_FLAGS) -I$(INCLUDE_PATH) -lm

.PHONY = clean
clean:
	rm -rf $(BUILD_PATH)
