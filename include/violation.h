#ifndef VIOLATION_H
#define VIOLATION_H

/**
 * @file violation.h
 * @brief Violation management module - Stories 3.1, 3.2, 3.3, 3.4, and 4.4.
 *
 * Handles recording violations, penalty calculation, threshold checks,
 * member self-service views, admin payment marking, and filtered
 * admin reporting views.
 */

#include "types.h"

/* ============================================================
 * Story 3.1 - Record Violation
 * ============================================================ */

int violationRecord(AppDatabase *db);

/* ============================================================
 * Story 3.2 - Out CLB Warning & Enforcement
 * ============================================================ */

int violationCheckOutThreshold(AppDatabase *db, Member *member);
void violationCheckAllOutClb(AppDatabase *db);

/* ============================================================
 * Story 3.3 - Mark Paid & View Own Violations/Fines
 * ============================================================ */

void violationViewOwn(AppDatabase *db);
void violationViewFines(AppDatabase *db);
void violationViewPaymentHistory(AppDatabase *db);
int violationMarkPaid(AppDatabase *db);

/* ============================================================
 * Story 3.4 - View All Violations with Filters
 * ============================================================ */

void violationViewAllFiltered(AppDatabase *db);

void violationViewByMSSV(AppDatabase *db);

/* ============================================================
 * Story 4.4 - Search Violations by Date Range
 * ============================================================ */

void violationSearchByDate(AppDatabase *db);

int violationVoid(AppDatabase *db);

#endif /* VIOLATION_H */
