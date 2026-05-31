# Hướng dẫn Demo & Test — Hệ thống Quản lý Vi phạm CLB F-Code

## 1. Chuẩn bị

### 1.1 Build + Seed + Run (1 lệnh duy nhất)

```cmd
run.bat
```

### 1.2 Hoặc từng bước thủ công

```cmd
mingw32-make
bin\seed_data.exe
bin\violation-management-system.exe
```

### 1.3 Kết quả seed data

- **72 thành viên** (70 sinh viên thật Challenge 3 + 2 BCN legacy)
- **76 vi phạm** (17 gốc + 59 bổ sung) trên khắp 4 ban
- **72 tài khoản** (mật khẩu = MSSV, riêng SE203055: `Phuc@2006`)
- **3 thành viên đã bị kick** (SE210946, SE210117, SE203367)
- **1 vi phạm OUT CLB** (SE200516 — đánh nhau)

### 1.4 Xóa data (reset)

```cmd
bin\seed_data.exe clear
```

---

## 2. Tài khoản Demo

### BCN (Admin)

| MSSV | Mật khẩu | Vai trò |
|------|----------|---------|
| **SE203055** | `Phuc@2006` | Super Admin (BCN) |
| BCN001 | `BCN001` | Legacy BCN |
| BCN002 | `BCN002` | Legacy BCN |

### Thành viên

Tất cả tài khoản có mật khẩu mặc định = **MSSV** (ví dụ: `SE201018` / `SE201018`).

Một số thành viên tiêu biểu:

| MSSV | Họ tên | Ban | Chức vụ | Ghi chú |
|------|--------|-----|---------|---------|
| SE201018 | Lam Hoang An | Học thuật | Leader | 1 VP (đã thu) |
| SE210946 | Nguyen Vu Hao | Học thuật | Member | **Đã bị kick**, 2 VP |
| SE200516 | Tran Vu Hai Duy | Học thuật | Leader | 2 VP, **1 OUT CLB** (đánh nhau) |
| SE212026 | Bui Phuoc Trong | Kế hoạch | Leader | 2 VP (1 chưa thu) |
| SE210117 | Nguyen Hung Hien | Kế hoạch | Member | **Đã bị kick** |
| SE203367 | Trinh Thi Minh Tam | Truyền thông | Member | **Đã bị kick**, 4 VP |
| SE200481 | Vang Khanh Khuyen | Truyền thông | Leader | 1 VP (chưa thu) |

---

## 3. Chạy ứng dụng

```cmd
bin\violation-management-system.exe
```

---

## 4. Kịch bản Demo — Menu BCN

**Đăng nhập:** MSSV `SE203055`, mật khẩu `Phuc@2006`

### 4.1 Xem danh sách thành viên (Option 13)

**Mục đích:** Hiển thị danh sách thành viên đang hoạt động, có phân trang.

**Thực hiện:**
1. Chọn `13`
2. Bảng hiện 5 cột: MSSV, Họ và tên, Email, SDT, Ban
3. Mỗi trang 15 dòng, nhấn `n` trang tiếp, `m` trang trước, `q` thoát

**Kết quả mong đợi:** 69 thành viên đang hoạt động (72 - 3 bị kick).

---

### 4.2 Thêm thành viên mới (Option 1)

**Mục đích:** Thêm thành viên thứ 73.

**Thực hiện:**
1. Chọn `1`
2. Nhập MSSV: `SE209999` (nhập `0` để hủy)
3. Nhập họ tên: `Nguyen Test Demo`
4. Nhập email: `test@fcode.vn`
5. Nhập SDT: `0987654321`
6. Chọn ban: `2` (Nhân sự)
7. Chọn chức vụ: `0` (Thành viên)

**Kết quả mong đợi:** `[OK] Thêm thành viên thành công`

---

### 4.3 Sửa thông tin thành viên (Option 2)

**Mục đích:** Sửa tên — kiểm tra validation re-prompt.

**Thực hiện:**
1. Chọn `2`
2. Nhập MSSV: `SE209999` (`0` để hủy)
3. Chọn trường sửa: `1` (Họ tên)
4. Nhập họ tên mới: `NguyenTest` (Enter để giữ nguyên)
5. Chọn `0` để kết thúc

