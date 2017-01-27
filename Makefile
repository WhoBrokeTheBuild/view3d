.PHONY: all clean install format valgrind gdb test check

FILE ?= assets/cube.obj
PREFIX ?= /usr/local
BUILD ?= debug

# Directory constants

BUILD_DIR = build
TESTS_DIR = tests
SRC_DIR = src
OBJ_DIR = obj
DEP_DIR = .dep

# Additional build configuration

CFLAGS  += -Wall -std=c11 -I$(SRC_DIR)
LDFLAGS +=
LDLIBS  += -lm -lz -lGL -lGLEW -lglut -lpng -lmdl

ifdef GLMM_DIR
CFLAGS  += -I$(GLMM_DIR)/include
endif

ifdef LIBMDL_DIR
CFLAGS  += -I$(LIBMDL_DIR)/include
LDFLAGS += -L$(LIBMDL_DIR)
endif

ifeq ($(BUILD),debug)
CFLAGS += -O0 -g -DDEBUG
else # release
CFLAGS += -O2 -s -DRELEASE
endif

# Dynamically get the sources/objects/tests

SOURCES = $(wildcard $(SRC_DIR)/**/*.c $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))
TESTS   = $(patsubst %.c,%,$(wildcard $(TESTS_DIR)/*_test.c))
TARGET  = $(BUILD_DIR)/v3d

# Create directories to prevent "cannot create file" errors

$(shell mkdir -p $(BUILD_DIR) $(TESTS_DIR) $(SRC_DIR) $(OBJ_DIR) $(DEP_DIR))
$(shell mkdir -p $(patsubst $(SRC_DIR)%,$(DEP_DIR)%,$(dir $(SOURCES))))
$(shell mkdir -p $(patsubst $(SRC_DIR)%,$(OBJ_DIR)%,$(dir $(SOURCES))))

# Build the main target

all: $(TARGET)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c $(DEP_DIR)/%.d
	$(CC) $(CFLAGS) -MM -MP -MT $@ -o $(DEP_DIR)/$*.d $<
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Clean dependencies, objects, or the main target

clean:
	rm -rf $(DEP_DIR) $(OBJ_DIR) $(TARGET)

clean_deps:
	rm -rf $(DEP_DIR)

clean_objects:
	rm -f $(OBJECTS)

clean_target:
	rm -f $(TARGET)

# Install to system

install: all
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/share/view3d/bin
	cp -rf share/ $(PREFIX)/share/view3d/
	cp -f $(TARGET) $(PREFIX)/share/view3d/bin/
	cp -rf shaders/ $(PREFIX)/share/view3d/
	cp -f v3d.sh $(PREFIX)/share/view3d/
	@-unlink $(PREFIX)/bin/v3d
	ln -s $(PREFIX)/share/view3d/v3d.sh $(PREFIX)/bin/v3d
	@-unlink $(PREFIX)/share/applications/view3d.desktop
	ln -s $(PREFIX)/share/view3d/view3d.desktop $(PREFIX)/share/applications/view3d.desktop

# Format code

format:
	clang-format -i $(SOURCES) $(HEADERS)

# Run the program normally

run: export LD_LIBRARY_PATH=/usr/local/lib64
run:
	$(TARGET) $(FILE)

# Run the program through valgrind

valgrind: export LD_LIBRARY_PATH=/usr/local/lib64
valgrind:
	valgrind $(TARGET) $(FILE)

# Run the program through gdb

gdb: export LD_LIBRARY_PATH=/usr/local/lib64
gdb:
	gdb --args $(TARGET) $(FILE)

# Builds test executables

$(TESTS): $(OBJECTS)
	$(CC) $(CFLAGS) -c -o $@.o $@.c
	$(CC) $(LDFLAGS) -o $@ $@.o $^ $(LDLIBS)
	@rm $@.o

# Build and execute tests
# Adds CFLAGS,LDFLAGS,LDLIBS for libcheck
# Cleans objects before and after

test: CFLAGS  += $(shell pkg-config check --cflags) -DTEST_BUILD
test: LDFLAGS += $(shell pkg-config check --libs-only-L)
test: LDLIBS  += $(shell pkg-config check --libs-only-l)
test: clean_objects $(TESTS)
	$(addsuffix ;,$(TESTS))
	rm -f $(TESTS)
	rm -f $(OBJECTS)

# Checks for potentially unsafe functions

BADFUNCS = '[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)'
check:
	@echo Files with potentially dangerous functions.
	@egrep $(BADFUNCS) $(SOURCES) || true

# Dependency file hacks
# Requires all dependency files, but doesn't error out if they
# aren't there

$(DEP_DIR)/%.d: ;
.PRECIOUS: $(DEP_DIR)/%.d
-include $(patsubst $(SRC_DIR)/%.c,$(DEP_DIR)/%.d,$(SOURCES))
