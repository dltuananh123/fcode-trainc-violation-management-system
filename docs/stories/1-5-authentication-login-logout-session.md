# Story 1.5: Authentication - Login, Logout, Session

Status: ready-for-dev

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

- [ ] Create `include/auth.h` and `src/auth.c`
- [ ] Implement login flow (AC: 1, 2, 4)
  - [ ] find account by `studentId`
  - [ ] reject locked accounts
  - [ ] compare password
  - [ ] increment and persist `failCount`
  - [ ] reset `failCount` on successful login and persist the account state
  - [ ] lock account on third failure
- [ ] Implement session handling (AC: 1, 3)
  - [ ] static current session in `auth.c`
  - [ ] `auth_get_session()`
  - [ ] clear session on logout
- [ ] Implement menu-routing contract with `main.c`
- [ ] Persist auth-side account updates through `fileio`

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

### File List

- `_bmad-output/implementation-artifacts/1-5-authentication-login-logout-session.md`
