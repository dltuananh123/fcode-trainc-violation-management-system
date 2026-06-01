#ifndef VALIDATE_H
#define VALIDATE_H

/**
 * @file validate.h
 * @brief Comprehensive input validation for all system fields.
 *
 * Every function returns 1 on success, 0 on failure.
 * Error messages are printed directly to stdout with colored prefixes.
 */

#include "types.h"

/* ============================================================
 * MSSV — FPT Student ID (format: XY123456, exactly 8 chars)
 *   X = Campus (S=HCM, H=HN, D=DN, C=CT, Q=QN)
 *   Y = Division (E=Engineering, A=Arts, S=Social)
 *   123456 = 6 digits
 * ============================================================ */

/** Validate MSSV format AND uniqueness in database. */
int validateMSSV(const char *mssv, const AppDatabase *db);

/** Validate MSSV format only (no uniqueness check). */
int validateMSSVFormat(const char *mssv);

/** Decode MSSV campus letter to Vietnamese name. */
const char *mssvCampusName(char campus);

/** Decode MSSV division letter to Vietnamese name. */
const char *mssvDivisionName(char division);

/** Auto-uppercase an MSSV string in-place. */
void mssvAutoUpper(char *mssv);

/* ============================================================
 * FULL NAME (2-5 words, letters only, Vietnamese OK)
 * ============================================================ */

/** Validate full name: 2-5 words, max 50 chars, letters+spaces only. */
int validateName(const char *name);

/** Auto-fix name: capitalize each word, trim spaces, remove consecutive spaces.
 */
void nameAutoFix(char *name);

/* ============================================================
 * EMAIL
 * ============================================================ */

/** Validate email format. */
int validateEmail(const char *email);

/** Validate email AND check uniqueness (excludeId = MSSV to skip, or NULL). */
int validateEmailUnique(const char *email, const AppDatabase *db,
                        const char *excludeId);

/* ============================================================
 * PHONE — Vietnamese only (10 digits, starts with 0, valid prefix)
 * ============================================================ */

/** Validate Vietnamese phone number. */
int validatePhone(const char *phone);

/** Normalize phone: +84/84 → 0x, strip spaces/dashes. Stores in same buffer. */
void phoneNormalize(char *phone);

/** Get carrier name from normalized phone number. */
const char *phoneCarrier(const char *phone);

/** Validate phone AND check uniqueness. */
int validatePhoneUnique(const char *phone, const AppDatabase *db,
                        const char *excludeId);

/* ============================================================
 * PASSWORD
 * ============================================================ */

/** Validate new password: min 6, max 30, has letter, has digit, no spaces. */
int validatePassword(const char *password);

/* ============================================================
 * GENERAL INPUT VALIDATION
 * ============================================================ */

/** Check string is not empty and not whitespace-only. */
int validateNotEmpty(const char *input);

/** Check integer is within [min, max] range. */
int validateInRange(int value, int min, int max);

/** Sanitize input: trim spaces, remove control chars, check injection chars.
 *  Returns 1 if clean, 0 if dangerous characters found. */
int sanitizeInput(char *input);

/** Check for SQL injection characters (S1-S9).
 *  Returns 1 if CLEAN, 0 if dangerous characters found.
 *  Prints specific error message for each forbidden char found. */
int checkInjectionChars(const char *input);

/** Validate date format dd/mm/yyyy with full calendar checks. */
int validateDate(const char *date);

/** Validate date range: start <= end, both valid, not future. */
int validateDateRange(const char *start, const char *end);

/* ============================================================
 * HELPER UTILITIES
 * ============================================================ */

/** Trim leading and trailing whitespace in-place. */
void trimSpaces(char *str);

/** Check if string contains only letters (a-z, A-Z) and spaces.
 *  Allows Vietnamese diacritics (UTF-8 bytes 0x80-0xBF are skipped). */
int isLettersOnly(const char *str);

/** Count words in a string (separated by spaces). */
int countWords(const char *str);

#endif /* VALIDATE_H */
