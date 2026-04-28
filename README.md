# F-Code TrainC Violation Management System

Hệ thống quản lý vi phạm thành viên của CLB F-Code, được phát triển bằng ngôn ngữ C và chạy trên môi trường terminal/CLI.

## Tổng quan

Dự án giải quyết bài toán theo dõi thành viên CLB F-Code vi phạm nội quy, ghi nhận mức phạt, theo dõi công nợ và hỗ trợ Ban Chủ Nhiệm xử lý minh bạch, nhất quán hơn.

Theo tài liệu yêu cầu, hệ thống hướng tới các chức năng chính:

- Đăng nhập, đăng xuất, đổi mật khẩu
- Quản lý thành viên: thêm, sửa, xóa, xem danh sách
- Ghi nhận lịch sử vi phạm và tính mức phạt theo vai trò
- Theo dõi trạng thái đã thu/chưa thu tiền phạt
- Cảnh báo ngưỡng Out CLB và xuất báo cáo ra file
- Lưu dữ liệu xuống file để tránh mất dữ liệu khi thoát chương trình

Hiện tại, repo đang ở giai đoạn scaffold ban đầu: đã có `Makefile`, cấu trúc thư mục cơ bản, công cụ format/tidy và `src/main.c` tối thiểu để xác nhận luồng build.

## Công nghệ và ràng buộc

- Ngôn ngữ: `C17`
- Compiler mục tiêu: `gcc`
- Loại ứng dụng: CLI / Terminal
- Cơ chế lưu trữ: file `.dat`
- Không sử dụng cơ sở dữ liệu, GUI hoặc thư viện ngoài C standard library


- Team Workflow: [`CONTRIBUTING.md`](CONTRIBUTING.md)
- Business Requirements: [`docs/requirement-docs/QuanLyViPhamCLBFCode_V1.md`](docs/requirement-docs/QuanLyViPhamCLBFCode_V1.md)
- Architecture and Module Design: [`docs/planning/architecture.md`](docs/planning/architecture.md)
- Story List: [`docs/stories/`](docs/stories/)


## Yêu cầu môi trường

Tài liệu này dùng luồng `cmd` trên Windows.

Nếu máy chưa cài gì, làm theo các bước sau trong `cmd`:

1. Cài `MSYS2` bằng Chocolatey:

```cmd
choco install msys2 -y
```

Theo cấu hình phổ biến của Chocolatey, MSYS2 sẽ nằm ở:

```text
C:\tools\msys64
```

2. Cập nhật hệ thống MSYS2 từ `cmd`:

```cmd
C:\tools\msys64\usr\bin\bash.exe -lc "pacman -Suy --noconfirm"
C:\tools\msys64\usr\bin\bash.exe -lc "pacman -Suy --noconfirm"
```

3. Cài toolchain và công cụ cần cho repo:

```cmd
C:\tools\msys64\usr\bin\bash.exe -lc "pacman -S --noconfirm mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-clang-tools-extra"
```

4. Thêm toolchain UCRT64 vào PATH của phiên `cmd` hiện tại:

```cmd
set "PATH=C:\tools\msys64\ucrt64\bin;%PATH%"
```

5. Kiểm tra nhanh:

```cmd
gcc --version
mingw32-make --version
clang-format --version
clang-tidy --version
```

Sau khi cài xong, môi trường sẽ có:

* `gcc`
* `mingw32-make`
* `clang-format`
* `clang-tidy`

Lưu ý quan trọng trên Windows:

* Với cấu hình hiện tại, lệnh build chuẩn là `mingw32-make`
* Khi dùng `cmd`, cần bảo đảm `C:\tools\msys64\ucrt64\bin` có trong PATH
* Không nên dùng `C:\tools\msys64\usr\bin\make.exe`, vì dễ trộn môi trường MSYS và UCRT64

## Cách build và chạy

Mở `cmd`, vào thư mục repo, rồi chạy:

```cmd
set "PATH=C:\tools\msys64\ucrt64\bin;%PATH%"
mingw32-make
```

Sau khi build thành công, file output sẽ nằm tại một trong các đường dẫn sau:

```text
bin/fcode-trainc
bin/fcode-trainc.exe
```

Chạy chương trình trên Windows:

```cmd
.\bin\fcode-trainc.exe
```

## Các lệnh hỗ trợ

Build project:

```cmd
set "PATH=C:\tools\msys64\ucrt64\bin;%PATH%"
mingw32-make
```

Xóa các file/thư mục sinh ra khi build:

```cmd
set "PATH=C:\tools\msys64\ucrt64\bin;%PATH%"
mingw32-make clean
```

Format source code:

```cmd
set "PATH=C:\tools\msys64\ucrt64\bin;%PATH%"
mingw32-make format
```

Phân tích tĩnh bằng `clang-tidy`:

```cmd
set "PATH=C:\tools\msys64\ucrt64\bin;%PATH%"
mingw32-make tidy
```

## Tài liệu liên quan

* Yêu cầu nghiệp vụ: `docs/requirement-docs/QuanLyViPhamCLBFCode_V1.md`
* Kiến trúc và định hướng module: `docs/planning/architecture.md`
* Danh sách story: `docs/stories/`
* Quy trình làm việc nhóm: `CONTRIBUTING.md`
* Các guideline bổ sung: `docs/guidelines/`

## Định hướng kiến trúc

Theo thiết kế hiện tại, dự án dự kiến tách thành các module sau:

* `main`: điều hướng menu và vòng lặp chính của chương trình
* `auth`: đăng nhập, đăng xuất, đổi/reset mật khẩu
* `member`: CRUD thành viên
* `violation`: ghi nhận vi phạm, tính mức phạt, cảnh báo Out CLB
* `fileio`: đọc/ghi `members.dat`, `violations.dat`, `accounts.dat`
* `report`: thống kê và xuất báo cáo
* `utils`: validate và các hàm dùng chung
