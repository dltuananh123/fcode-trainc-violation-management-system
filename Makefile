# Use Windows cmd
SHELL := cmd
.SHELLFLAGS := /C

# Compiler settings
CC := gcc

# Compiler flags (flags = cờ biên dịch):
#   -std=c17             = dùng chuẩn C17
#   -m64                 = biên dịch 64-bit
#   -Wall                = cảnh báo phổ biến
#   -Wextra              = cảnh báo thêm
#   -Wpedantic           = nghiêm ngặt theo chuẩn
#   -Wshadow             = biến bị che khuất (shadowing)
#   -Wconversion         = mất dữ liệu khi ép kiểu
#   -Wformat=2           = kiểm tra printf/scanf chặt
#   -Wformat-security    = ngăn lỗi format string
#   -Wfloat-equal        = cảnh báo so sánh float bằng ==
#   -Wundef              = dùng macro chưa định nghĩa
#   -Wsign-conversion    = chuyển đổi có dấu / không dấu
#   -Wcast-align         = ép kiểu làm mất căn chỉnh
#   -Wcast-qual          = ép kiểu bỏ const/volatile
#   -Wmissing-prototypes = hàm chưa khai báo nguyên mẫu
#   -Wmissing-declarations = hàm chưa khai báo trước
#   -Wunreachable-code   = code không thể chạy tới
#   -Wnull-dereference   = truy cập con trỏ NULL
#   -Wimplicit-fallthrough = switch case bị rơi qua
#   -Wswitch-enum        = switch thiếu case enum
#   -Wpointer-arith      = tính toán trên void*
#   -Iinclude            = tìm header files trong include/
CFLAGS := -std=c17 -m64 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wformat-security -Wfloat-equal -Wundef -Wsign-conversion -Wcast-align -Wcast-qual -Wmissing-prototypes -Wmissing-declarations -Wunreachable-code -Wnull-dereference -Wimplicit-fallthrough -Wswitch-enum -Wpointer-arith -Iinclude

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
# gcc on Windows creates .exe, so we copy to remove the .exe extension
# This keeps the program name consistent across platforms
$(TARGET): $(TARGET_EXE)
	copy /Y $(subst /,\,$(TARGET_EXE)) $(subst /,\,$(TARGET)) >NUL

$(TARGET_EXE): $(OBJS) | $(BINDIR)
	$(CC) $(OBJS) -o $@

# Build each .o file from each .c file
# $< = input file (.c)
# $@ = output file (.o)
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
# sắp xếp / làm đẹp code tự động
format:
ifeq ($(strip $(FORMAT_FILES)),)
	@echo "No source or header files found to format."
else
	clang-format -i $(FORMAT_FILES)
endif

# Run clang-tidy on all .c files
# clang-tidy = soi lỗi / cảnh báo code nâng cao
tidy:
ifeq ($(strip $(SRCS)),)
	@echo "No source files found to analyze."
else
	clang-tidy --config-file=.clang-tidy $(SRCS) -- -std=c17 -Iinclude
endif
