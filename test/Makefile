CC=clang

CFLAGS= -Wall -Wextra -fsanitize=address
#-ggdb

$(info ---------------------)
$(info CFLAGS => $(CFLAGS))
$(info ---------------------)

XFCE_FLAGS= \
	`pkg-config --cflags --libs libxfce4ui-2` \
	`pkg-config --cflags --libs libxfce4panel-2.0` \

CUNIT_FLAGS= \
	`pkg-config cunit --libs --cflags`

$(info XFCE_FLAGS => $(XFCE_FLAGS))
$(info ---------------------)

INCLUDE_PATH=../include
BUILD_PATH=./build
SRC_PATH=../src

SOURCES=$(wildcard $(SRC_PATH)/*)

$(info SOURCES => $(SOURCES))
$(info HEADERS => $(wildcard $(INCLUDE_PATH)/*))
$(info ---------------------)

PO_PATH=./panel-po
POT_FILE=$(PO_PATH)/xpt.pot 
LANGS_PO=$(wildcard $(PO_PATH)/*/*.po)
LANGS_MO=$(patsubst $(PO_PATH)/%/xpt.po, $(PO_PATH)/%/LC_MESSAGES/xpt.mo, $(LANGS_PO))

default: all

all: tests 

tests: test_calculate_times test

test:test.c
	@mkdir -p $(BUILD_PATH)
	$(CC) -o $(BUILD_PATH)/$@ $^ $(CFLAGS) -I$(INCLUDE_PATH) $(CUNIT_FLAGS)

test_calculate_times: test_calculate_times.c
	@mkdir -p $(BUILD_PATH)
	$(CC) -o $(BUILD_PATH)/$@ $^ $(CFLAGS) -I$(INCLUDE_PATH) $(CUNIT_FLAGS)

edge: edge_cases.c ../src/calculate-times.c
	@mkdir -p $(BUILD_PATH)
	$(CC) -o $(BUILD_PATH)/$@ $^ $(CFLAGS) -I$(INCLUDE_PATH)

.PHONY = clean
clean:
	rm -rf $(BUILD_PATH)

