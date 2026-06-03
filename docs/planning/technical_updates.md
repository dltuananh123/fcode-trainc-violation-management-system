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

---

## 6. Hệ thống Ghi & Xem Nhật ký (Audit Log System)

Hệ thống nhật ký được thiết kế để lưu vết mọi hành động quản trị nhằm phục vụ kiểm tra, truy xuất trách nhiệm và hỗ trợ giám sát hoạt động của Ban Chủ Nhiệm.

### 6.1. Cơ chế Ghi nhật ký (`logSystemAction`)

Mỗi thao tác nghiệp vụ quan trọng (thêm/sửa/xóa thành viên, ghi nhận vi phạm, thu tiền phạt, kick/khôi phục, reset mật khẩu) đều tự động gọi `logSystemAction(actor, action, target)`:

```
[dd/mm/yyyy HH:MM:SS] [actor    ] ACTION: action                   | TARGET: target
```

Hàm ghi đồng thời vào **2 file**:
- `data/system_audit.log` — nhật ký dạng text thuần túy, dễ đọc
- `data/simulated_webhooks.log` — mô phỏng webhook cho các hệ thống tích hợp bên ngoài

### 6.2. Chức năng Xem nhật ký (`viewSystemLogs`)

Chức năng này cho phép BCN xem toàn bộ nhật ký hệ thống trực tiếp từ terminal với các đặc điểm:

- **Đọc file** `system_audit.log` từ thư mục `data/`
- **Phân giải cấu trúc** dòng log bằng `sscanf` — trích xuất thời gian, người thực hiện, hành động, mục tiêu
- **Tô màu theo thành phần**:
  - Thời gian: xám (`COLOR_DIM`)
  - Người thực hiện: xanh lá (`COLOR_GREEN`)
  - Hành động: vàng (`COLOR_YELLOW`)
  - Mục tiêu: xanh dương (`COLOR_CYAN`)
- **Phân trang** 20 dòng/lần, nhấn `Enter` xem tiếp, nhấn `q` + Enter thoát
- Xử lý lỗi khi file không tồn tại (hiển thị đường dẫn cho BCN kiểm tra)
- Tương thích với dòng log không đúng chuẩn (hiển thị toàn bộ dòng gốc)

```
Log format:
[%4d. ║ [thời_gian] [người_thực_hiện] hành_động | mục_tiêu]
```

### 6.3. Vị trí code

| Thành phần | File | Hàm |
|---|---|---|
| Ghi log | `src/utils.c:420-458` | `logSystemAction()` |
| Xem log | `src/utils.c:460-551` | `viewSystemLogs()` |
| Khai báo | `include/utils.h:190` | `logSystemAction()` |
| Khai báo | `include/utils.h:197` | `viewSystemLogs()` |
| Menu entry | `src/main.c:269-271` | `case 20: viewSystemLogs()` |

---

## 7. Nâng cấp Giao diện & Xác thực (v2.2 UI & Validation Enhancements)

### 7.1. Chuẩn hóa Alignment Toàn bộ Menu (UI Alignment Overhaul)

Toàn bộ các menu con trong hệ thống trước đây dùng chuỗi hardcode với số lượng khoảng trắng thủ công, dẫn đến lỗi lệch viền khung (`║`) ở nhiều màn hình. Đã triển khai giải pháp động:

* **Thêm hàm `uiDrawMenuRow()`** (`src/ui.c`) — tự động padding khoảng trắng để nội dung giữa hai viền `║` luôn đúng **68 ký tự** (`UI_TERM_WIDTH - 2`):
  ```c
  void uiDrawMenuRow(const char *text);
  void uiDrawMenuRowFmt(const char *fmt, ...);
  ```
* **Hỗ trợ ANSI Color**: Hàm `uiVisibleLen()` bỏ qua mã màu ANSI (`\033[...m`) khi tính độ dài hiển thị, đảm bảo padding chính xác cho các dòng có tô màu.
* **Các file được sửa**:

| File | Số dòng sửa | Mô tả |
|---|---|---|
| `src/violation.c` | 15 menu items | Thay hardcode bằng `uiDrawMenuRow()` |
| `src/report.c` | ~20 dòng | Sort menu, dashboard headers, format `%-62s`→`%-66s`, pad target 66→68 |
| `src/member.c` | ~15 dòng | Profile `%-39s`→`%-54s`, status line, archive/kicked empty message |
| `src/main.c` | 4 dòng | Member menu rows 3,4 và admin menu row 10 |

