---
stepsCompleted: [1, 2, 3, 4]
inputDocuments:
  - requirement-docs/QuanLyViPhamCLBFCode_V1.md
  - requirement-docs/Rule_TrainC.md
  - _bmad-output/planning-artifacts/architecture.md
workflowType: 'epics'
project_name: 'fcode trainc'
user_name: 'Admin'
date: '2026-04-19'
lastStep: 4
status: 'complete'
completedAt: '2026-04-19'
---

# fcode trainc - Epic Breakdown

## Overview

This document provides the complete epic and story breakdown for **fcode trainc** — F-Code CLB Violation Management System. Requirements are decomposed from the project spec (`requirement-docs/QuanLyViPhamCLBFCode_V1.md`) and technical decisions from `architecture.md`.

## Requirements Inventory

### Functional Requirements

**Group 0 — Authentication & Authorization (3 required):**

```
FR-0.1: Đăng nhập hệ thống — Nhập MSSV + mật khẩu, xác định role, load menu tương ứng. Sai 3 lần liên tiếp → khóa tài khoản + thoát chương trình
FR-0.2: Đăng xuất — Xóa session, quay về màn hình đăng nhập. Không thoát chương trình
FR-0.3: Đổi mật khẩu — Thành viên đổi mật khẩu của chính mình (cần nhập mật khẩu cũ). BCN có thể reset mật khẩu bất kỳ thành viên
```

**Group 1 — Member Role Features (4 required):**

```
FR-1.1: Xem profile cá nhân — Hiển thị: họ tên, MSSV, email, SĐT, ban, chức vụ, trạng thái thành viên. Không được sửa trực tiếp
FR-1.2: Xem lịch sử vi phạm của bản thân — Chỉ xem vi phạm của chính mình: lý do, ngày, mức phạt, trạng thái đã thu/chưa thu. Không xem được vi phạm người khác
FR-1.3: Xem tổng tiền phạt còn nợ — Tính tổng các vi phạm có isPaid=0. Hiển thị rõ từng khoản và tổng cộng
FR-1.4: Xem danh sách thành viên CLB — Chỉ hiển thị: họ tên, MSSV, ban, chức vụ. Không hiển thị SĐT, email, số lần vi phạm của người khác
```

**Group 2 — Admin (BCN) Role Features (8 required + 2 advanced + 1 optional):**

```
FR-2.1: Thêm thành viên mới — Nhập: họ tên, MSSV, email, SĐT, ban (team), chức vụ, mật khẩu mặc định. Validate MSSV không trùng, email hợp lệ. Tự động tạo tài khoản trong accounts.dat
FR-2.2: Sửa thông tin thành viên — Sửa được tất cả trừ MSSV. Nếu thay đổi chức vụ → cập nhật lại mức phạt cho các vi phạm chưa thu tiền
FR-2.3: Xóa thành viên — Xóa thành viên + lịch sử vi phạm + tài khoản đăng nhập. Yêu cầu xác nhận lại trước khi thực hiện xóa
FR-2.4: Ghi nhận vi phạm — Chọn thành viên (tìm theo MSSV/tên), chọn lý do, hệ thống tự ghi thời gian và tính mức phạt. Lý do bạo lực → cảnh báo và confirm Out CLB
FR-2.5: Đánh dấu đã thu tiền phạt — Chọn từng vi phạm để đánh dấu isPaid=1. Cập nhật totalFine của thành viên ngay lập tức
FR-2.6: Xem danh sách vi phạm toàn CLB — Hiển thị toàn bộ vi phạm, lọc theo: ban, lý do vi phạm, trạng thái thu tiền
FR-2.7: Thống kê tiền phạt theo ban — Tổng đã thu / chưa thu của từng ban. Hiển thị dạng bảng trên terminal
FR-2.8: Cảnh báo thành viên ngưỡng Out CLB — Tự động kiểm tra khi ghi vi phạm: consecutiveAbsences >= 2 → cảnh báo. Nếu = 3 → thông báo Out CLB, yêu cầu BCN xác nhận xử lý
FR-2.9: Sắp xếp danh sách theo số lần vi phạm (Nâng cao) — Tăng/giảm. Thuật toán tự cài, không dùng qsort
FR-2.10: Xuất báo cáo vi phạm ra file .txt (Nâng cao) — Có timestamp, tổng hợp theo ban, danh sách thành viên còn nợ tiền phạt
FR-2.11: Tìm kiếm vi phạm theo khoảng thời gian (Tùy chọn) — Nhập ngày bắt đầu - kết thúc, lọc theo violationTime bằng time_t
```