Các trường đều có vòng lặp re-prompt: nếu nhập sai định dạng (vd email thiếu `@`, SDT có chữ) sẽ yêu cầu nhập lại, Enter để giữ nguyên.

**Kết quả mong đợi:** `[OK] Cập nhật thành công`

---

### 4.4 Ghi nhận vi phạm (Option 4)

**Mục đích:** Tạo vi phạm mới với re-prompt MSSV.

**Thực hiện:**
1. Chọn `4`
2. Nhập MSSV: `SE209999` (`0` để hủy)
3. Xem thông tin thành viên
4. Chọn lý do: `0` (Không mặc áo CLB)
5. Nhập ghi chú: `Test demo`

**Kết quả mong đợi:** Phạt 20000 VND, ghi nhận thành công.

---

### 4.5 Xem danh sách vi phạm có lọc (Option 6)

**Mục đích:** Xem tất cả vi phạm với phân trang.

**Test 1 — Xem tất cả:**
1. Chọn `6` → `4` (Xem tất cả)
2. Bảng 8 cột với phân trang (15 dòng/trang)
3. Nhấn `n`/`m`/`q` để điều hướng

**Kết quả mong đợi:** Tổng 76+ vi phạm, nhiều trang.

**Test 2 — Lọc theo ban:**
1. Chọn `6` → `1` (Lọc theo ban)
2. Chọn ban: `1` (Học thuật)
3. Xem danh sách vi phạm ban Học thuật

**Kết quả mong đợi:** ~25 vi phạm (nhiều hơn 1 trang).

**Test 3 — Lọc chưa thu:**
1. Chọn `6` → `3` (Lọc trạng thái)
2. Chọn `1` (Chưa thu)
3. Xem danh sách vi phạm chưa đóng tiền

---

### 4.6 Thu tiền phạt (Option 5)

**Mục đích:** Thu tiền phạt từ thành viên.

**Thực hiện:**
1. Chọn `5`
2. Nhập MSSV: `SE203367` (`0` để hủy) — thành viên đã bị kick (có thể không thu được)
   Hoặc: `SE212026` (đang có 1 VP chưa thu)
3. Xem danh sách vi phạm chưa đóng
4. Nhập `99` để thu tất cả, hoặc chọn STT

**Kết quả mong đợi:** `[OK] Đã thu tiền thành công!`

---

### 4.7 Thống kê tiền phạt theo ban (Option 7)

**Mục đích:** Xem báo cáo tổng hợp 4 ban.

**Thực hiện:**
1. Chọn `7`

**Kết quả mong đợi:** Bảng thống kê với biểu đồ thanh tiến trình.

---

### 4.8 Kiểm tra ngưỡng Out CLB (Option 8)

**Mục đích:** Xem thành viên gần hoặc đã quá ngưỡng Out CLB.

**Thực hiện:**
1. Chọn `8`

**Kết quả mong đợi:** SE200516 hiện "Out CLB" (đánh nhau).

---

### 4.9 Sắp xếp thành viên theo số lần vi phạm (Option 9)

**Thực hiện:**
1. Chọn `9`
2. Chọn `2` (Giảm dần)

**Kết quả mong đợi:** Các thành viên vi phạm nhiều nhất đứng đầu.

---

### 4.10 Xuất báo cáo ra file .txt (Option 10)

**Thực hiện:**
1. Chọn `10`
2. Kiểm tra file .txt trong thư mục `report/`

---

### 4.11 Tìm kiếm vi phạm theo ngày (Option 11)

**Test 1 — Toàn bộ 2026 (có phân trang):**
1. Chọn `11`
2. Ngày bắt đầu: `01/01/2026`
3. Ngày kết thúc: `31/12/2026`
4. Nhấn `n`/`m`/`q` để điều hướng

**Kết quả mong đợi:** 76 vi phạm, nhiều trang.

**Test 2 — Sai định dạng:**
1. Chọn `11`
2. Ngày bắt đầu: `2026-01-01`

**Kết quả mong đợi:** `[LOI] Định dạng ngày không hợp lệ`

---

### 4.12 Xem profile cá nhân (Option 12)

**Thực hiện:** Chọn `12`

**Kết quả mong đợi:** Thông tin của SE203055 (Nguyen Ngoc Phuc).

---

### 4.13 Đổi mật khẩu (Option 14)

