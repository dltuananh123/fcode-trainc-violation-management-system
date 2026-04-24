# Story 3.2: Out CLB Warning & Enforcement

Status: ready-for-dev

## Story

As a BCN admin,
I want the system to automatically warn when members approach Out CLB threshold,
so that I can take action before or at the threshold.

## Acceptance Criteria

1. **Given** a member's `consecutiveAbsences` reaches `3`  
   **When** the triggering violation is recorded  
   **Then** the system displays a warning that the member has missed 3 consecutive sessions
2. **Given** a member's `consecutiveAbsences` reaches `4`  
   **When** the triggering violation is recorded  
   **Then** the system announces the member has exceeded 3 consecutive absences and should be considered Out CLB  
   **And** BCN is prompted to confirm enforcement  
   **And** confirmed enforcement sets `member.isActive=STATUS_OUT_CLB` and saves `members.dat`
3. **Given** a non-absent violation is recorded  
   **When** the member was present for that event  
   **Then** `consecutiveAbsences` is reset to `0`

## Tasks / Subtasks

- [ ] Add a reusable Out CLB threshold checker in `src/violation.c`
- [ ] Show the 3-absence warning without forcing status change
- [ ] Prompt for BCN confirmation at the 4-absence threshold and update active status when accepted
- [ ] Reset absence streak on non-absent violation reasons
- [ ] Save member data immediately after confirmed Out CLB changes
- [ ] Ensure behavior integrates cleanly with Story 3.1 instead of splitting duplicate logic

## Dev Notes

- The architecture resolves the spec ambiguity as warning at `>= 3` and Out CLB confirmation at `>= 4`; follow that documented interpretation consistently.
- The streak belongs on `Member.consecutiveAbsences`; do not attempt to infer it lazily from every report view.
- Keep warning and error prefixes aligned with existing CLI conventions: `[CANH BAO]` and `[LOI]`.

### References

- Story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-32-Out-CLB-Warning--Enforcement]
- Out CLB threshold rationale: [Source: _bmad-output/planning-artifacts/architecture.md#Consecutive-Absence-Tracking]
- Requirement-to-function mapping: [Source: _bmad-output/planning-artifacts/architecture.md#Requirements-to-Structure-Mapping]
- Relevant test cases T18-T20: [Source: _bmad-output/planning-artifacts/architecture.md#Test-Plan]

## Dev Agent Record

### Agent Model Used

gpt-5

### Completion Notes List

- Story prepared with the documented 3-warning / 4-confirmation threshold behavior and streak-reset rule

### File List

- `docs/stories/3-2-out-clb-warning-enforcement.md`
