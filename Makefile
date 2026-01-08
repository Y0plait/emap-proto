#
# 'make'        build executable file 'main'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
CC = gcc

# define any compile-time flags
CFLAGS	:= -Wall -Wextra -g

# define source directory
SRC		:= src

# define include directory
INCLUDE	:= include

# define lib directory
LIB		:= lib

# define build directory
BUILD_DIR	:= build

# define output directory
OUTPUT	:= output

ifeq ($(OS),Windows_NT)
SOURCEDIRS	:= $(SRC)
INCLUDEDIRS	:= $(INCLUDE)
FIXPATH = $(subst /,\,$1)
RM			:= del /q /f
MD	:= mkdir
AR	:= ar
else
SOURCEDIRS	:= $(shell find $(SRC) -type d)
INCLUDEDIRS	:= $(shell find $(INCLUDE) -type d)
FIXPATH = $1
RM = rm -rf
MD	:= mkdir -p
AR	:= ar
endif

# Library name
LIBNAME		:= emap
STATIC_LIB	:= $(BUILD_DIR)/lib$(LIBNAME).a

# Test executable
TEST_MAIN	:= emap_tests
TEST_OUTPUT	:= $(OUTPUT)/$(TEST_MAIN)
TEST_SOURCES	:= tests/test_emap.c \
				   tests/test_encode_decode.c \
				   tests/test_error_handling.c \
				   tests/test_byte_order.c \
				   tests/test_packet.c \
				   tests/test_socket.c \
				   $(SRC)/emap_high_level.c \
				   $(SRC)/emap_low_level.c

# define any directories containing header files other than /usr/include
INCLUDES	:= $(patsubst %,-I%, $(INCLUDEDIRS:%/=%))

# define the C source files
SOURCES		:= $(wildcard $(patsubst %,%/*.c, $(SOURCEDIRS)))

# define the C object files 
OBJECTS		:= $(patsubst $(SRC)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

# define the dependency output files
DEPS		:= $(OBJECTS:.o=.d)

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

build: $(STATIC_LIB)
	@echo "Build complete: $(STATIC_LIB)"

build-dir:
	$(MD) $(BUILD_DIR)

$(OUTPUT):
	$(MD) $(OUTPUT)

$(STATIC_LIB): build-dir $(OBJECTS)
	$(AR) rcs $(STATIC_LIB) $(OBJECTS)
	@echo "Static library created: $(STATIC_LIB)"

# include all .d files
-include $(DEPS)

# this is a suffix replacement rule for building .o's and .d's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# -MMD generates dependency output files same name as the .o file
# (see the gnu make manual section about automatic variables)
$(BUILD_DIR)/%.o: $(SRC)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c -MMD $< -o $@

test: $(OUTPUT) $(TEST_OUTPUT)
	./$(TEST_OUTPUT)
	@echo "Test suite completed"

$(TEST_OUTPUT): $(TEST_SOURCES)
	@mkdir -p $(OUTPUT)
	$(CC) $(CFLAGS) $(INCLUDES) -I tests -o $(TEST_OUTPUT) $(TEST_SOURCES)

.PHONY: clean
clean:
	$(RM) $(call FIXPATH,$(BUILD_DIR))/*
	$(RM) $(call FIXPATH,$(OUTPUT))/*
	@echo "Cleanup complete!"

doc:
	doxygen Doxyfile
	@echo "Documentation generated in docs/html/index.html"

pdf: doc
	@echo "Building PDF from LaTeX..."
	cd docs/latex && make
	cp docs/latex/refman.pdf docs/EMAP_Documentation.pdf
	@echo "PDF generated: docs/EMAP_Documentation.pdf"

.PHONY: build build-dir clean doc pdf test