**Lưu ý:** Mật khẩu mới phải >= 8 ký tự, có chữ hoa, chữ thường, số và ký tự đặc biệt.

**Thực hiện:**
1. Chọn `14`
2. Nhập mật khẩu cũ: `Phuc@2006`
3. Nhập mật khẩu mới: `NewPass@123`
4. Xác nhận: `NewPass@123`

**Kết quả mong đợi:** `[OK] Đổi mật khẩu thành công`

---

### 4.14 Reset mật khẩu thành viên (Option 15)

**Thực hiện:**
1. Chọn `15`
2. Nhập MSSV: `SE201018` (nhập `0` để hủy)
3. Xác nhận

**Kết quả mong đợi:** Mật khẩu reset về `SE201018`.

---

### 4.15 Kick thành viên (Option 17)

**Mục đích:** Kick thành viên vi phạm ra khỏi CLB.

**Thực hiện:**
1. Chọn `17`
2. Nhập MSSV: `SE209999` (thành viên vừa thêm, `0` để hủy)
3. Nhập lý do: `Test kick`
4. Xác nhận: `1` (Có)
5. Gõ lại MSSV để xác nhận: `SE209999`

**Kết quả mong đợi:** `[OK] Da kick thanh vien`

### 4.16 Xem danh sách đã kick (Option 19)

**Mục đích:** Xem các thành viên đã bị kick kèm lý do.

**Thực hiện:**
1. Chọn `19`

**Kết quả mong đợi:** Danh sách 4 thành viên đã kick kèm lý do.

---

### 4.17 Xem danh sách vi phạm (Option 6 — phân trang)

Tất cả các bảng danh sách vi phạm (xem tất cả, lọc, tìm kiếm theo ngày) đều có phân trang:
- **15 dòng / trang**
- `n`: trang tiếp | `m`: trang trước | `q`: thoát

---

### 4.18 Xem nhật ký hệ thống (Option 20)

**Thực hiện:**
1. Chọn `20`
2. Xem audit log với tô màu
3. `Enter` xem tiếp, `q` thoát

---

## 5. Kịch bản Demo — Menu Thành viên

**Đăng nhập:** MSSV `SE201018`, mật khẩu `SE201018`

### 5.1 Xem profile (Option 1)

**Thực hiện:** Chọn `1`

**Kết quả mong đợi:** Lam Hoang An, SE201018, Ban Học thuật, Leader.

### 5.2 Xem lịch sử vi phạm (Option 2)

**Thực hiện:** Chọn `2`

**Kết quả mong đợi:** 1 vi phạm (10/03/2026, không mặc áo CLB, đã thu).

### 5.3 Xem tổng tiền phạt còn nợ (Option 3)

**Thực hiện:** Chọn `3`

**Kết quả mong đợi:** 0 VND (đã thu hết).

### 5.4 Xem danh sách thành viên (Option 4)

**Thực hiện:** Chọn `4`

**Kết quả mong đợi:** Bảng 69 thành viên đang hoạt động, phân trang.

---

## 6. Kịch bản — Đăng nhập thất bại

### 6.1 Sai mật khẩu 3 lần

1. MSSV: `SE203055`, Mật khẩu: `sai`
2. Lặp lại 3 lần

**Kết quả mong đợi:** `[CANH BAO] Tài khoản đã bị khóa sau 3 lần đăng nhập sai`

### 6.2 Tài khoản không tồn tại

**Thực hiện:** MSSV: `XXXXX`, Mật khẩu: `batky`

**Kết quả mong đợi:** `[LOI] Tài khoản không tồn tại`

---

## 7. Xóa và nạp lại data

```cmd
bin\seed_data.exe clear
bin\seed_data.exe
bin\violation-management-system.exe
```

---

## 8. Lưu ý quan trọng

### 8.1 `run.bat` — Build + Seed + Run

