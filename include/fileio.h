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

/**
 * @brief Applies XOR encryption/decryption to a data buffer in-place.
 *
 * Uses a machine-derived XOR key. Calling twice on the same buffer
 * restores the original data (symmetric operation).
 *
 * @param data Pointer to the data buffer.
 * @param size Size of the data buffer in bytes.
 */
void xorBuffer(unsigned char *data, size_t size);

/**
 * @brief Export all database files (members, violations, accounts) into a
 * single archive file. Encrypts the archive data using a 4-digit PIN.
 */
int fileioExportArchive(AppDatabase *db);

/**
 * @brief Import database files from an archive.
 * Decrypts using a 4-digit PIN.
 */
int fileioImportArchive(AppDatabase *db);

#endif /* FILEIO_H */
