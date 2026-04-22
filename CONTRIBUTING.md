# Contributing Guide

This project uses a lightweight delivery model:

- Trello for planning and status tracking
- GitHub Flow for implementation and review
- Scrum-lite for sprint cadence

This guide defines how the team should work day to day.

## Core Rules

1. One Trello card should map to one branch and one pull request.
2. Work must not go directly into `main`.
3. A card is not `Done` until the PR is approved, merged, and verified.
4. Keep changes small enough to review in the same day when possible.
5. If a task grows beyond 1-2 days, split it before continuing.

## Branch Naming

Use short descriptive branch names:

- `feat/story-1.1-project-scaffold`
- `feat/story-2.1-add-member`
- `feat/story-3.1-record-violation`
- `fix/story-1.5-login-lockout`
- `chore/setup-ci`

## Commit Style

Use focused commits with a module-oriented prefix:

- `feat(auth): add login attempt lockout`
- `feat(member): add member creation flow`
- `fix(fileio): persist accounts after password reset`
- `refactor(report): simplify stats aggregation`

Rules:

- one logical change per commit
- avoid mixing refactor and feature work in the same commit
- commit messages should explain what changed, not just that something changed

## Pull Request Rules

Every PR should:

- reference exactly one Trello card
- state the story ID
- summarize the user-visible or system-visible outcome
- list the main files touched
- describe how the change was verified

PRs should stay reviewable:

- prefer small PRs
- open a draft PR early if interface feedback is needed
- do not wait until a large feature is fully complete before asking for review

## Review Rules

Minimum merge standard:

- at least 1 approving review
- no unresolved review comments
- build passes
- formatting/static checks pass if configured

Reviewer expectations:

- verify behavior against acceptance criteria
- check for regression risk in touched modules
- check persistence behavior for any write path
- check role/permission rules for auth and member flows

## Trello Usage Rules

Board flow:

- `Product Backlog`
- `Ready for Sprint`
- `Sprint Backlog`
- `In Progress`
- `In Review`
- `Blocked`
- `Done`

Rules:

- do not keep more than one primary active card per developer
- move a card to `Blocked` as soon as progress depends on someone else
- attach the PR link to the Trello card once opened
- only move a card to `Done` after merge and verification

## Sprint Rhythm

Recommended cadence:

- Sprint Planning every 2 weeks
- Daily Scrum every workday for 15 minutes
- Backlog refinement once mid-sprint
- Sprint review and retrospective at sprint end

Daily scrum should answer:

- What was completed yesterday?
- What is being worked on today?
- What is blocked?

## Project-Specific Expectations

This codebase is a standard C project with file-based persistence. Be strict about:

- `C17` compatibility
- `gcc -std=c17 -Wall -Wextra`
- no unnecessary global variables
- immediate persistence after mutation operations
- small, explicit module boundaries

Modules expected by the architecture:

- `main`
- `auth`
- `member`
- `violation`
- `fileio`
- `report`
- `utils`

## Definition Of Done

A story is done only if all items below are true:

- acceptance criteria are met
- code compiles cleanly
- data-saving behavior works for affected flows
- manual verification is completed
- PR is approved
- PR is merged
- Trello card is moved to `Done`

For the full checklist, see [docs/definition-of-done.md](docs/definition-of-done.md).