* **Hằng số `UI_TERM_WIDTH`**: Di chuyển từ `src/ui.c` sang `include/ui.h` để các file khác có thể sử dụng.

### 7.2. Cơ chế Pause trước khi về Menu (Post-Action Pause)

Trước đây, sau khi thực hiện một tác vụ (thêm thành viên, xem danh sách,...), kết quả hiển thị bị xóa ngay lập tức khi menu chính được vẽ lại, gây khó chịu cho người dùng.

* **Thêm hàm `uiPause()`** (`src/ui.c`): Hiển thị `"Nhan Enter de tiep tuc..."` và chờ người dùng nhấn Enter.
* **Tích hợp vào menu chính**: Gọi `uiPause()` sau mỗi `switch (choice)` nếu `choice != 0` — áp dụng cho cả `memberMenu()` và `adminMenu()`.
* **Gỡ pause dư thừa**: Xóa `"Nhan Enter de quay lai..."` khỏi `viewSystemLogs()` để tránh nhắc đúp.

### 7.3. Nâng cấp Validate Mật khẩu (Password Validation Enhancement)

Cập nhật hàm `validatePassword()` trong `src/validate.c` với các tiêu chuẩn bảo mật cao hơn, tương ứng regex "Thông dụng" (`^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)(?=.*[@$!%*?&])[A-Za-z\d@$!%*?&]{8,}$`):

| Tiêu chí | Cũ | Mới |
|---|---|---|
| Độ dài tối thiểu | 6 ký tự | **8 ký tự** |
| Chữ hoa | ❌ Không kiểm tra | ✅ **Phải có 1 chữ hoa** |
| Chữ thường | ✅ Phải có | ✅ Giữ nguyên |
| Chữ số | ✅ Phải có | ✅ Giữ nguyên |
| Ký tự đặc biệt | ❌ Không kiểm tra | ✅ **Phải có 1 ký tự đặc biệt** (@, $, !, %, *, ?, &, ...) |
| Khoảng trắng | ✅ Không được chứa | ✅ Giữ nguyên |

Mỗi điều kiện có thông báo lỗi riêng bằng tiếng Việt, dễ hiểu cho người dùng cuối.

### 7.4. Cải tiến Flow Đổi Mật khẩu (Password Change Flow Fix)

Khi người dùng xác nhận mật khẩu mới không khớp, thay vì chỉ yêu cầu nhập lại xác nhận (dễ gây nhầm lẫn), flow mới yêu cầu nhập lại toàn bộ từ bước **"Nhap mat khau moi"**:

* Trước: `Nhap mat khau moi` → `Xac nhan` (sai) → `Xac nhan` (sai) → `Xac nhan`...
* Sau: `Nhap mat khau moi` → `Xac nhan` (sai) → `Nhap mat khau moi` → `Xac nhan`...

Số lần nhập mật khẩu có thể tăng, nhưng giảm thiểu rủi ro người dùng không nhận ra mình đã gõ sai từ bước đầu. Lược bỏ bước kiểm tra `confirmPass == ""` vì đã có `validatePassword()` ở bước trước.

---

## 8. Nâng cấp Giao diện, Validation & Seed Data (v2.3 UI/UX & Data Enhancements)

### 8.1. Re-prompt Toàn bộ Edit Field (Member Edit Validation Loop)

Trước đây, khi sửa thông tin thành viên (`memberEdit`), nếu nhập dữ liệu không hợp lệ (email thiếu `@`, phone có chữ), hệ thống chỉ báo lỗi và kết thúc mà không cho nhập lại. Đã sửa thành vòng lặp `while(1)` cho tất cả các trường:

| Trường | Cơ chế | File |
|--------|--------|------|
| Họ tên | `editField()` + `validateName()` — đã có sẵn | `src/member.c:337` |
| Email | `while(1)` + `validateEmailUnique()` — Enter giữ nguyên, sai nhập lại | `src/member.c:340-355` |
| SĐT | `while(1)` + `phoneNormalize()` + `validatePhoneUnique()` | `src/member.c:357-373` |
| Team/Role | `while(1)` với menu chọn | `src/member.c:375-406` |
| Mật khẩu cũ | `while(1)` trong `authChangePassword` | `src/auth.c:207-228` |

