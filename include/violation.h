#ifndef VIOLATION_H
#define VIOLATION_H

/* violation.h — Story 3.1 & 3.2
 * Violation recording, Out CLB warning/enforcement.
 *
 * Depends on: types.h, member.h (for memberFindById) */

#include "types.h"

/* ============================================================
 * Story 3.1 — Record Violation
 * ============================================================ */

/**
 * @brief Record a new violation for a member.
 *
 * Prompts BCN for MSSV, violation reason, calculates fine based
 * on member role, and auto-records timestamp. Handles special
 * cases: violence (immediate Out CLB) and absences (consecutive
 * absence tracking + Out CLB threshold check).
 *
 * @param db Pointer to the AppDatabase
 * @return 0 on success, -1 on failure
 */
int violationRecord(AppDatabase *db);

/* ============================================================
 * Story 3.2 — Out CLB Warning & Enforcement
 * ============================================================ */

/**
 * @brief Check Out CLB threshold for a specific member after
 *        recording an absence violation.
 *
 * Rules:
 *   - consecutiveAbsences == 3 → warning message
 *   - consecutiveAbsences >= 4 → Out CLB prompt, BCN confirms
 *
 * @param db     Pointer to the AppDatabase (for saving)
 * @param member Pointer to the Member to check
 * @return 0 if no action taken or warning only,
 *         1 if member was set to Out CLB,
 *        -1 on error
 */
int violationCheckOutThreshold(AppDatabase *db, Member *member);

/**
 * @brief Scan all active members and display those at or above
 *        the Out CLB warning threshold.
 *
 * Used by admin menu option 8 for manual threshold review.
 * Does NOT modify any data — display only.
 *
 * @param db Pointer to the AppDatabase
 */
void violationCheckAllOutClb(AppDatabase *db);

#endif /* VIOLATION_H */

/* ============================================================
 * Story 3.3 — Mark Paid & View Own Violations/Fines
 * ============================================================ */

/**
 * @brief View all violations of the currently logged-in member.
 */
void violationViewOwn(AppDatabase *db);

/**
 * @brief View unpaid fines and total debt of the currently logged-in member.
 */
void violationViewFines(AppDatabase *db);

/**
 * @brief Admin marks a specific violation as paid and recalculates totalFine.
 */
int violationMarkPaid(AppDatabase *db);

