# Story 4.2: Sort Members by Violation Count

Status: ready-for-dev

## Story

As a BCN admin,
I want to sort members by violation count (ascending or descending),
so that I can identify the most or least problematic members.

## Acceptance Criteria

1. **Given** BCN selects Sort by Violations and chooses ascending or descending  
   **When** the system sorts the data  
   **Then** a copy of the member list is sorted using `Member* sorted[MAX_MEMBERS]`
2. **Given** the sorting feature runs  
   **When** the output is displayed  
   **Then** the original `members[]` array order remains unchanged
3. **Given** implementation is reviewed  
   **When** the sorting logic is inspected  
   **Then** it uses a self-implemented algorithm and does not call `qsort`

## Tasks / Subtasks

- [ ] Add the sort-by-violations report entry point in `src/report.c`
- [ ] Build a pointer-array copy of members before sorting
- [ ] Implement ascending and descending ordering without `qsort`
- [ ] Count each member's violations accurately from current violation data
- [ ] Display sorted rows with name, MSSV, team, and violation count
- [ ] Verify the source `members[]` order is untouched after the report finishes

## Dev Notes

- The pointer-copy constraint is explicit in architecture and must not be weakened to an in-place sort.
- Keep the algorithm simple and auditable for grading; a manual bubble sort or selection sort is acceptable if implemented cleanly.
- This is a report concern, not a member module mutation.

### References

- Story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-42-Sort-Members-by-Violation-Count]
- Pointer-copy sort rule: [Source: _bmad-output/planning-artifacts/architecture.md#Requirements-to-Structure-Mapping]
- Outlined architectural requirement AR-9: [Source: _bmad-output/planning-artifacts/epics.md#Additional-Requirements-from-Architecture]
- Relevant test case T23: [Source: _bmad-output/planning-artifacts/architecture.md#Test-Plan]

## Dev Agent Record

### Agent Model Used

gpt-5

### Completion Notes List

- Story prepared with the non-`qsort`, pointer-copy sorting constraint and report-only behavior

### File List

- `docs/stories/4-2-sort-members-by-violation-count.md`
