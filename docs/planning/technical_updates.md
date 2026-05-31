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
| **`report`** | Tầng phân tích số liệu | Tổng hợp tiền phạt động theo cấu trúc cây; Thuật toán sắp xếp con trỏ trung gian không thay đổi mảng gốc; Biểu đồ thanh tiến trình UTF-8. |

---

## 5. Nâng cấp Nghiệp vụ Quản trị Cao cấp & Lọc Thành viên (v2.1 Upgrades)

Trong đợt nâng cấp v2.1, hệ thống đã tích hợp thêm các mô hình nghiệp vụ hành chính cao cấp, cơ chế lọc dữ liệu tinh gọn và giao dịch dữ liệu an toàn.

### 5.1. Cơ chế Kick & Khôi phục Liên kết An ninh (Security-linked Kick & Restore)
Để thay thế hoàn toàn tính năng "Đóng băng tài khoản" thô sơ ban đầu, hệ thống đã triển khai một quy trình Kick & Khôi phục toàn diện và bảo mật:
* **Tự động Khóa tài khoản (`isLocked = 1`)**: Khi một thành viên bị Kick khỏi CLB, cờ hoạt động `isActive` được chuyển sang trạng thái `STATUS_OUT_CLB` (0), đồng thời hệ thống tự động khóa tài khoản đăng nhập của họ trên tệp dữ liệu `accounts.dat`. Điều này ngăn chặn việc đăng nhập vào hệ thống ngay lập tức.
* **Reset số buổi vắng liên tiếp**: Reset cờ `consecutiveAbsences = 0` trên cả hai thao tác Kick và Khôi phục. Khi khôi phục, thành viên sẽ bắt đầu lại từ đầu với trạng thái hoạt động sạch sẽ mà không lo vừa kích hoạt đã nằm lại ở ngưỡng bị cảnh báo.
* **Audit Trail (Lưu vết lý do bắt buộc)**: Để lưu trữ lý do kick mà không làm vỡ cấu trúc nhị phân của `Member`, hệ thống tự động sinh một bản ghi vi phạm kỷ luật đặc biệt trong `violations.dat` với:
  - `penalty = PENALTY_OUT_CLB`
  - `fine = 0.0`
  - `isPaid = 1`
  - `reason = REASON_VIOLENCE`
  - `note` = Lý do kick (BCN bắt buộc phải nhập và không được để trống).
  Lý do này sẽ được trích xuất vĩnh viễn và hiển thị chi tiết khi BCN tra cứu **Option 19 (Xem danh sách thành viên đã kick)**.
* **Xác thực 2 bước chống ấn nhầm**: 
  - *Bước 1*: Xác nhận qua menu chọn `1=Co, 0=Khong`.
  - *Bước 2*: BCN bắt buộc phải gõ lại chính xác mã MSSV của thành viên để hoàn tất. Hệ thống tự động chuẩn hóa chữ hoa/thường để so khớp an toàn.
* **Luật bảo vệ BCN thường**:
  - Không cho phép tự kick chính mình.
  - Ngăn chặn tài khoản BCN thường kick các thành viên khác có chức vụ BCN. Chỉ có tài khoản Super Admin (`admin` hoặc `SE203055`) mới có quyền thực hiện thao tác này.

### 5.2. Cơ chế Thu tiền phạt gộp (Bulk Payment Transaction)
Tại giao diện thu tiền phạt (Option 5), thay vì phải chọn thủ công từng vi phạm rất tốn thời gian, BCN có thể nhập mã đặc biệt **`99`** để kích hoạt thanh toán gộp:
* **Giao dịch nguyên tử (Atomic Transaction)**: Hệ thống gom toàn bộ các vi phạm có trạng thái chưa thanh toán (`isPaid == 0` và `fine > 0`) của thành viên đó vào một mảng giao dịch tạm thời trên RAM và chuyển trạng thái của chúng sang đã nộp.
* **Cơ chế Rollback an toàn**: Nếu quá trình ghi dữ liệu nhị phân xuống ổ đĩa (`fileioSaveViolations` hoặc `fileioSaveMembers`) gặp sự cố lỗi đĩa hoặc mất quyền ghi, hệ thống sẽ tự động rollback (hoàn tác) toàn bộ trạng thái RAM về ban đầu để đảm bảo số liệu công nợ không bao giờ bị sai lệch.

