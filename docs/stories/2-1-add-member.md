# Story 2.1: Add Member

Status: ready-for-dev

## Story

As a BCN admin,
I want to add new members with full information,
so that the CLB member database is maintained.

## Acceptance Criteria

1. **Given** BCN selects Add Member  
   **When** they enter valid member information  
   **Then** a new `Member` is added and a new `Account` with password = MSSV is added  
   **And** `members.dat` and `accounts.dat` are saved immediately
2. **Given** duplicate MSSV is entered  
   **When** validation runs  
   **Then** the system shows `[LOI] MSSV da ton tai`
3. **Given** invalid email is entered  
   **When** validation runs  
   **Then** the system shows `[LOI] Email khong hop le`
4. **Given** the member count has reached `1000`  
   **When** BCN tries to add another member  
   **Then** the system shows the max-capacity error

## Tasks / Subtasks

- [ ] Create `include/member.h` and `src/member.c`
- [ ] Implement add-member workflow
  - [ ] collect profile fields
  - [ ] validate MSSV uniqueness
  - [ ] validate email
  - [ ] enforce capacity limit
- [ ] Create matching account record with password = MSSV
- [ ] Map access role and member role correctly when creating the account/member pair
- [ ] Persist `members` and `accounts` immediately

## Dev Notes

- This is the first member CRUD story; keep scope strictly to add flow.
- `Account.role` and `Member.role` are different fields. Do not conflate access role with fine-calculation role. [Source: _bmad-output/planning-artifacts/architecture.md#Two-Role-Systems-IMPORTANT--do-not-confuse]
- Member capacity hard limit is 1000.
- Save both affected files immediately after mutation.
- Leader/Vice differs from regular Member in fine rate, not automatically in menu access. Only BCN gets BCN menu access.

### References

- Story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-21-Add-Member]
- Capacity and validation constraints: [Source: _bmad-output/planning-artifacts/architecture.md#Cross-Cutting-Concerns-Identified]
- Test cases T08-T10, T28: [Source: _bmad-output/planning-artifacts/architecture.md#Test-Plan]

## Dev Agent Record

### Agent Model Used

gpt-5

### Completion Notes List

- Story prepared with validation, account-creation, and persistence guardrails

### File List

- `_bmad-output/implementation-artifacts/2-1-add-member.md`
