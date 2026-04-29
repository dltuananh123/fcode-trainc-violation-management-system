# Story 2.4: View Profile & Member List

Status: done

## Story

As a member,
I want to view my own profile and the CLB member list,
so that I can see my information and know who else is in the club.

## Acceptance Criteria

1. **Given** any logged-in user selects View Profile  
   **When** the system reads the session MSSV  
   **Then** it displays full profile fields including status `(Active/Out CLB)`
2. **Given** any logged-in user selects View Member List  
   **When** the system displays all members  
   **Then** only `ho ten`, `MSSV`, `ban`, and `chuc vu` are shown  
   **And** sensitive fields such as phone, email, and violation count are hidden for other members
3. **Given** the member list is long  
   **When** the list is displayed  
   **Then** pagination is used `(20 per page, Enter to continue, q to quit)`

## Tasks / Subtasks

- [x] Implement view-profile flow using current session MSSV
- [x] Display full profile with member status
- [x] Implement member-list display with restricted columns
- [x] Add pagination for member list
- [x] Keep output suitable for terminal demo/review

## Dev Notes

- Session comes from `auth_get_session()`. Do not duplicate session state elsewhere.
- This story is display-oriented, not editing.
- Privacy rule matters: other members' phone, email, and violation count must not be shown.

### References

- Story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-24-View-Profile--Member-List]
- Group 1 requirement mapping: [Source: _bmad-output/planning-artifacts/architecture.md#Requirements-to-Structure-Mapping]
- Test case T27 is related access control context: [Source: _bmad-output/planning-artifacts/architecture.md#Test-Plan]

## Dev Agent Record

### Agent Model Used

gpt-5

### Completion Notes List

- Story prepared with profile visibility and member-list privacy boundaries

### Post-Implementation Fixes

#### Fix #1: Table formatting bị hỏng với data thực
- **Problem:** Cột quá hẹp — MSSV 4 chars (thực tế 8+), Chuc vu 9 chars (`"Truong nhom/Pho nhom"` = 20). `%-4s` chỉ set min width, không truncate → table bị lệch hoàn toàn.
- **Fix:** Mở rộng cột (MSSV=10, Name=20, Ban=12, Chuc vu=20) + dùng precision specifier `%-10.10s` để vừa pad vừa truncate nếu quá dài.

#### Fix #2: Status text "Active" vs "Hoat dong" inconsistency
- **Problem:** `memberViewProfile` dùng `"Active"` / `"Out CLB"` (English), trong khi `displayMemberInfo` (edit) và `memberDelete` dùng `"Hoat dong"` / `"Da Out CLB"` (Vietnamese).
- **Fix:** Đổi thành `"Hoat dong"` / `"Da Out CLB"` cho nhất quán với toàn bộ UI.

### File List

- `_bmad-output/implementation-artifacts/2-4-view-profile-member-list.md`