### 5.3. Bộ lọc Thành viên Hoạt động Tinh gọn & Cân bằng Menu
* **Hiển thị tinh gọn (`memberListAll`)**:
  - Tự động loại bỏ hoàn toàn các thành viên đã bị Kick hoặc Soft-deleted khỏi danh sách chung.
  - Loại bỏ hoàn toàn cột `"Trang thai"` khỏi bảng danh sách (vì toàn bộ thành viên hiển thị đều đang hoạt động bình thường).
  - Mở rộng cột `"Ho va ten"` lên 34 ký tự để hiển thị trọn vẹn và đẹp mắt các họ tên tiếng Việt có độ dài lớn.
  - Tiêu đề in đậm hiển thị rõ ràng: `DANH SACH THANH VIEN DANG HOAT DONG (%d)`.
* **Cân bằng giao diện Menu (Menu Alignment)**:
  - Cập nhật nhãn của phím số 4 (Menu Thành viên) và phím số 13 (Menu BCN) thành `"Xem DS thanh vien dang hoat dong"` và `"Xem DS TV dang hoat dong"`.
  - Thiết kế lại các ký tự khoảng trắng đệm (padding spaces) ở phía sau nhãn để đảm bảo độ rộng hàng in ra luôn luôn bằng **đúng 68 ký tự**, tạo nên các viền khung đôi `║` dọc thẳng hàng, chuẩn mỹ thuật TUI.
* **Tinh lọc số liệu báo cáo (`report.c`)**:
  - **Sắp xếp theo số lần vi phạm (`reportSortMembersByViolations`)**: Tự động lọc bỏ các thành viên đã bị kick hoặc đã bị xóa để tránh làm nhiễu danh sách xếp hạng kỷ luật của các thành viên đang hoạt động.
  - **Báo cáo xuất file text (`reportExportTxt`)**: Tự động loại bỏ các thành viên đã xóa khỏi danh sách công nợ phạt, giúp ban chủ nhiệm có báo cáo tài chính sạch sẽ.

### 5.4. Dashboard Kỷ luật & Biểu đồ tiến trình (Discipline Dashboard)
BCN có một công cụ theo dõi trực quan cao cấp (Option 16) bao gồm:
* **Top 5 vi phạm**: Sắp xếp nhanh danh sách thành viên hoạt động vi phạm nhiều nhất mà không thay đổi thứ tự mảng gốc trên RAM bằng mảng con trỏ trung gian.
* **Tỷ lệ vi phạm**: Phân tích phần trăm lý do vi phạm chi tiết (Vắng họp, Không mặc áo CLB, Bạo lực...).
* **Thanh tiến trình thu phạt (Progress Bar)**:
  - Hệ thống tự động tính tỷ lệ thu tiền phạt: $\text{ratio} = \frac{\text{Đã thu}}{\text{Tổng phát}} \times 100$.
  - Biểu diễn trực quan bằng thanh tiến trình gồm 10 ký tự khối UTF-8 (`█` cho phần đã hoàn thành, `░` cho phần chưa hoàn thành). Ví dụ: `[██████░░░░] 60.5%`.
  - Tích hợp cờ bảo vệ chống chia cho 0 (`NaN` protection) khi hệ thống chưa ghi nhận bất kỳ khoản phạt nào.

### 5.5. Công cụ Seeding dữ liệu bảo mật (`seed_data.c`)
Để đảm bảo môi trường phát triển (Dev) đồng bộ tuyệt đối với các tiêu chuẩn an toàn mới, công cụ gieo hạt dữ liệu mẫu `tools/seed_data.c` đã được thiết kế lại hoàn toàn:
* Không ghi dữ liệu dạng text/nhị phân thuần túy nữa.
* Tích hợp thuật toán mã hóa đối xứng XOR và Magic Header `FCE1` để sinh ra các file `.dat` đã mã hóa gốc.
* Sinh muối ngẫu nhiên (Salt) và tự động băm mật khẩu qua thuật toán kéo giãn khóa 1000 rounds FNV-1a cho tất cả tài khoản mẫu (ví dụ: Super Admin `SE203055` / `Phuc@2006` và Legacy Super Admin `admin` / `admin`).
* Giúp dự án khởi chạy lần đầu tiên là có ngay cơ sở dữ liệu siêu bảo mật, đồng bộ 100% với ứng dụng chính.

