#ifndef MEMBER_H
#define MEMBER_H

/* member.h — Stories 2.1, 2.2, 2.3, 2.4
 * Member CRUD operations and display functions.
 *
 * Dependencies: types.h (structs + constants). */

#include "types.h"

/* ============================================================
 * Shared helper — used by multiple modules
 * ============================================================ */

/* Find a member by studentId.
 * Returns the index in db->members[], or -1 if not found. */
int member_find_by_id(const AppDatabase *db, const char *studentId);

/* ============================================================
 * Story 2.1 — Add Member
 * ============================================================ */

/* BCN adds a new member with full profile info.
 * Also creates a matching Account with password = MSSV.
 * Validates: MSSV uniqueness, email format, capacity limit.
 * Returns 0 on success, -1 on failure. */
int member_add(AppDatabase *db);

/* ============================================================
 * Story 2.2 — Edit Member
 * ============================================================ */

/* BCN edits member fields (all except MSSV).
 * If role changes, recalculates all unpaid violation fines.
 * Returns 0 on success, -1 on failure. */
int member_edit(AppDatabase *db);

/* ============================================================
 * Story 2.3 — Delete Member (cascade)
 * ============================================================ */

/* BCN deletes a member by MSSV.
 * Cascade: removes member, all their violations, and their account.
 * Requires confirmation before deleting. Persists all three .dat files.
 * Returns 0 on success, -1 on failure or cancellation. */
int member_delete(AppDatabase *db);

/* ============================================================
 * Story 2.4 — View Profile & Member List
 * ============================================================ */

/* Display full profile of the currently logged-in user.
 * Uses auth_get_session() to determine which member to show.
 * Returns 0 on success, -1 if member not found. */
int member_view_profile(const AppDatabase *db);

/* Display all members in a table with restricted columns:
 * ho ten, MSSV, ban, chuc vu only.
 * Paginated (20 per page).
 * Returns 0 on success. */
int member_list_all(const AppDatabase *db);

#endif /* MEMBER_H */
