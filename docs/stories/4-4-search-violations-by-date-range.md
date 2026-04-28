# Story 4.4: Search Violations by Date Range

Status: ready-for-dev

## Story

As a BCN admin,
I want to search violations within a date range,
so that I can analyze violations for a specific period.

## Acceptance Criteria

1. **Given** BCN selects Search by Date  
   **When** they enter start and end dates in `dd/mm/yyyy` format  
   **Then** the system converts them to `time_t` using `parse_date()` with day-bound normalization
2. **Given** valid dates are provided  
   **When** the search executes  
   **Then** all violations whose `violationTime` falls within `[start, end]` are displayed
3. **Given** an invalid date format is entered  
   **When** parsing fails  
   **Then** the system shows `[LOI] Dinh dang ngay khong hop le (dd/mm/yyyy)`

## Tasks / Subtasks

- [ ] Add the BCN search-by-date flow in `src/violation.c`
- [ ] Prompt for start and end dates with the documented CLI format
- [ ] Use `parse_date()` to normalize start-of-day and end-of-day boundaries
- [ ] Filter `violations[]` by inclusive `time_t` comparison
- [ ] Display matching violations or an empty-result message without mutating stored data

## Dev Notes

- Use the shared utility parser; do not reimplement date parsing separately inside the violation module.
- The feature is read-only over in-memory violations and should not trigger file I/O.
- Inclusive range behavior matters; preserve `[start, end]` exactly as documented.

### References

- Story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-44-Search-Violations-by-Date-Range]
- Date parsing requirement and utility guidance: [Source: _bmad-output/planning-artifacts/architecture.md#Cross-Cutting-Concerns-Mapping]
- Search-by-date integration flow: [Source: _bmad-output/planning-artifacts/architecture.md#Data-Flow--Search-by-Date-211]
- Date-search requirement mapping and integration notes: [Source: _bmad-output/planning-artifacts/architecture.md#Requirements-to-Structure-Mapping]

## Dev Agent Record

### Agent Model Used

gpt-5

### Completion Notes List

- Story prepared with shared-date-parser usage and inclusive range filtering requirements

### File List

- `docs/stories/4-4-search-violations-by-date-range.md`
