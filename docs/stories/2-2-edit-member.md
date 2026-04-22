# Story 2.2: Edit Member

Status: ready-for-dev

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

- [ ] Implement member edit flow
- [ ] Keep MSSV immutable
- [ ] Update normal editable fields and persist
- [ ] Detect role changes
- [ ] Recalculate unpaid violation fines on role change
- [ ] Recalculate `totalFine` and save both members and violations

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

### File List

- `_bmad-output/implementation-artifacts/2-2-edit-member.md`

