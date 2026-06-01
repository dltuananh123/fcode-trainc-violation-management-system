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

/**
 * @brief Reads a password with masked input (shows **** on screen).
 *
 * Uses _getch() on Windows for character-by-character input.
 * Each typed character shows as '*'. Backspace deletes.
 *
 * @param buffer The buffer to store the password.
 * @param size The maximum size of the buffer.
 */
void readPassword(char *buffer, size_t size);

/**
 * @brief Reads a menu choice with immediate re-prompt on invalid input.
 *
 * Loops until the user enters a valid integer within [min, max].
 *
 * @param prompt The prompt text to display.
 * @param min Minimum valid value.
 * @param max Maximum valid value.
 * @return The valid integer choice.
 */
int readMenuChoice(const char *prompt, int min, int max);

/* ============================================================
 * VALIDATION HELPERS
 * ============================================================ */

/**
 * @brief Checks if a student ID (MSSV) is valid.
 *
 * @param id The ID string to validate.
 * @return 1 if valid, 0 otherwise.
 */
int isIdValid(const char *id);

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

/* ============================================================
 * PATH & DIRECTORY HELPERS
 * ============================================================ */

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

/**
 * @brief Gets the directory where the executable is located.
 *
 * @param buffer The buffer to store the directory path.
 * @param size The size of the buffer.
 */
void getExeDir(char *buffer, size_t size);

/**
 * @brief Salter and Hasher for passwords using key stretched FNV-1a.
 *
 * @param password The plain text password.
 * @param salt The salt value.
 * @param outHashHex 32-character buffer to hold output hex string.
 */
void hashPassword(const char *password, const char *salt, char *outHashHex);

/**
 * @brief Generates a random salt.
 *
 * @param salt The output salt buffer.
 * @param size Size of the buffer.
 */
void generateSalt(char *salt, size_t size);

/**
 * @brief Safely zeroes out memory containing sensitive information like
 * passwords.
 *
 * @param ptr Pointer to the memory block.
 * @param len Size of the memory block in bytes.
 */
void secureZero(void *ptr, size_t len);

/**
 * @brief Logs an administrative action to data/system_audit.log and triggers a
 * simulated webhook to data/simulated_webhooks.log.
 *
 * @param actor The actor (e.g. BCN's student ID or username).
 * @param action The action performed.
 * @param target The target member or ID.
 */
void logSystemAction(const char *actor, const char *action, const char *target);

/**
 * @brief View system audit log with pagination and color-coded display.
 */
void viewSystemLogs(void);

#endif /* UTILS_H */
