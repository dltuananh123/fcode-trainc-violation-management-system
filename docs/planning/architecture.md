---
stepsCompleted: [1, 2, 3, 4, 5, 6, 7, 8]
inputDocuments:
  - requirement-docs/QuanLyViPhamCLBFCode_V1.md
  - requirement-docs/Rule_TrainC.md
workflowType: 'architecture'
project_name: 'fcode trainc'
user_name: 'Admin'
date: '2026-04-18'
lastStep: 8
status: 'complete'
completedAt: '2026-04-18'
---

# Architecture Decision Document

_This document builds collaboratively through step-by-step discovery. Sections are appended as we work through each architectural decision together._

## Project Context Analysis

### Requirements Overview

**Functional Requirements (18 total):**

| Group | Features | Priority |
|---|---|---|
| Group 0 — Authentication | Login, Logout, Change Password | 3 required |
| Group 1 — Member Role | View profile, View own violations, View own fines, View member list | 4 required |
| Group 2 — Admin Role | Add/Edit/Delete members, Record violation, Mark paid, View all violations, Statistics by team, Out CLB warning, Sort by violations, Export report, Search by date | 8 required + 2 advanced + 1 optional |

**Non-Functional Requirements:**

- **Data durability**: Save to file after every mutation — zero data loss on crash/exit
- **Capacity**: Max 1,000 members at any time
- **Security**: 3 failed login attempts = account lock; role-based menu access
- **Portability**: 100% standard C only — must compile with gcc/mingw on any platform
- **No global variables** unless absolutely necessary (scoring criterion)

**Scale & Complexity:**

- Primary domain: CLI / Systems Programming
- Complexity level: Low-Medium
- Estimated architectural components: 8 modules (main, types, auth, member, violation, fileio, report, utils)
- Data entities: 3 structs (Member, Violation, Account)

### Technical Constraints & Dependencies

- Language: C17 (`-std=c17`), standard library only
- No dynamic linking, no threads, no networking
- File-based persistence (.dat files in `data/` directory)
- Suggested compiler: gcc/MinGW
- Input: keyboard only; Output: terminal (stdout)

### Cross-Cutting Concerns Identified

1. **Authentication & Authorization** — every operation must verify role before executing
2. **Data Persistence** — all CRUD operations must immediately flush to disk
3. **Input Validation** — MSSV uniqueness, email format, phone format, menu bounds
4. **Auto-calculation** — fine amounts depend on member role; consecutive absences tracked across violations
5. **Business Rules Enforcement** — violence = Out CLB, 3 consecutive absences = Out CLB, fine rates by role

## Starter Template Evaluation

### Primary Technology Domain

**CLI / Systems Programming** — Pure C terminal application, no frameworks, no external dependencies.

### Selected Approach

**Build System:** Makefile — industry standard for C projects, compiles only changed files (`make`, `make clean`, `make run`).

**File Format:** Binary (.dat) — compact, fast read/write using `fwrite`/`fread`, struct-level I/O. Each struct is written as a fixed-size record.

**Directory Structure (Modular — src/include/build/bin):**
```
fcode-trainc/
├── Makefile
├── .clang-format            # Auto-format config
├── include/                 # All header files
│   ├── types.h              # All struct definitions + constants (no circular deps)
│   ├── auth.h
│   ├── member.h
│   ├── violation.h
│   ├── fileio.h
│   ├── report.h
│   └── utils.h
├── src/                     # All source files
│   ├── main.c               # Entry point, menu dispatch, main loop
│   ├── auth.c
│   ├── member.c
│   ├── violation.c
│   ├── fileio.c
│   ├── report.c
│   └── utils.c
├── build/                   # Object files (.o) — gitignored
├── bin/                     # Compiled executable — gitignored
├── data/
│   ├── members.dat
│   ├── violations.dat
│   └── accounts.dat
└── requirement-docs/                   # Requirements documents (reference only)
```

### Architectural Decisions

**Language & Compiler:**
- C17 standard (`-std=c17 -m64`), compiled with gcc/MinGW (default, for evaluation compatibility)
- 64-bit target ensures fixed `size_t` and `time_t` (8 bytes on all platforms)
- Development: `make CC=clang` — better error messages, faster compilation
- Warning flags: `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wformat-nonliteral -Wformat-security -Wfloat-equal -Wundef -Wsign-conversion -Wcast-align -Wcast-qual -Wmissing-prototypes -Wmissing-declarations -Wunreachable-code -Wnull-dereference -Wimplicit-fallthrough -Wswitch-enum -Wpointer-arith`

**Code Quality Toolchain (LLVM):**
- **clang-format** — auto-format all `.c`/`.h` files before commit, ensures team-wide style consistency
- **clang-tidy** — static analysis with checks: `bugprone-*`, `readability-*`, `performance-*` (no `modernize-*` — this is pure C, not C++)
- Run `clang-format -i src/*.c include/*.h` before each commit
- Run `clang-tidy src/*.c -- -std=c17 -Iinclude` before final submission

**Code Organization:**
- Each module = one `.h` in `include/` (declarations) + one `.c` in `src/` (implementations)
- Header guards (`#ifndef`) on every `.h` file
- `src/main.c` contains only menu dispatch and main loop

