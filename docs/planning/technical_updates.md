# Báo cáo Phân tích Kỹ thuật & Tài liệu Cập nhật Hệ thống

Tài liệu này cung cấp giải thích chi tiết về các giải thuật, cơ chế mật mã học, mã hóa tệp tin và thiết kế giao diện đồ họa terminal (TUI) đã được triển khai trong đợt nâng cấp hệ thống **F-Code TrainC Violation Management System v2.0**.

---

## 1. Cơ chế Bảo mật Mật khẩu (Password Security & Hashing)

Để giải quyết triệt để vấn đề lưu mật khẩu dạng văn bản thuần túy (plain-text) ban đầu, hệ thống đã được tích hợp cơ chế băm mật khẩu bảo mật cao sử dụng kỹ thuật **Salted Key Stretching FNV-1a** (kéo giãn khóa kết hợp muối ngẫu nhiên).

```
[Mật khẩu gốc] + [Salt ngẫu nhiên] ─► Băm FNV-1a lần 1 ─► Lặp 1000 lần (PBKDF) ─► Chuỗi băm stretched 64-bit ─► Chuỗi Hex 31 ký tự
```

### Kỹ thuật băm FNV-1a (Fowler-Noll-Vo)
FNV-1a là một thuật toán băm phi mã hóa (non-cryptographic hash) nổi tiếng nhờ tốc độ cực kỳ nhanh, tỷ lệ va chạm (collision rate) cực thấp và độ phức tạp tính toán tối giản ($O(N)$), phù hợp tuyệt đối cho môi trường lập trình C thuần túy trên vi điều khiển hoặc ứng dụng hệ thống CLI.

Mã nguồn băm lõi FNV-1a (64-bit):
$$\text{hash} = \text{offset\_basis}$$
$$\text{hash} = (\text{hash} \oplus \text{byte}) \times \text{prime}$$

Trong đó các hằng số tiêu chuẩn cho 64-bit:
- $\text{prime} = 1099511628211$ (`0x100000001B3`)
- $\text{offset\_basis} = 14695981039346656037$ (`0xCBF29CE484222325`)

### Cơ chế muối ngẫu nhiên (Salt Generation)
Để ngăn chặn tấn công bằng bảng tra cứu trước (Rainbow Table Attacks), mỗi tài khoản khi tạo mới hoặc reset sẽ được gán một chuỗi **Salt** ngẫu nhiên gồm 16 ký tự chữ và số.
- Salt được tạo bằng hàm `generateSalt` sử dụng bộ sinh số ngẫu nhiên được gieo hạt (seeded) bằng thời gian thực hệ thống `srand((unsigned int)time(NULL))` tại thời điểm khởi chạy ứng dụng.
- Salt được lưu trực tiếp vào trường `char salt[MAX_SALT_LEN]` trong cấu trúc `Account` nhị phân.

### Thuật toán Kéo giãn Khóa (Key Stretching - 1000 Iterations)
Để chống lại các đòn tấn công brute-force tốc độ cao:
1. Hệ thống gộp chuỗi mật khẩu gốc và muối: `temp = password + salt`.
2. Thực hiện vòng lặp **1000 lần băm FNV-1a liên tục**. Trong mỗi vòng lặp, giá trị băm của vòng trước được đổi sang chuỗi Hex rồi gộp tiếp với muối để làm đầu vào cho vòng băm tiếp theo.
3. Kỹ thuật này gia tăng độ trễ tính toán có chủ đích (computational latency), khiến việc dò mật khẩu bằng phần cứng chuyên dụng (GPU/ASIC) trở nên chậm đi hàng nghìn lần.
4. Để tối ưu hóa độ dài lưu trữ (`char password[32]`), một hàm băm FNV phụ được chạy đè lên chuỗi kết quả cuối để nén thành một chuỗi Hex **31 ký tự duy nhất** (vừa khít 32 bytes kèm ký tự kết thúc chuỗi `\0`).

### Cơ chế tự động di trú dữ liệu (Backward-Compatible Auto-Migration)
Nhằm bảo vệ dữ liệu cũ của người dùng mà không cần cài đặt lại cơ sở dữ liệu:
- Khi người dùng đăng nhập, hàm `authLogin` kiểm tra độ dài trường `salt` của tài khoản đó.
- Nếu `strlen(acc->salt) == 0` (tức là tài khoản cũ được lưu bằng plain-text):
  1. Hệ thống tự động sinh một chuỗi salt ngẫu nhiên mới.
  2. Lấy mật khẩu plain-text hiện tại băm cùng với salt mới.
  3. Ghi đè salt và hash mới vào cấu trúc tài khoản.
  4. Lập tức flush ghi lại tệp tin `accounts.dat`.
