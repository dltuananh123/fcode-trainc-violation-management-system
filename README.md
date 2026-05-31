# F-Code TrainC Violation Management System

Hệ thống quản lý vi phạm thành viên của CLB F-Code, được phát triển bằng ngôn ngữ C thuần (C17) và chạy trên môi trường terminal/CLI.

## Tổng quan

Dự án giải quyết bài toán theo dõi thành viên CLB F-Code vi phạm nội quy, ghi nhận mức phạt, theo dõi công nợ và hỗ trợ Ban Chủ Nhiệm (BCN) xử lý minh bạch.

**Chức năng chính:**
- Đăng nhập, đăng xuất, đổi/reset mật khẩu (Salted FNV-1a key stretching)
- Quản lý thành viên: thêm, sửa, tìm kiếm & xem chi tiết, kick/khôi phục, xem danh sách (phân trang)
- Ghi nhận vi phạm và tính mức phạt theo vai trò (member: 20k, leader: 50k)
- Thu tiền phạt (đơn lẻ hoặc bulk), theo dõi công nợ
- Cảnh báo và tự động Out CLB khi vắng >= 4 buổi liên tiếp
- Bảng điều khiển kỷ luật (Discipline Dashboard) với biểu đồ thanh
- Xem/thống kê vi phạm theo ban, lý do, trạng thái (phân trang)
- Tìm kiếm vi phạm theo khoảng ngày (phân trang)
- Sắp xếp thành viên theo số lần vi phạm
- Xuất báo cáo ra file .txt
- Nhật ký hệ thống (audit log) với tô màu và phân trang
- Lưu trữ dữ liệu mã hóa XOR + Magic Header `FCE1`
- Seed data với 72 thành viên thật từ Challenge 3, 76 vi phạm

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

## Cách build và chạy (Nhanh)

Sử dụng script `run.bat` để build + seed data + chạy chỉ 1 lần:

```cmd
run.bat
```

Hoặc từng bước:

```cmd
set "PATH=C:\tools\msys64\ucrt64\bin;%PATH%"
mingw32-make
bin\seed_data.exe
bin\violation-management-system.exe
```

## Các lệnh hỗ trợ

| Lệnh | Mô tả |
|------|-------|
| `mingw32-make` | Build project |
| `mingw32-make clean` | Xóa file build |
| `mingw32-make format` | Format code (clang-format) |
| `mingw32-make tidy` | Phân tích tĩnh (clang-tidy) |
| `bin\seed_data.exe` | Ghi seed data (72 members, 76 violations) |
| `bin\seed_data.exe clear` | Xóa sạch data |
| `run.bat` | Build + seed + chạy tự động |

## Tài khoản Demo

### BCN (Admin)

| MSSV | Mật khẩu | Ghi chú |
|------|----------|---------|
| SE203055 | `Phuc@2006` | Super Admin (BCN), không thể bị kick |
| BCN001 | MSSV (`BCN001`) | Legacy BCN (tự động tạo) |
| BCN002 | MSSV (`BCN002`) | Legacy BCN (tự động tạo) |

### Thành viên

Tất cả tài khoản thành viên có mật khẩu mặc định là **MSSV của chính họ** (vd: SE201018 có mật khẩu `SE201018`). Seed data gồm **70 thành viên thật** từ lớp Challenge 3, chia đều 4 ban:
- **Học thuật** (Academic): 20 thành viên
- **Kế hoạch** (Planning): 15 thành viên
- **Nhân sự** (HR): 15 thành viên
- **Truyền thông** (Media): 18 thành viên
- **BCN**: 2 legacy (BCN001, BCN002)

Trong đó 3 thành viên đã bị kick (SE210946, SE210117, SE203367).

## Tài liệu liên quan

* Yêu cầu nghiệp vụ: [`docs/requirement-docs/QuanLyViPhamCLBFCode_V1.md`](docs/requirement-docs/QuanLyViPhamCLBFCode_V1.md)
* Kiến trúc chi tiết: [`docs/planning/architecture.md`](docs/planning/architecture.md)
* Technical updates (v2.0-v2.3): [`docs/planning/technical_updates.md`](docs/planning/technical_updates.md)
* Danh sách story: [`docs/stories/`](docs/stories/)
* Hướng dẫn test: [`docs/test/demo-and-test-guide.md`](docs/test/demo-and-test-guide.md)
* Quy trình làm việc nhóm: [`CONTRIBUTING.md`](CONTRIBUTING.md)
* Code review: [`docs/code-review/`](docs/code-review/)

## Cấu trúc module

| Module | Vai trò |
|--------|---------|
| `main` | Điều hướng menu chính |
| `auth` | Đăng nhập, đổi/reset mật khẩu, session |
| `member` | Thêm, sửa, tìm kiếm chi tiết, kick/restore, danh sách (phân trang) |
| `violation` | Ghi nhận vi phạm, thu tiền, Out CLB, xem DS (phân trang) |
| `fileio` | Đọc/ghi file `.dat` mã hóa XOR |
| `report` | Thống kê ban, sort, discipline dashboard, xuất báo cáo |
| `utils` | Validate (email, phone, password), audit log, date parser |
| `ui` | Vẽ khung menu, breadcrumb, separator |
