# Story 2.1: Add Member

Status: done

## Story

As a BCN admin,
I want to add new members with full information,
so that the CLB member database is maintained.

## Acceptance Criteria

1. **Given** BCN selects Add Member  
   **When** they enter valid member information  
   **Then** a new `Member` is added and a new `Account` with password = MSSV is added  
   **And** `members.dat` and `accounts.dat` are saved immediately
2. **Given** duplicate MSSV is entered  
   **When** validation runs  
   **Then** the system shows `[LOI] MSSV da ton tai`
3. **Given** invalid email is entered  
   **When** validation runs  
   **Then** the system shows `[LOI] Email khong hop le`
4. **Given** the member count has reached `1000`  
   **When** BCN tries to add another member  
   **Then** the system shows the max-capacity error

## Tasks / Subtasks

- [x] Create `include/member.h` and `src/member.c`
- [x] Implement add-member workflow
  - [x] collect profile fields
  - [x] validate MSSV uniqueness
  - [x] validate email
  - [x] enforce capacity limit
- [x] Create matching account record with password = MSSV
- [x] Map access role and member role correctly when creating the account/member pair
- [x] Persist `members` and `accounts` immediately

## Dev Notes

- This is the first member CRUD story; keep scope strictly to add flow.
- `Account.role` and `Member.role` are different fields. Do not conflate access role with fine-calculation role. [Source: _bmad-output/planning-artifacts/architecture.md#Two-Role-Systems-IMPORTANT--do-not-confuse]
- Member capacity hard limit is 1000.
- Save both affected files immediately after mutation.
- Leader/Vice differs from regular Member in fine rate, not automatically in menu access. Only BCN gets BCN menu access.

### References

- Story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-21-Add-Member]
- Capacity and validation constraints: [Source: _bmad-output/planning-artifacts/architecture.md#Cross-Cutting-Concerns-Identified]
- Test cases T08-T10, T28: [Source: _bmad-output/planning-artifacts/architecture.md#Test-Plan]

## Dev Agent Record

### Agent Model Used

gpt-5

### Completion Notes List

- Story prepared with validation, account-creation, and persistence guardrails

### Post-Implementation Fixes

#### Fix #1: `strncpy` thêm explicit null-terminate
- **Problem:** `strncpy` không guarantee null-terminate nếu source dài >= n. Code chạy đúng nhờ `memset` trước đó, nhưng newbie copy pattern này mà quên `memset` → bug âm thầm.
- **Fix:** Thêm `buf[MAX_LEN - 1] = '\0'` sau mỗi `strncpy`.

#### Fix #2: Rollback double-fail handling
- **Problem:** Khi `fileioSaveAccounts` fail, code rollback cả member + account rồi gọi `fileioSaveMembers` để re-save. Nhưng nếu re-save cũng fail → memory state inconsistent.
- **Fix:** Thêm check kết quả của re-save và print thêm error message nếu thất bại.

#### Fix #3: `is_email_valid` chặt hơn
- **Problem:** Validation cũ chỉ check có `@` và `.` ở bất kỳ đâu → `"@."` pass validation.
- **Fix:** Check `@` không ở đầu, phải có `.` sau `@` với ít nhất 1 ký tự giữa chúng, không kết thúc bằng `.`.

#### Fix #4: Validate `fullName` và `phone` không rỗng
- **Problem:** Tên rỗng và số điện thoại rỗng được chấp nhận.
- **Fix:** Thêm check `strlen == 0` trong `memberValidateInput` cho cả `fullName` và `phone`.

#### Fix #5: Early MSSV validation
- **Problem:** User nhập hết MSSV, tên, email, phone, team, role rồi mới validate. Nếu MSSV trùng → mất công nhập lại tất cả.
- **Fix:** Validate MSSV (format + unique) ngay sau khi nhập, trước khi nhập các trường khác. `memberValidateInput` giờ chỉ validate name, email, phone.

#### Fix #6: Thêm `const` cho header signatures
- **Problem:** `memberFindById` và `memberValidateInput` nhận `AppDatabase *db` non-const, nhưng implementation chỉ đọc.
- **Fix:** Đổi thành `const AppDatabase *db` ở cả header lẫn implementation.

### File List

- `_bmad-output/implementation-artifacts/2-1-add-member.md`
