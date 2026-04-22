---
artifactType: 'delivery-board-blueprint'
project_name: 'fcode trainc'
user_name: 'Admin'
date: '2026-04-19'
sourceDocuments:
  - _bmad-output/planning-artifacts/epics.md
  - _bmad-output/planning-artifacts/research/technical-github-flow-trello-scrum-research-2026-04-19.md
status: 'ready'
---

# Trello Board Blueprint - fcode trainc

## Purpose

This document turns the current epic/story breakdown into a Trello board structure the team can use immediately for:

- project management
- task splitting
- sprint planning
- progress tracking
- PR/status synchronization

Recommended board name:

- `fcode trainc Delivery Board`

---

## Board Structure

Create these lists in order:

1. `Product Backlog`
2. `Ready for Sprint`
3. `Sprint Backlog`
4. `In Progress`
5. `In Review`
6. `Blocked`
7. `Done`

Working rules:

- `Product Backlog`: all unscheduled stories and bugs
- `Ready for Sprint`: refined cards with acceptance criteria, estimate, and assignee candidate
- `Sprint Backlog`: only cards committed in current sprint
- `In Progress`: one primary active card per developer
- `In Review`: card already has PR or is waiting for review/demo verification
- `Blocked`: card cannot move without external help or design decision
- `Done`: merged, verified, and accepted only

---

## Labels

Create these labels:

- `Epic 1`
- `Epic 2`
- `Epic 3`
- `Epic 4`
- `bug`
- `tech-debt`
- `blocked`
- `priority-high`
- `priority-medium`
- `priority-low`

Optional people/system labels:

- `backend-core`
- `persistence`
- `reporting`
- `auth`

---

## Card Naming Convention

Use this format:

`[Story ID] Short action-focused title`

Examples:

- `[1.1] Scaffold project and Makefile`
- `[1.5] Implement login/logout/session`
- `[2.1] Add member creation flow`
- `[3.1] Record violation and auto-fine`
- `[4.3] Export violation report to txt`

---

## Card Template

Use this template for every story card:

### Title

`[Story ID] <short title>`

### Description

```text
Story:
<copy short story statement from epics>

Expected outcome:
<what the team should be able to demo when this card is done>

Acceptance criteria:
- ...
- ...
- ...

Files/modules likely touched:
- include/<...>
- src/<...>
- data/<...> if relevant

Definition of done:
- Code implemented
- Build passes
- Data persistence/edge cases checked
- PR opened and linked
- PR approved
- Merged to main
```

### Custom fields to add in Trello

- `Story ID`
- `Epic`
- `Sprint`
- `Estimate`
- `Owner`
- `Reviewer`
- `PR Link`

### Checklist template

- Understand story + acceptance criteria
- Confirm touched modules/interfaces
- Implement code
- Verify build/manual run
- Update/prepare test data if needed
- Open PR
- Address review comments
- Merge

---

## Trello Automation Rules

Recommended Butler rules:

1. When a card is moved to `In Progress`, set start date and add card member.
2. When a card is moved to `In Review`, add checklist item `PR link attached`.
3. When a card is moved to `Blocked`, add red `blocked` label.
4. When due date is within 24 hours and card is not in `Done`, add `priority-high`.
5. When a card is moved to `Done`, remove `blocked` label automatically.

---

## Sprint Setup

Recommended initial sprint plan based on the current delivery window:

- `Sprint 1`: 2026-04-20 to 2026-05-03
- `Sprint 2`: 2026-05-04 to 2026-05-16
- `Buffer / demo prep`: 2026-05-17

Point scale:

- `1` = tiny isolated task
- `2` = straightforward one-module story
- `3` = moderate cross-module story
- `5` = complex multi-module story

Rule:

- no story above `5`
- if larger than `5`, split before commitment

---

## Seed Cards

### Sprint 1 - Commit These First

These are the best candidates to create first and move toward `Ready for Sprint`.

#### Epic 1 - Setup & Authentication

