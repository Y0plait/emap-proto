#
# 'make'        build executable file 'main'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
CC = gcc

# define any compile-time flags
CFLAGS	:= -Wall -Wextra -g

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS =

# define output directory
OUTPUT	:= output

# define source directory
SRC		:= src

# define include directory
INCLUDE	:= include

# define lib directory
LIB		:= lib

# define build directory
BUILD_DIR	:= build

# Submodules to compile

# --- EMAP library ---
EMAP_DIR = $(LIB)/emap-lib
EMAP_BUILD_DIR = $(EMAP_DIR)/build
EMAP_LIB = $(EMAP_BUILD_DIR)/libemap.a

# Build wrappers.c locally to ensure helper symbols are available even if
# the submodule archive omits them
EMAP_WRAPPERS_SRC := $(EMAP_DIR)/src/wrappers.c
EMAP_WRAPPERS_OBJ := $(BUILD_DIR)/emap_wrappers.o

# --- All built libs under lib/ ---
LIBS_TO_BUILD = $(EMAP_LIB)

ifeq ($(OS),Windows_NT)
MAIN	:= noometernal-server.exe
SOURCEDIRS	:= $(SRC)
INCLUDEDIRS	:= $(INCLUDE)
LIBDIRS		:= $(LIB)
FIXPATH = $(subst /,\,$1)
RM			:= del /q /f
MD	:= mkdir
else
MAIN	:= noometernal-server
SOURCEDIRS	:= $(shell find $(SRC) -type d)
INCLUDEDIRS	:= $(shell find $(INCLUDE) -type d)
LIBDIRS		:= $(shell find $(LIB) -type d)
FIXPATH = $1
RM = rm -f
MD	:= mkdir -p
endif

# define any directories containing header files other than /usr/include
INCLUDES	:= $(patsubst %,-I%, $(INCLUDEDIRS:%/=%)) -I$(EMAP_DIR)/include

# define the C lib search paths
LIBPATHS	:= $(patsubst %,-L%, $(LIBDIRS:%/=%)) -L$(EMAP_BUILD_DIR)

# define the C source files
SOURCES		:= $(wildcard $(patsubst %,%/*.c, $(SOURCEDIRS)))

# define the C object files 
OBJECTS		:= $(patsubst $(SRC)/%.c,$(BUILD_DIR)/%.o,$(SOURCES)) $(EMAP_WRAPPERS_OBJ)

# define the dependency output files
DEPS		:= $(OBJECTS:.o=.d)

# define target executable
TARGET		:= $(OUTPUT)/$(MAIN)

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

# ---------- Default target ----------
build: $(TARGET)

# ---------- Build all libs in lib/ ----------
build-lib: $(LIBS_TO_BUILD)

# EMAP library via its own Makefile
$(EMAP_LIB):
	$(MAKE) -C $(EMAP_DIR)

# ---------- Build project ----------
$(TARGET): build-lib $(OBJECTS)
	@mkdir -p $(OUTPUT)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJECTS) $(LIBPATHS) -lemap -lpthread -o $(TARGET)
	@echo "Build complete: $(TARGET)"

# ---------- Compile .c → .o in build/ ----------
$(BUILD_DIR)/%.o: $(SRC)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Explicit build of EMAP wrappers from submodule sources
$(EMAP_WRAPPERS_OBJ): $(EMAP_WRAPPERS_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(EMAP_DIR)/include -c $< -o $@

# ---------- CLEAN ----------
clean:
	rm -rf $(BUILD_DIR) $(OUTPUT)

clean-lib:
	$(MAKE) -C $(EMAP_DIR) clean || true

clean-all: clean clean-lib

# ---------- RUN ----------
run: build
	./$(TARGET)

# ---------- DOCUMENTATION ----------
doc:
	doxygen Doxyfile
	@echo "Documentation generated in docs/html/index.html"

.PHONY: build build-lib clean clean-lib clean-all run doc