Script tự động:
1. Xóa `bin\` cũ
2. Build toàn bộ project
3. Build `seed_data.exe`
4. Chạy seed data
5. Chạy app

### 8.2 Các validation mới

| Field | Validation |
|-------|-----------|
| Email | Phải có `@`, không `,` trong domain, TLD >= 2 ký tự, không `..` |
| Phone | Chỉ số, đúng 10 số, đầu 0, unique trong hệ thống |
| Password | >= 8 ký tự, có chữ hoa, thường, số, ký tự đặc biệt |
| MSSV | Tất cả prompt có `0 để quay lại` |
| Edit fields | Re-prompt `while(1)` khi nhập sai, Enter giữ nguyên |

### 8.3 Giao diện

- Terminal width: **100 ký tự**
- Danh sách thành viên: 5 cột (MSSV, Họ tên, Email, SDT, Ban)
- Tất cả bảng vi phạm: 8 cột, phân trang 15 dòng
- Phân trang: `n` tiếp, `m` trước, `q` thoát

### 1.2 Build seed data tool

```bash
gcc -std=c17 -m64 -Wall -Iinclude tools/seed_data.c -o bin/seed_data.exe
```

### 1.3 Nap demo data

```bash
# Chay tu thu muc goc project
bin\seed_data.exe

