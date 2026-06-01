# Story 2.3: Search and View Member Details (Replacing Delete Member)

Status: done

## Story

As a BCN admin,
I want to search for a member by Name or MSSV and view their complete detailed profile and history,
so that I can easily assess their status, contact details, role, and discipline/violation history in one place, since member deletion is already handled securely via Kick/Restore.

## Acceptance Criteria

1. **Given** BCN selects Option 3: "Tim kiem & xem CT TV"
   **When** they enter a valid MSSV or a fuzzy Name keyword
   **Then** the system retrieves and lists matching members (if multiple) or navigates directly to the profile details (if single match)
2. **Given** BCN is viewing the member details card
   **When** the details are displayed
   **Then** the card shows Full Name, MSSV, Email, Phone, Team, Role, Active Status, Consecutive Absences, Violation count, and Total Fines
   **And** it lists all recorded violations of this member with their timestamp, reason, fine, notes, and payment status
   **And** the UI box aligns perfectly with a width of exactly 70 characters.

## Tasks / Subtasks

- [x] Remove the redundant soft-delete function (`memberDelete`)
- [x] Implement fuzzy search by name and exact search by MSSV (`memberSearchDetails`)
- [x] Design a premium detailed member dashboard card aligning perfectly to 70 character width
- [x] Retrieve and output the list of all violations matching the searched member
- [x] Wire Option 3 in the BCN Menu to the new function in `main.c`

## Dev Notes

- The old "Delete Member" was redundant because BCN uses **Kick and Restore** (Option 18) to soft-delete and restore members, while maintaining full history.
- The new `memberSearchDetails` allows BCN to safely view any member's detailed profile (including private info like email and phone, which are hidden from the public member list).
