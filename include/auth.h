#ifndef AUTH_H
#define AUTH_H

/* auth.h — Story 1.5 & 1.6
 * Authentication, session management, and password operations.
 *
 * Session rule: currentSession is static inside auth.c.
 * All other modules access it ONLY through auth_get_session().
 * Never use extern globals for session state. */

#include "types.h"

/* ============================================================
 * Story 1.5 — Login / Logout / Session
 * ============================================================ */

/* Run the login prompt in a loop until:
 *   - Login succeeds → returns 0 (caller shows menu)
 *   - Account locked after 3rd failed attempt → returns -1 (caller exits)
 *
 * A pre-locked account shows an error but does NOT exit — user can
 * try a different MSSV. Only reaching failCount >= 3 in a live session
 * triggers exit. */
int auth_login(AppDatabase *db);

/* Clear the current session. Show logout confirmation message.
 * The program must NOT exit — caller loops back to auth_login(). */
void auth_logout(AppDatabase *db);

/* Return a pointer to the current in-memory session Account.
 * Returns NULL if no user is currently logged in.
 * Use: auth_get_session()->role to check access level. */
Account *auth_get_session(void);

/* ============================================================
 * Story 1.6 — Change / Reset Password
 * ============================================================ */

/* Self-service: logged-in user changes their own password.
 * Requires correct current password before accepting new one.
 * Returns 0 on success, -1 on validation or auth failure. */
int auth_change_password(AppDatabase *db);

/* BCN only: reset a member's password to their MSSV (default password).
 * Any non-BCN caller is rejected immediately.
 * Returns 0 on success, -1 on permission or lookup failure. */
int auth_reset_password(AppDatabase *db, const char *targetStudentId);

#endif /* AUTH_H */