### NonFunctional Requirements

```
NFR-1: Data durability — Save to file after every mutation operation, ensuring zero data loss on crash/exit
NFR-2: Capacity — Support maximum 1,000 members and 10,000 violations at any time
NFR-3: Security — 3 failed login attempts = account lock; role-based menu access (Member vs BCN)
NFR-4: Portability — 100% standard C (C17) only, no external libraries, must compile with gcc/MinGW
NFR-5: No global variables — Avoid unnecessary global variables (scoring criterion 5.2)
NFR-6: Code organization — Split into multiple .h/.c modules, header guards (#ifndef), consistent naming
NFR-7: Language purity — 100% pure C, absolutely no C++, Python, Java or any mixed-language code
NFR-8: File persistence — Binary .dat files using fwrite/fread, load on startup, save on every mutation
```

Naming convention note: use `PascalCase` for struct/type names, `camelCase` for shared struct fields defined by the requirement, `snake_case` for local variables, and `UPPER_SNAKE_CASE` for constants.

### Additional Requirements (from Architecture)

```
AR-1: Directory structure — src/ for .c files, include/ for .h files, build/ for .o files, bin/ for executable
AR-2: Build system — Makefile with targets: make, make clean, make run, make format, make tidy
AR-3: AppDatabase struct — All arrays wrapped in single struct, functions receive AppDatabase* only
AR-4: Session management — static Account in auth.c with getter function (no extern globals)
AR-5: Two role systems — Account.role (menu access: Member=0, BCN=1) vs Member.role (fine calc: Member=0, Leader=1, BCN=2)
AR-6: Fine recalculation — When member role changes, recalculate all unpaid violation fines and totalFine
AR-7: First-run admin — ADMIN/ADMIN account created automatically on first run (implementation decision)
AR-8: Out CLB threshold — consecutiveAbsences >= 3 warns, >= 4 triggers Out CLB confirmation (matches "quá 3 buổi")
AR-9: Sort must use copy — Sort operations on pointer array copy, never modify original array order
AR-10: Date parsing — parse_date() uses sscanf + struct tm + mktime() for dd/mm/yyyy to time_t conversion
AR-11: Binary file format — First 4 bytes = record count (int header), then fixed-size struct records
AR-12: Code quality — clang-format for formatting, clang-tidy for static analysis, -Wall -Wextra zero warnings
```

### UX Design Requirements

```
(Not applicable — this is a CLI terminal application, no GUI/UX design document)
```

### FR Coverage Map

```
FR-0.1: Epic 1 — Login
FR-0.2: Epic 1 — Logout
FR-0.3: Epic 1 — Change/Reset password
FR-1.1: Epic 2 — View profile
FR-1.2: Epic 3 — View own violations
FR-1.3: Epic 3 — View own fines
FR-1.4: Epic 2 — View member list
FR-2.1: Epic 2 — Add member
FR-2.2: Epic 2 — Edit member
FR-2.3: Epic 2 — Delete member
FR-2.4: Epic 3 — Record violation
FR-2.5: Epic 3 — Mark paid
FR-2.6: Epic 3 — View all violations
FR-2.7: Epic 4 — Stats by team
FR-2.8: Epic 3 — Out CLB warning
FR-2.9: Epic 4 — Sort by violations
FR-2.10: Epic 4 — Export report
FR-2.11: Epic 4 — Search by date
```

## Epic List

### Epic 1: Project Setup & Authentication
Users can log into the system and the application foundation is fully operational — project structure, Makefile, data types, file I/O, authentication, and session management all working.

**FRs covered:** FR-0.1, FR-0.2, FR-0.3
**NFRs covered:** NFR-1, NFR-2, NFR-4, NFR-5, NFR-6, NFR-7, NFR-8
**ARs covered:** AR-1 to AR-12

### Epic 2: Member Management
BCN can manage members (add/edit/delete), members can view their own profile and the CLB member list.

**FRs covered:** FR-1.1, FR-1.4, FR-2.1, FR-2.2, FR-2.3

### Epic 3: Violation Tracking & Enforcement
BCN can record violations, mark payments, view all violations with filters. Members can view their own violations and fines. System auto-detects Out CLB thresholds.

