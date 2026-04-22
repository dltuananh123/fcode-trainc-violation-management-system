

# 1. Cấu trúc chung của Makefile

Makefile thường có 3 nhóm cú pháp chính:

1. **Khai báo biến**
2. **Khai báo target/rule**
3. **Lệnh chạy bên trong target**

Ví dụ file của bạn có đủ cả 3:

```make
CC := gcc
```

là **biến**

```make
all: $(TARGET)
```

là **rule**

```make
	$(CC) $(OBJS) -o $@
```

là **lệnh trong rule**

---

# 2. Syntax khai báo biến

```make
CC := gcc
```

## Ý nghĩa

* `CC` là tên biến
* `:=` là toán tử gán
* `gcc` là giá trị

## Cú pháp tổng quát

```make
TEN_BIEN := gia_tri
```

## Ví dụ trong file của bạn

```make
SHELL := cmd
CC := gcc
SRCDIR := src
BINDIR := bin
```
---

# 3. Cách dùng biến

Syntax:

```make
$(TEN_BIEN)
```

Ví dụ:

```make
$(CC)
$(TARGET)
$(SRCS)
```

## Trong file của bạn

```make
TARGET := $(BINDIR)/fcode-trainc
```

nghĩa là:

* lấy giá trị `BINDIR`
* ghép với `/fcode-trainc`

Nếu `BINDIR = bin` thì kết quả là:

```make
bin/fcode-trainc
```

---

# 5. Gọi hàm trong Makefile

Makefile có các hàm built-in (có sẵn trong hệ thống).
Syntax chung:

```make
$(ten_ham doi_so1,doi_so2,...)
```

## a) `wildcard`

```make
SRCS := $(wildcard $(SRCDIR)/*.c)
```

### Ý nghĩa

Lấy tất cả file `.c` trong thư mục `src`.

Nếu `src` có:

* `main.c`
* `utils.c`

thì `SRCS` thành:

```make
src/main.c src/utils.c
```

### Cú pháp chung

```make
$(wildcard mau_file)
```

---

## b) `patsubst`
cái này dùng tạo danh sách `.o` để build

```make
OBJS := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))
```

### Ý nghĩa

Đổi:

* `src/main.c` -> `build/main.o`
* `src/utils.c` -> `build/utils.o`

### Cú pháp chung

```make
$(patsubst mau_cu,mau_moi,danh_sach)
```

Trong đó `%` là phần được giữ lại.

---

## c) `strip`

```make
FORMAT_FILES := $(strip $(SRCS) $(HDRS))
```

### Ý nghĩa

Nối `SRCS` và `HDRS`, rồi xóa khoảng trắng thừa.

### Cú pháp chung

```make
$(strip chuoi_hoac_danh_sach)
```

---

# 6. Rule / Target syntax

Đây là phần xương sống của Makefile.

## Cú pháp chung

```make
target: dependencies
	commands
```

* `target`: thứ bạn muốn tạo / việc bạn muốn chạy
* `dependencies`: target cần gì trước
* `commands`: lệnh shell để thực hiện

---

## Ví dụ 1

```make
all: $(TARGET)
```

### Ý nghĩa

Target `all` phụ thuộc vào `$(TARGET)`.

Khi bạn gõ trên repo này:

```bash
mingw32-make
```

thì Make mặc định chạy target đầu tiên, ở đây là `all`.

---

## Ví dụ 2

```make
$(TARGET): $(OBJS)
	@if not exist $(BINDIR) mkdir $(BINDIR)
	$(CC) $(OBJS) -o $@
```

### Ý nghĩa

Để tạo `$(TARGET)`, cần có `$(OBJS)` trước.

Sau đó chạy 2 lệnh:

1. tạo thư mục `bin` nếu chưa có
2. link các file object thành executable

---

# 7. Pattern rule syntax

```make
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@if not exist $(BUILDDIR) mkdir $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@
```

## `%` là gì?

`%` là ký tự đại diện.

Rule này nghĩa là:

* muốn có `build/main.o` thì cần `src/main.c`
* muốn có `build/auth.o` thì cần `src/auth.c`

## Cú pháp chung

```make
mau_target: mau_dependency
	commands
```

Ví dụ:

```make
build/%.o: src/%.c
```

---

# 8. Automatic variables

