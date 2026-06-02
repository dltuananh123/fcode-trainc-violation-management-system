#ifndef AUTH_H
#define AUTH_H

/**
 * @file auth.h
 * @brief Authentication and session management — Stories 1.5 & 1.6.
 *
 * Handles user credentials, session state, and password operations.
 */

#include "types.h"

/* ============================================================
 * Story 1.5 — LOGIN / LOGOUT / SESSION
 * ============================================================ */

/**
 * @brief Run the login prompt in a loop until success or account lockout.
 *
 * @param db Pointer to the AppDatabase.
 * @return 0 on success, -1 on account lockout or exit.
 */
int authLogin(AppDatabase *db);

/**
 * @brief Clear the current session and show logout confirmation.
 *
 * @param db Pointer to the AppDatabase.
 */
void authLogout(AppDatabase *db);

/**
 * @brief Return a pointer to the current in-memory session.
 *
 * @return Account* Pointer to current session, or NULL if not logged in.
 */
Account *authGetSession(void);

/* ============================================================
 * Story 1.6 — PASSWORD MANAGEMENT
 * ============================================================ */

/**
 * @brief Allow the logged-in user to change their own password.
 *
 * @param db Pointer to the AppDatabase.
 * @param isFirstLogin If 1, bypass old password prompt (forced flow).
 * @return 0 on success, -1 on failure.
 */
int authChangePassword(AppDatabase *db, int isFirstLogin);

/**
 * @brief Director only: reset a member's password to their MSSV.
 *
 * @param db Pointer to the AppDatabase.
 * @param targetStudentId The student ID of the account to reset.
 * @return 0 on success, -1 on failure.
 */
int authResetPassword(AppDatabase *db, const char *targetStudentId);

#define REQUIRE_DIRECTOR(session)                                              \
  do {                                                                         \
    if ((session) == NULL) {                                                   \
      printf(ERR_LOI "Ban phai dang nhap de thuc hien!\n");                    \
      return RC_ERR_AUTH;                                                      \
    }                                                                          \
    if ((session)->role != ACCOUNT_ROLE_DIRECTOR) {                            \
      printf(ERR_LOI "Chi Ban Chu Nhiem moi co quyen thuc hien!\n");           \
      return RC_ERR_AUTH;                                                      \
    }                                                                          \
  } while (0)

#endif /* AUTH_H */
