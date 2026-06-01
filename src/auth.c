#include "auth.h"
#include "fileio.h"
#include "types.h"
#include "ui.h"
#include "utils.h"
#include "validate.h"
#include <stdio.h>
#include <string.h>

/* ============================================================
 * Module-private state
 * Rule: never access currentSession from outside this file.
 * ============================================================ */
static Account currentSession;
static int sessionActive = 0;

/* ============================================================
 * Private helpers
 * ============================================================ */

static int findAccountIndex(const AppDatabase *db, const char *studentId) {
  for (int i = 0; i < db->accountCount; i++) {
    if (strcmp(db->accounts[i].studentId, studentId) == 0) {
      return i;
    }
  }
  return -1;
}

static void printLoginBanner(void) {
  printf("\n");
  printf(COLOR_BLUE COLOR_BOLD);
  printf("  " BOX_TL);
  for (int i = 0; i < LOGIN_BOX_W; i++) {
    printf(BOX_H);
  }
  printf(BOX_TR "\n");
  printf("  " BOX_V "                                          " BOX_V "\n");
  printf("  " BOX_V "    FCODE VIOLATION MANAGEMENT SYSTEM     " BOX_V "\n");
  printf("  " BOX_V "    Quan ly Vi pham TrainC CLB            " BOX_V "\n");
  printf("  " BOX_V "                                          " BOX_V "\n");
  printf("  " BOX_BL);
  for (int i = 0; i < LOGIN_BOX_W; i++) {
    printf(BOX_H);
  }
  printf(BOX_BR "\n");
  printf(COLOR_RESET "\n");
}

/* ============================================================
 * Login / Logout / Session
 * ============================================================ */

int authLogin(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  char studentId[MAX_MSSV_LEN];
  char password[MAX_PASS_LEN];

  /* Loop until success or account lockout */
  while (1) {
    printLoginBanner();

    /* MSSV input with re-prompt */
    while (1) {
      printf(COLOR_CYAN "  MSSV: " COLOR_RESET);
      readString(studentId, sizeof(studentId));
      trimSpaces(studentId);
      if (validateNotEmpty(studentId)) {
        break;
      }
    }

    /* Password input with masking */
    printf(COLOR_CYAN "  Mat khau: " COLOR_RESET);
    readPassword(password, sizeof(password));

    if (strlen(password) == 0) {
      printf(ERR_LOI "Mat khau khong duoc de trong!\n");
      secureZero(password, sizeof(password));
      continue;
    }

    int idx = findAccountIndex(db, studentId);
    if (idx == -1) {
      printf(ERR_LOI "Tai khoan khong ton tai!\n");
      secureZero(password, sizeof(password));
      continue;
    }

    Account *acc = &db->accounts[idx];

    if (acc->isLocked) {
      printf(ERR_LOI "Tai khoan da bi khoa! "
                     "Vui long lien he BCN de mo khoa.\n");
      secureZero(password, sizeof(password));
      continue;
    }

    char inputHashed[MAX_PASS_LEN];
    /* Backward-compatible migration: If legacy account has no salt, generate
     * one and hash password in-place */
    if (strlen(acc->salt) == 0) {
      generateSalt(acc->salt, sizeof(acc->salt));
      char oldPlain[MAX_PASS_LEN];
      strncpy(oldPlain, acc->password, MAX_PASS_LEN - 1);
      oldPlain[MAX_PASS_LEN - 1] = '\0';
      hashPassword(oldPlain, acc->salt, acc->password);
      (void)fileioSaveAccounts(db);
    }

    hashPassword(password, acc->salt, inputHashed);
    if (strcmp(acc->password, inputHashed) != 0) {
      acc->failCount++;
      printf(ERR_LOI "Mat khau sai!\n");

      if (acc->failCount >= 3) {
        acc->isLocked = 1;
        printf(ERR_CANH_BAO "Tai khoan da bi khoa sau 3 lan nhap sai! "
                            "Lien he BCN de mo khoa.\n");
        (void)fileioSaveAccounts(db);
        secureZero(password, sizeof(password));
        return -1;
      }

      (void)fileioSaveAccounts(db);
      printf(ERR_INFO "Con %d lan thu.\n", 3 - acc->failCount);
      secureZero(password, sizeof(password));
      continue;
    }

    /* Login successful */
    acc->failCount = 0;
    (void)fileioSaveAccounts(db);
    currentSession = *acc;
    sessionActive = 1;

    printf(ERR_OK "Dang nhap thanh cong! "
                  "Chao mung %s (%s)\n",
           studentId, accountRoleName(acc->role));

    /* Force change default password if needed */
    if (acc->isDefaultPassword) {
      printf("\n");
      printf(ERR_CANH_BAO "Ban dang su dung mat khau mac dinh!\n");
      printf("Bat buoc doi mat khau truoc khi tiep tuc.\n\n");

      /* Must change password before proceeding */
      int changed = 0;
      while (!changed) {
        changed = (authChangePassword(db) == 0);
        if (!changed) {
          printf(ERR_LOI "Ban phai doi mat khau truoc khi "
                         "su dung he thong!\n\n");
        }
      }

      /* Update isDefaultPassword */
      idx = findAccountIndex(db, studentId);
      if (idx != -1) {
        db->accounts[idx].isDefaultPassword = 0;
        currentSession.isDefaultPassword = 0;
        (void)fileioSaveAccounts(db);
      }
    }

    secureZero(password, sizeof(password));
    return 0;
  }
}