1. `[1.1] Scaffold project and Makefile`
   - Label: `Epic 1`
   - Estimate: `3`
   - Suggested owner: Dev A
   - Modules: `Makefile`, `src/main.c`, `include/`, `src/`, `build/`, `bin/`
   - Demo outcome: `mingw32-make`, `mingw32-make clean`, and base structure work

2. `[1.2] Define shared data types and constants`
   - Label: `Epic 1`
   - Estimate: `2`
   - Suggested owner: Dev C
   - Modules: `include/types.h`
   - Demo outcome: all structs/constants compile and are reusable

3. `[1.3] Implement utility helpers`
   - Label: `Epic 1`
   - Estimate: `2`
   - Suggested owner: Dev C
   - Modules: `include/utils.h`, `src/utils.c`
   - Demo outcome: validation, formatting, and naming helpers work

4. `[1.4] Build file I/O and first-run initialization`
   - Label: `Epic 1`
   - Estimate: `5`
   - Suggested owner: Dev D
   - Modules: `include/fileio.h`, `src/fileio.c`, `data/`
   - Demo outcome: app loads/saves `.dat` files and creates default admin

5. `[1.5] Implement login/logout/session`
   - Label: `Epic 1`
   - Estimate: `5`
   - Suggested owner: Dev B
   - Modules: `include/auth.h`, `src/auth.c`, `src/main.c`
   - Demo outcome: correct login route, lockout logic, logout flow

6. `[1.6] Implement change and reset password`
   - Label: `Epic 1`
   - Estimate: `3`
   - Suggested owner: Dev B
   - Modules: `include/auth.h`, `src/auth.c`, `src/fileio.c`
   - Demo outcome: member can change password, BCN can reset password

#### Epic 2 - Member Management

7. `[2.1] Add member creation flow`
   - Label: `Epic 2`
   - Estimate: `3`
   - Suggested owner: Dev E
   - Modules: `include/member.h`, `src/member.c`, `src/fileio.c`
   - Demo outcome: add member, validate MSSV/email, create account

8. `[2.2] Edit member and recalculate unpaid fines`
   - Label: `Epic 2`
   - Estimate: `5`
   - Suggested owner: Dev E
   - Modules: `include/member.h`, `src/member.c`, `src/violation.c`, `src/fileio.c`
   - Demo outcome: update member info and recalc unpaid fine state

Stretch cards for late Sprint 1 if capacity remains:

9. `[2.3] Delete member and cascade related data`
   - Label: `Epic 2`
   - Estimate: `3`
   - Suggested owner: Dev E
   - Modules: `src/member.c`, `src/fileio.c`
   - Demo outcome: member, account, and related violations are removed safely

10. `[2.4] View profile and member list`
   - Label: `Epic 2`
   - Estimate: `2`
   - Suggested owner: Dev A
   - Modules: `src/member.c`, `src/main.c`
   - Demo outcome: member profile and club list display correctly by role

### Sprint 2 - Plan These Next

#### Epic 3 - Violation Tracking & Enforcement

11. `[3.1] Record violation and auto-calculate penalty`
   - Label: `Epic 3`
   - Estimate: `5`
   - Suggested owner: Dev A
   - Modules: `include/violation.h`, `src/violation.c`, `src/member.c`
   - Demo outcome: violation recorded with correct reason/date/penalty

12. `[3.2] Implement Out CLB warning and enforcement`
   - Label: `Epic 3`
   - Estimate: `3`
   - Suggested owner: Dev A
   - Modules: `src/violation.c`, `src/member.c`
   - Demo outcome: consecutive absence and violence rules trigger correctly

13. `[3.3] Mark paid and show own violations/fines`
   - Label: `Epic 3`
   - Estimate: `5`
   - Suggested owner: Dev B
   - Modules: `src/violation.c`, `src/member.c`, `src/main.c`
   - Demo outcome: payment state updates and member can see debt summary