**Binary File I/O Strategy:**
- Structs written directly to binary files using `fwrite`/`fread`
- Fixed-size records for predictable seek/read operations
- File open → write → close on every mutation (immediate persistence)
- On startup: load all records into memory arrays
- No struct padding issues: all string fields use fixed-size `char` arrays (e.g. `char fullName[MAX_NAME_LEN]`). Do NOT use `#pragma pack(1)` — it is non-standard and causes unaligned memory access on some platforms.

**Development Experience:**
- `make` — build all (default: gcc), output to `bin/fcode-trainc`
- `make CC=clang` — build with clang for better diagnostics
- `make clean` — remove build/ and bin/ directories
- `make run` — build and run
- `make format` — run clang-format on all source files
- `make tidy` — run clang-tidy static analysis

**Makefile Structure (key variables):**
```makefile
CC      = gcc
CFLAGS  = -std=c17 -m64 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wformat-nonliteral -Wformat-security -Wfloat-equal -Wundef -Wsign-conversion -Wcast-align -Wcast-qual -Wmissing-prototypes -Wmissing-declarations -Wunreachable-code -Wnull-dereference -Wimplicit-fallthrough -Wswitch-enum -Wpointer-arith -Iinclude
SRCDIR  = src
INCDIR  = include
BUILDDIR = build
BINDIR  = bin
TARGET  = $(BINDIR)/fcode-trainc
SRCS    = $(wildcard $(SRCDIR)/*.c)
OBJS    = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))
```

## Core Architectural Decisions

### Decision Priority Analysis

**Critical Decisions (Block Implementation):**
1. In-memory data management — static arrays
2. Violations capacity — 10,000 fixed max
3. File corruption handling — error + exit

**Important Decisions (Shape Architecture):**
4. Violation reason representation — int with `#define` constants
5. Consecutive absence tracking — counter in Member struct

### Data Architecture

**In-Memory Storage — AppDatabase struct:**
All arrays and counts are wrapped in a single struct defined in `types.h`:

```c
typedef struct {
    Member members[MAX_MEMBERS];
    int memberCount;
    Violation violations[MAX_VIOLATIONS];
    int violationCount;
    Account accounts[MAX_MEMBERS];
    int accountCount;
} AppDatabase;
```

**Rationale:** Eliminates parameter bloat. Instead of passing 7 parameters to every function (members[], *memberCount, violations[], *violationCount, accounts[], *accountCount), every module function receives a single `AppDatabase *db`. Cleaner, fewer bugs, easier to maintain.

**Member Status Constants:**
```c
#define STATUS_ACTIVE   1
#define STATUS_OUT_CLB  0
```

Member struct includes `isActive` field (int). Set to `STATUS_OUT_CLB` when Out CLB is confirmed (violence or >= 3 consecutive absences). Requirement 1.1 displays "trang thai thanh vien" using this field.

**Rationale:** Static arrays eliminate malloc/free complexity, prevent memory leaks, and match the spec's hard cap of 1,000 members. Predictable memory footprint (~few MB max).

**Violation Reason Constants:**
```c
#define REASON_NO_JACKET   0
#define REASON_ABSENT      1
#define REASON_NO_ACTIVITY 2
#define REASON_VIOLENCE    3
```

**Team Constants:**
```c
#define TEAM_ACADEMIC  0
#define TEAM_PLANNING  1
#define TEAM_HR        2
#define TEAM_MEDIA     3
```

**Penalty Constants:**
```c
#define PENALTY_FINE     0
#define PENALTY_OUT_CLB  1
```

**Account Role Constants (menu access):**
```c
#define ACCOUNT_ROLE_MEMBER 0
#define ACCOUNT_ROLE_BCN    1
```

**Member Role Constants (fine calculation):**
```c
#define MEMBER_ROLE_MEMBER     0   /* Phạt 20.000đ/lần */
#define MEMBER_ROLE_LEADER     1   /* Phạt 50.000đ/lần */
#define MEMBER_ROLE_BCN        2   /* Phạt 50.000đ/lần */
```

**Struct Field Sizes:**
```c
#define MAX_NAME_LEN    50
#define MAX_MSSV_LEN    10
#define MAX_EMAIL_LEN   50
#define MAX_PHONE_LEN   15
#define MAX_NOTE_LEN    100
#define MAX_PASS_LEN    20
```

**Consecutive Absence Tracking:**
- `consecutiveAbsences` field in Member struct, incremented on `REASON_ABSENT`
- Reset to 0 on any non-absent violation (the member was present at that event, so streak breaks)
- Auto-check: if `consecutiveAbsences >= 3` → warn, if `>= 4` → prompt Out CLB
- **Rationale**: Spec says "Vắng họp **quá 3 buổi** liên tiếp không phép sẽ Out CLB" — "quá 3" means more than 3, i.e. from the 4th absence onward. This matches the literal wording.
- **Design decision**: The spec does not explicitly define when the counter resets. The architecture resets it on any non-absent violation type (e.g. no jacket, no activity) — this means the member was physically present, so the absence streak should break. This is an intentional interpretation.
- **Edge case — deleting violations**: The spec does not provide a function to delete individual violations (only delete member which cascades all violations). Therefore `consecutiveAbsences` is never recalculated on violation deletion — it only increments on `REASON_ABSENT` recording and resets on non-absent violation recording.

