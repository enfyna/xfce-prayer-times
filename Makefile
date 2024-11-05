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

PO_PATH=./panel-po
POT_FILE=$(PO_PATH)/xpt.pot 
LANGS_PO=$(wildcard $(PO_PATH)/*/*.po)
LANGS_MO=$(patsubst $(PO_PATH)/%/xpt.po, $(PO_PATH)/%/LC_MESSAGES/xpt.mo, $(LANGS_PO))

$(info POT_FILE => $(POT_FILE))
$(info LANGS_PO => $(LANGS_PO))
$(info LANGS_MO => $(LANGS_MO))
$(info ---------------------)

default: all

all: po-build libprayer-times-plugin.so

libprayer-times-plugin.so: $(SOURCES)
	@mkdir -p $(BUILD_PATH)
	$(CC) -o $(BUILD_PATH)/$@ $^ -shared -fPIC $(CFLAGS) $(XFCE_FLAGS) -I$(INCLUDE_PATH) -lm

# Build translations
po-build: $(LANGS_MO)
	$(info Translations built succesfully.)
	$(info ---------------------)

$(PO_PATH)/%/LC_MESSAGES/xpt.mo: $(PO_PATH)/%/xpt.po 
	@mkdir -p $(dir $@)
	msgfmt --output-file=$@ $<

# Run after adding a translatable string to source.
po-update:
	xgettext --keyword=_ --language=C --add-comments -o $(POT_FILE) $(SOURCES)
	@for po in $(LANGS_PO); do \
		echo "Updating $$po"; \
		msgmerge --update $$po $(POT_FILE); \
	done

# Rule to initialize a new .po file for a given language
# Usage: make po-init LANG=tr
po-init:
	@echo "Initializing PO file for language $$LANG ..."
	@mkdir -p $(PO_PATH)/$$LANG/
	@msginit --input=$(POT_FILE) --locale=$$LANG --output=$(PO_PATH)/$$LANG/xpt.po

.PHONY = clean
clean:
	rm -rf $(BUILD_PATH)
	rm -rf $(PO_PATH)/*/LC_MESSAGES/