### 8.2. "0 để quay lại" — Thoát an toàn từ MSSV Prompt

Tất cả các prompt yêu cầu nhập MSSV đều có cơ chế nhập `0` để hủy thao tác và quay lại, tránh kẹt menu:

| Chức năng | File |
|-----------|------|
| Sửa thành viên | `src/member.c:299` |
| Xóa thành viên | `src/member.c:471` |
| Kick/Restore | `src/member.c:891, 967` |
| Thêm thành viên (team chỉ định) | `src/member.c:128` |
| Ghi nhận vi phạm | `src/violation.c` |
| Thu tiền phạt | `src/violation.c` |
| Reset mật khẩu | `src/main.c` |
| Tìm kiếm theo ngày | `src/violation.c` |

### 8.3. Email Validation Chặt chẽ

`validateEmail()` (`src/validate.c:321-418`) được viết lại hoàn toàn:

| Điều kiện | Mô tả |
|-----------|-------|
| Có đúng 1 `@` | `a@b` — OK, `a@@b` — FAIL |
| Không `,` trong domain | `test@f,c.vn` — FAIL |
| Không `.` ở đầu/cuối local part | `.abc@d.com` — FAIL |
| Không `.` liên tiếp | `a..b@c.com` — FAIL |
| TLD >= 2 ký tự | `a@b.c` — FAIL, `a@b.com` — OK |
| Không ký tự đặc biệt trong TLD | `a@b.c_o_m` — FAIL |
| Ký tự hợp lệ: `a-z A-Z 0-9 . - _ +` | `a(b)@c.com` — FAIL |

### 8.4. Member List — 5 Cột + Phân Trang

Bảng danh sách thành viên (`memberListAll`, `src/member.c`) được thiết kế lại:

- **5 cột**: MSSV (10), Họ và tên (20), Email (24), SDT (12), Ban (10)
- **Terminal width**: `UI_TERM_WIDTH` = 100 (`include/ui.h:45`)
- **Phân trang**: 15 dòng/trang, `n`: tiếp, `m`: trước, `q`: thoát
- **Truncation**: `%-20.20s` format để tránh tràn cột
- **Header động**: Dùng `printf("%-10s", "MSSV")` thay vì chuỗi hardcode
- **`ROWS_PER_PAGE`**: Chuyển từ `member.c` lên `include/ui.h` để dùng chung

### 8.5. Violation Table Column Fix

Bảng vi phạm bị lệch cột "Chờ đóng phạt" và "Trạng thái":

| Cột | Trước | Sau |
|-----|-------|-----|
| Chờ đóng phạt | LINE_H=14 (header 15 ký tự) | LINE_H=15 (khớp header) |
| Trạng thái | Chuỗi 10 ký tự (cột 12) | Chuỗi 12 ký tự (đệm đúng) |
| Border bottom | 14 ở 2 vị trí | 15 ở 2 vị trí |

Tổng cộng sửa 5 bottom border trong `src/violation.c`.

### 8.6. Violation List — Phân trang

Hai view chính của vi phạm được thêm phân trang:

- `violationViewAllFiltered()` — xem tất cả/lọc
- `violationSearchByDate()` — tìm theo ngày

Cơ chế:
1. Duyệt toàn bộ violations, lưu index match vào `matchIdx[]`
2. Nếu `found == 0` → thông báo và return
3. Vòng lặp `while(1)` với `uiClear()` mỗi trang
4. `n`/`m`/`q` điều hướng (giống member list)
5. Nếu chỉ 1 trang → hiện `"Nhấn Enter để tiếp tục"` và thoát

### 8.7. Seed Data Overhaul (`tools/seed_data.c`)

Dữ liệu seed được thay thế hoàn toàn từ dữ liệu giả (SV0001-SV0012) sang sinh viên thật Challenge 3:

| Thông số | Cũ | Mới |
|----------|-----|------|
| Thành viên | 14 (12 SV + 2 BCN) | **72** (70 thật + 2 BCN) |
| Vi phạm | 16 | **76** (17 + 59 bổ sung) |
| Tài khoản | 15 | **72** |
| Bị kick | 0 | **3** (SE210946, SE210117, SE203367) |
| OUT CLB | 0 | **1** (SE200516) |

**Team mapping**: Nhóm 1-4 → Học thuật, 5-7 → Kế hoạch, 8-10 → Nhân sự, 11-14 → Truyền thông

