# Story 3.4: View All Violations with Filters

Status: ready-for-dev

## Story

As a BCN admin,
I want to view all violations with filtering options,
so that I can analyze violations by team, reason, or payment status.

## Acceptance Criteria

1. **Given** BCN selects View All Violations  
   **When** the system opens the feature  
   **Then** it presents a sub-menu with filter options for View All, Team, Reason, and Payment Status
2. **Given** BCN selects one of the supported filters  
   **When** the filter is applied  
   **Then** only matching violations are displayed with full details
3. **Given** no violations match the active filter  
   **When** the result set is empty  
   **Then** the system displays `Khong co vi pham nao`

## Tasks / Subtasks

- [ ] Add the BCN view-all-violations entry point and filter sub-menu
- [ ] Implement filter predicates for team, reason, and payment status
- [ ] Display each violation with member context and payment state
- [ ] Handle empty results cleanly without crashing or printing stale rows
- [ ] Keep the feature read-only and avoid mutating violation order during display

## Dev Notes

- This is a reporting view over in-memory data; no file writes should occur here.
- Team filtering will likely need member lookup by MSSV to map each violation back to the member's team.
- Preserve CLI consistency with table formatting and menu patterns already defined in architecture guidance.

### References

- Story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-34-View-All-Violations-with-Filters]
- Filter behavior mapping: [Source: _bmad-output/planning-artifacts/architecture.md#Requirements-to-Structure-Mapping]
- Violation filtering cross-cutting note: [Source: _bmad-output/planning-artifacts/architecture.md#Cross-Cutting-Concerns-Mapping]
- Module ownership: [Source: _bmad-output/planning-artifacts/architecture.md#Module-Responsibility-Matrix]

## Dev Agent Record

### Agent Model Used

gpt-5

### Completion Notes List

- Story prepared with BCN-only filtered read access over the full violation dataset

### File List

- `docs/stories/3-4-view-all-violations-with-filters.md`
