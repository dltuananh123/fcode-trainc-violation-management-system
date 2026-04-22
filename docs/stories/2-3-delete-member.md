# Story 2.3: Delete Member

Status: ready-for-dev

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

- [ ] Implement delete-member lookup and confirmation
- [ ] Remove member record by shifting array
- [ ] Remove all related violations by shifting array
- [ ] Remove related account by shifting array
- [ ] Persist all three stores
- [ ] Keep cancel path side-effect free

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

### File List

- `_bmad-output/implementation-artifacts/2-3-delete-member.md`

