# Story 1.5: Authentication - Login, Logout, Session

Status: done

## Story

As a user,
I want to log in with my MSSV and password,
so that I can access the system with my assigned role.

## Acceptance Criteria

1. **Given** the program starts  
   **When** the user enters correct MSSV and password  
   **Then** the system loads the role-appropriate menu  
   **And** the session is tracked via `auth_get_session()` using a static session in `auth.c`
2. **Given** the user enters wrong password  
   **When** `failCount` reaches `3`  
   **Then** the account is locked, saved to file, and the program exits
3. **Given** the user is logged in  
   **When** the user selects Logout  
   **Then** the session is cleared and the login screen is shown without terminating the program
4. **Given** a locked account tries to log in  
   **When** any password is entered  
   **Then** the system displays `[LOI] Tai khoan da bi khoa` and refuses login

## Tasks / Subtasks

- [x] Create `include/auth.h` and `src/auth.c`
- [x] Implement login flow (AC: 1, 2, 4)
  - [x] find account by `studentId`
  - [x] reject locked accounts
  - [x] compare password
  - [x] increment and persist `failCount`
  - [x] reset `failCount` on successful login and persist the account state
  - [x] lock account on third failure
- [x] Implement session handling (AC: 1, 3)
  - [x] static current session in `auth.c`
  - [x] `auth_get_session()`
  - [x] clear session on logout
- [x] Implement menu-routing contract with `main.c`
- [x] Persist auth-side account updates through `fileio`

## Dev Notes

- Two role systems are different:
  - `Account.role` controls menu access
  - `Member.role` controls fine calculation
- Session must be `static` inside `src/auth.c`; do not introduce `extern` globals. [Source: _bmad-output/planning-artifacts/architecture.md#Session-Management]
- Login flow is already defined in architecture. Follow it exactly rather than inventing another model. [Source: _bmad-output/planning-artifacts/architecture.md#Login-Flow]
- Logout returns to login flow, not process exit. [Source: _bmad-output/planning-artifacts/architecture.md#Data-Flow--Logout]
- Successful login resets `failCount` to `0`; failed login increments and may lock the account.

### References

- Story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-15-Authentication--Login-Logout-Session]
- Authentication and security: [Source: _bmad-output/planning-artifacts/architecture.md#Authentication--Security]
- Test cases T01-T05: [Source: _bmad-output/planning-artifacts/architecture.md#Test-Plan]

## Dev Agent Record

### Agent Model Used

gpt-5

### Completion Notes List

- Story prepared with lockout and session guardrails

### Post-Implementation Fixes

#### Fix: Login fail thoát chương trình ngay lập tức (Critical bug)
- **Problem:** `authLogin()` return `-1` cho mọi trường hợp fail (sai mật khẩu, tài khoản không tồn tại, tài khoản bị khóa). `main.c` gọi `break` khi `authLogin` return `-1` → chương trình thoát ngay sau 1 lần nhập sai.
- **Expected behavior:** Sai mật khẩu (< 3 lần) hoặc tài khoản không tồn tại → cho retry. Account bị khóa do đủ 3 lần sai liên tiếp → mới thoát chương trình.
- **Fix:** Thêm `while(1)` loop bên trong `authLogin()`. Các lỗi không nghiêm trọng dùng `continue` để retry. Chỉ `return -1` khi account bị khóa do đủ 3 lần sai (lockout), hoặc khi `fileioSaveAccounts` fail (không thể persist trạng thái).

### File List

- `_bmad-output/implementation-artifacts/1-5-authentication-login-logout-session.md`