**FRs covered:** FR-1.2, FR-1.3, FR-2.4, FR-2.5, FR-2.6, FR-2.8

### Epic 4: Reports & Advanced Features
BCN has full reporting capabilities: statistics by team, sort members by violations, export report to .txt, search violations by date range.

**FRs covered:** FR-2.7, FR-2.9, FR-2.10, FR-2.11

---

## Epic 1: Project Setup & Authentication

_Goal: Set up the complete project foundation and implement the authentication system so users can securely log in and out._

### Story 1.1: Project Scaffold & Build System

As a developer,
I want a complete project structure with Makefile and tooling configuration,
So that the team can start implementing features immediately with consistent build and formatting.

**Acceptance Criteria:**

**Given** the project repository is initialized
**When** a developer runs `make`
**Then** the project compiles successfully with zero warnings using gcc -std=c17 -Wall -Wextra
**And** the executable is placed in `bin/fcode-trainc`

**Given** the project structure exists
**When** a developer runs `make clean`
**Then** the `build/` and `bin/` directories are removed

**Given** the project structure exists
**When** a developer runs `make format`
**Then** clang-format is applied to all `src/*.c` and `include/*.h` files

### Story 1.2: Data Types & Constants

As a developer,
I want all struct definitions and constants centralized in a single header,
So that all modules share consistent data types without circular dependencies.

**Acceptance Criteria:**

**Given** `include/types.h` is created
**When** any module includes it
**Then** it provides Member, Violation, Account structs and AppDatabase typedef
**And** all #define constants are defined (MAX_MEMBERS, REASON_*, TEAM_*, PENALTY_*, ROLE_*, STATUS_*, field sizes)
**And** types.h includes only `<time.h>` — no other project headers

### Story 1.3: Utility Functions

As a developer,
I want common validation and formatting helpers in a utility module,
So that all modules can reuse input validation, time formatting, and display name mapping.

**Acceptance Criteria:**

**Given** `include/utils.h` and `src/utils.c` are created
**When** modules call `is_email_valid(email)`
**Then** it returns 1 if email contains '@' and '.', 0 otherwise

**Given** `format_time(time_t)` is called
**When** a time_t value is passed
**Then** it returns a formatted string "dd/mm/yyyy HH:MM"

**Given** `parse_date(buffer, &time_t)` is called
**When** user inputs "15/04/2026"
**Then** it converts to time_t using sscanf + struct tm + mktime with proper normalization (start=00:00:00, end=23:59:59)

**Given** display name functions are called
**When** `team_name(0)`, `member_role_name(1)`, `reason_name(3)` are called
**Then** they return Vietnamese display strings: "Academic", "Leader/Vice", "Bao luc"

### Story 1.4: File I/O — Load/Save with First-Run Init

As a developer,
I want all data persistence handled by a single fileio module,
So that data is saved to binary .dat files on every mutation and loaded on startup.

**Acceptance Criteria:**

**Given** the program starts and `data/*.dat` files do not exist
**When** `fileio_load_all()` is called
**Then** empty .dat files are created with header (record count = 0)
**And** a default BCN account ADMIN/ADMIN is created in accounts.dat

**Given** .dat files exist with data
**When** `fileio_load_all()` is called
**Then** all Member, Violation, Account records are loaded into AppDatabase arrays

**Given** any mutation occurs (add member, record violation, etc.)
**When** the corresponding `fileio_save_*()` function is called
**Then** the entire file is rewritten (count header + all records) using fwrite
**And** data is not lost if the program crashes after save

### Story 1.5: Authentication — Login, Logout, Session

As a user,
I want to log in with my MSSV and password,
So that I can access the system with my assigned role.

**Acceptance Criteria:**

**Given** the program starts
**When** the user enters correct MSSV and password
**Then** the system loads the role-appropriate menu (Member or BCN)
**And** the session is tracked via `auth_get_session()` (static in auth.c, no extern)

**Given** the user enters wrong password
**When** `failCount` reaches 3
**Then** the account is locked (`isLocked = 1`), saved to file, and the program exits

**Given** the user is logged in
**When** the user selects Logout
**Then** the session is cleared and the login screen is shown (program does NOT exit)

**Given** a locked account tries to log in
**When** any password is entered
**Then** the system displays "[LOI] Tai khoan da bi khoa" and refuses login

### Story 1.6: Change & Reset Password

As a user,
I want to change my password or have BCN reset it,
So that I can maintain account security.

