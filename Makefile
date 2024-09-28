CC=clang

CFLAGS= -Wall -Wextra 
#-ggdb

XFCE_FLAGS= \
	`pkg-config --cflags --libs libxfce4ui-2` \
	`pkg-config --cflags --libs libxfce4panel-2.0` \

# $(info XFCE_FLAGS is $(XFCE_FLAGS))

INCLUDE_PATH=./include
SRC_PATH=./src
SOURCES=$(wildcard $(SRC_PATH)/*)

BUILD_PATH=./build
# LIBS=$(patsubst $(SRC_PATH)/%.c, $(BUILD_PATH)/%.so, $(SOURCES))

# $(info SOURCES is $(SOURCES))
# $(info LIBS is $(LIBS))

$(shell mkdir -p $(BUILD_PATH))

default: prayer-times-plugin.so

prayer-times-plugin.so: $(SOURCES)
	$(CC) -o $(BUILD_PATH)/$@ $^ -shared $(CFLAGS) $(XFCE_FLAGS) -I$(INCLUDE_PATH) -DRELEASE -lm

prayer-times-plugin: $(SOURCES)
	$(CC) -o $(BUILD_PATH)/$@ $^ $(CFLAGS) $(XFCE_FLAGS) -I$(INCLUDE_PATH) -lm -DRELEASE_CALCULATOR

calculate-times: $(SRC_PATH)/calculate-times.c
	$(CC) -o $(BUILD_PATH)/$@ $^ $(CFLAGS) -I$(INCLUDE_PATH) -lm

calculate-times.o: $(SRC_PATH)/calculate-times.c
	$(CC) -c -o $(BUILD_PATH)/$@ $^ $(CFLAGS) -I$(INCLUDE_PATH) -DRELEASE_CALCULATOR

.PHONY = clean
clean:
	rm -rf $(BUILD_PATH)
