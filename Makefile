# Compiler
CC = gcc
CFLAGS = -Wall -Wextra -g

# Dossiers internes
SRC = src
INCLUDE = include
BUILD_DIR = build

# Sortie
LIB_NAME = libemap.a
LIB_OUTPUT = $(BUILD_DIR)/$(LIB_NAME)

# Fichiers sources et objets
SOURCES = $(wildcard $(SRC)/*.c)
OBJECTS = $(SOURCES:$(SRC)/%.c=$(BUILD_DIR)/%.o)

# ---------- Cible par défaut ----------
all: $(LIB_OUTPUT)

# ---------- Construction de la library ----------
$(LIB_OUTPUT): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	ar rcs $@ $^

# ---------- Compilation des fichiers .c ----------
$(BUILD_DIR)/%.o: $(SRC)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE) -c $< -o $@

# ---------- Tests ----------
test: $(LIB_OUTPUT)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE) tests/test_emap.c $(LIB_OUTPUT) -o $(BUILD_DIR)/test_emap
	@./$(BUILD_DIR)/test_emap

# ---------- Nettoyage ----------
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean test
