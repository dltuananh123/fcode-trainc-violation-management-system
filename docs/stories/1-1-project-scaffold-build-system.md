# Story 1.1: Project Scaffold & Build System

Status: done

<!-- Note: Validation is optional. Run validate-create-story for quality check before dev-story. -->

## Story

As a developer,
I want a complete project structure with Makefile and tooling configuration,
so that the team can start implementing features immediately with consistent build and formatting.

## Acceptance Criteria

1. **Given** the project repository is initialized  
   **When** a developer runs `make`  
   **Then** the project compiles successfully with zero warnings using `gcc -std=c17 -Wall -Wextra`  
   **And** the executable is placed in `bin/fcode-trainc`
2. **Given** the project structure exists  
   **When** a developer runs `make clean`  
   **Then** the `build/` and `bin/` directories are removed
3. **Given** the project structure exists  
   **When** a developer runs `make format`  
   **Then** `clang-format` is applied to all `src/*.c` and `include/*.h` files
4. **Given** the project structure exists  
   **When** a developer runs `make tidy`  
   **Then** `clang-tidy` runs against project source files using the `C17` compile context

## Tasks / Subtasks

- [x] Create the baseline project structure (AC: 1, 2)
  - [x] Add `include/`, `src/`, `data/`, `build/`, and `bin/`
  - [x] Add a minimal `src/main.c` with `int main(void)` returning success
  - [x] Ensure `build/` and `bin/` are treated as generated output, not permanent source folders
- [x] Create the initial Makefile (AC: 1, 2, 3)
  - [x] Add `make tidy`
  - [x] Set target output to `bin/fcode-trainc`
  - [x] Compile all `src/*.c` into `build/*.o`
  - [x] Use `-std=c17` as the language standard
  - [x] Use `-Wall -Wextra` at minimum
  - [x] Add `make clean`
  - [x] Add `make format`
- [x] Align the scaffold with architecture rules (AC: 1)
  - [x] Keep `src/main.c` limited to entry-point scaffolding only
  - [x] Keep headers in `include/` and implementations in `src/`
  - [x] Do not introduce non-standard libraries or platform-specific code
- [x] Verify the story outcome locally (AC: 1, 2, 3)
  - [x] Run `make tidy`
  - [x] Run `make`
  - [x] Run `make clean`
  - [x] Run `make format`
  - [x] Confirm the executable path and generated directory behavior

## Dev Notes

- This story creates the shared codebase skeleton for all later work. Keep it minimal and correct; do not pull future module logic into this story.
- The architecture document defines the canonical project structure as:
  - `Makefile`
  - `include/`
  - `src/`
  - `build/`
  - `bin/`
  - `data/`
- The agreed language standard is `C17`. Do not use `C99` flags anywhere.
- Use standard C only. No `conio.h`, no platform-only helpers, no external runtime dependencies. [Source: _bmad-output/planning-artifacts/architecture.md#Technical Constraints & Dependencies]
- `main.c` must remain a thin entry point. Real business logic belongs in later modules. [Source: _bmad-output/planning-artifacts/architecture.md#Code Organization]

### Technical Requirements

- Compiler baseline: `gcc`
- Language standard: `-std=c17`
- Minimum warnings required by story: `-Wall -Wextra`
- Preferred target name: `bin/fcode-trainc`
- Generated object files belong under `build/`
- `make format` should target `src/*.c` and `include/*.h`
- `make tidy` should run `clang-tidy src/*.c -- -std=c17 -Iinclude`
- Repo-level tooling config must not conflict with the requirement-driven naming used by later shared data models: struct/type names stay `PascalCase`, shared struct fields stay `camelCase`

### Architecture Compliance

- Follow `src/` for `.c` files and `include/` for `.h` files.
- Do not create module files in the repo root.
- Use `int main(void)`, not `void main()`. [Source: _bmad-output/planning-artifacts/architecture.md#Anti-Patterns]
- This story should not create application features beyond the scaffold.
- Tooling created in this story should support later requirement-aligned naming instead of enforcing conflicting field conventions.

### File Structure Requirements

- Expected files/folders from this story:
  - `Makefile`
  - `src/main.c`
  - `include/`
  - `src/`
  - `data/`
  - `build/`
  - `bin/`
- Optional supporting files may already exist in repo root:
  - `.clang-format`
  - `.clang-tidy`
  - `.editorconfig`
  - `.gitattributes`
- Reuse existing repo-level tool config if present; do not duplicate it under subfolders.

### Testing Requirements

- Manual verification is enough for this story:
  - `make` produces `bin/fcode-trainc`
  - `make clean` removes generated build output
  - `make format` runs against project source/header globs without failing
  - `make tidy` runs against current project source with C17 include settings
- Zero warnings is part of the AC, so keep the initial `main.c` minimal.

### Project Structure Notes

- This story is the first implementation priority in the architecture handoff. [Source: _bmad-output/planning-artifacts/architecture.md#Implementation Handoff]
- Keep the scaffold ready for Story 1.2:
  - `include/types.h` must fit naturally into the new structure
  - The Makefile should automatically compile future `.c` files under `src/`

### References

- Epic/story definition: [Source: _bmad-output/planning-artifacts/epics.md#Epic-1-Project-Setup--Authentication]
- Directory structure and Makefile expectations: [Source: _bmad-output/planning-artifacts/architecture.md#Directory-Structure-Modular--srcincludebuildbin]
- Compiler/tooling expectations: [Source: _bmad-output/planning-artifacts/architecture.md#Makefile-Structure-key-variables]
- Code organization guardrails: [Source: _bmad-output/planning-artifacts/architecture.md#Code-Organization]
- Tooling targets: [Source: _bmad-output/planning-artifacts/architecture.md#Development-Experience]

## Dev Agent Record

### Agent Model Used

gpt-5

### Debug Log References

- Story created from `_bmad-output/planning-artifacts/epics.md`
- Architecture guardrails extracted from `_bmad-output/planning-artifacts/architecture.md`

### Completion Notes List

- Project scaffold completed with `Makefile`, `src/main.c`, `include/`, `data/`, generated `build/` and `bin/`
- Verified equivalent Windows workflow with `mingw32-make`, `mingw32-make clean`, `mingw32-make format`, and `mingw32-make tidy`
- `src/main.c` remains intentionally minimal so the scaffold builds cleanly and is ready for Story 1.2

### File List

- `_bmad-output/implementation-artifacts/1-1-project-scaffold-build-system.md`
- `Makefile`
- `src/main.c`
- `include/.gitkeep`