Trong recipe, bạn thấy:

```make
$@
$<
```

Đây là biến tự động.

## a) `$@`

Tên target hiện tại.

Ví dụ trong rule này:

```make
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@
```

thì `$@` = `bin/fcode-trainc`

---

## b) `$<`

Dependency đầu tiên.

Ví dụ trong rule compile:

```make
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
```

nếu đang build `build/main.o`, thì:

* `$<` = `src/main.c`
* `$@` = `build/main.o`

---

## c) Thêm vài cái hay gặp

Dù file bạn chưa dùng, bạn nên biết luôn:

* `$^` = toàn bộ dependencies
* `$?` = dependencies mới hơn target

Ví dụ hay gặp:

```make
$(CC) $^ -o $@
```

---

# 9. `.PHONY`

```make
.PHONY: all clean format tidy
```

## Cú pháp

```make
.PHONY: ten_target1 ten_target2 ...
```

## Ý nghĩa

Báo cho Make biết đây là target “ảo”, không phải file thật.

Ví dụ `clean` không tạo ra file tên `clean`, nó chỉ là một lệnh đặc biệt.

Nếu không khai báo `.PHONY`, mà lỡ có file tên `clean`, Make có thể hiểu nhầm.

---

# 10. Commands trong recipe

Các dòng lệnh dưới target phải bắt đầu bằng **tab**.

Ví dụ:

```make
clean:
	@if exist $(BUILDDIR) rmdir /S /Q $(BUILDDIR)
```

## Cú pháp cực quan trọng

Phải là:

* dòng target bình thường
* dòng lệnh bên dưới bắt đầu bằng **tab**

Không phải nhiều space thường.

---

# 11. Ký hiệu `@` trước lệnh

Bạn có:

```make
@if exist $(BINDIR) mkdir $(BINDIR)
```

## Ý nghĩa

`@` làm cho Make **không in nguyên dòng lệnh đó ra màn hình**.

Nếu không có `@`, màn hình sẽ hiện cả lệnh trước khi chạy.

---

# 12. Condition syntax trong Makefile

Bạn có:

```make
format:
ifndef FORMAT_FILES
	@echo "No source or header files found to format."
else
	clang-format -i $(FORMAT_FILES)
endif
```

và:

```make
tidy:
ifndef SRCS
	@echo "No source files found to analyze."
else
	clang-tidy --config-file=.clang-tidy $(SRCS) -- -std=c17 -Iinclude
endif
```

## Đây là cú pháp điều kiện của Make

### Cú pháp chung

```make
ifdef TEN_BIEN
  ...
else
  ...
endif
```

hoặc

```make
ifndef TEN_BIEN
  ...
else
  ...
endif
```

## Ý nghĩa

* `ifdef` = nếu biến có định nghĩa
* `ifndef` = nếu biến chưa định nghĩa

### Lưu ý rất quan trọng

`ifndef SRCS` **không có nghĩa chắc chắn là “không có file source”**.
Nó chỉ có nghĩa là biến `SRCS` có được định nghĩa hay không.

Mà trong file bạn đã viết:

```make
SRCS := $(wildcard $(SRCDIR)/*.c)
```

nên `SRCS` gần như luôn được định nghĩa, kể cả khi rỗng.

## Cách kiểm tra rỗng đúng hơn

Nên dùng:

```make
ifeq ($(strip $(SRCS)),)
	@echo "No source files found to analyze."
else
	clang-tidy --config-file=.clang-tidy $(SRCS) -- -std=c17 -Iinclude
endif
```

và với `FORMAT_FILES` cũng tương tự.

---

# 13. `ifeq` / `ifneq`

Đây là syntax điều kiện bạn nên biết thêm.

## Cú pháp

```make
ifeq (gia_tri_1,gia_tri_2)
  ...
else
  ...
endif
```

hoặc:

```make
ifneq (gia_tri_1,gia_tri_2)
  ...
else
  ...
endif
```

## Ví dụ kiểm tra rỗng

```make
ifeq ($(strip $(FORMAT_FILES)),)
	@echo "No files"
else
	clang-format -i $(FORMAT_FILES)
endif
```

Cái này thực tế dùng nhiều hơn `ifndef` khi kiểm tra danh sách file.

---

