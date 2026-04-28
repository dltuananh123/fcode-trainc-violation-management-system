# Contributing Guide

This project uses a lightweight delivery model:

- Trello for planning and status tracking
- GitHub Flow for implementation and review
- Scrum-lite for sprint cadence

This guide is the **single source of truth** for all team conventions.

---

## Core Rules

1. One Trello card maps to one branch and one pull request.
2. Work must not be pushed directly to `main`.
3. A card is not `Done` until the PR is approved, merged, and verified.
4. Keep changes small enough to review in the same day when possible.
5. If a task grows beyond 1–2 days, split it before continuing.
6. Merge only after review is completed.
7. Delete the branch after merging.

---

## Branch Naming

### Format

```text
<type>-story-<story-id>-<short-description>
```

For non-story work:

```text
<type>-<short-description>
```

### Branch Types

| Type | Purpose |
|------|---------|
| `feat` | New feature |
| `fix` | Bug fix |
| `refactor` | Restructure code without changing behavior |
| `chore` | Setup, config, tooling, formatting |
| `docs` | Documentation only |

### Examples

```text
feat-story-1.1-project-scaffold
feat-story-2.1-add-member
fix-story-1.5-login-lockout
refactor-story-2.2-member-edit-flow
chore-format-tidy-config
docs-branch-commit-guidelines
docs-story-1.4-fileio-notes
```

### Naming Rules

- Use lowercase with hyphens (`-`), no spaces.
- Keep descriptions to 2–5 words.
- Prefer verb + object: `add-member`, `login-session`, `password-reset`.

---

## Commit Style

### Format

```text
<type>(<module>): <short action description>
```

### Commit Types

| Type | Purpose |
|------|---------|
| `feat` | Add functionality |
| `fix` | Fix a bug |
| `refactor` | Restructure code |
| `chore` | Config, format, lint, tooling |
| `docs` | Documentation changes |
| `test` | Add or modify tests |

### Module Names

Use the project's module boundaries:

`main`, `auth`, `member`, `violation`, `fileio`, `report`, `utils`, `types`, `makefile`, `repo`, `readme`, `docs`

### Examples

```text
feat(auth): add login attempt lockout
feat(member): add member creation flow
fix(fileio): persist accounts after password reset
refactor(repo): rename all identifiers to camelBack convention
chore(repo): apply clang-format to all source files
docs(readme): add build instructions
```

### Commit Rules

- One logical change per commit.
- Do not mix refactor and feature work in the same commit.
- Commit messages must explain **what** changed.
- Keep commits small enough to review quickly.
- A story may only need 1–3 commits if it is simple.

---

## Coding Standards

### Language & Compiler

- Standard: **C17** (`-std=c17`).
- Compiler: `gcc` with full warning flags (see Makefile).
- Target: **Windows 64-bit** (`-m64`).
- Code must compile with **zero warnings**.

### Naming Convention (enforced by `.clang-tidy`)

| Element | Style | Example |
|---------|-------|---------|
| Functions | `camelBack` | `authLogin`, `memberAdd`, `readString` |
| Variables | `camelBack` | `studentId`, `newMember`, `vIndex` |
| Parameters | `camelBack` | `teamId`, `bufSize`, `isEndOfDay` |
| Structs | `CamelCase` | `Member`, `Account`, `AppDatabase` |
| Constants / Macros | `UPPER_CASE` | `MAX_MEMBERS`, `STATUS_ACTIVE` |
| Struct members | `camelBack` | `studentId`, `fullName`, `totalFine` |

### Formatting (enforced by `.clang-format`)

- Style: **LLVM** (`BasedOnStyle: LLVM`).
- Indentation: **2 spaces** (no tabs).
- Max line length: **80 characters** for `.c` and `.h` files.
- Line endings: **LF** (Unix-style).
- Run `make format` before every commit.

### Comment Style

