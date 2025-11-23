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
RM = rm -f
MD	:= mkdir -p
AR	:= ar
endif

# Library name
LIBNAME		:= emap
STATIC_LIB	:= $(BUILD_DIR)/lib$(LIBNAME).a

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

$(BUILD_DIR):
	$(MD) $(BUILD_DIR)

$(STATIC_LIB): $(BUILD_DIR) $(OBJECTS)
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

.PHONY: clean
clean:
	$(RM) $(call FIXPATH,$(STATIC_LIB))
	$(RM) $(call FIXPATH,$(OBJECTS))
	$(RM) $(call FIXPATH,$(DEPS))
	@echo "Cleanup complete!"

doc:
	doxygen Doxyfile
	@echo "Documentation generated in docs/html/index.html"

.PHONY: build clean doc
