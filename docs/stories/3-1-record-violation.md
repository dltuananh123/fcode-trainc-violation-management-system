# Story 3.1: Record Violation

Status: ready-for-dev

## Story

As a BCN admin,
I want to record a violation for a member,
so that violations are tracked with correct fines and timestamps.

## Acceptance Criteria

1. **Given** BCN selects Record Violation and enters a valid MSSV  
   **When** they select the violation reason  
   **Then** the system stores the current time via `time()` and calculates the fine from `Member.role`  
   **And** regular members are fined `20000`, while Leader/BCN member roles are fined `50000`
2. **Given** the selected reason is `REASON_VIOLENCE`  
   **When** the violation is processed  
   **Then** the violation uses `fine=0` and `penalty=PENALTY_OUT_CLB`  
   **And** the system warns BCN and asks for explicit Out CLB confirmation  
   **And** confirmed enforcement sets `member.isActive=STATUS_OUT_CLB` and resets `consecutiveAbsences`
3. **Given** the selected reason is `REASON_ABSENT`  
   **When** the violation is recorded  
   **Then** `member.consecutiveAbsences` is incremented  
   **And** the Out CLB threshold check runs immediately

## Tasks / Subtasks

- [ ] Add the BCN record-violation flow in `src/violation.c`
- [ ] Reuse member lookup by MSSV/name so BCN can target the correct member
- [ ] Create violation records with current timestamp, reason, penalty, paid flag, and computed fine
- [ ] Handle the violence branch separately with confirmation-driven Out CLB update
- [ ] Persist `violations.dat` and `members.dat` after successful mutation
- [ ] Keep terminal prompts and validation messages consistent with the existing CLI style

## Dev Notes

- Keep all violation state inside `AppDatabase`; do not introduce new globals.
- `Account.role` controls menu access, but fine calculation must use `Member.role`.
- File persistence must follow the existing save-after-mutation rule and the binary file strategy in `fileio`.
- This story owns the base recording flow; threshold messaging logic should call shared Out CLB checks instead of duplicating business rules.

### References

- Story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-31-Record-Violation]
- Violation module ownership and fine rules: [Source: _bmad-output/planning-artifacts/architecture.md#Requirements-to-Structure-Mapping]
- Two role systems and Out CLB handling: [Source: _bmad-output/planning-artifacts/architecture.md#Authentication--Security]
- Mutation persistence flow: [Source: _bmad-output/planning-artifacts/architecture.md#Data-Flow--Mutation-eg-Record-Violation]

## Dev Agent Record

### Agent Model Used

gpt-5

### Completion Notes List

- Story prepared with the base violation-recording path, role-based fines, and violence-specific Out CLB confirmation

### File List

- `docs/stories/3-1-record-violation.md`