**totalFine Update Rules:**
- `totalFine` is recalculated (not manually tracked) whenever:
  1. A new violation is recorded → add fine to totalFine
  2. A violation is marked as paid (`isPaid` = 0→1) → subtract fine from totalFine
  3. Member role is changed → recalculate all unpaid fines, then recalculate totalFine
- Recalculation logic: `totalFine = sum of violations[i].fine where studentId matches AND isPaid == 0`

### File I/O Architecture

**Binary Format (.dat):**
- `members.dat` — array of `Member` structs (fixed-size records)
- `violations.dat` — array of `Violation` structs (fixed-size records)
- `accounts.dat` — array of `Account` structs (fixed-size records)
- First 4 bytes of each file: `int` record count (header)

**Read/Write Strategy:**
- On startup: read count from header → `fread` all records into arrays
- On mutation: use **atomic write** pattern to prevent data loss on crash:
  1. `fopen("filename.dat.tmp", "wb")` — write to temp file
  2. `fwrite` count header + all records
  3. `fclose` the temp file
  4. `remove("filename.dat")` — delete old file
  5. `rename("filename.dat.tmp", "filename.dat")` — atomically replace
- Always rewrite entire file to prevent corruption
- On startup: if `.tmp` files exist (crash residue), delete them and use `.dat` files
- **Rationale**: If the program crashes during step 2–3, the original `.dat` file is untouched. Only after successful write + close does the temp replace the original. This prevents catastrophic data loss on power failure or segfault.

**File Corruption Handling:**
- On startup: if `.dat` files do not exist → create empty files with header (record count = 0)
- If files exist but cannot be read (format corruption) → print error, exit
- First-run initialization is handled automatically in `fileio_load_all()`

**First-Run Admin Account (Implementation Decision):**
> **Note:** This is an implementation decision by the team, not specified in the requirements. Documented here for transparency and to support testing/grading.

When `accounts.dat` is empty (record count = 0), `fileio_load_all()` creates a default BCN account:
- MSSV: `ADMIN`
- Password: `ADMIN`
- Role: `ACCOUNT_ROLE_BCN` (1)
- isLocked: 0, failCount: 0

This ensures at least one admin can login on first run. BCN should create real member accounts and change the default password immediately.

### Authentication & Security

**Two Role Systems (IMPORTANT — do not confuse):**

| Field | Struct | Purpose | Values |
|---|---|---|---|
| `Account.role` | Account | Menu access control | 0=Member, 1=BCN |
| `Member.role` | Member | Fine calculation | 0=Member (20K), 1=Leader/Vice (50K), 2=BCN (50K) |

`Account.role` determines what menu the user sees after login.
`Member.role` determines the fine amount when recording a violation.
These are separate fields serving different purposes.

**Leader/Vice role mapping:**
- Leader/Vice has `Member.role = MEMBER_ROLE_LEADER (1)` → fined 50,000đ/violation
- Leader/Vice has `Account.role = ACCOUNT_ROLE_MEMBER (0)` → sees Member menu only
- They are NOT BCN — they cannot add/edit/delete members or record violations
- Only difference from regular Member: higher fine rate (50K vs 20K)

**Login Flow:**
1. Prompt MSSV + password
2. Scan `accounts[]` for matching `studentId`
3. If found: check `isLocked`, then compare `password`
4. On success: reset `failCount`, set session, load role-based menu
5. On failure: increment `failCount`; if `failCount >= 3` → set `isLocked = 1`, save to file, exit

**Session Management:**
- `currentSession` is a `static Account` variable inside `src/auth.c` — not accessible by other modules directly
- Other modules access session via getter: `Account* auth_get_session(void);`
- `auth_get_session()->role == ACCOUNT_ROLE_MEMBER` → Member menu
- `auth_get_session()->role == ACCOUNT_ROLE_BCN` → Admin menu
- This eliminates the need for `extern` global variable, satisfying scoring criterion 5.2 ("no unnecessary global variables")

**Password:**
- Plain text (as per spec)
- Default password = MSSV (set when BCN creates member)
- BCN can reset any member's password

### Error Handling Strategy

**Input Validation:**
- MSSV: check format, check uniqueness on add
- Email: validate `@` and `.` presence
- Menu choice: bounds checking on all `scanf` inputs
- All string inputs: `fgets` + strip newline (avoid `scanf` buffer overflow)

**Error Reporting:**
- Clear Vietnamese messages to terminal: "Loi: MSSV da ton tai!"
- Consistent prefix format: `[LOI]` for errors, `[CANH BAO]` for warnings

### Module Responsibility Matrix

| Module | Owns | Reads From | Writes To |
|---|---|---|---|
| `types` | Struct definitions, constants | — | — |
| `auth` | Account, session | accounts.dat | accounts.dat |
| `member` | Member CRUD | members.dat | members.dat |
| `violation` | Violation CRUD, Out CLB logic | violations.dat, members.dat | violations.dat, members.dat |
| `fileio` | Load/Save all data | all .dat files | all .dat files |
| `report` | Statistics, export | violations.dat, members.dat | report .txt files |
| `utils` | Validation helpers, time formatting, date parsing | — | — |
| `main` | Menu dispatch, main loop | — | — |

