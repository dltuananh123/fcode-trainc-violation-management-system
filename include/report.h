#ifndef REPORT_H
#define REPORT_H

/**
 * @file report.h
 * @brief Report module - Stories 4.1, 4.2, and 4.3.
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

/**
 * @brief Display members sorted by violation count.
 *
 * Uses a pointer-array copy of members and a self-implemented sort.
 * The original members[] array order remains unchanged.
 *
 * @param db Pointer to the global application database.
 */
void reportSortMembersByViolations(const AppDatabase *db);

/**
 * @brief Export a violation report to a timestamped text file.
 *
 * The report includes team summaries and members with outstanding fines.
 *
 * @param db Pointer to the global application database.
 */
void reportExportTxt(const AppDatabase *db);

/**
 * @brief Displays a comprehensive, beautiful discipline and financial dashboard.
 *
 * Includes top 5 violators, violation reason analysis, and fine collection progress.
 *
 * @param db Pointer to the global database.
 */
void reportDashboard(const AppDatabase *db);

#endif /* REPORT_H */
