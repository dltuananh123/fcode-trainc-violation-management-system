# Story 1.4: File I/O - Load/Save with First-Run Init

Status: ready-for-dev

## Story

As a developer,
I want all data persistence handled by a single fileio module,
so that data is saved to binary `.dat` files on every mutation and loaded on startup.

## Acceptance Criteria

1. **Given** the program starts and `data/*.dat` files do not exist  
   **When** `fileio_load_all()` is called  
   **Then** empty `.dat` files are created with header `(record count = 0)`  
   **And** a default BCN account `ADMIN/ADMIN` is created in `accounts.dat`
2. **Given** `.dat` files exist with data  
   **When** `fileio_load_all()` is called  
   **Then** all `Member`, `Violation`, and `Account` records are loaded into `AppDatabase`
3. **Given** any mutation occurs  
   **When** `fileio_save_*()` is called  
   **Then** the entire file is rewritten `(count header + all records)`  
   **And** data is protected using the architecture's temp-file replace strategy

## Tasks / Subtasks

- [ ] Create `include/fileio.h` and `src/fileio.c`
- [ ] Implement startup load entrypoint (AC: 1, 2)
  - [ ] create missing `.dat` files
  - [ ] create first-run admin account if `accounts.dat` is empty
  - [ ] clean up crash-residue `.tmp` files on startup
  - [ ] load all data into `AppDatabase`
- [ ] Implement save functions for each store (AC: 3)
  - [ ] `members.dat`
  - [ ] `violations.dat`
  - [ ] `accounts.dat`
- [ ] Apply atomic-write temp-file flow from architecture (AC: 3)
- [ ] Handle corruption and startup edge cases

## Dev Notes

- File format is binary with a 4-byte `int` record count header, then fixed-size records. [Source: _bmad-output/planning-artifacts/architecture.md#Binary-Format-dat]
- Save flow must use temp file -> close -> replace old file. Do not overwrite partially in place. [Source: _bmad-output/planning-artifacts/architecture.md#ReadWrite-Strategy]
- On first run:
  - create empty files if missing
  - create default BCN account `ADMIN/ADMIN`
- If `.tmp` files exist on startup, clean them up before normal load.
- If files exist but cannot be read as valid format, fail clearly instead of guessing.

### References

- Story definition: [Source: _bmad-output/planning-artifacts/epics.md#Story-14-File-IO--LoadSave-with-First-Run-Init]
- File I/O architecture: [Source: _bmad-output/planning-artifacts/architecture.md#File-IO-Architecture]
- First-run admin rule: [Source: _bmad-output/planning-artifacts/architecture.md#First-Run-Admin-Account-Implementation-Decision]
- Startup cleanup and corruption behavior: [Source: _bmad-output/planning-artifacts/architecture.md#ReadWrite-Strategy]

## Dev Agent Record

### Agent Model Used

gpt-5

### Completion Notes List

- Story prepared with persistence and startup guardrails

### File List

- `_bmad-output/implementation-artifacts/1-4-file-io-load-save-first-run-init.md`
