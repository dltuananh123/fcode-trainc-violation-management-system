#ifndef MEMBER_H
#define MEMBER_H

/**
 * @file member.h
 * @brief Member management module — Stories 2.1 to 2.4.
 *
 * Provides CRUD operations for CLB members, including
 * cascade deletion of violations and accounts.
 */

#include "types.h"

/* ============================================================
 * Story 2.1 — Add Member
 * ============================================================ */

/**
 * @brief Add a new member to the system.
 *
 * Prompts BCN for member information, validates input, creates both
 * Member and Account records, and persists to files.
 *
 * @param db Pointer to the AppDatabase
 * @return 0 on success, -1 on failure
 */
int memberAdd(AppDatabase *db);

/**
 * @brief Find a member by their student ID.
 *
 * @param db Pointer to the AppDatabase
 * @param studentId The student ID to search for
 * @return Index of member in array, or -1 if not found
 */
int memberFindById(const AppDatabase *db, const char *studentId);

/**
 * @brief Search members by name (case-insensitive substring match).
 *
 * @param db Pointer to the AppDatabase
 * @param keyword Substring to search for in fullName
 * @param outIndices Output array to store matching member indices
 * @param maxResults Maximum number of results to return
 * @return Number of matches found
 */
int memberSearchByName(const AppDatabase *db, const char *keyword,
                       int *outIndices, int maxResults);

/**
 * @brief Validate member input data.
 *
 * Checks: studentId not empty, unique, email format valid
 *
 * @param m Pointer to the Member to validate
 * @param db Pointer to the AppDatabase (for uniqueness check)
 * @return 0 if valid, -1 if invalid
 */
int memberValidateInput(const Member *m, const AppDatabase *db);

/* ============================================================
 * Story 2.2 — Edit Member
 * ============================================================ */

/**
 * @brief Edit an existing member's information.
 *
 * Note: MSSV cannot be changed. If role changes, recalculate fines.
 *
 * @param db Pointer to the AppDatabase
 * @return 0 on success, -1 on failure
 */
int memberEdit(AppDatabase *db);

/* ============================================================
 * Story 2.3 — Delete Member
 * ============================================================ */

/**
 * @brief Delete a member and all associated data.
 *
 * Cascades: member, violations, account records.
 * Requires BCN confirmation before deletion.
 *
 * @param db Pointer to the AppDatabase
 * @return 0 on success, -1 on failure
 */
int memberDelete(AppDatabase *db);

/* ============================================================
 * Story 2.4 — View Member List
 * ============================================================ */

/**
 * @brief View the profile of the currently logged-in member.
 *
 * Shows all personal information including fines and violation count.
 * Requires an active session.
 *
 * @param db Pointer to the AppDatabase
 */
void memberViewProfile(AppDatabase *db);

/**
 * @brief Display list of all members.
 *
 * Shows: fullName, studentId, team, role only.
 * Hides: email, phone, violationCount, fines.
 * Includes pagination (20 members per page).
 *
 * @param db Pointer to the AppDatabase
 */
void memberListAll(AppDatabase *db);

/**
 * @brief View archived (soft-deleted) members and restore them.
 *
 * @param db Pointer to the AppDatabase.
 */
void memberViewArchive(AppDatabase *db);

/**
 * @brief Freeze or unfreeze an account.
 *
 * @param db Pointer to the AppDatabase.
 */
void memberFreezeAccount(AppDatabase *db);

#endif /* MEMBER_H */
