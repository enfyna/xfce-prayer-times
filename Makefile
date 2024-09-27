CC=clang

CFLAGS= -Wall -Wextra
#-ggdb

XFCE_FLAGS= \
	`pkg-config --cflags --libs libxfce4ui-2` \
	`pkg-config --cflags --libs libxfce4panel-2.0` \

$(info XFCE_FLAGS is $(XFCE_FLAGS))

SRC_PATH=./src
SOURCES=$(wildcard $(SRC_PATH)/*)

LIB_PATH=./lib
LIBS=$(patsubst $(SRC_PATH)/%.c, $(LIB_PATH)/%.so, $(SOURCES))

$(info SOURCES is $(SOURCES))
$(info LIBS is $(LIBS))

$(shell mkdir -p ./lib)

default: $(LIBS)

$(LIB_PATH)/%.so: $(SRC_PATH)/%.c
	$(CC) -o $@ $< -shared $(CFLAGS) $(XFCE_FLAGS)

.PHONY = clean
clean:
	rm -rf ./lib
