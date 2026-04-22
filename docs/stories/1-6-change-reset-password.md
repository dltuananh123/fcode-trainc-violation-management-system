# Story 1.6: Change & Reset Password

Status: ready-for-dev

## Story

As a user,
I want to change my password or have BCN reset it,
so that I can maintain account security.

## Acceptance Criteria

1. **Given** a logged-in user selects Change Password  
   **When** they enter the correct old password and a valid new password  
   **Then** the password is updated in `accounts[]` and saved to `accounts.dat`
2. **Given** a logged-in user enters wrong old password  
   **When** attempting to change password  
   **Then** the system displays `[LOI] Mat khau cu khong dung`
3. **Given** a BCN user selects Reset Password  
   **When** they enter a target MSSV  
   **Then** that member's password is reset to their MSSV and saved

## Tasks / Subtasks

- [ ] Extend auth module for password changes
- [ ] Implement self-service password change flow (AC: 1, 2)
- [ ] Implement BCN reset-password flow (AC: 3)
- [ ] Persist password updates through account save path
- [ ] Enforce old-password check for self-change only

## Dev Notes

- This story builds directly on Story 1.5; keep implementation inside `auth` rather than scattering it.
- BCN reset target password defaults to MSSV by architecture decision and epic definition.
- Keep account mutation logic consistent with file persistence rules.

### References

- Story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-16-Change--Reset-Password]
- Test cases T06, T07, T30: [Source: _bmad-output/planning-artifacts/architecture.md#Test-Plan]

## Dev Agent Record

### Agent Model Used

gpt-5

### Completion Notes List

- Story prepared as auth extension, not separate subsystem

### File List

- `_bmad-output/implementation-artifacts/1-6-change-reset-password.md`

