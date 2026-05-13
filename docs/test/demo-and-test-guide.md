# Huong dan Demo & Test - He thong Quan ly Vi pham CLB F-Code

## 1. Chuan bi

### 1.1 Build project

```bash
mingw32-make
```

### 1.2 Nap demo data

```bash
bin\seed_data.exe
```

Ket qua:
- 14 thanh vien (4 ban + 2 BCN)
- 16 vi pham (10 da thu, 6 chua thu)
- 15 tai khoan
- Du lieu tu dong duoc copy vao `bin\data\`

### 1.3 Xoa data (reset ve trang thai ban dau)

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

# Thoat app, nap lai demo data
bin\seed_data.exe

# Chay app lai voi data moi
bin\violation-management-system.exe
```