| Element | Convention |
|---------|-----------|
| Language | **English only** in code comments. Vietnamese only in user-facing `printf` output. |
| Header guard | `/* */` style: `#endif /* MODULE_H */` |
| Section banner | 60-char `=` width: `/* ============...============ */` |
| Public API docs | Doxygen `/** @brief ... */` required for all functions in `.h` files |
| File-level docs | Each `.h` must have `@file` and `@brief` describing the module |
| Inline comments | Only where logic is complex or non-obvious |

### Include Order

1. Project headers first (`"auth.h"`, `"fileio.h"`, etc.), sorted alphabetically.
2. System headers second (`<stdio.h>`, `<string.h>`, etc.), sorted alphabetically.

### Architecture Rules

- No unnecessary global variables.
- Pass data via `AppDatabase *db` pointer.
- Immediate persistence after any array mutation (write to `.dat` file).
- Use atomic temp-file strategy: write to `.tmp`, then rename to `.dat`.
- Small, explicit module boundaries.

### Verification Before Commit

Run the following commands and ensure all pass:

```bash
make format    # Must be idempotent (no changes)
make tidy      # Zero warnings
make clean
make           # Zero compiler warnings
```

---

## Pull Request Rules

Every PR should:

- Reference exactly one Trello card.
- State the story ID.
- Summarize the user-visible or system-visible outcome.
- List the main files touched.
- Describe how the change was verified.
- Follow the template in `.github/PULL_REQUEST_TEMPLATE.md`.

PRs should remain reviewable:

- Prefer small PRs.
- Open a draft PR early if interface feedback is needed.
- Do not wait until a large feature is fully complete before asking for review.
- If the PR includes docs changes, describe which docs were updated.

---

## Review Rules

Minimum merge standard:

- At least 1 approving review.
- No unresolved review comments.
- Build passes with zero warnings.
- `make format` and `make tidy` pass.

Reviewer expectations:

- Verify behavior against acceptance criteria.
- Check for regression risk in touched modules.
- Check persistence behavior for any write path.
- Check role/permission rules for auth and member flows.
- Verify naming convention compliance (camelBack).

---

## Documentation Rules

### When docs must be updated

If a PR changes any of the following, related docs must be updated in the same PR (or a clear reason given why not):

- Interface between modules.
- Struct, constant, or file format.
- Business rule.
- Build / run / format / tidy flow.
- Naming convention, branch rule, or PR rule.

### Branch & commit for docs

If only updating docs:

```text
Branch: docs-<short-name>
Commit: docs(<module>): <action>
```

If docs are part of a story, include them in the story's PR.

---

## Trello Usage Rules

Board flow:

`Product Backlog` → `Ready for Sprint` → `Sprint Backlog` → `In Progress` → `In Review` → `Done`

Use `Blocked` when progress depends on someone else.

Rules:

- Do not keep more than one primary active card per developer.
- Attach the PR link to the Trello card once opened.
- Only move a card to `Done` after merge and verification.

---

## Sprint Rhythm

- Sprint Planning every 2 weeks.
- Daily Scrum every workday for 15 minutes.
- Backlog refinement once mid-sprint.
- Sprint review and retrospective at sprint end.

---

## Definition of Done

A story is done only if **all** items below are true:

- [ ] Acceptance criteria are met.
- [ ] Code compiles with zero warnings (`make`).
- [ ] Code passes formatting check (`make format` is idempotent).
- [ ] Code passes static analysis (`make tidy` zero naming warnings).
- [ ] Naming convention is followed (camelBack for functions/variables/parameters).
- [ ] All comments are in English.
- [ ] Data-saving behavior works for affected flows.
- [ ] Manual verification is completed.
- [ ] PR is approved.
- [ ] PR is merged.
- [ ] Trello card is moved to `Done`.

---

## Quick Reference

```text
Branch:  <type>-story-<id>-<short-name>  or  <type>-<short-name>
Commit:  <type>(<module>): <action>
Naming:  functions/variables = camelBack, structs = CamelCase, constants = UPPER_CASE
Format:  make format → make tidy → make (all zero warnings)
```