**Acceptance Criteria:**

**Given** a logged-in user selects Change Password
**When** they enter correct old password and valid new password
**Then** the password is updated in accounts[] and saved to accounts.dat

**Given** a logged-in user enters wrong old password
**When** attempting to change password
**Then** the system displays "[LOI] Mat khau cu khong dung"

**Given** a BCN user selects Reset Password
**When** they enter a target MSSV
**Then** that member's password is reset to their MSSV (default) and saved to file

---

## Epic 2: Member Management

_Goal: Enable BCN to manage the member database and members to view their own info and the member list._

### Story 2.1: Add Member

As a BCN admin,
I want to add new members with full information,
So that the CLB member database is maintained.

**Acceptance Criteria:**

**Given** BCN selects Add Member
**When** they enter: họ tên, MSSV, email, SĐT, ban (team), chức vụ
**Then** a new Member is added to members[] and a new Account (password=MSSV) is added to accounts[]
**And** MSSV uniqueness is validated — duplicate shows "[LOI] MSSV da ton tai"
**And** email format is validated — invalid shows "[LOI] Email khong hop le"
**And** members.dat and accounts.dat are saved immediately

**Given** the member count has reached 1000
**When** BCN tries to add another member
**Then** the system shows "[LOI] Da dat so luong thanh vien toi da (1000)"

### Story 2.2: Edit Member

As a BCN admin,
I want to edit member information,
So that member records stay up to date.

**Acceptance Criteria:**

**Given** BCN selects Edit Member and enters a valid MSSV
**When** they update any field (except MSSV which is read-only)
**Then** the member record is updated and members.dat is saved

**Given** BCN changes a member's role (e.g., Member → Leader)
**When** the edit is confirmed
**Then** all unpaid violations (isPaid==0) for that member are recalculated with the new role's fine rate
**And** the member's totalFine is recalculated
**And** both violations.dat and members.dat are saved

### Story 2.3: Delete Member

As a BCN admin,
I want to remove a member from the system,
So that departed members no longer appear in the database.

**Acceptance Criteria:**

**Given** BCN selects Delete Member and enters a valid MSSV
**When** the system shows the member's info and asks for confirmation
**Then** if confirmed: the member is removed from members[], all their violations are removed from violations[], and their account is removed from accounts[]
**And** all three .dat files are saved

**Given** BCN selects Delete Member
**When** the system asks for confirmation and BCN cancels
**Then** no data is modified and the menu is shown again

### Story 2.4: View Profile & Member List

As a member,
I want to view my own profile and the CLB member list,
So that I can see my information and know who else is in the club.

**Acceptance Criteria:**

**Given** any logged-in user selects View Profile
**When** the system reads the session's MSSV
**Then** it displays: họ tên, MSSV, email, SĐT, ban, chức vụ, trạng thái (Active/Out CLB)

**Given** any logged-in user selects View Member List
**When** the system displays all members
**Then** only the following fields are shown: họ tên, MSSV, ban, chức vụ
**And** SĐT, email, violation count are NOT displayed for other members
**And** the list is paginated (20 per page, press Enter to continue, 'q' to quit)

---

## Epic 3: Violation Tracking & Enforcement

_Goal: Enable BCN to record and manage violations, members to view their own violations/fines, and the system to auto-enforce Out CLB rules._

### Story 3.1: Record Violation

As a BCN admin,
I want to record a violation for a member,
So that violations are tracked with correct fines and timestamps.

**Acceptance Criteria:**

**Given** BCN selects Record Violation and enters a valid MSSV
**When** they select the violation reason
**Then** the system auto-records the current time (time()) and calculates the fine:
- Member.role=0 → fine=20000
- Member.role=1 or 2 → fine=50000
- REASON_VIOLENCE → fine=0, penalty=PENALTY_OUT_CLB

**Given** the violation reason is REASON_VIOLENCE
**When** the system processes it
**Then** fine=0, penalty=PENALTY_OUT_CLB, the system shows a warning and asks BCN to confirm Out CLB
**And** if confirmed: member.isActive=STATUS_OUT_CLB, member.consecutiveAbsences=0

**Given** the violation reason is REASON_ABSENT
**When** the system records it
**Then** member.consecutiveAbsences is incremented
**And** the system checks Out CLB thresholds automatically

### Story 3.2: Out CLB Warning & Enforcement