# 14. Syntax của shell command bên trong recipe

Vì bạn đang dùng:

```make
SHELL := cmd
.SHELLFLAGS := /C
```

nên các lệnh trong recipe dùng cú pháp của **Windows cmd**.

Ví dụ:

## Tạo thư mục nếu chưa có

```make
@if not exist $(BINDIR) mkdir $(BINDIR)
```

## Xóa thư mục nếu có

```make
@if exist $(BUILDDIR) rmdir /S /Q $(BUILDDIR)
```

## In ra màn hình

```make
@echo "No source files found to analyze."
```

Đây không phải syntax của Make, mà là syntax của `cmd`.

Make chỉ đóng vai trò gọi shell để chạy.

---

# 15. Target không sinh file vs target sinh file

## a) Target sinh file

Ví dụ:

```make
$(TARGET): $(OBJS)
```

Target này sinh ra file thật `bin/fcode-trainc`.

## b) Target lệnh

Ví dụ:

```make
clean:
format:
tidy:
```

Các target này không tạo file cùng tên, mà chỉ chạy lệnh.
Vì thế mới cần `.PHONY`.

---

# 16. Tách file của bạn thành nhóm syntax

## Nhóm 1: biến

```make
SHELL := cmd
CC := gcc
CFLAGS := -std=c17 -Wall -Wextra -Iinclude
SRCDIR := src
TARGET := $(BINDIR)/fcode-trainc
```

## Nhóm 2: hàm

```make
$(wildcard $(SRCDIR)/*.c)
$(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))
$(strip $(SRCS) $(HDRS))
```

## Nhóm 3: special target

```make
.PHONY: all clean format tidy
```

## Nhóm 4: target/rule

```make
all: $(TARGET)
$(TARGET): $(OBJS)
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
clean:
format:
tidy:
```

## Nhóm 5: lệnh shell

```make
@if not exist $(BINDIR) mkdir $(BINDIR)
$(CC) $(OBJS) -o $@
clang-format -i $(FORMAT_FILES)
clang-tidy --config-file=.clang-tidy $(SRCS) -- -std=c17 -Iinclude
```

## Nhóm 6: điều kiện

```make
ifndef FORMAT_FILES
else
endif
```

---

# 18. Phiên bản “khung syntax” để bạn tự viết Makefile sau này

Bạn có thể nhớ khung này:

```make
# 1) variables
CC := gcc
CFLAGS := -std=c17 -Wall -Wextra
SRCDIR := src
BINDIR := bin
TARGET := $(BINDIR)/app

SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c,build/%.o,$(SRCS))

# 2) phony targets
.PHONY: all clean

# 3) default target
all: $(TARGET)

# 4) link rule
$(TARGET): $(OBJS)
	command

# 5) compile rule
build/%.o: src/%.c
	command

# 6) utility targets
clean:
	command
```

---

# 19. Riêng target `tidy` của bạn đang viết syntax gì?

```make
tidy:
ifndef SRCS
	@echo "No source files found to analyze."
else
	clang-tidy --config-file=.clang-tidy $(SRCS) -- -std=c17 -Iinclude
endif
```

Nó dùng các syntax sau:

* `tidy:` = target
* `ifndef SRCS` = điều kiện make
* `@echo ...` = lệnh shell có ẩn echo
* `clang-tidy ...` = lệnh shell thực thi static analysis

Nhưng như mình nói, nếu muốn kiểm tra “không có source file”, syntax tốt hơn là:

```make
tidy:
ifeq ($(strip $(SRCS)),)
	@echo "No source files found to analyze."
else
	clang-tidy --config-file=.clang-tidy $(SRCS) -- -std=c17 -Iinclude
endif
```

---

# 20. Chốt lại thật dễ nhớ

Makefile này dùng 6 loại syntax chính:

* **biến**: `CC := gcc`
* **dùng biến**: `$(CC)`
* **hàm**: `$(wildcard ...)`, `$(patsubst ...)`
* **rule**: `target: deps`
* **pattern**: `%`
* **automatic variables**: `$@`, `$<`
* **điều kiện**: `ifdef`, `ifndef`, `ifeq`, `ifneq`
* **phony**: `.PHONY`
* **lệnh shell**: `mkdir`, `rmdir`, `clang-format`, `clang-tidy`