### Implementation Sequence

1. `include/types.h` — all structs + constants (no implementation, declarations only)
2. `utils` — helpers used everywhere
3. `fileio` — load/save foundation
4. `auth` — login before any other operation
5. `member` — CRUD for members + accounts
6. `violation` — core business logic
7. `report` — statistics and export
8. `main` — wire everything together

## Implementation Patterns & Consistency Rules

### Critical Conflict Points Identified

6 areas where different developers could make inconsistent choices: naming, struct layout, file headers, I/O patterns, error handling, menu display.

### Naming Patterns

**Functions:**
- Format: `moduleName_actionNoun` — lowercase + underscore
- Examples: `member_add()`, `member_find_by_id()`, `auth_login()`, `violation_record()`, `fileio_save_members()`
- Boolean-returning functions: prefix `is_` or `has_` — `is_email_valid()`, `auth_is_locked()`

**Variables:**
- `snake_case` — `member_count`, `current_session`, `total_fine`
- Loop counters: `i`, `j`, `k` (short, conventional)
- Constants: `UPPER_SNAKE_CASE` — `MAX_MEMBERS`, `REASON_ABSENT`

**Structs:**
- `PascalCase` for type names — `Member`, `Violation`, `Account`
- `typedef struct` — always typedef for cleaner declarations

**File names:**
- `lowercase.h` / `lowercase.c` — `auth.h`, `member.h`, `violation.h`

### Structure Patterns

**Header File Template (.h):**
```c
#ifndef MODULE_NAME_H
#define MODULE_NAME_H

#include <standard headers>

#include "types.h"           /* Always first — all structs + constants */
#include "project headers"   /* utils.h, etc. — resolved via -Iinclude */

int function_declarations(void);

#endif
```

**Include Order:**
1. Standard C headers (`<stdio.h>`, `<string.h>`, etc.)
2. Project headers (`"member.h"`, `"utils.h"`, etc. — resolved via `-Iinclude`)

**Rules:**
- `.h` files (in `include/`): ONLY declarations (structs, typedefs, function prototypes, constants)
- `.c` files (in `src/`): implementations + `#include` own `.h` first
- No `#include <conio.h>` — not standard C (no `getch()`, `clrscr()`)

### Format Patterns

**Menu Display:**
```
=== TEN MENU ===
1. Lua chon mot
2. Lua chon hai
0. Quay lai
Nhap lua chon: _
```
- Vietnamese labels, English code
- Always include `0. Quay lai` / `0. Thoat` option
- Use `===` for menu title borders

**Table Display:**
```
+------+------------------+--------+
| MSSV | Ho va ten        | Ban    |
+------+------------------+--------+
| SE01 | Nguyen Van A     | Aca    |
+------+------------------+--------+
```
- Fixed-width columns using `%-Ns` format specifiers

**Input Handling:**
- Always use `fgets(buffer, size, stdin)` for strings — never `scanf("%s")`
- Strip trailing `\n` from `fgets`: `buffer[strcspn(buffer, "\n")] = '\0'`
- For int menu choices: `scanf("%d", &choice)` + `while(getchar() != '\n')` to flush

### Error Handling Patterns

**Return Codes:**
- Functions return `int`: `0` = success, `-1` = failure
- Or return index/pointer on success, `-1`/`NULL` on failure

**Error Messages:**
- Format: `[LOI] <message in Vietnamese>`
- Warning: `[CANH BAO] <message>`
- Success: `[OK] <message>`
- Example: `[LOI] MSSV da ton tai!`, `[OK] Them thanh vien thanh cong.`

**Validation:**
- Validate BEFORE modifying any data
- Print specific error message for each validation failure
- Always ask for confirmation before destructive operations (delete member)

### Process Patterns

**CRUD Operation Flow:**
1. Validate input
2. Check business rules (uniqueness, constraints)
3. Modify in-memory array
4. Save to file immediately
5. Print success message

**Pagination for Long Lists:**
- Display 20 items per page
- Prompt: "Nhan Enter de xem tiep, 'q' de quay lai"
- Use `getchar()` to wait for user input

### Enforcement Guidelines

**All Developers MUST:**
- Follow `moduleName_functionName` naming for all public functions
- Use `fgets` for all string input (never bare `scanf`)
- Save to file after EVERY mutation operation
- Include header guards in every `.h` file
- Use Vietnamese for terminal output, English for code
- Keep functions under 50 lines — split if longer
- Run `make format` before every commit

**Pattern Enforcement:**
- Compile with full warning flags — zero warnings allowed
- Run `clang-format -i src/*.c include/*.h` before push — no manual style debates
- Each member commits to their assigned module(s)
- Code review before merge to main branch

### Pattern Examples

**Good:**
```c
int member_find_by_id(AppDatabase *db, const char *studentId) {
    for (int i = 0; i < db->memberCount; i++) {
        if (strcmp(db->members[i].studentId, studentId) == 0) return i;
    }
    return -1;
}
```

