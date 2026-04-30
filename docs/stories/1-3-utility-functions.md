# Story 1.3: Utility Functions

Status: done

## Story

As a developer,
I want common validation and formatting helpers in a utility module,
so that all modules can reuse input validation, time formatting, and display name mapping.

## Acceptance Criteria

1. **Given** `include/utils.h` and `src/utils.c` are created  
   **When** modules call `is_email_valid(email)`  
   **Then** it returns `1` if email contains `@` and `.`, otherwise `0`
2. **Given** `format_time(time_t, buffer, size)` is called  
   **When** a `time_t` value is passed  
   **Then** it writes a formatted string `dd/mm/yyyy HH:MM` into the caller buffer
3. **Given** `parse_date(buffer, &time_t, is_end_of_day)` is called  
   **When** user inputs `15/04/2026`  
   **Then** it converts to `time_t` using `sscanf + struct tm + mktime` with proper day normalization
4. **Given** display-name helpers are called  
   **When** `team_name(0)`, `member_role_name(1)`, `reason_name(3)` are called  
   **Then** they return the expected Vietnamese display strings

## Tasks / Subtasks

- [x] Create `include/utils.h` and `src/utils.c` (AC: 1, 2, 3, 4)
- [x] Implement input validation helpers (AC: 1)
  - [x] `is_email_valid`
  - [x] safe input helpers needed by later modules
- [x] Implement time/date helpers (AC: 2, 3)
  - [x] `format_time`
  - [x] `parse_date`
- [x] Implement display-name mapping helpers (AC: 4)
  - [x] `team_name`
  - [x] `member_role_name`
  - [x] `account_role_name`
  - [x] `reason_name`
- [x] Keep helpers reusable and free of business workflow side effects

## Dev Notes

- `utils` depends only on `types`. Keep it generic and reusable. [Source: _bmad-output/planning-artifacts/architecture.md#Module-Responsibility-Matrix]
- Input handling rules are already fixed by architecture:
  - use `fgets` for strings
  - strip trailing newline
  - flush after numeric `scanf` input where needed
- `parse_date` should use `sscanf` and `mktime`, not non-standard parsing helpers. [Source: _bmad-output/planning-artifacts/architecture.md#Data-Flow--Search-by-Date-211]
- Keep Vietnamese display strings in output helpers, but variable/function names in English.

### References

- Story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-13-Utility-Functions]
- Input handling patterns: [Source: _bmad-output/planning-artifacts/architecture.md#Input-Handling]
- Module boundary: [Source: _bmad-output/planning-artifacts/architecture.md#Module-Responsibility-Matrix]

## Dev Agent Record

### Agent Model Used

gpt-5

### Completion Notes List

- Utility helpers are implemented in `include/utils.h` and `src/utils.c`
- Public utility function names follow snake_case to match architecture naming rules
- Member and violation modules already reuse the shared helpers for input, validation, formatting, and display mapping

### Post-Implementation Fixes

#### Fix #1: `is_email_valid` chặt hơn (v2)
- **Problem:** Validation trước đó vẫn cho pass một số edge case: nhiều `@`, bắt đầu/kết thúc bằng `@` hoặc `.`, domain có `..`, ký tự không hợp lệ.
- **Fix:** Rewrite toàn bộ — check đúng 1 dấu `@`, không bắt đầu/kết thúc bằng `@` hay `.`, domain phải có `.` với các phần không rỗng, chỉ cho phép ký tự hợp lệ (letters, digits, `.`, `_`, `%`, `+`, `-`, `@`).

#### Fix #2: `is_id_valid` thêm min length và first-char check
- **Problem:** MSSV có thể ngắn 1 ký tự hoặc bắt đầu bằng số.
- **Fix:** Thêm check tối thiểu 4 ký tự, ký tự đầu phải là chữ cái (`isalpha`).

#### Fix #3: Thêm hàm `is_phone_valid`
- **Problem:** Chưa có hàm validate số điện thoại riêng — member validation chỉ check rỗng.
- **Fix:** Thêm `is_phone_valid()` — chỉ cho phép chữ số, 7-15 ký tự, cho phép `+` ở đầu.

### File List

- `_bmad-output/implementation-artifacts/1-3-utility-functions.md`
- `include/utils.h`
- `src/utils.c`

