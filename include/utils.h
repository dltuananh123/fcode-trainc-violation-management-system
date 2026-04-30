#ifndef UTILS_H
#define UTILS_H

/**
 * @file utils.h
 * @brief Utility module — Story 1.3.
 *
 * Provides shared helpers for input handling, validation,
 * time/date formatting, and display name mapping.
 */

#include <stddef.h>
#include <time.h>

/* ============================================================
 * INPUT HANDLING HELPERS
 * ============================================================ */

/**
 * @brief Reads a string from stdin safely, stripping the trailing newline.
 *
 * @param buffer The buffer to store the string.
 * @param size The maximum size of the buffer.
 */
void readString(char *buffer, size_t size);

/**
 * @brief Reads an integer from stdin and clears the remaining buffer safely.
 *
 * @param value Pointer to the integer to store the read value.
 * @return 1 on success, 0 on failure.
 */
int readInt(int *value);

/* ============================================================
 * VALIDATION HELPERS
 * ============================================================ */

/**
 * @brief Checks if an email is valid (contains '@' and '.').
 *
 * @param email The email string to validate.
 * @return 1 if valid, 0 otherwise.
 */
int isEmailValid(const char *email);

/**
 * @brief Checks if a student ID (MSSV) is valid.
 *
 * @param id The ID string to validate.
 * @return 1 if valid, 0 otherwise.
 */
int isIdValid(const char *id);

/**
 * @brief Checks if a phone number is valid (digits only, 7-15 chars, optional leading '+').
 *
 * @param phone The phone string to validate.
 * @return 1 if valid, 0 otherwise.
 */
int is_phone_valid(const char *phone);

/* ============================================================
 * TIME & DATE HELPERS
 * ============================================================ */

/**
 * @brief Formats a time_t value into a readable string (dd/mm/yyyy HH:MM).
 *
 * @param t The time value to format.
 * @param buffer The buffer to store the formatted string.
 * @param bufSize The size of the buffer.
 */
void formatTime(time_t t, char *buffer, size_t bufSize);

/**
 * @brief Parses a date string (dd/mm/yyyy) into a time_t value.
 *
 * @param buffer The input string to parse.
 * @param t Pointer to the time_t variable to store the result.
 * @param isEndOfDay If 1, sets time to 23:59:59. If 0, sets to 00:00:00.
 * @return 1 on success, 0 on failure.
 */
int parseDate(const char *buffer, time_t *t, int isEndOfDay);

/* ============================================================
 * DISPLAY NAME MAPPERS
 * ============================================================ */

/**
 * @brief Returns the Vietnamese display string for a team ID.
 *
 * @param teamId The team ID (from types.h).
 * @return A constant string representing the team name.
 */
const char *teamName(int teamId);

/**
 * @brief Returns the Vietnamese display string for a member role.
 *
 * @param roleId The role ID (from types.h).
 * @return A constant string representing the member role name.
 */
const char *memberRoleName(int roleId);

/**
 * @brief Returns the Vietnamese display string for an account role.
 *
 * @param roleId The account role ID (from types.h).
 * @return A constant string representing the account role name.
 */
const char *accountRoleName(int roleId);

/**
 * @brief Returns the Vietnamese display string for a violation reason.
 *
 * @param reasonId The reason ID (from types.h).
 * @return A constant string representing the reason name.
 */
const char *reasonName(int reasonId);

#endif /* UTILS_H */
