#include "auth.h"
#include "fileio.h"
 
#include <stdio.h>
#include <string.h>
 
/* ============================================================
 * Module-private state
 * Rule: never access currentSession from outside this file.
 * Use auth_get_session() instead.
 * ============================================================ */
static Account current_session;
static int     session_active = 0;
 
/* ============================================================
 * Private helpers
 * ============================================================ */
 
/* Read a line from stdin into buf, strip newline.
 * Replaces raw fgets calls throughout the module. */
static void read_input(const char *prompt, char *buf, int size) {
    printf("%s", prompt);
    if (fgets(buf, size, stdin) != NULL) {
        buf[strcspn(buf, "\n")] = '\0';
    } else {
        buf[0] = '\0';
    }
}
 
/* Linear scan — accounts[] is never large enough to need a hash.
 * Returns the index in db->accounts[], or -1 if not found. */
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
    printf("============================================\n");
    printf("  HE THONG QUAN LY VI PHAM - CLB F-CODE   \n");
    printf("============================================\n");
    printf("  Vui long dang nhap de tiep tuc\n");
    printf("--------------------------------------------\n");
}
 
/* ============================================================
 * Single login attempt
 *
 * Returns  1: credentials accepted, session set.
 * Returns  0: incorrect credentials — caller should retry.
 * Returns -1: account has been locked this attempt — caller exits.
 * ============================================================ */
static int try_login(AppDatabase *db) {
    char mssv[MAX_MSSV_LEN];
    char pass[MAX_PASS_LEN];
 
    print_login_banner();
    read_input("MSSV    : ", mssv, (int)sizeof(mssv));
    read_input("Mat khau: ", pass, (int)sizeof(pass));
 
    int idx = find_account_index(db, mssv);
    if (idx < 0) {
        printf("[LOI] MSSV khong ton tai trong he thong.\n");
        return 0;
    }
 
    Account *acc = &db->accounts[idx];
 
    /* Pre-locked account: show error but allow trying another MSSV. */
    if (acc->isLocked) {
        printf("[LOI] Tai khoan [%s] da bi khoa.\n", mssv);
        printf("      Lien he BCN de duoc ho tro mo khoa.\n");
        return 0;
    }
 
    /* Wrong password. */
    if (strcmp(acc->password, pass) != 0) {
        acc->failCount++;
 
        if (acc->failCount >= 3) {
            acc->isLocked = 1;
            fileio_save_accounts(db); /* Persist failCount + isLocked. */
            printf("[LOI] Nhap sai mat khau lan thu 3.\n");
            printf("      Tai khoan [%s] da bi khoa. Chuong trinh se thoat.\n", mssv);
            return -1; /* Signal program exit. */
        }
 
        fileio_save_accounts(db); /* Persist updated failCount. */
        printf("[LOI] Mat khau khong chinh xac. Con %d lan thu.\n",
               3 - acc->failCount);
        return 0;
    }
 
    /* Correct password — reset fail counter and set session. */
    acc->failCount = 0;
    fileio_save_accounts(db);
 
    current_session = *acc;
    session_active  = 1;
 
    printf("[OK] Dang nhap thanh cong! Xin chao, %s.\n", acc->studentId);
    return 1;
}
 
/* ============================================================
 * Public API — Story 1.5
 * ============================================================ */
 
int auth_login(AppDatabase *db) {
    int result;
    /* Loop until success (1) or fatal lockout (-1). */
    do {
        result = try_login(db);
    } while (result == 0);
 
    return (result == 1) ? 0 : -1;
}
 
void auth_logout(AppDatabase *db) {
    (void)db; /* No file I/O needed on logout. */
    memset(&current_session, 0, sizeof(Account));
    session_active = 0;
    printf("\n[OK] Da dang xuat. Hen gap lai!\n");
}
 
Account *auth_get_session(void) {
    if (!session_active) return NULL;
    return &current_session;
}
 
/* ============================================================
 * Public API — Story 1.6
 * ============================================================ */
 
int auth_change_password(AppDatabase *db) {
    if (!session_active) {
        printf("[LOI] Chua dang nhap.\n");
        return -1;
    }
 
    int idx = find_account_index(db, current_session.studentId);
    if (idx < 0) {
        printf("[LOI] Khong tim thay tai khoan trong he thong.\n");
        return -1;
    }
 
    Account *acc = &db->accounts[idx];
    char old_pass[MAX_PASS_LEN];
    char new_pass[MAX_PASS_LEN];
 
    printf("\n=== DOI MAT KHAU ===\n");
    read_input("Mat khau cu : ", old_pass, (int)sizeof(old_pass));
 
    if (strcmp(acc->password, old_pass) != 0) {
        printf("[LOI] Mat khau cu khong dung.\n");
        return -1;
    }
 
    read_input("Mat khau moi: ", new_pass, (int)sizeof(new_pass));
 
    if (new_pass[0] == '\0') {
        printf("[LOI] Mat khau moi khong duoc de trong.\n");
        return -1;
    }
 
    strncpy(acc->password, new_pass, MAX_PASS_LEN - 1);
    acc->password[MAX_PASS_LEN - 1] = '\0';
 
    /* Update in-memory session to reflect the new password. */
    current_session = *acc;
 
    fileio_save_accounts(db);
    printf("[OK] Doi mat khau thanh cong.\n");
    return 0;
}
 
int auth_reset_password(AppDatabase *db, const char *targetStudentId) {
    if (!session_active) {
        printf("[LOI] Chua dang nhap.\n");
        return -1;
    }
 
    /* Only BCN is allowed to reset other members' passwords. */
    if (current_session.role != ACCOUNT_ROLE_BCN) {
        printf("[LOI] Chi BCN moi co quyen reset mat khau.\n");
        return -1;
    }
 
    int idx = find_account_index(db, targetStudentId);
    if (idx < 0) {
        printf("[LOI] Khong tim thay tai khoan voi MSSV [%s].\n",
               targetStudentId);
        return -1;
    }
 
    Account *acc = &db->accounts[idx];
 
    /* Reset password to MSSV (default password per architecture). */
    strncpy(acc->password, acc->studentId, MAX_PASS_LEN - 1);
    acc->password[MAX_PASS_LEN - 1] = '\0';
 
    /* Implementation decision: also unlock the account and reset
     * failCount, so BCN does not need a separate "unlock" action.
     * This is not explicitly required by the spec but is the
     * only practical way to recover a locked account. */
    acc->isLocked  = 0;
    acc->failCount = 0;
 
    fileio_save_accounts(db);
    printf("[OK] Da reset mat khau cua [%s] ve mac dinh (MSSV).\n",
           targetStudentId);
    return 0;
}