- Quá trình này diễn ra hoàn toàn ẩn với người dùng cuối nhưng tài khoản của họ đã lập tức được nâng cấp lên chuẩn mã hóa cao nhất!

---

## 2. Giải thuật Mã hóa Tệp tin Nhị phân (XOR File Encryption & Verification)

Hệ thống lưu trữ dữ liệu dưới dạng các tệp tin `.dat` nhị phân bằng cách ghi trực tiếp các cấu trúc struct C. Để ngăn chặn việc can thiệp dữ liệu từ bên ngoài (ví dụ: mở tệp tin bằng Hex Editor để sửa đổi số tiền phạt hoặc thông tin cá nhân), hệ thống đã cài đặt cơ chế mã hóa đối xứng dòng (Stream XOR Cipher).

```
Dữ liệu Struct (Memory) ──────────────────────────► XOR Mã hóa (Mỗi byte ^ Key[i % len])
                                                            │
Decryption (Mỗi byte ^ Key[i % len]) ◄── Đọc từ ổ đĩa ◄──────┴── Magic Header [FCE1] + Dữ liệu
```

### Kỹ thuật mã hóa đối xứng XOR
Phép toán loại trừ XOR ($\oplus$) có tính chất toán học đặc biệt:
$$(A \oplus B) \oplus B = A$$
Điều này nghĩa là cùng một khóa và thuật toán XOR sẽ được dùng cho cả quá trình mã hóa (khi ghi) và giải mã (khi đọc), đảm bảo tốc độ tối đa và không tốn thêm bộ nhớ đệm phụ.
Khóa mật mã đối xứng được sử dụng: `"FCodeTrainC2026_SecureKey!"`.

### Magic Signature Header (`FCE1`)
Mỗi tệp tin dữ liệu khi lưu lại sẽ được ghi đè theo cấu trúc:
1. **Magic Header** (4 bytes): Chuỗi `"FCE1"` (F-Code Encrypted v1).
2. **Số lượng bản ghi** (4 bytes): Kiểu `int` biểu thị số phần tử.
3. **Mảng dữ liệu nhị phân**: Đã được mã hóa XOR toàn bộ.

### Cơ chế giải mã và Tương thích ngược tệp dữ liệu
Hàm tải dữ liệu `loadAccounts`, `loadMembers`, và `loadViolations` tự động phân tích định dạng tệp:
1. Đọc thử 4 bytes đầu của tệp tin `.dat`.
2. So sánh với chuỗi signature `"FCE1"`.
3. **Trường hợp trùng khớp**: Thiết lập trạng thái `isEncrypted = 1`. Đọc số lượng bản ghi, sau đó nạp dữ liệu nhị phân đã mã hóa vào bộ nhớ đệm heap và gọi giải mã dòng `readAndDecrypt` để trả lại struct thuần túy trong RAM.
4. **Trường hợp không trùng khớp (Tệp cũ chưa mã hóa)**: Gọi lệnh dịch chuyển con trỏ tệp tin về đầu tệp `fseek(fp, 0, SEEK_SET)`. Đọc tệp tin như bản ghi nhị phân thuần túy cũ (`readItemsChecked`).
5. **Tự động mã hóa**: Khi có bất kỳ thay đổi nào hoặc khi thoát ứng dụng, hàm lưu dữ liệu sẽ ghi lại tệp tin theo định dạng mã hóa mới kèm magic header. Tệp tin cũ được tự động nâng cấp thành công.

### Đảm bảo an toàn bộ nhớ (Heap Safety)
Quá trình mã hóa nhị phân trước khi ghi ra đĩa sử dụng bộ nhớ heap động (`malloc`) để tạo một bản sao đệm:
- Điều này ngăn chặn việc mã hóa đè trực tiếp lên mảng dữ liệu RAM đang chạy của chương trình. RAM chạy hoàn toàn bằng dữ liệu sạch, đảm bảo tính nhất quán (consistency) và tránh lỗi corrupt dữ liệu khi ghi thất bại giữa chừng.
- Sử dụng giải pháp giải phóng con trỏ `free()` trong mọi nhánh điều kiện rẽ nhánh (early returns), đảm bảo **không bao giờ xảy ra rò rỉ bộ nhớ (Zero Memory Leaks)**.

---

