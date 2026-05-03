#ifndef REPORT_H
#define REPORT_H

/**
 * @file report.h
 * @brief Report module — Story 4.1.
 *
 * Provides statistics and export functionalities for the system.
 */

#include "types.h"

/* ============================================================
 * REPORT EXPORT FUNCTIONS
 * ============================================================ */

/**
 * @brief Displays statistics of fines by team.
 *
 * Calculates the total fines collected and outstanding for each
 * defined team by cross-referencing members and violations.
 * Displays the result in a formatted terminal table.
 *
 * @param db Pointer to the global application database.
 */
void reportTeamStats(const AppDatabase *db);

/* ============================================================
 * Story 4.2 — Sort Members by Violation Count
 * ============================================================ */

/**
 * @brief Sorts and displays members by violation count.
 *
 * Prompts BCN to choose ascending or descending order, then builds
 * a pointer-array copy (Member* sorted[MAX_MEMBERS]) and sorts it
 * using a self-implemented algorithm (selection sort). The original
 * members[] array order is never modified.
 *
 * Displayed columns: Ho ten, MSSV, Ban, So lan vi pham.
 *
 * @param db Pointer to the global application database.
 */
void reportSortByViolations(const AppDatabase *db);

#endif /* REPORT_H */
