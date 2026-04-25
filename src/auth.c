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
 * Private helpers
 * ============================================================ */

static void read_input(const char *prompt, char *buf, int size) {
  printf("%s", prompt);
  if (fgets(buf, size, stdin) != NULL) {
    buf[strcspn(buf, "\n")] = '\0';
  } else {
    buf[0] = '\0';
  }
}

static int find_account_index(const AppDatabase *db, const char *studentId) {
  for (int i = 0; i < db->accountCount; i++) {
    if (strcmp(db->accounts[i].studentId, studentId) == 0) {
      return i;
    }
  }
  return -1;
}

static void print_login_banner(void) {
  printf("\n");
  printf("  HE THONG QUAN LY VI PHAM - CLB F-CODE   \n");
  printf("  Vui long dang nhap de tiep tuc\n");
}

/* ============================================================
 * Story 1.5 — Login / Logout / Session
 * ============================================================ */

int auth_login(AppDatabase *db) {
  if (db == NULL) return -1;

  char studentId[MAX_MSSV_LEN];
  char password[MAX_PASS_LEN];

  print_login_banner();

  read_input("MSSV: ", studentId, MAX_MSSV_LEN);
  read_input("Mat khau: ", password, MAX_PASS_LEN);

  int idx = find_account_index(db, studentId);
  if (idx == -1) {
    printf("[LOI] Tai khoan khong ton tai\n");
    return -1;
  }

  Account *acc = &db->accounts[idx];

  if (acc->isLocked) {
    printf("[LOI] Tai khoan da bi khoa. Vui long lien he BCN\n");
    return -1;
  }

  if (strcmp(acc->password, password) != 0) {
    acc->failCount++;
    printf("[LOI] Mat khau sai\n");

    if (acc->failCount >= 3) {
      acc->isLocked = 1;
      printf("[CANH BAO] Tai khoan da bi khoa sau 3 lan dang nhap sai\n");
      /* Save to persist lock state */
      FILE *fp = fopen("data/accounts.dat.tmp", "wb");
      if (fp != NULL) {
        fwrite(&db->accountCount, sizeof(int), 1, fp);
        fwrite(db->accounts, sizeof(Account), (size_t)db->accountCount, fp);
        fclose(fp);
        remove("data/accounts.dat");
        rename("data/accounts.dat.tmp", "data/accounts.dat");
      }
      return -1;
    }

    printf("[THONG BAO] Con lai %d lan thu\n", 3 - acc->failCount);
    return -1;
  }

  /* Login successful */
  acc->failCount = 0;
  current_session = *acc;
  session_active = 1;

  printf("[OK] Dang nhap thanh cong\n");
  return 0;
}

void auth_logout(AppDatabase *db) {
  (void)db; /* unused */
  session_active = 0;
  memset(&current_session, 0, sizeof(Account));
  printf("[OK] Da dang xuat\n");
}

Account *auth_get_session(void) {
  if (session_active) {
    return &current_session;
  }
  return NULL;
}

/* ============================================================
 * Story 1.6 — Change / Reset Password
 * ============================================================ */

int auth_change_password(AppDatabase *db) {
  /* TODO: Implement in later commit */
  (void)db;
  printf("[CANH BAO] Chua cai dat chuc nang doi mat khau\n");
  return -1;
}

int auth_reset_password(AppDatabase *db, const char *targetStudentId) {
  /* TODO: Implement in later commit */
  (void)db;
  (void)targetStudentId;
  printf("[CANH BAO] Chua cai dat chuc nang reset mat khau\n");
  return -1;
}
