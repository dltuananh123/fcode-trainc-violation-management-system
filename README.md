# F-Code TrainC Violation Management System

A CLI-based violation tracking system for F-Code club members, developed in C to manage infractions, penalties, and member status transparently.

## Overview

This project addresses the need for tracking F-Code club member violations against club rules, recording penalties, monitoring outstanding debts, and assisting the Executive Board in handling issues consistently and fairly.

Based on the requirements, the system targets the following core functionalities:

- Authentication: Login, logout, password change/reset
- Member management: Add, edit, delete, view member profiles and lists
- Violation recording: Track violation history and calculate penalties based on roles
- Payment tracking: Monitor paid/unpaid penalty status
- Alerts: Warn about Out CLB thresholds and generate file-based reports
- Data persistence: Save data to files to prevent loss on program exit

Currently, the repository is in the initial scaffold phase: basic `Makefile`, directory structure, formatting/linting tools, and minimal `src/main.c` to verify the build pipeline.

## Technologies and Constraints

- **Language**: C17
- **Application Type**: CLI/Terminal
- **Storage Mechanism**: `.dat` files
- **Restrictions**: No databases, GUIs, or external libraries beyond the C standard library

## Related Documentation

- Team Workflow: [`CONTRIBUTING.md`](CONTRIBUTING.md)
- Business Requirements: [`docs/requirement-docs/QuanLyViPhamCLBFCode_V1.md`](docs/requirement-docs/QuanLyViPhamCLBFCode_V1.md)
- Architecture and Module Design: [`docs/planning/architecture.md`](docs/planning/architecture.md)
- Story List: [`docs/stories/`](docs/stories/)

## Architectural Overview

Per the current design, the project is expected to be modularized as follows:

- `main`: Menu navigation and main program loop
- `auth`: Login, logout, password change/reset
- `member`: Member CRUD operations
- `violation`: Violation recording, penalty calculation, Out CLB alerts
- `fileio`: Read/write `members.dat`, `violations.dat`, `accounts.dat`
- `report`: Statistics and report generation
- `utils`: Validation and shared utility functions
