# Story 4.1: Statistics by Team

Status: ready-for-dev

## Story

As a BCN admin,
I want to see fine statistics broken down by team,
so that I can understand which teams have the most violations and outstanding payments.

## Acceptance Criteria

1. **Given** BCN selects Statistics by Team  
   **When** the system calculates totals  
   **Then** each team row shows total fines collected, total fines outstanding, and grand total
2. **Given** violations exist across multiple teams  
   **When** the report is displayed  
   **Then** the table includes all defined teams: Academic, Planning, HR, and Media

## Tasks / Subtasks

- [ ] Add `report_team_stats()` in `src/report.c`
- [ ] Aggregate collected and unpaid fines by team using member-team mapping
- [ ] Print a stable terminal table with one row per team and a total column
- [ ] Handle teams with zero violations without skipping rows
- [ ] Keep the feature read-only and consistent with report-module responsibilities

## Dev Notes

- Team labels should come from shared display helpers rather than hard-coded ad hoc strings in report logic.
- The report reads from `members[]` and `violations[]`; do not create duplicate aggregate storage on disk.
- This story opens Epic 4, so it should preserve the no-global-state and standard-C constraints used elsewhere.

### References

- Story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-41-Statistics-by-Team]
- Report module ownership: [Source: _bmad-output/planning-artifacts/architecture.md#Module-Responsibility-Matrix]
- Requirement mapping for team stats: [Source: _bmad-output/planning-artifacts/architecture.md#Requirements-to-Structure-Mapping]

## Dev Agent Record

### Agent Model Used

gpt-5

### Completion Notes List

- Story prepared with team-level collected/outstanding aggregation and stable terminal table output

### File List

- `docs/stories/4-1-statistics-by-team.md`
