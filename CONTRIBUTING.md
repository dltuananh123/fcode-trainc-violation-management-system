# Contributing Guide

This project uses a lightweight delivery model:

- Trello for planning and status tracking
- GitHub Flow for implementation and review
- Scrum-lite for sprint cadence

This guide outlines how the team works day-to-day.

## Core Rules

1. One Trello card maps to one branch and one pull request.
2. Work must not be pushed directly to `main`.
3. A card is not `Done` until the PR is approved, merged, and verified.
4. Keep changes small enough to review in the same day when possible.
5. If a task grows beyond 1-2 days, split it before continuing.

## Branch Naming

Use short, descriptive branch names:

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

- One logical change per commit.
- Avoid mixing refactor and feature work in the same commit.
- Commit messages should explain what changed, not just that something changed.

## Pull Request Rules

Every PR should:

- Reference exactly one Trello card.
- State the story ID.
- Summarize the user-visible or system-visible outcome.
- List the main files touched.
- Describe how the change was verified.

PRs should remain reviewable:

- Prefer small PRs.
- Open a draft PR early if interface feedback is needed.
- Do not wait until a large feature is fully complete before asking for review.

## Review Rules

Minimum merge standard:

- At least 1 approving review.
- No unresolved review comments.
- Build passes.
- Formatting/static checks pass if configured.

Reviewer expectations:

- Verify behavior against acceptance criteria.
- Check for regression risk in touched modules.
- Check persistence behavior for any write path.
- Check role/permission rules for auth and member flows.

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

- Do not keep more than one primary active card per developer.
- Move a card to `Blocked` as soon as progress depends on someone else.
- Attach the PR link to the Trello card once opened.
- Only move a card to `Done` after merge and verification.

## Sprint Rhythm

Recommended cadence:

- Sprint Planning every 2 weeks.
- Daily Scrum every workday for 15 minutes.
- Backlog refinement once mid-sprint.
- Sprint review and retrospective at sprint end.

Daily scrum should answer:

- What was completed yesterday?
- What is being worked on today?
- What is blocked?

## Project-Specific Expectations

This codebase is a standard C project with file-based persistence. Be strict about:

- `C17` compatibility.
- No unnecessary global variables.
- Immediate persistence after mutation operations.
- Small, explicit module boundaries.

Modules expected by the architecture:

- `main`
- `auth`
- `member`
- `violation`
- `fileio`
- `report`
- `utils`

## Definition of Done

A story is done only if all items below are true:

- Acceptance criteria are met.
- Code compiles cleanly.
- Data-saving behavior works for affected flows.
- Manual verification is completed.
- PR is approved.
- PR is merged.
- Trello card is moved to `Done`.

For the full checklist, see [docs/definition-of-done.md](docs/definition-of-done.md).