As a BCN admin,
I want the system to automatically warn when members approach Out CLB threshold,
So that I can take action before or at the threshold.

**Acceptance Criteria:**

**Given** a member's consecutiveAbsences reaches 3
**When** the violation is recorded
**Then** the system displays "[CANH BAO] Thanh vien [name] da vang 3 buoi lien tiep"

**Given** a member's consecutiveAbsences reaches 4
**When** the violation is recorded
**Then** the system displays "Thanh vien [name] da vang qua 3 buoi lien tiep → Out CLB"
**And** BCN is asked to confirm Out CLB
**And** if confirmed: member.isActive=STATUS_OUT_CLB, members.dat saved

**Given** a member receives a non-absent violation (no jacket, no activity, violence)
**When** the violation is recorded
**Then** member.consecutiveAbsences is reset to 0 (member was present)

### Story 3.3: Mark Paid & View Own Violations/Fines

As a member,
I want to view my own violations and outstanding fines,
So that I know what I owe and my payment status.

**Acceptance Criteria:**

**Given** a member selects View Own Violations
**When** the system reads the session's MSSV
**Then** all violations for that MSSV are displayed: lý do, ngày, mức phạt, trạng thái đã thu/chưa thu
**And** violations of other members are NOT accessible

**Given** a member selects View Own Fines
**When** the system calculates the total
**Then** all unpaid violations (isPaid==0) are listed individually with amounts
**And** a total sum is displayed at the bottom

As a BCN admin,
**Given** BCN selects Mark Paid and chooses a violation
**When** the violation is marked as paid (isPaid=1)
**Then** the member's totalFine is immediately recalculated
**And** violations.dat and members.dat are saved

### Story 3.4: View All Violations with Filters

As a BCN admin,
I want to view all violations with filtering options,
So that I can analyze violations by team, reason, or payment status.

**Acceptance Criteria:**

**Given** BCN selects View All Violations
**When** the system presents a filter sub-menu
**Then** options include: View All, Filter by Ban, Filter by Reason, Filter by Payment Status

**Given** BCN selects a filter (e.g., "Filter by Ban → HR")
**When** the system applies the filter
**Then** only matching violations are displayed with full details
**And** if no violations match, the system shows "Khong co vi phạm nao"

---

## Epic 4: Reports & Advanced Features

_Goal: Provide BCN with comprehensive reporting tools — statistics, sorting, export, and date-based search._

### Story 4.1: Statistics by Team

As a BCN admin,
I want to see fine statistics broken down by team,
So that I can understand which teams have the most violations and outstanding payments.

**Acceptance Criteria:**

**Given** BCN selects Statistics by Team
**When** the system calculates totals
**Then** a table is displayed showing each team (Academic, Planning, HR, Media) with:
- Total fines collected (isPaid==1)
- Total fines outstanding (isPaid==0)
- Grand total

### Story 4.2: Sort Members by Violation Count

As a BCN admin,
I want to sort members by violation count (ascending or descending),
So that I can identify the most/least problematic members.

**Acceptance Criteria:**

**Given** BCN selects Sort by Violations and chooses ascending or descending
**When** the system sorts
**Then** a copy of the member array is sorted (pointer array Member* sorted[MAX_MEMBERS])
**And** the original members[] array order is NOT modified
**And** the sorting algorithm is self-implemented (NOT qsort)
**And** the sorted list is displayed with: họ tên, MSSV, ban, số lần vi phạm

### Story 4.3: Export Report to .txt

As a BCN admin,
I want to export a violation report to a .txt file,
So that I have a permanent record for reporting purposes.

**Acceptance Criteria:**

**Given** BCN selects Export Report
**When** the system generates the report
**Then** a .txt file is created containing:
- Timestamp of export
- Summary by team (collected/outstanding)
- List of members with outstanding fines (name, MSSV, amount owed)
**And** the filename includes the export date/time

### Story 4.4: Search Violations by Date Range

As a BCN admin,
I want to search violations within a date range,
So that I can analyze violations for a specific period.

**Acceptance Criteria:**

**Given** BCN selects Search by Date
**When** they enter start date and end date in dd/mm/yyyy format
**Then** the system uses parse_date() to convert to time_t (start=00:00:00, end=23:59:59)
**And** all violations where violationTime falls within [start, end] are displayed
**And** invalid date format shows "[LOI] Dinh dang ngay khong hop le (dd/mm/yyyy)"
