# Use Windows cmd
SHELL := cmd
.SHELLFLAGS := /C

# Compiler settings
CC := gcc
CFLAGS := -std=c17 -m64 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wformat-nonliteral -Wformat-security -Wfloat-equal -Wundef -Wsign-conversion -Wcast-align -Wcast-qual -Wmissing-prototypes -Wmissing-declarations -Wunreachable-code -Wnull-dereference -Wimplicit-fallthrough -Wswitch-enum -Wpointer-arith -Wformat-nonliteral -Iinclude

# Folders
SRCDIR := src
INCDIR := include
BUILDDIR := build
BINDIR := bin

# Final program name
TARGET := $(BINDIR)/violation-management-system
TARGET_EXE := $(TARGET).exe

# Find all .c files in src/
SRCS := $(wildcard $(SRCDIR)/*.c)

# Find all .h files in include/
HDRS := $(wildcard $(INCDIR)/*.h)

# Change src/main.c -> build/main.o
OBJS := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

# Files used for formatting
FORMAT_FILES := $(strip $(SRCS) $(HDRS))

# These are commands, not real files
.PHONY: all clean format tidy

# Default command: build program
all: $(TARGET)

# Build final program from object files
$(TARGET): $(TARGET_EXE)
	copy /Y $(subst /,\,$(TARGET_EXE)) $(subst /,\,$(TARGET)) >NUL
$(TARGET_EXE): $(OBJS) | $(BINDIR)
	$(CC) $(OBJS) -o $@

# Build each .o file from each .c file
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILDDIR):
	@if not exist $(BUILDDIR) mkdir $(BUILDDIR)
$(BINDIR):
	@if not exist $(BINDIR) mkdir $(BINDIR)

# Delete build/ and bin/
clean:
	@if exist $(BUILDDIR) rmdir /S /Q $(BUILDDIR)
	@if exist $(BINDIR) rmdir /S /Q $(BINDIR)

# Format all .c and .h files
format:
ifeq ($(strip $(FORMAT_FILES)),)
	@echo "No source or header files found to format."
else
	clang-format -i $(FORMAT_FILES)
endif

# Run clang-tidy on all .c files
tidy:
ifeq ($(strip $(SRCS)),)
	@echo "No source files found to analyze."
else
	clang-tidy --config-file=.clang-tidy $(SRCS) -- -std=c17 -Iinclude
endif
