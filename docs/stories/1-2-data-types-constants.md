# Story 1.2: Data Types & Constants

Status: ready-for-dev

<!-- Note: Validation is optional. Run validate-create-story for quality check before dev-story. -->

## Story

As a developer,
I want all struct definitions and constants centralized in a single header,
so that all modules share consistent data types without circular dependencies.

## Acceptance Criteria

1. **Given** `include/types.h` is created  
   **When** any module includes it  
   **Then** it provides `Member`, `Violation`, `Account` structs and `AppDatabase` typedef
2. **Given** `include/types.h` is created  
   **When** it is reviewed against the architecture  
   **Then** all required `#define` constants are defined, including `MAX_MEMBERS`, `REASON_*`, `TEAM_*`, `PENALTY_*`, `ROLE_*`, `STATUS_*`, and field sizes
3. **Given** `include/types.h` is created  
   **When** dependencies are checked  
   **Then** it includes only `<time.h>` and no other project headers

## Tasks / Subtasks

- [ ] Create `include/types.h` as the central shared header (AC: 1, 2, 3)
  - [ ] Add header guard
  - [ ] Include only `<time.h>`
  - [ ] Do not include any other project header
- [ ] Define all shared constants from the architecture (AC: 2)
  - [ ] Capacity limits: `MAX_MEMBERS`, `MAX_VIOLATIONS`
  - [ ] Field size constants: name, MSSV, email, phone, note, password lengths
  - [ ] Domain constants: team, violation reason, penalty, status, roles
- [ ] Define all core structs (AC: 1)
  - [ ] `Member`
  - [ ] `Violation`
  - [ ] `Account`
  - [ ] `AppDatabase`
- [ ] Validate architectural shape (AC: 1, 2, 3)
  - [ ] Ensure `Violation` uses `time_t`
  - [ ] Ensure `AppDatabase` wraps arrays and counters
  - [ ] Ensure no circular include risk is introduced
- [ ] Verify downstream readiness (AC: 1)
  - [ ] Confirm later modules can include `types.h` first
  - [ ] Confirm names match the architecture terminology

## Dev Notes

- This story is the keystone dependency for almost every later module. Keep names stable once committed.
- `types.h` is intentionally the only shared foundation header. Other module headers should include it first. [Source: _bmad-output/planning-artifacts/architecture.md#Header-File-Template-h]
- The architecture explicitly chose `AppDatabase *db` as the shared parameter model to avoid passing multiple arrays and counts around. [Source: _bmad-output/planning-artifacts/architecture.md#In-Memory-Storage--AppDatabase-struct]
- Avoid inventing alternate models such as separate global arrays or multiple independent count variables.

### Technical Requirements

- `types.h` must define:
  - `Member`
  - `Violation`
  - `Account`
  - `AppDatabase`
- `Violation` must use `time_t violationTime`
- Constants expected from architecture include:
  - `STATUS_ACTIVE`, `STATUS_OUT_CLB`
  - `REASON_NO_JACKET`, `REASON_ABSENT`, `REASON_NO_ACTIVITY`, `REASON_VIOLENCE`
  - `TEAM_ACADEMIC`, `TEAM_PLANNING`, `TEAM_HR`, `TEAM_MEDIA`
  - `PENALTY_FINE`, `PENALTY_OUT_CLB`
  - `ACCOUNT_ROLE_MEMBER`, `ACCOUNT_ROLE_BCN`
  - `MEMBER_ROLE_MEMBER`, `MEMBER_ROLE_LEADER`, `MEMBER_ROLE_BCN`
  - field length constants such as `MAX_NAME_LEN`, `MAX_MSSV_LEN`, `MAX_EMAIL_LEN`, `MAX_PHONE_LEN`, `MAX_NOTE_LEN`, `MAX_PASS_LEN`

### Architecture Compliance

- `types.h` depends only on `<time.h>`. No `stdio.h`, `string.h`, or project headers in this file. [Source: _bmad-output/planning-artifacts/epics.md#Story-12-Data-Types--Constants]
- Keep all struct fields aligned with the architecture:
  - `Member` contains identity, role, team, counts, fines, and active status
  - `Violation` contains `studentId`, `reason`, `violationTime`, `fine`, `isPaid`, `penalty`, `note`
  - `Account` contains `studentId`, `password`, `role`, `isLocked`, `failCount`
- `AppDatabase` must wrap arrays plus `memberCount`, `violationCount`, and `accountCount`.

### File Structure Requirements

- Primary file for this story:
  - `include/types.h`
- This story should not introduce unrelated implementation files unless required for compile verification.
- It depends on Story 1.1 having created the `include/` directory and build scaffold.

### Previous Story Intelligence

- Story 1.1 establishes the project folder layout and Makefile; Story 1.2 must fit into that scaffold cleanly.
- Do not hardcode paths or compile logic inside `types.h`; keep it as a pure declaration header.

### Testing Requirements

- Validation for this story is structural:
  - `include/types.h` exists
  - it can be included by future modules without circular dependency risk
  - it exposes the required types and constants
  - it only includes `<time.h>`
- If compile verification is available after Story 1.1, add a minimal include path check using the scaffolded build.

### Project Structure Notes

- Architecture implementation sequence explicitly starts with `include/types.h` after scaffolding. [Source: _bmad-output/planning-artifacts/architecture.md#Implementation-Sequence]
- This file becomes the contract for all later modules, so naming changes later will be expensive.

### References

- Epic/story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-12-Data-Types--Constants]
- AppDatabase and constants: [Source: _bmad-output/planning-artifacts/architecture.md#In-Memory-Storage--AppDatabase-struct]
- Module boundaries and sequencing: [Source: _bmad-output/planning-artifacts/architecture.md#Module-Responsibility-Matrix]
- Header dependency rules: [Source: _bmad-output/planning-artifacts/architecture.md#Header-File-Template-h]

## Dev Agent Record

### Agent Model Used

gpt-5

### Debug Log References

- Story created from `_bmad-output/planning-artifacts/epics.md`
- Data model guardrails extracted from `_bmad-output/planning-artifacts/architecture.md`

### Completion Notes List

- Ultimate context engine analysis completed - comprehensive developer guide created
- Story scoped to declarations and constants only; implementation belongs to later module stories

### File List

- `_bmad-output/implementation-artifacts/1-2-data-types-constants.md`
