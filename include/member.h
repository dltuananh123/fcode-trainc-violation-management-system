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
 * @brief Rebuild the sorted member index.
 *
 * @param db Pointer to the AppDatabase
 */
void memberRebuildIndex(AppDatabase *db);

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

/**
 * @brief Edit a member's fields in-place (used for pre-confirmation editing).
 *
 * @param db Pointer to the AppDatabase
 * @param m Pointer to the Member struct to modify
 * @param canEditMSSV Boolean flag indicating if MSSV can be edited
 * @param isFirstRun Boolean flag indicating if this is called during first-run
 * setup
 * @return 0 on success, -1 if cancelled/aborted
 */
int memberEditInPlace(AppDatabase *db, Member *m, int canEditMSSV,
                      int isFirstRun);

/* ============================================================
 * Story 2.3 — Search and View Member Details
 * ============================================================ */

/**
 * @brief Search for a member by Name or MSSV and display their complete
 * details.
 *
 * Provides a detailed summary including contact info, role, active status,
 * consecutive absences, violation statistics, and recent violations history.
 *
 * @param db Pointer to the AppDatabase
 * @return 0 on success, -1 on failure
 */
int memberSearchDetails(AppDatabase *db);

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
 * @brief Display personal statistics dashboard for current user.
 *
 * Shows total violations, paid/unpaid fines, violation breakdown by reason.
 * @param db Pointer to the AppDatabase
 */
void memberViewStats(AppDatabase *db);

/**
 * @brief Display the Out CLB threshold and current violation count/absences
 * status for current user.
 * @param db Pointer to the AppDatabase
 */
void memberViewOutThreshold(AppDatabase *db);

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
 * @return 0 on success, or ResultCode error
 */
int memberViewArchive(AppDatabase *db);

/**
 * @brief Kick a member (forces Out CLB + locks account) or restore/readmit
 * them.
 *
 * @param db Pointer to the AppDatabase.
 * @return 0 on success, or ResultCode error
 */
int memberKickOrRestore(AppDatabase *db);

int memberViewKicked(AppDatabase *db);

void memberPurgeExpired(AppDatabase *db, int retentionDays);

#endif /* MEMBER_H */