void authLogout(AppDatabase *db) {
  (void)db; /* unused */
  sessionActive = 0;
  memset(&currentSession, 0, sizeof(Account));
  printf(ERR_OK "Da dang xuat!\n");
  uiClear();
}

Account *authGetSession(void) {
  if (sessionActive) {
    return &currentSession;
  }
  return NULL;
}

/* ============================================================
 * Change / Reset Password
 * ============================================================ */

int authChangePassword(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  Account *session = authGetSession();
  if (session == NULL) {
    printf(ERR_LOI "Ban phai dang nhap de doi mat khau!\n");
    return -1;
  }

  /* Find account in database */
  int idx = findAccountIndex(db, session->studentId);
  if (idx == -1) {
    printf(ERR_LOI "Khong tim thay tai khoan!\n");
    return -1;
  }

  /* Old password — re-prompt on invalid */
  char oldPass[MAX_PASS_LEN];
  while (1) {
    printf(COLOR_CYAN "  Nhap mat khau cu (0 de quay lai): " COLOR_RESET);
    readPassword(oldPass, sizeof(oldPass));
    if (strlen(oldPass) == 0) {
      printf(ERR_LOI "Vui long nhap mat khau cu!\n");
      continue;
    }
    if (strcmp(oldPass, "0") == 0) {
      printf(ERR_INFO "Huy doi mat khau.\n");
      secureZero(oldPass, sizeof(oldPass));
      return -1;
    }

    char oldHashed[MAX_PASS_LEN];
    /* If legacy account has no salt, generate one and hash it */
    if (strlen(db->accounts[idx].salt) == 0) {
      generateSalt(db->accounts[idx].salt, sizeof(db->accounts[idx].salt));
      char oldPlain[MAX_PASS_LEN];
      strncpy(oldPlain, db->accounts[idx].password, MAX_PASS_LEN - 1);
      oldPlain[MAX_PASS_LEN - 1] = '\0';
      hashPassword(oldPlain, db->accounts[idx].salt,
                   db->accounts[idx].password);
      (void)fileioSaveAccounts(db);
    }

    hashPassword(oldPass, db->accounts[idx].salt, oldHashed);
    if (strcmp(db->accounts[idx].password, oldHashed) == 0) {
      secureZero(oldHashed, sizeof(oldHashed));
      break;
    }
    printf(ERR_LOI "Mat khau cu khong dung!\n");
    secureZero(oldHashed, sizeof(oldHashed));
  }

  /* New password with confirmation loop */
  char newPass[MAX_PASS_LEN];
  char confirmPass[MAX_PASS_LEN];
  while (1) {
    printf(COLOR_CYAN "  Nhap mat khau moi (0 de huy): " COLOR_RESET);
    readPassword(newPass, sizeof(newPass));

    if (strlen(newPass) == 0) {
      printf(ERR_LOI "Vui long nhap mat khau moi!\n");
      continue;
    }

    if (strcmp(newPass, "0") == 0) {
      printf(ERR_INFO "Huy doi mat khau.\n");
      secureZero(oldPass, sizeof(oldPass));
      secureZero(newPass, sizeof(newPass));
      return -1;
    }

    if (!validatePassword(newPass)) {
      continue;
    }

    if (strcmp(newPass, oldPass) == 0) {
      printf(ERR_LOI "Mat khau moi phai khac mat khau cu!\n");
      continue;
    }

    printf(COLOR_CYAN "  Xac nhan mat khau moi (0 de huy): " COLOR_RESET);
    readPassword(confirmPass, sizeof(confirmPass));

    if (strcmp(confirmPass, "0") == 0) {
      printf(ERR_INFO "Huy doi mat khau.\n");
      secureZero(oldPass, sizeof(oldPass));
      secureZero(newPass, sizeof(newPass));
      secureZero(confirmPass, sizeof(confirmPass));
      return -1;
    }

    if (strlen(confirmPass) == 0) {
      printf(ERR_LOI "Vui long xac nhan mat khau!\n");
      continue;
    }

    if (strcmp(newPass, confirmPass) != 0) {
      printf(ERR_LOI "Mat khau xac nhan khong khop! "
                     "Vui long nhap lai mat khau moi.\n");
      continue;
    }

    break;
  }

  /* Update password and session with a fresh salt and hash */
  generateSalt(db->accounts[idx].salt, sizeof(db->accounts[idx].salt));
  hashPassword(newPass, db->accounts[idx].salt, db->accounts[idx].password);

  strncpy(session->salt, db->accounts[idx].salt, MAX_SALT_LEN - 1);
  session->salt[MAX_SALT_LEN - 1] = '\0';
  strncpy(session->password, db->accounts[idx].password, MAX_PASS_LEN - 1);
  session->password[MAX_PASS_LEN - 1] = '\0';

  secureZero(oldPass, sizeof(oldPass));
  secureZero(newPass, sizeof(newPass));
  secureZero(confirmPass, sizeof(confirmPass));

  if (fileioSaveAccounts(db) != 0) {
    printf(ERR_LOI "Khong the luu mat khau moi!\n");
    return -1;
  }

  printf(ERR_OK "Doi mat khau thanh cong!\n");
  return 0;
}