14. `[3.4] View all violations with filters`
   - Label: `Epic 3`
   - Estimate: `3`
   - Suggested owner: Dev D
   - Modules: `src/violation.c`, `src/report.c`, `src/main.c`
   - Demo outcome: filter by team, reason, payment status

#### Epic 4 - Reports & Advanced Features

15. `[4.1] Show statistics by team`
   - Label: `Epic 4`
   - Estimate: `2`
   - Suggested owner: Dev B
   - Modules: `include/report.h`, `src/report.c`
   - Demo outcome: terminal table of paid/unpaid totals by team

16. `[4.2] Sort members by violation count`
   - Label: `Epic 4`
   - Estimate: `2`
   - Suggested owner: Dev C
   - Modules: `src/report.c`, `src/member.c`
   - Demo outcome: sorted view without changing original array order

17. `[4.3] Export violation report to txt`
   - Label: `Epic 4`
   - Estimate: `3`
   - Suggested owner: Dev D
   - Modules: `src/report.c`
   - Demo outcome: report file with timestamp and unpaid summary is generated

18. `[4.4] Search violations by date range`
   - Label: `Epic 4`
   - Estimate: `3`
   - Suggested owner: Dev C
   - Modules: `src/report.c`, `src/utils.c`
   - Demo outcome: date filtering by `time_t` works correctly

### Non-story operational cards

Create these cards too:

19. `[OPS-1] Set up GitHub repository and protected main`
   - Label: `tech-debt`
   - Estimate: `2`
   - Owner: project lead

20. `[OPS-2] Create PR template and review checklist`
   - Label: `tech-debt`
   - Estimate: `1`
   - Owner: Scrum Master or lead dev

21. `[OPS-3] Prepare demo dataset for review and final demo`
   - Label: `tech-debt`
   - Estimate: `2`
   - Owner: Dev D

22. `[OPS-4] Sprint retro action items`
   - Label: `tech-debt`
   - Estimate: `1`
   - Owner: rotating Scrum Master

---

## Recommended Initial Placement

On day 1, place cards like this:

### `Sprint Backlog`

- `[1.1] Scaffold project and Makefile`
- `[1.2] Define shared data types and constants`
- `[1.3] Implement utility helpers`
- `[1.4] Build file I/O and first-run initialization`
- `[1.5] Implement login/logout/session`
- `[2.1] Add member creation flow`
- `[OPS-1] Set up GitHub repository and protected main`
- `[OPS-2] Create PR template and review checklist`

### `Ready for Sprint`

- `[1.6] Implement change and reset password`
- `[2.2] Edit member and recalculate unpaid fines`
- `[2.3] Delete member and cascade related data`
- `[2.4] View profile and member list`

### `Product Backlog`

- all Epic 3 and Epic 4 cards
- remaining operational cards

---

## Daily Board Usage Rules

Use these rules every day:

1. Every developer updates their card before or during daily scrum.
2. No developer should have more than one primary card in `In Progress`.
3. Any blocker older than one day must be escalated immediately.
4. A card does not move to `Done` before merge and verification.
5. Every PR must be attached to exactly one Trello card.
6. If work expands beyond the card scope, split the card instead of hiding scope creep inside comments.

---

## Weekly Review Checklist For Scrum Master

- Are there too many cards in `In Progress`?
- Is any card stuck in `In Review` for more than 24 hours?
- Is `Blocked` actually being used?
- Does every active card have an owner, estimate, and clear next step?
- Is `Done` only accepted and merged work?
- Did the sprint backlog grow without explicit replanning?

---

## Fast Start Procedure

To stand this up quickly:

1. Create the board and the seven lists.
2. Add the labels from this blueprint.
3. Create the 22 seed cards above.
4. Put Sprint 1 cards into `Sprint Backlog`.
5. Assign owners and reviewers.
6. Start work with WIP limit of one primary card per dev.
7. Use daily scrum to update board state live.

This is enough to get the team operational without needing a more complex PM tool.
