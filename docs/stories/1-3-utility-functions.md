# Story 1.3: Utility Functions

Status: ready-for-dev

## Story

As a developer,
I want common validation and formatting helpers in a utility module,
so that all modules can reuse input validation, time formatting, and display name mapping.

## Acceptance Criteria

1. **Given** `include/utils.h` and `src/utils.c` are created  
   **When** modules call `is_email_valid(email)`  
   **Then** it returns `1` if email contains `@` and `.`, otherwise `0`
2. **Given** `format_time(time_t)` is called  
   **When** a `time_t` value is passed  
   **Then** it returns a formatted string `dd/mm/yyyy HH:MM`
3. **Given** `parse_date(buffer, &time_t)` is called  
   **When** user inputs `15/04/2026`  
   **Then** it converts to `time_t` using `sscanf + struct tm + mktime` with proper day normalization
4. **Given** display-name helpers are called  
   **When** `team_name(0)`, `member_role_name(1)`, `reason_name(3)` are called  
   **Then** they return the expected Vietnamese display strings

## Tasks / Subtasks

- [ ] Create `include/utils.h` and `src/utils.c` (AC: 1, 2, 3, 4)
- [ ] Implement input validation helpers (AC: 1)
  - [ ] `is_email_valid`
  - [ ] any safe input helper needed by later modules
- [ ] Implement time/date helpers (AC: 2, 3)
  - [ ] `format_time`
  - [ ] `parse_date`
- [ ] Implement display-name mapping helpers (AC: 4)
  - [ ] `team_name`
  - [ ] `member_role_name`
  - [ ] `account_role_name`
  - [ ] `reason_name`
- [ ] Keep helpers reusable and free of business workflow side effects

## Dev Notes

- `utils` depends only on `types`. Keep it generic and reusable. [Source: _bmad-output/planning-artifacts/architecture.md#Module-Responsibility-Matrix]
- Input handling rules are already fixed by architecture:
  - use `fgets` for strings
  - strip trailing newline
  - flush after numeric `scanf` input where needed
- `parse_date` should use `sscanf` and `mktime`, not non-standard parsing helpers. [Source: _bmad-output/planning-artifacts/architecture.md#Data-Flow--Search-by-Date-211]
- Keep Vietnamese display strings in output helpers, but variable/function names in English.

### References

- Story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-13-Utility-Functions]
- Input handling patterns: [Source: _bmad-output/planning-artifacts/architecture.md#Input-Handling]
- Module boundary: [Source: _bmad-output/planning-artifacts/architecture.md#Module-Responsibility-Matrix]

## Dev Agent Record

### Agent Model Used

gpt-5

### Completion Notes List

- Story prepared for implementation with shared helper scope only

### File List

- `_bmad-output/implementation-artifacts/1-3-utility-functions.md`