int authResetPassword(AppDatabase *db, const char *targetStudentId) {
  if (db == NULL || targetStudentId == NULL) {
    return RC_ERR_NULL;
  }

  Account *session = authGetSession();
  REQUIRE_BCN(session);

  int idx = findAccountIndex(db, targetStudentId);
  if (idx == -1) {
    printf(ERR_LOI "Khong tim thay tai khoan voi MSSV: %s!\n", targetStudentId);
    return RC_ERR_NOT_FOUND;
  }

  /* Reset password to MSSV, unlock, mark as default using fresh salt and hash
   */
  generateSalt(db->accounts[idx].salt, sizeof(db->accounts[idx].salt));
  hashPassword(targetStudentId, db->accounts[idx].salt,
               db->accounts[idx].password);
  db->accounts[idx].failCount = 0;
  db->accounts[idx].isLocked = 0;
  db->accounts[idx].isDefaultPassword = 1;

  if (fileioSaveAccounts(db) != 0) {
    printf(ERR_LOI "Khong the luu mat khau moi!\n");
    return RC_ERR_IO;
  }

  printf(ERR_OK "Da reset mat khau cua %s ve MSSV. "
                "Thanh vien se phai doi mat khau khi dang nhap.\n",
         targetStudentId);
  return RC_OK;
}
