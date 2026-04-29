# Story 2.2: Edit Member

Status: done

## Story

As a BCN admin,
I want to edit member information,
so that member records stay up to date.

## Acceptance Criteria

1. **Given** BCN selects Edit Member and enters a valid MSSV  
   **When** they update any field except MSSV  
   **Then** the member record is updated and `members.dat` is saved
2. **Given** BCN changes a member's role  
   **When** the edit is confirmed  
   **Then** all unpaid violations for that member are recalculated with the new fine rate  
   **And** `totalFine` is recalculated  
   **And** `violations.dat` and `members.dat` are saved

## Tasks / Subtasks

- [x] Implement member edit flow
- [x] Keep MSSV immutable
- [x] Update normal editable fields and persist
- [x] Detect role changes
- [x] Recalculate unpaid violation fines on role change
- [x] Recalculate `totalFine` and save both members and violations

## Dev Notes

- Recalculation of unpaid fines is mandatory, not optional. [Source: _bmad-output/planning-artifacts/architecture.md#Issue-2--Fine-Calculation-Strategy-RESOLVED]
- Only unpaid violations (`isPaid == 0`) are recalculated.
- `totalFine` is derived state and must be recalculated, not trusted as manually edited data.

### References

- Story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-22-Edit-Member]
- `totalFine` rules: [Source: _bmad-output/planning-artifacts/architecture.md#totalFine-Update-Rules]
- Edit-member data flow: [Source: _bmad-output/planning-artifacts/architecture.md#Data-Flow--Edit-Member-role-change]
- Test cases T11, T12, T29: [Source: _bmad-output/planning-artifacts/architecture.md#Test-Plan]

## Dev Agent Record

### Agent Model Used

gpt-5

### Completion Notes List

- Story prepared with role-change recalculation rule emphasized

### Post-Implementation Fixes

#### Fix #1: Đổi role member nhưng không update Account.role (Critical)
- **Problem:** `editRole()` chỉ update `Member.role`, không update `Account.role` tương ứng. Promote member lên BCN → user đăng nhập lại vẫn thấy member menu thay vì BCN menu.
- **Fix:** Sau khi role thay đổi, tìm account tương ứng và sync `Account.role` (BCN → BCN, còn lại → MEMBER). Save `accounts.dat` cùng lúc với members + violations.

#### Fix #2: `recalcFines` save violations trước members → inconsistent risk
- **Problem:** `recalcFines` tự save `violations.dat` bên trong. Sau đó `memberEdit` mới save `members.dat`. Nếu save members fail → violations đã có fine mới nhưng totalFine vẫn cũ.
- **Fix:** `recalcFines` chỉ update memory, không save file. `memberEdit` save theo thứ tự: members → violations → accounts. Nếu members fail, chưa commit violations → consistent.

#### Fix #3: `editName`/`editPhone` cho pass toàn space
- **Problem:** Nhập tên hoặc phone chỉ toàn space → được chấp nhận.
- **Fix:** Thêm helper `isBlankString()` check xem string có chứa ít nhất 1 non-whitespace char. Dùng trong cả `editName` và `editPhone`.

### File List

- `_bmad-output/implementation-artifacts/2-2-edit-member.md`