**Anti-Patterns:**
- `scanf("%s", members[i].studentId)` — buffer overflow risk
- Global variables scattered across `.c` files without `extern`
- `void main()` — always use `int main()`
- Mixing Vietnamese and English in variable/function names

## Project Structure & Boundaries

### Complete Project Directory Structure

```
fcode-trainc/
├── Makefile
├── .clang-format
├── include/                      # All header files
│   ├── types.h                   # All struct definitions (Member, Violation, Account) + constants
│   ├── auth.h                    # Auth function prototypes
│   ├── member.h                  # Member CRUD prototypes
│   ├── violation.h               # Violation CRUD + business logic prototypes
│   ├── fileio.h                  # Load/save function prototypes
│   ├── report.h                  # Statistics & export prototypes
│   └── utils.h                   # Validation helpers, time formatting, date parsing
├── src/                          # All source files
│   ├── main.c                    # Entry point, menu dispatch, main loop
│   ├── auth.c                    # Login, logout, change password, session
│   ├── member.c                  # Add, edit, delete, find, list members
│   ├── violation.c               # Record, mark paid, Out CLB, search
│   ├── fileio.c                  # Binary read/write for all .dat files
│   ├── report.c                  # Stats by team, export .txt, sort
│   └── utils.c                   # is_email_valid, format_time, parse_date, input helpers
├── build/                        # Object files (.o) — gitignored
├── bin/                          # Compiled executable — gitignored
├── data/                         # Binary data files (gitignored or seeded)
│   ├── members.dat               # Member records (binary)
│   ├── violations.dat            # Violation records (binary)
│   └── accounts.dat              # Account/login records (binary)
└── requirement-docs/                        # Requirements documents (reference only)
```

### Architectural Boundaries

**Module Dependencies (who includes whom):**

```
include/types.h              (leaf — all structs + constants, no other includes)
  ↑
include/utils.h              (depends on types.h only)
  ↑
include/fileio.h             (depends on types.h only — knows struct layouts for fread/fwrite)
  ↑
include/auth.h               (depends on types.h, utils.h, fileio.h)
include/member.h             (depends on types.h, utils.h, fileio.h)
include/violation.h          (depends on types.h, utils.h, member.h, fileio.h)
include/report.h             (depends on types.h, utils.h, member.h, violation.h)
  ↑
src/main.c                   (includes all headers)
```

