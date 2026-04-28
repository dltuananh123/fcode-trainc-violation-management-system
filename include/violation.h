#ifndef VIOLATION_H
#define VIOLATION_H

/* violation.h — Story 3.1 & 3.2
 * Violation recording, Out CLB warning/enforcement.
 *
 * Depends on: types.h, member.h (for member_find_by_id) */

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
int violation_record(AppDatabase *db);

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
int violation_check_out_threshold(AppDatabase *db, Member *member);

/**
 * @brief Scan all active members and display those at or above
 *        the Out CLB warning threshold.
 *
 * Used by admin menu option 8 for manual threshold review.
 * Does NOT modify any data — display only.
 *
 * @param db Pointer to the AppDatabase
 */
void violation_check_all_out_clb(AppDatabase *db);

#endif /* VIOLATION_H */
