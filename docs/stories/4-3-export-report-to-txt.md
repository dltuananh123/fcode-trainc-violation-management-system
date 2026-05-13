# Story 4.3: Export Report to .txt

Status: done

## Story

As a BCN admin,
I want to export a violation report to a `.txt` file,
so that I have a permanent record for reporting purposes.

## Acceptance Criteria

1. **Given** BCN selects Export Report  
   **When** the report is generated  
   **Then** a `.txt` file is created with the export timestamp in the filename
2. **Given** the export succeeds  
   **When** the file content is inspected  
   **Then** it includes a summary by team with collected and outstanding totals  
   **And** it includes a list of members who still owe fines with name, MSSV, and amount

## Tasks / Subtasks

- [x] Add `report_export_txt()` in `src/report.c`
- [x] Generate a timestamped output filename suitable for the local filesystem
- [x] Write report header, team summary, and outstanding-member section to the `.txt` file
- [x] Reuse existing aggregation logic where possible instead of duplicating business calculations
- [x] Show a clear CLI success or failure message after export

## Dev Notes

- The report module is allowed to write `.txt` outputs directly, but it should not change application `.dat` state.
- Keep the export format plain text and readable in a terminal/editor without extra tooling.
- Timestamp formatting should be deterministic and based on standard C time APIs only.

### References

- Story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-43-Export-Report-to-txt]
- Report module responsibility: [Source: _bmad-output/planning-artifacts/architecture.md#Module-Responsibility-Matrix]
- Requirement mapping for export: [Source: _bmad-output/planning-artifacts/architecture.md#Requirements-to-Structure-Mapping]
- Relevant test case T24: [Source: _bmad-output/planning-artifacts/architecture.md#Test-Plan]

## Dev Agent Record

### Agent Model Used

gpt-5

### Completion Notes List

- Story prepared with timestamped text export requirements and reusable reporting calculations
- Wired BCN menu option 10 to the text export flow
- Reused shared team aggregation logic for both terminal statistics and exported summaries
- Generated deterministic timestamped filenames using standard C time APIs only
- Exported both team totals and the list of members with outstanding fines

### File List

- `_bmad-output/implementation-artifacts/4-3-export-report-to-txt.md`
- `include/report.h`
- `src/main.c`
- `src/report.c`