**Circular Dependency Prevention:**
`include/types.h` contains ONLY struct definitions (Member, Violation, Account) and constants (#define). It does NOT include any other project header. It includes only `<time.h>` (needed for `time_t violationTime` field in Violation struct). All other modules include `types.h` first, ensuring no circular includes.

**Dependency Rules:**
- `types` depends on nothing (leaf module — only standard C headers)
- `utils` depends on `types` only
- `fileio` depends on `types` only (needs struct layouts for binary I/O)
- `auth` depends on `types`, `utils`, `fileio`
- `member` depends on `types`, `utils`, `fileio`
- `violation` depends on `types`, `utils`, `member`, `fileio` (saves violations + members after recording)
- `report` depends on `types`, `utils`, `member`, `violation` (reads from arrays only, does NOT need fileio — uses fopen/fclose directly for .txt export)
- `main` depends on all modules

### Requirements to Structure Mapping

| Requirement | File(s) | Key Functions | Business Rules |
|---|---|---|---|
| 0.1 Login | src/auth.c | `auth_login()` | 3 sai → khóa tài khoản + thoát |
| 0.2 Logout | src/auth.c | `auth_logout()` | Quay về đăng nhập, không thoát chương trình |
| 0.3 Change password | src/auth.c | `auth_change_password()`, `auth_reset_password()` | Cần nhập mật khẩu cũ. BCN reset được bất kỳ |
| 1.1 View profile | src/member.c | `member_view_profile()` | Xem đủ trường, không được sửa |
| 1.2 View own violations | src/violation.c | `violation_view_own()` | Chỉ xem của mình, không xem người khác |
| 1.3 View own fines | src/violation.c | `violation_view_fines()` | Tính tổng isPaid==0, hiện từng khoản + tổng |
| 1.4 View member list | src/member.c | `member_list_all()` | Chỉ hiện: họ tên, MSSV, ban, chức vụ. Ẩn SĐT, email, vi phạm |
| 2.1 Add member | src/member.c | `member_add()` | BCN nhập: họ tên, MSSV, email, SĐT, **ban (team)**, chức vụ, mật khẩu mặc định. Validate MSSV không trùng, email hợp lệ. Tự tạo Account (mật khẩu=MSSV) |
| 2.2 Edit member | src/member.c | `member_edit()` | Sửa được tất cả **trừ MSSV**. Nếu đổi role → quét violations[] recalculate fine cho tất cả isPaid==0, cập nhật totalFine |
| 2.3 Delete member | src/member.c | `member_delete()` | Cascade: xóa thành viên + vi phạm + tài khoản. **Yêu cầu xác nhận** trước khi xóa |
| 2.4 Record violation | src/violation.c | `violation_record()` | Tự ghi thời gian, tính tiền phạt theo Member.role. **Bạo lực**: fine=0, penalty=OUT_CLB, cảnh báo + confirm Out CLB ngay |
| 2.5 Mark paid | src/violation.c | `violation_mark_paid()` | Chọn từng vi phạm → isPaid=1. **Cập nhật totalFine ngay lập tức** |
| 2.6 View all violations | src/violation.c | `violation_list_all()` | Hiển thị toàn bộ vi phạm. **Lọc theo**: ban (team), lý do vi phạm (reason), trạng thái thu tiền (isPaid). User chọn bộ lọc từ sub-menu trước khi hiện danh sách |
| 2.7 Stats by team | src/report.c | `report_team_stats()` | Tổng đã thu / chưa thu từng ban. Hiển thị dạng bảng |
| 2.8 Out CLB warning | src/violation.c | `violation_check_out_clb()` | consecutiveAbsences >= 3 → cảnh báo. >= 4 → thông báo Out CLB, BCN xác nhận |
| 2.9 Sort by violations | src/report.c | `report_sort_by_violations()` | Tăng/giảm. Tự cài thuật toán, **không dùng qsort**. **Phải sort trên mảng copy** (pointer array `Member* sorted[MAX_MEMBERS]`), không sửa thứ tự mảng `members[]` gốc |
| 2.10 Export report | src/report.c | `report_export_txt()` | Xuất file .txt: timestamp, tổng hợp theo ban, danh sách nợ tiền phạt |
| 2.11 Search by date | src/violation.c | `violation_search_by_date()` | Nhập ngày bắt đầu - kết thúc (dd/mm/yyyy), parse sang `time_t` bằng `strptime` hoặc `sscanf` + `mktime`, lọc `violationTime` trong khoảng `[start, end]` |

### Cross-Cutting Concerns Mapping

| Concern | Where | How |
|---|---|---|
| Struct definitions | include/types.h | All 3 structs (Member, Violation, Account) + all constants (#define) in one place |
| Input validation | src/utils.c | `is_email_valid()`, `is_id_valid()`, `read_string()` |
| File persistence | src/fileio.c | `fileio_load_*()`, `fileio_save_*()` — called after every mutation |
| Session tracking | src/auth.c | `static Account currentSession` with `auth_get_session()` getter — checked by menu functions |
| Error messages | All modules | `[LOI]`, `[CANH BAO]`, `[OK]` prefix format |
| Time formatting | src/utils.c | `format_time()` — converts `time_t` to readable string ("dd/mm/yyyy HH:MM") |
| Date parsing | src/utils.c | `parse_date()` — converts user input "dd/mm/yyyy" to `time_t` for range queries. Uses `sscanf` to extract day/month/year into `struct tm`, then `mktime()` to produce `time_t`. Start date normalized to 00:00:00, end date to 23:59:59 |
| Display name mapping | src/utils.c | `team_name()`, `member_role_name()`, `account_role_name()`, `reason_name()` — converts int to Vietnamese display string |
| Violation filtering | src/violation.c | `violation_list_all()` — sub-menu cho user chọn lọc: tất cả / theo ban / theo lý do / theo trạng thái thu tiền. Apply filter rồi hiển thị kết quả |

### Integration Points

**Data Flow — Startup:**
```
main() → fileio_load_all() → members[], violations[], accounts[]
       → auth_login() → currentSession
       → menu loop
```

**Data Flow — Mutation (e.g. Record Violation):**
```
violation_record()
  → member_find_by_id()          (find member)
  → calculate fine by Member.role:
      Member.role=0 → fine=20000
      Member.role=1,2 → fine=50000
  → if reason == REASON_VIOLENCE:
      fine = 0
      penalty = PENALTY_OUT_CLB
      confirm Out CLB with BCN
  → else:
      penalty = PENALTY_FINE
  → add to violations[]          (in-memory)
  → member.violationCount++      (update member)
  → if reason == REASON_ABSENT:
      member.consecutiveAbsences++
      check Out CLB threshold
  → else:
      member.consecutiveAbsences = 0
  → fileio_save_violations()     (persist)
  → fileio_save_members()        (persist)
```

**Data Flow — Data Files:**
```
accounts.dat ←→ accounts[]     (1:1 with members)
members.dat  ←→ members[]      (1000 max)
violations.dat ←→ violations[] (10000 max)
```

**Data Flow — Edit Member (role change):**
```
member_edit()
  → update member fields (all except MSSV)
  → if role changed:
      scan violations[] for matching studentId where isPaid == 0
      for each match: recalculate fine based on new Member.role
      recalculate member.totalFine = sum of all isPaid==0 violations
      → fileio_save_violations()     (persist updated fines)
      → fileio_save_members()        (persist updated totalFine)
  → else:
      → fileio_save_members()        (persist normal edits)
```

**Data Flow — Delete Member:**
```
member_delete()
  → ask confirmation
  → if confirmed:
      remove from members[]           (shift array)
      remove all matching violations[] (shift array, update violationCount)
      remove matching from accounts[]  (shift array, update accountCount)
      → fileio_save_members()
      → fileio_save_violations()
      → fileio_save_accounts()
```

**Data Flow — Logout:**
```
auth_logout()
  → clear currentSession (set to zeroed struct)
  → loop back to auth_login() — do NOT exit program
```

**Data Flow — Mark Paid (with totalFine update):**
```
violation_mark_paid()
  → select violation, set isPaid = 1
  → recalculate member.totalFine = sum of all isPaid==0 violations for that member
  → fileio_save_violations()
  → fileio_save_members()
```

**Data Flow — Search by Date (2.11):**
```
violation_search_by_date()
  → prompt user: "Ngay bat dau (dd/mm/yyyy): " → read into char buffer
  → parse_date(buffer, &startDate)
      → sscanf(buf, "%d/%d/%d", &tm.tm_mday, &tm.tm_mon, &tm.tm_year)
      → tm.tm_hour = 0, tm.tm_min = 0, tm.tm_sec = 0  (start of day)
      → tm.tm_mon -= 1  (months 0-11)
      → tm.tm_year -= 1900
      → startDate = mktime(&tm)
  → prompt user: "Ngay ket thuc (dd/mm/yyyy): " → read into char buffer
  → parse_date(buffer, &endDate)
      → same parse, but tm.tm_hour = 23, tm.tm_min = 59, tm.tm_sec = 59  (end of day)
  → loop violations[]: if violation.violationTime >= startDate && <= endDate → display
  → no file I/O needed (read-only from memory)
```

### Shared Data Access

Arrays are declared in `src/main.c` and passed to modules via function parameters. No global variables for data arrays. Session is encapsulated in `src/auth.c` as `static` with a getter function — no `extern` needed.

```c
/* src/main.c — single instance */
AppDatabase db = {0};  /* Zero-initialized: all counts = 0 */
```

All module functions receive `AppDatabase *db` as single parameter:

```c
/* Example signature in include/member.h */
int member_add(AppDatabase *db);
int member_delete(AppDatabase *db, const char *studentId);
```

## Architecture Validation Results

### Coherence Validation ✅

**Decision Compatibility:** All decisions work together — static arrays + binary files + C17 + Makefile + module structure are fully compatible. No contradictions.

**Pattern Consistency:** Naming (`moduleName_action`), header guards, `fgets` input, error prefixes (`[LOI]`/`[OK]`) are consistent across all modules.

**Structure Alignment:** Module dependency graph is a clean DAG (no circular dependencies). `types` → `utils` → `fileio` → `auth`/`member` → `violation` → `report` → `main`. All structs centralized in `include/types.h` prevents circular include issues. `src/` and `include/` separation provides clean source/header isolation.

### Requirements Coverage Validation ✅

All 18 functional requirements mapped to specific functions:

- Group 0 (3/3): src/auth.c covers login, logout, change/reset password ✅
- Group 1 (4/4): src/member.c + src/violation.c covers profile, own violations, fines, member list ✅
- Group 2 (11/11): all admin features mapped to src/member.c, src/violation.c, src/report.c ✅

### Validation Issues Resolved

**Issue 1 — Initial Data Setup (RESOLVED):**
First-run initialization path: if `.dat` files do not exist → create empty files with header (record count = 0). Only exit if files exist but cannot be read (format corruption). Implemented in `fileio_load_all()`.

**Issue 2 — Fine Calculation Strategy (RESOLVED):**
Fine amount is calculated at the time of violation recording based on the member's role at that moment (Member=20K, Leader/Vice/BCN=50K). However, when BCN changes a member's role (requirement 2.2), the system **must recalculate fines** for all unpaid violations (`isPaid == 0`) belonging to that member. This is required by the spec. The recalculation scans `violations[]` for matching `studentId` where `isPaid == 0` and updates `fine` to the new role's rate. `totalFine` in Member struct is recalculated after this update.

### Architecture Completeness Checklist

**✅ Requirements Analysis**
- [x] Project context thoroughly analyzed
- [x] Scale and complexity assessed
- [x] Technical constraints identified
- [x] Cross-cutting concerns mapped

**✅ Architectural Decisions**
- [x] Data architecture (static arrays, binary files)
- [x] Authentication & security (login, session, locking)
- [x] Error handling strategy (return codes, message format)
- [x] Module responsibility matrix

**✅ Implementation Patterns**
- [x] Naming conventions established
- [x] Header file template defined
- [x] Input handling patterns (fgets, scanf flush)
- [x] Error message format ([LOI], [CANH BAO], [OK])

**✅ Project Structure**
- [x] Complete directory structure defined
- [x] Module dependency graph established
- [x] All 18 FRs mapped to specific functions
- [x] Data flow documented

### Architecture Readiness Assessment

**Overall Status:** READY FOR IMPLEMENTATION

**Confidence Level:** High

**Key Strengths:**
- Requirements are clear and well-defined by the spec
- Architecture matches the suggested structure in the spec (scoring alignment)
- No complex tech decisions (pure C, no frameworks)
- Fine recalculation on role change — matches spec requirement 2.2
- Clear implementation sequence defined

### Implementation Handoff

**First Implementation Priority:**
1. Create project structure (`src/`, `include/`, `build/`, `bin/`, `data/`) + Makefile
2. Define `include/types.h` (all structs + constants)
3. Implement `include/utils.h` / `src/utils.c` (helpers)
4. Implement `include/fileio.h` / `src/fileio.c` (load/save with first-run init)
5. Then follow the implementation sequence: auth → member → violation → report → main

## Q&A Defense Prep Notes

Known spec ambiguities that may come up during viva (Requirement 5.5):

**1. "Quá 3 buổi" vs requirement 2.8 "= 3":**
- Spec line 1.1 says "Vắng họp **quá 3 buổi** liên tiếp" → literally "more than 3" = from 4th absence
- But requirement 2.8 says "consecutiveAbsences = 3 → thông báo Out CLB"
- Architecture chose `>= 4` to match "quá 3" wording from 1.1
- **Recommended answer if asked**: "Nhóm em nhận thấy có sự bất nhất giữa 2 dòng yêu cầu. Dòng tổng quát nói 'quá 3 buổi' nên nhóm chốt Out CLB từ lần vắng thứ 4, đảm bảo tuân thủ đúng nghĩa đen của từ 'quá'"

**2. Why Leader/Vice sees Member menu:**
- Spec only defines 2 menu roles: Member and BCN. Leader/Vice is not a separate access level
- Leader/Vice only differs in fine rate (50K vs 20K), not in system access
- **Recommended answer**: "Leader/Vice khác Member ở mức phạt (50K thay vì 20K) nhưng quyền truy cập hệ thống giống Member — đề bài chỉ phân 2 role menu: Member và BCN"

## Test Plan

### Critical Path Test Cases

| ID | Flow | Input | Expected Result |
|---|---|---|---|
| **Authentication** | | | |
| T01 | Login đúng | MSSV + đúng password | Vào menu theo role |
| T02 | Login sai 1 lần | Sai password | Thông báo sai, còn 2 lần |
| T03 | Login sai 3 lần | Sai password 3 lần liên tiếp | Khóa tài khoản, thoát |
| T04 | Login tài khoản bị khóa | MSSV đã bị khóa | Thông báo bị khóa, từ chối |
| T05 | Logout | Chọn logout | Quay về màn hình đăng nhập |
| T06 | Đổi mật khẩu | Nhập đúng old pass + new pass | Mật khẩu cập nhật |
| T07 | Đổi mật khẩu sai | Sai old pass | [LOI] Mật khẩu cũ không đúng |
| **Member CRUD** | | | |
| T08 | Thêm member mới | Đầy đủ thông tin, MSSV chưa tồn tại | Thêm thành công + tự tạo Account |
| T09 | Thêm trùng MSSV | MSSV đã tồn tại | [LOI] MSSV đã tồn tại |
| T10 | Thêm email sai | Email không có @ | [LOI] Email không hợp lệ |
| T11 | Sửa member | Sửa tên, ban, chức vụ | Cập nhật thành công |
| T12 | Sửa MSSV | Thử sửa MSSV | MSSV không đổi (read-only) |
| T13 | Xóa member | Chọn member + xác nhận | Xóa member + violations + account |
| T14 | Xóa member hủy | Chọn member + hủy | Không xóa, quay lại |
| **Violation** | | | |
| T15 | Ghi vi phạm Member | Member vi phạm vắng họp | fine=20000, penalty=0 |
| T16 | Ghi vi phạm Leader | Leader vi phạm vắng họp | fine=50000, penalty=0 |
| T17 | Ghi vi phạm bạo lực | Member vi phạm bạo lực | fine=0, penalty=1, confirm Out CLB |
| T18 | Vắng 2 lần liên tiếp | 2 vi phạm vắng họp liên tiếp | Không cảnh báo |
| T19 | Vắng 3 lần liên tiếp | 3 vi phạm vắng họp liên tiếp | [CANH BAO] Cảnh báo |
| T20 | Vắng 4 lần liên tiếp | 4 vi phạm vắng họp liên tiếp | Thông báo Out CLB, BCN xác nhận |
| T21 | Đánh dấu đã thu | Chọn vi phạm, mark paid | isPaid=1, totalFine giảm |
| **Report** | | | |
| T22 | Thống kê theo ban | Xem stats | Bảng: tổng đã thu / chưa thu mỗi ban |
| T23 | Sắp xếp vi phạm | Sort giảm dần | Danh sách member xếp theo violationCount |
| T24 | Xuất báo cáo | Export | File .txt với timestamp, tổng hợp, danh sách nợ |
| **Edge Cases** | | | |
| T25 | Data persistence | Thêm member, tắt chương trình, mở lại | Dữ liệu vẫn còn |
| T26 | First-run | Xóa hết .dat data, chạy lại | Tự tạo file rỗng + admin account (login ADMIN/ADMIN thành công) |
| T27 | Member xem người khác | Member xem vi phạm member khác | Chỉ xem được của mình |
| T28 | Full capacity | Thêm 1000 members | Thành công. Thêm 1001 → [LOI] |
| T29 | Đổi role → recalculate fine | Member (20K) có 2 vi phạm chưa thu, đổi thành Leader | Cả 2 violation.fine cập nhật thành 50K, totalFine cập nhật |
| T30 | BCN reset mật khẩu | BCN reset pass của member khác | Mật khẩu mới = MSSV, member login được bằng pass mới |
