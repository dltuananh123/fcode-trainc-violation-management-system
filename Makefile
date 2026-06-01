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
BINDIR := bin
OBJDIR := obj

# Find all .c files in src/
SRCS := $(wildcard $(SRCDIR)/*.c)

# Find all .h files in include/
INCS := $(wildcard $(INCDIR)/*.h)

# Object files (src/foo.c -> obj/foo.o)
OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

# Object files excluding main.o (for linking seed_data)
LIB_OBJS := $(filter-out $(OBJDIR)/main.o,$(OBJS))


# These are commands, not real files
.PHONY: all clean format tidy seed

# Default command: build program
all: $(BINDIR)/violation-management-system.exe

# Link object files into final executable
$(BINDIR)/violation-management-system.exe: $(OBJS) | $(BINDIR)
	$(CC) $(CFLAGS) $(OBJS) -o $@

# Compile each .c to .o (incremental build)
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile and link seed tool against project object files
# This eliminates code duplication between seed_data.c and the main project
seed: $(BINDIR)/seed_data.exe

$(BINDIR)/seed_data.exe: tools/seed_data.c $(LIB_OBJS) | $(BINDIR)
	$(CC) $(CFLAGS) $< $(LIB_OBJS) -o $@

# Create output directories
$(BINDIR):
	@if not exist $(BINDIR) mkdir $(BINDIR)

$(OBJDIR):
	@if not exist $(OBJDIR) mkdir $(OBJDIR)

# Delete bin/ and obj/
clean:
	@if exist $(BINDIR) rmdir /S /Q $(BINDIR)
	@if exist $(OBJDIR) rmdir /S /Q $(OBJDIR)

# Run clang-format on all .c and .h files
format:
ifeq ($(strip $(SRCS) $(INCS)),)
	@echo "No source or header files found to format."
else
	clang-format -i $(SRCS) $(INCS)
endif

# Run clang-tidy on all .c files
tidy:
ifeq ($(strip $(SRCS)),)
	@echo "No source files found to analyze."
else
	clang-tidy --config-file=.clang-tidy $(SRCS) -- -std=c17 -Iinclude
endif