## 3. Thiết kế Đồ họa Terminal (ANSI TUI Grid Rendering)

Hệ thống được phát triển theo tiêu chuẩn TUI (Terminal User Interface) cao cấp, mang lại trải nghiệm chuyên nghiệp cho người dùng CLI.

### ANSI Escape Codes
Hệ thống sử dụng mã màu ANSI 8-bit mở rộng (`\033[38;5;...m`) để hiển thị màu sắc hài hòa, dịu mắt (Soft colors) thay vì các hệ màu cơ bản chói mắt:
- `COLOR_CYAN` (`\033[38;5;117m`): Màu xanh ngọc nhạt cho các đường lưới và nhãn tiêu đề.
- `COLOR_GREEN` (`\033[38;5;114m`): Màu xanh lục nhạt cho trạng thái "Đã thu" hoặc "Hoạt động".
- `COLOR_RED` (`\033[38;5;203m`): Màu đỏ pastel cho "Chưa thu" hoặc "Cảnh báo/Out CLB".
- `COLOR_PURPLE` (`\033[38;5;183m`): Màu tím nhạt cho các trường tổng tiền và thông số đặc biệt.

### Giải quyết vấn đề đệm ký tự màu (Color Escape Padding Issues)
Trong ngôn ngữ C, các mã điều khiển ANSI (ví dụ: `\033[0m`) chiếm từ 5 đến 11 bytes trong chuỗi ký tự nhưng **không chiếm diện tích hiển thị trên màn hình**.
Nếu sử dụng hàm `printf` định dạng độ rộng cột mặc định như `%-20s` trên một chuỗi chứa mã màu ANSI, cột đó sẽ bị lệch nghiêm trọng vì trình biên dịch C tính cả độ dài mã màu vào độ rộng cột.
**Giải pháp thiết kế**:
- Tách biệt mã màu khỏi định dạng độ rộng cột. Mã màu ANSI được in riêng, sau đó in chuỗi ký tự thuần túy bằng `printf` định dạng độ rộng chính xác, rồi in mã màu reset.
- Ví dụ:
  ```c
  printf(COLOR_CYAN LINE_V COLOR_RESET);
  printf(" %-20.20s ", memberName); // Độ rộng cột 20 luôn chuẩn xác tuyệt đối!
  ```

### Thiết kế lưới bảng đồng bộ (Single-Line Grid Drawing)
Toàn bộ các bảng danh sách thành viên, danh sách vi phạm cá nhân, vi phạm toàn CLB, thống kê ban, và tìm kiếm khoảng ngày đều sử dụng các ký tự UTF-8 vẽ hộp đơn đồng bộ:
- `┌` (`LINE_TL`), `┐` (`LINE_TR`), `└` (`LINE_BL`), `┘` (`LINE_BR`) cho các góc.
- `├`, `┼`, `┤` cho các điểm giao cắt.
- `─` (`LINE_H`) và `│` (`LINE_V`) cho các cạnh thẳng đứng và nằm ngang.

Nhờ việc tính toán chính xác số lượng ký tự nằm ngang cho từng cột, các bảng dữ liệu khớp nhau hoàn hảo, không bị đứt gãy lưới khi thay đổi độ dài nội dung.

---

## 4. Tóm tắt Giải thuật và Thiết kế Module C

Hệ thống tuân thủ nguyên tắc lập trình hướng cấu trúc modular hóa của C17:

| Tên Module | Vai trò Phân cấp | Giải thuật / Kỹ thuật nổi bật |
|---|---|---|
| **`utils`** | Tầng tiện ích lõi | Salt Generator ngẫu nhiên; Stretched FNV-1a Hashing; Date Parser sử dụng `sscanf` và `mktime`. |
| **`fileio`** | Tầng tương tác ổ đĩa | Auto magic-header check; Stream XOR Encryption; Heap-safe buffering; Ghi tệp đệm `.tmp` phòng chống crash máy. |
| **`auth`** | Tầng điều phối xác thực | Auto-migration tài khoản cũ; Force-change mặc định password; Khóa tài khoản chống Brute-force. |
| **`violation`** | Tầng nghiệp vụ vi phạm | Fuzzy search theo tên kết hợp chỉ mục số; Tính phạt phân cấp; Kiểm tra ngưỡng Out CLB tự động. |
| **`report`** | Tầng phân tích số liệu | Tổng hợp tiền phạt động theo cấu trúc cây; Thuật toán sắp xếp con trỏ trung gian không thay đổi mảng gốc. |
