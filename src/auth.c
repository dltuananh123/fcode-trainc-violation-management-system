#include "auth.h"
#include "fileio.h"
#include <stdio.h>
#include <string.h>

/* ============================================================
 * Module-private state
 * Rule: never access currentSession from outside this file.
 * ============================================================ */
static Account current_session;
static int session_active = 0;

/* ============================================================
 * Story 1.5 — Login / Logout / Session
 * ============================================================ */

int auth_login(AppDatabase *db) {
  /* TODO: Implement login flow
   * 1. Show login banner
   * 2. Prompt for MSSV and password
   * 3. Find account by studentId
   * 4. Check if locked
   * 5. Verify password
   * 6. Update failCount (increment on fail, reset on success)
   * 7. Lock account after 3 failures and exit
   * 8. Set session on success
   *
   * Return: 0 on success, -1 on locked account exit
   */
  if (db == NULL) return -1;
  return -1;
}

void auth_logout(AppDatabase *db) {
  /* TODO: Clear current session and show confirmation
   * 1. Clear session_active
   * 2. Zero out current_session
   * 3. Print logout message
   * Note: Program should NOT exit - caller loops back to login
   */
  (void)db; /* unused */
}

Account *auth_get_session(void) {
  /* TODO: Return pointer to current session
   * Returns: pointer to current_session if active, NULL otherwise
   */
  if (session_active) {
    return &current_session;
  }
  return NULL;
}

/* ============================================================
 * Story 1.6 — Change / Reset Password
 * ============================================================ */

int auth_change_password(AppDatabase *db) {
  /* TODO: Implement self-service password change
   * 1. Check session is active
   * 2. Prompt for current password
   * 3. Verify current password
   * 4. Prompt for new password twice
   * 5. Update account in database
   * 6. Save to file
   *
   * Return: 0 on success, -1 on failure
   */
  if (db == NULL) return -1;
  return -1;
}

int auth_reset_password(AppDatabase *db, const char *targetStudentId) {
  /* TODO: Implement BCN-only password reset
   * 1. Check caller is BCN (auth_get_session()->role)
   * 2. Find target account by studentId
   * 3. Set password = studentId (default)
   * 4. Reset failCount = 0, isLocked = 0
   * 5. Save to file
   *
   * Return: 0 on success, -1 on failure
   */
  if (db == NULL || targetStudentId == NULL) return -1;
  return -1;
}
