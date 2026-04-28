#ifndef FILEIO_H
#define FILEIO_H

/**
 * @file fileio.h
 * @brief File I/O module — Story 1.4.
 *
 * Handles binary data persistence for members, violations, and accounts.
 * Uses atomic temp-file strategy to prevent data corruption.
 */

#include "types.h"

/**
 * @brief Load all data from .dat files into the AppDatabase.
 *
 * Creates empty files and a default ADMIN account on first run.
 *
 * @param db Pointer to the AppDatabase.
 * @return 0 on success, -1 on failure.
 */
int fileioLoadAll(AppDatabase *db);

/**
 * @brief Save all member records to members.dat.
 *
 * @param db Pointer to the AppDatabase.
 * @return 0 on success, -1 on failure.
 */
int fileioSaveMembers(AppDatabase *db);

/**
 * @brief Save all violation records to violations.dat.
 *
 * @param db Pointer to the AppDatabase.
 * @return 0 on success, -1 on failure.
 */
int fileioSaveViolations(AppDatabase *db);

/**
 * @brief Save all account records to accounts.dat.
 *
 * @param db Pointer to the AppDatabase.
 * @return 0 on success, -1 on failure.
 */
int fileioSaveAccounts(AppDatabase *db);

#endif /* FILEIO_H */
