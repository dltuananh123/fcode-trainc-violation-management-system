#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <time.h>

#include "types.h"

/* =========================================
 * INPUT HANDLING HELPERS
 * ========================================= */

/**
 * @brief Reads a string from stdin safely, stripping the trailing newline.
 *
 * @param buffer The buffer to store the string.
 * @param size The maximum size of the buffer.
 */
void read_string(char *buffer, size_t size);

/**
 * @brief Reads an integer from stdin and clears the remaining buffer safely.
 *
 * @param value Pointer to the integer to store the read value.
 * @return 1 on success, 0 on failure.
 */
int read_int(int *value);

/* =========================================
 * VALIDATION HELPERS
 * ========================================= */

/**
 * @brief Checks if an email is valid (contains '@' and '.').
 *
 * @param email The email string to validate.
 * @return 1 if valid, 0 otherwise.
 */
int is_email_valid(const char *email);

/**
 * @brief Checks if a student ID (MSSV) is valid.
 *
 * @param id The ID string to validate.
 * @return 1 if valid, 0 otherwise.
 */
int is_id_valid(const char *id);

/* =========================================
 * TIME & DATE HELPERS
 * ========================================= */

/**
 * @brief Formats a time_t value into a readable string (dd/mm/yyyy HH:MM).
 *
 * @param t The time value to format.
 * @param buffer The buffer to store the formatted string.
 * @param buf_size The size of the buffer.
 */
void format_time(time_t t, char *buffer, size_t buf_size);

/**
 * @brief Parses a date string (dd/mm/yyyy) into a time_t value.
 *
 * @param buffer The input string to parse.
 * @param t Pointer to the time_t variable to store the result.
 * @param is_end_of_day If 1, sets time to 23:59:59. If 0, sets to 00:00:00.
 * @return 1 on success, 0 on failure.
 */
int parse_date(const char *buffer, time_t *t, int is_end_of_day);

/* =========================================
 * DISPLAY NAME MAPPERS
 * ========================================= */

/**
 * @brief Returns the Vietnamese display string for a team ID.
 *
 * @param team_id The team ID (from types.h).
 * @return A constant string representing the team name.
 */
const char *team_name(int team_id);

/**
 * @brief Returns the Vietnamese display string for a member role.
 *
 * @param role_id The role ID (from types.h).
 * @return A constant string representing the member role name.
 */
const char *member_role_name(int role_id);

/**
 * @brief Returns the Vietnamese display string for an account role.
 *
 * @param role_id The account role ID (from types.h).
 * @return A constant string representing the account role name.
 */
const char *account_role_name(int role_id);

/**
 * @brief Returns the Vietnamese display string for a violation reason.
 *
 * @param reason_id The reason ID (from types.h).
 * @return A constant string representing the reason name.
 */
const char *reason_name(int reason_id);

#endif /* UTILS_H */
