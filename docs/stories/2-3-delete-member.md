# Story 2.3: Delete Member

Status: done

## Story

As a BCN admin,
I want to remove a member from the system,
so that departed members no longer appear in the database.

## Acceptance Criteria

1. **Given** BCN selects Delete Member and enters a valid MSSV  
   **When** the system shows the member info and confirmation is accepted  
   **Then** the member is removed from `members[]`, their violations are removed from `violations[]`, and their account is removed from `accounts[]`  
   **And** all three `.dat` files are saved
2. **Given** BCN cancels deletion  
   **When** confirmation is declined  
   **Then** no data is modified and the menu is shown again

## Tasks / Subtasks

- [x] Implement delete-member lookup and confirmation
- [x] Remove member record by shifting array
- [x] Remove all related violations by shifting array
- [x] Remove related account by shifting array
- [x] Persist all three stores
- [x] Keep cancel path side-effect free

## Dev Notes

- Delete is cascade delete by design. There is no separate delete-violation story.
- Follow the architecture's delete-member data flow exactly rather than inventing per-file deletion behavior. [Source: _bmad-output/planning-artifacts/architecture.md#Data-Flow--Delete-Member]

### References

- Story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-23-Delete-Member]
- Delete-member data flow: [Source: _bmad-output/planning-artifacts/architecture.md#Data-Flow--Delete-Member]
- Test cases T13, T14: [Source: _bmad-output/planning-artifacts/architecture.md#Test-Plan]

## Dev Agent Record

### Agent Model Used

gpt-5

### Completion Notes List

- Story prepared with cascade-delete requirement explicit

### Post-Implementation Fixes

#### Fix #1: Save order — accounts trước, members sau
- **Problem:** Save theo thứ tự members → violations → accounts. Nếu accounts save fail → member đã xóa khỏi file nhưng account vẫn còn → user đã bị xóa vẫn đăng nhập được.
- **Fix:** Đổi thứ tự: accounts → violations → members. Revoke access trước, xóa data sau. Nếu fail giữa chừng, worst case member record vẫn tồn tại nhưng user không login được → an toàn hơn.

#### Fix #2: Hiện đầy đủ thông tin trước khi confirm xóa
- **Problem:** Chỉ hiện tên, ban, chức vụ. BCN cần thấy đủ thông tin để quyết định xóa.
- **Fix:** Hiện thêm email, phone, trạng thái, số violation, totalFine trước confirmation prompt.

#### Fix #3: Ngăn BCN tự xóa account của chính mình
- **Problem:** BCN nhập MSSV của mình → xóa chính mình → logout → không còn account nào để quản lý hệ thống.
- **Fix:** Check session studentId vs target studentId, reject nếu trùng với thông báo yêu cầu BCN khác thực hiện.

### File List

- `_bmad-output/implementation-artifacts/2-3-delete-member.md`

