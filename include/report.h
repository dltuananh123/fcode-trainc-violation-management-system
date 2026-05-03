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

#endif /* REPORT_H */

/* ============================================================
 * 4.3 REPORT VIOLATION TXT
 * ============================================================ */
/**
 * @brief Explore report violations and collected to a timestamped .txt file.
 * * @param db Pointer to the global application database.
 * @return 0 if success, -1 if fail.
 */
int reportExportTxt(const AppDatabase *db);