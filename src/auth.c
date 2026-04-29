#include "auth.h"
#include "fileio.h"
#include "types.h"
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

static void readInput(const char *prompt, char *buf, int size) {
  printf("%s", prompt);
  if (fgets(buf, size, stdin) != NULL) {
    buf[strcspn(buf, "\n")] = '\0';
  } else {
    buf[0] = '\0';
  }
}

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
  printf("  HE THONG QUAN LY VI PHAM - CLB F-CODE   \n");
  printf("  Vui long dang nhap de tiep tuc\n");
}

/* ============================================================
 * Story 1.5 — Login / Logout / Session
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

    readInput("MSSV: ", studentId, MAX_MSSV_LEN);
    readInput("Mat khau: ", password, MAX_PASS_LEN);

    int idx = findAccountIndex(db, studentId);
    if (idx == -1) {
      printf("[LOI] Tai khoan khong ton tai\n");
      continue;
    }

    Account *acc = &db->accounts[idx];

    if (acc->isLocked) {
      printf("[LOI] Tai khoan da bi khoa. Vui long lien he BCN\n");
      continue;
    }

    if (strcmp(acc->password, password) != 0) {
      acc->failCount++;
      printf("[LOI] Mat khau sai\n");

      if (acc->failCount >= 3) {
        acc->isLocked = 1;
        printf("[CANH BAO] Tai khoan da bi khoa sau 3 lan dang nhap sai\n");
        if (fileioSaveAccounts(db) != 0) {
          printf("[LOI] Khong the luu trang thai khoa tai khoan\n");
        }
        return -1;
      }

      if (fileioSaveAccounts(db) != 0) {
        printf("[LOI] Khong the luu so lan dang nhap sai\n");
        return -1;
      }

      printf("[THONG BAO] Con lai %d lan thu\n", 3 - acc->failCount);
      continue;
    }

    /* Login successful */
    acc->failCount = 0;
    if (fileioSaveAccounts(db) != 0) {
      printf("[LOI] Khong the cap nhat trang thai tai khoan sau dang nhap\n");
      return -1;
    }
    currentSession = *acc;
    sessionActive = 1;

    printf("[OK] Dang nhap thanh cong\n");
    return 0;
  }
}

void authLogout(AppDatabase *db) {
  (void)db; /* unused */
  sessionActive = 0;
  memset(&currentSession, 0, sizeof(Account));
  printf("[OK] Da dang xuat\n");
}

Account *authGetSession(void) {
  if (sessionActive) {
    return &currentSession;
  }
  return NULL;
}

/* ============================================================
 * Story 1.6 — Change / Reset Password
 * ============================================================ */

int authChangePassword(AppDatabase *db) {
  /* TODO: Implement in later commit */
  (void)db;
  printf("[CANH BAO] Chua cai dat chuc nang doi mat khau\n");
  return -1;
}

int authResetPassword(AppDatabase *db, const char *targetStudentId) {
  /* TODO: Implement in later commit */
  (void)db;
  (void)targetStudentId;
  printf("[CANH BAO] Chua cai dat chuc nang reset mat khau\n");
  return -1;
}