# HOAC chay tu ben trong bin\
cd bin
seed_data.exe
```

Ket qua:
- 14 thanh vien (4 ban + 2 BCN)
- 16 vi pham (10 da thu, 6 chua thu)
- 15 tai khoan
- Du lieu tu dong duoc copy vao `bin\data\`

### 1.4 Xoa data (reset ve trang thai ban dau)

```bash
bin\seed_data.exe clear
```

---

## 2. Tai khoan Demo

### Tai khoan Admin (BCN)

| MSSV | Mat khau | Vai tro |
|------|----------|---------|
| ADMIN | ADMIN | Ban Chu Nhiem |

### Tai khoan Thanh vien

Tat ca mat khau mac dinh: `123456`

| MSSV | Ho ten | Ban | Chuc vu | So VP | No (VND) | Ghi chu |
|------|--------|-----|---------|-------|-----------|---------|
| SV0001 | Nguyen Van An | Hoc thuat | Truong nhom | 1 | 0 | Da thu het |
| SV0002 | Tran Thi Bich | Hoc thuat | Thanh vien | 3 | 20000 | 1 chua thu |
| SV0003 | Le Hoang Cuong | Hoc thuat | Thanh vien | 0 | 0 | Sach |
| SV0004 | Pham Minh Duc | Ke hoach | Truong nhom | 2 | 50000 | 1 chua thu (leader: 50k/vp) |
| SV0005 | Vo Thi Mai | Ke hoach | Thanh vien | 1 | 20000 | Chua thu |
| SV0006 | Bui Quoc Phong | Ke hoach | Thanh vien | 1 | 0 | Da thu |
| SV0007 | Do Thanh Giang | Nhan su | Truong nhom | 0 | 0 | Sach |
| SV0008 | Ngo Thi Hanh | Nhan su | Thanh vien | 2 | 20000 | 1 chua thu, 3 vang LT |
| SV0009 | Ly Minh Kien | Nhan su | Thanh vien | 1 | 0 | Da thu |
| SV0010 | Ha Thanh Long | Truyen thong | Truong nhom | 1 | 0 | Da thu (leader: 50k) |
| SV0011 | Dang Thi Ngoc | Truyen thong | Thanh vien | 4 | 40000 | 2 chua thu, **4 vang LT** |
| SV0012 | Cao Van Phu | Truyen thong | Thanh vien | 0 | 0 | Sach |
| BCN001 | Tran Quoc Bao | Hoc thuat | BCN | 0 | 0 | BCN |
| BCN002 | Pham Thi Cuc | Ke hoach | BCN | 0 | 0 | BCN |

### Danh sach vi pham trong data

| # | MSSV | Ngay | Ly do | Tien phat | Trang thai |
|---|------|------|-------|-----------|------------|
| 1 | SV0001 | 10/03/2026 | Khong mac ao CLB | 20000 | Da thu |
| 2 | SV0002 | 05/02/2026 | Vang mat | 20000 | Da thu |
| 3 | SV0002 | 12/03/2026 | Khong mac ao CLB | 20000 | Da thu |
| 4 | SV0002 | 20/04/2026 | Vang mat | 20000 | **Chua thu** |
| 5 | SV0004 | 15/01/2026 | Khong tham gia HD | 50000 | Da thu |
| 6 | SV0004 | 01/04/2026 | Vang mat | 50000 | **Chua thu** |
| 7 | SV0005 | 22/03/2026 | Khong mac ao CLB | 20000 | **Chua thu** |
| 8 | SV0006 | 28/02/2026 | Vang mat | 20000 | Da thu |
| 9 | SV0008 | 05/03/2026 | Vang mat | 20000 | Da thu |
| 10 | SV0008 | 10/04/2026 | Khong mac ao CLB | 20000 | **Chua thu** |
| 11 | SV0009 | 20/01/2026 | Vang mat | 20000 | Da thu |
| 12 | SV0010 | 14/02/2026 | Khong tham gia HD | 50000 | Da thu |
| 13 | SV0011 | 08/01/2026 | Vang mat | 20000 | Da thu |
| 14 | SV0011 | 12/02/2026 | Vang mat | 20000 | Da thu |
| 15 | SV0011 | 18/03/2026 | Vang mat | 20000 | **Chua thu** |
| 16 | SV0011 | 25/04/2026 | Vang mat | 20000 | **Chua thu** |

---

## 3. Chay ung dung

```bash
bin\violation-management-system.exe
```

Hoac tu thu muc goc:

```bash
bin\violation-management-system
```

---

## 4. Kich ban Demo - Menu BCN

**Dang nhap:** MSSV `ADMIN`, mat khau `ADMIN`

### 4.1 Xem danh sach thanh vien (Option 13)

**Muc dich:** Hien thi toan bo 14 thanh vien

**Thuc hien:**
1. Chon `13`
2. Xem bang danh sach voi MSSV, ho ten, ban, chuc vu, trang thai

**Ket qua mong doi:** 14 dong du lieu, tat ca trang thai "Hoat dong"

---

### 4.2 Them thanh vien moi (Option 1)

**Muc dich:** Them thanh vien thu 15

**Thuc hien:**
1. Chon `1`
2. Nhap MSSV: `SV0099`
3. Nhap ho ten: `Test Thanh Vien`
4. Nhap email: `test@fcode.vn`
5. Nhap SDT: `0987654321`
6. Chon ban: `2` (Nhan su)
7. Chon chuc vu: `0` (Thanh vien)

**Ket qua mong doi:** `[OK] Them thanh vien thanh cong`

---

### 4.3 Sua thong tin thanh vien (Option 2)

**Muc dich:** Doi ten thanh vien

**Thuc hien:**
1. Chon `2`
2. Nhap MSSV can sua: `SV0099`
3. Chon thong tin can sua: `1` (Ho ten)
4. Nhap ho ten moi: `Test Da Sua`
5. Chon `0` de ket thuc sua

**Ket qua mong doi:** `[OK] Cap nhat thanh cong`

---

### 4.4 Ghi nhan vi pham (Option 4)

**Muc dich:** Tao vi pham moi cho thanh vien

**Test 1 - Vi pham thuong (Khong mac ao):**
1. Chon `4`
2. Nhap MSSV: `SV0099`
3. Xem thong tin thanh vien hien ra
4. Chon ly do: `0` (Khong mac ao CLB)
5. Nhap ghi chu: `Test vi pham` (hoac Enter de bo qua)

**Ket qua mong doi:** Phat 20000 VND, ghi nhan thanh cong

**Test 2 - Vi pham vang mat (kiem tra dem vang lien tiep):**
1. Chon `4`
2. Nhap MSSV: `SV0005` (dang co 1 vang LT)
3. Chon ly do: `1` (Vang hop)
4. Ghi chu: Enter

**Ket qua mong doi:** Thong bao "So buoi vang lien tiep: 2"

---

### 4.5 Xem danh sach vi pham co loc (Option 6)

**Test 1 - Xem tat ca:**
1. Chon `6` -> `1`
2. Xem toan bo 16+ vi pham trong bang

**Test 2 - Loc theo ban:**
1. Chon `6` -> `2`
2. Chon ban: `0` (Hoc thuat)
3. Xem chi tiet vi pham cua ban Hoc thuat

**Test 3 - Loc theo trang thai:**
1. Chon `6` -> `4`
2. Chon: `1` (Chua thu)
3. Xem danh sach 6 vi pham chua dong tien

---

### 4.6 Thu tien phat (Option 5)

**Muc dich:** Danh dau da thu tien tu thanh vien

**Thuc hien:**
1. Chon `5`
2. Nhap MSSV: `SV0002`
3. Xem danh sach vi pham chua dong: 1 khoan (20/04/2026, 20000 VND)
4. Chon STT: `1`
5. Xac nhan

**Ket qua mong doi:** `[OK] Da thu tien thanh cong! Tong no con lai: 0 VND`

---

### 4.7 Thong ke tien phat theo ban (Option 7)

**Muc dich:** Xem bao cao tong hop 4 ban

**Thuc hien:**
1. Chon `7`

**Ket qua mong doi:** Bang thong ke voi 4 dong:

| Ban | Da thu (VND) | Con no (VND) | Tong (VND) |
|-----|-------------|-------------|-----------|
| Hoc thuat | ~60000 | ~20000 | ~80000 |
| Ke hoach | ~70000 | ~70000 | ~140000 |
| Nhan su | ~40000 | ~20000 | ~60000 |
| Truyen thong | ~90000 | ~40000 | ~130000 |

*(So tuy thuoc vao buoc 4.6 co thu tien hay chua)*

---

### 4.8 Kiem tra nguong Out CLB (Option 8)

**Muc dich:** Xem thanh vien gan hoac da qua nguong Out CLB

**Thuc hien:**
1. Chon `8`

**Ket qua mong doi:** SV0011 (4 vang LT) hien voi trang thai "QUA NGUONG", SV0008 (3 vang LT) hien "CANH BAO"

---

### 4.9 Sap xep thanh vien theo so lan vi pham (Option 9)

**Muc dich:** Sap xep va xem ranking

**Test 1 - Giam dan:**
1. Chon `9`
2. Chon `2` (Giam dan)

**Ket qua mong doi:** SV0011 (4 vp) dung dau, cac thanh vien 0 vp dung cuoi

**Test 2 - Tang dan:**
1. Chon `9`
2. Chon `1` (Tang dan)

**Ket qua mong doi:** SV0003/SV0007/SV0012/BCN001/BCN002 (0 vp) dung dau

---

### 4.10 Xuat bao cao ra file .txt (Option 10)

**Muc dich:** Export bao cao ra file

**Thuc hien:**
1. Chon `10`

**Ket qua mong doi:**
- Thong bao: `[OK] Da xuat bao cao ra file: <duong_dan>\violation_report_YYYYMMDD_HHMMSS.txt`
- File chua: header, thoi gian xuat, tong hop theo ban, danh sach thanh vien con no
- Mo file .txt bang Notepad de kiem tra noi dung

---

### 4.11 Tim kiem vi pham theo ngay (Option 11)

**Test 1 - Toan bo nam 2026:**
1. Chon `11`
2. Ngay bat dau: `01/01/2026`
3. Ngay ket thuc: `31/12/2026`

**Ket qua mong doi:** Tong 16 vi pham

**Test 2 - Chi thang 3-4/2026:**
1. Chon `11`
2. Ngay bat dau: `01/03/2026`
3. Ngay ket thuc: `30/04/2026`

**Ket qua mong doi:** ~8 vi pham (ngay 10/03, 12/03, 18/03, 22/03, 05/03, 01/04, 10/04, 20/04, 25/04)

**Test 3 - Sai dinh dang ngay:**
1. Chon `11`
2. Ngay bat dau: `2026-01-01`

**Ket qua mong doi:** `[LOI] Dinh dang ngay khong hop le (dd/mm/yyyy)`

**Test 4 - Ngay bat dau > ngay ket thuc:**
1. Chon `11`
2. Ngay bat dau: `31/12/2026`
3. Ngay ket thuc: `01/01/2026`

**Ket qua mong doi:** `[LOI] Ngay bat dau phai truoc hoac bang ngay ket thuc`

---

### 4.12 Xem profile ca nhan (Option 12)

**Thuc hien:**
1. Chon `12`
2. Xem thong tin ca nhan cua tai khoan ADMIN (BCN001)

---

### 4.13 Doi mat khau (Option 14)

**Thuc hien:**
1. Chon `14`
2. Nhap mat khau cu: `ADMIN`
3. Nhap mat khau moi: `NEWPASS`
4. Xac nhan mat khau moi: `NEWPASS`

**Ket qua mong doi:** `[OK] Doi mat khau thanh cong`

*Luu y: Sau khi doi, lan dang nhap sau dung mat khau moi.*

---

### 4.14 Reset mat khau thanh vien (Option 15)

**Muc dich:** BCN reset mat khau cho thanh vien

**Thuc hien:**
1. Chon `15`
2. Nhap MSSV: `SV0002`
3. Xac nhan

**Ket qua mong doi:** Mat khau cua SV0002 duoc reset ve mat khau mac dinh

---

### 4.15 Xoa thanh vien (Option 3)

**Muc dich:** Xoa thanh vien khoi he thong

**Thuc hien:**
1. Chon `3`
2. Nhap MSSV: `SV0099` (thanh vien vua them o buoc 4.2)
3. Xac nhan xoa

**Ket qua mong doi:** `[OK] Da xoa thanh vien`

---

---

### 4.16 Xem nhat ky he thong (Option 20)

**Muc dich:** Xem lich su cac thao tac da duoc thuc hien trong he thong

**Thuc hien:**
1. Chon `20`
2. Xem bang nhat ky voi cac cot:
   - Thoi gian (mau xam)
   - Nguoi thuc hien (mau xanh la)
   - Hanh dong (mau vang)
   - Muc tieu (mau xanh duong)
3. Nhan `Enter` de xem trang tiep theo
4. Nhap `q` + Enter de thoat giua chang

**Ket qua mong doi:** Hien thi danh sach cac hanh dong da thuc hien tu luc bat dau ung dung, phan trang 20 dong/lan, to mau theo tung thanh phan.

---

## 5. Kich ban Demo - Menu Thanh vien

**Dang nhap:** MSSV `SV0002`, mat khau `123456`

### 5.1 Xem profile ca nhan (Option 1)

**Thuc hien:** Chon `1`

**Ket qua mong doi:** Hien thi thong tin: Tran Thi Bich, SV0002, Ban Hoc thuat, 3 vi pham

---

### 5.2 Xem lich su vi pham (Option 2)

**Thuc hien:** Chon `2`

**Ket qua mong doi:** Bang 3 vi pham cua SV0002 (05/02, 12/03, 20/04), vi pham cuoi chua thu

---

### 5.3 Xem tong tien phat con no (Option 3)

**Thuc hien:** Chon `3`

**Ket qua mong doi:** Hien thi cac khoan chua dong + tong: 20000 VND (neu chua thu o buoc 4.6) hoac 0 VND (neu da thu)

---

### 5.4 Xem danh sach thanh vien (Option 4)

**Thuc hien:** Chon `4`

**Ket qua mong doi:** Bang 14 thanh vien

---

## 6. Kich ban Demo - Dang nhap that bai

### 6.1 Sai mat khau

**Thuc hien:**
1. MSSV: `ADMIN`, Mat khau: `sai`
2. Lap lai 3 lan

**Ket qua mong doi:** Sau 3 lan sai -> `[CANH BAO] Tai khoan da bi khoa sau 3 lan dang nhap sai`

### 6.2 Tai khoan khong ton tai

**Thuc hien:** MSSV: `XXXXX`, Mat khau: `batky`

**Ket qua mong doi:** `[LOI] Tai khoan khong ton tai`

---

## 7. Kich ban Demo - Xoa va nap lai data

```bash
# Xoa toan bo data (reset ve trang thai trong)
bin\seed_data.exe clear

# Chay app -> Tu tao tai khoan ADMIN/ADMIN
bin\violation-management-system.exe

# Thoat app, nap lai demo data (chay tu bat ky thu muc nao)
bin\seed_data.exe

# Chay app lai voi data moi
bin\violation-management-system.exe
```

---

## 8. Luu y quan trong

### 8.1 Build seed_data rieng

Seed data tool **khong nam trong Makefile** vi co `main()` rieng. Build thu cong:

```bash
gcc -std=c17 -m64 -Wall -Iinclude tools/seed_data.c -o bin/seed_data.exe
```

### 8.2 Chay seed_data tu bat ky dau

`seed_data.exe` tu dong tao thu muc `data\` va `bin\data\` neu chua ton tai.
Ban co the chay no tu thu muc goc project hoac tu ben trong `bin\`.

### 8.3 Chay app

App luon doc data tu `bin\data\` (nam cung thu muc voi file .exe).

```bash
# Tu thu muc goc
bin\violation-management-system.exe

# Hoac vao bin\
cd bin
violation-management-system.exe
```