**Mật khẩu**: 
- Tất cả thành viên: mật khẩu = MSSV
- SE203055 (Super Admin): `Phuc@2006` (hashed với salted FNV-1a)
- `Violation violations[40]` → `[100]` để chứa đủ

### 8.8. `ROWS_PER_PAGE` Centralized

```c
// include/ui.h (thêm mới)
#define ROWS_PER_PAGE 15

// src/member.c (xóa)
-#define ROWS_PER_PAGE 15
```

Dùng chung cho tất cả view có phân trang.

### 8.9. `run.bat` — Build + Seed + Run

Script `run.bat` tự động hóa toàn bộ quy trình:

```batch
@echo off
if exist bin\ rmdir /s /q bin\
mkdir bin\data
mingw32-make
gcc -std=c17 -m64 -Wall -Iinclude tools/seed_data.c -o bin/seed_data.exe
bin\seed_data.exe
bin\violation-management-system.exe
```

---

## 9. Các Bản Cập Nhật Nghiệp Vụ Mới (v2.4 Business Logic & Data Transfer Updates)

### 9.1. Nhập Vi Phạm Hàng Loạt từ file CSV (CSV Import)
Để tránh việc nhập thủ công từng thành viên, hệ thống hỗ trợ import nhanh vi phạm từ file CSV (chứa 3 cột: `studentId,reasonCode,notes`).
* **Dry-run validation**: Hệ thống kiểm tra trước toàn bộ file CSV, phân loại và in ra trạng thái từng dòng (`[OK]` hoặc `[FAIL - lý do lỗi]`).
* **Xử lý chọn lọc**: Admin xác nhận chỉ ghi nhận những dòng `[OK]`, tự động bỏ qua những dòng `[FAIL]` thay vì hủy bỏ cả file.
* **Thời gian ghi nhận**: Tự động gán thời gian thực hiện qua `time(NULL)` tại thời điểm import.
* **Tự động Reset vắng**: Bất kỳ thành viên nào không bị ghi nhận lỗi vắng họp (`reasonCode = 1`) trong file CSV import sẽ tự động được reset số buổi vắng liên tiếp (`consecutiveAbsences`) về `0` và in ra thông báo chi tiết.

### 9.2. Xem Ngưỡng Out CLB của Bản Thân (Self-view Out Threshold)
Thêm chức năng `[6] Xem nguong Out CLB & Vi pham` vào Menu Thành viên:
* Cho phép thành viên tự theo dõi trạng thái hoạt động và số buổi vắng họp liên tiếp hiện tại trên tổng số tối đa (`consecutiveAbsences` / 3 buổi).
* Thống kê số lượng vi phạm chưa nộp phạt thực tế.
* Hiển thị chi tiết bảng quy chế Out CLB để thành viên đối chiếu và chủ động đi họp chuyên cần.

### 9.3. Mã Hóa Nhật Ký Hệ Thống (Encrypted Audit Logging)
* Để nâng cao tính bảo mật, nội dung ghi vào file `data/system_audit.log` giờ đây được tự động mã hóa XOR với khóa `0x5A` (giữ lại ký tự xuống dòng `\n` để bảo vệ cấu trúc file).
* Khi xem nhật ký qua giao diện hệ thống (`viewSystemLogs`), chương trình tự động giải mã để hiển thị trực quan thông tin gốc.

### 9.4. Đóng gói & Di Chuyển Dữ Liệu (PIN-protected Export/Import Archive)
Hệ thống cho phép export/import dữ liệu dễ dàng giữa các máy tính khác nhau thông qua một tệp nén duy nhất bảo mật bằng mã PIN 4 số:
* **Export (Menu Hệ thống -> [5])**: Đóng gói toàn bộ cơ sở dữ liệu (`accounts.dat`, `members.dat`, `violations.dat`) và file nhật ký hành trình (`system_audit.log`) vào một file backup duy nhất. Dữ liệu được mã hóa bằng khóa byte sinh ra từ mã PIN bảo mật 4 chữ số do admin tự đặt.
* **Import (Menu Hệ thống -> [6])**: Yêu cầu nhập mã PIN bảo mật. Nếu mã PIN khớp với chữ ký hash ghi nhận trong file, hệ thống sẽ tiến hành giải mã, phục hồi toàn bộ cơ sở dữ liệu cục bộ và phục hồi lại cả file nhật ký lịch sử `system_audit.log` trên máy tính mới.


