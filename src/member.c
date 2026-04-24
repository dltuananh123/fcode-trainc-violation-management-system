#include "member.h"
#include "auth.h"
#include "fileio.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

/* ============================================================
 * Shared helper
 * ============================================================ */

int member_find_by_id(const AppDatabase *db, const char *studentId) {
    for (int i = 0; i < db->memberCount; i++) {
        if (strcmp(db->members[i].studentId, studentId) == 0) {
            return i;
        }
    }
    return -1;
}

/* ============================================================
 * Private helpers
 * ============================================================ */

/* Find an account by studentId.
 * Returns the index in db->accounts[], or -1 if not found. */
static int findAccountIndex(const AppDatabase *db, const char *studentId) {
    for (int i = 0; i < db->accountCount; i++) {
        if (strcmp(db->accounts[i].studentId, studentId) == 0) {
            return i;
        }
    }
    return -1;
}

/* Display a single member's info for confirmation before delete. */
static void printMemberInfo(const Member *m) {
    printf("  MSSV     : %s\n", m->studentId);
    printf("  Ho ten   : %s\n", m->fullName);
    printf("  Email    : %s\n", m->email);
    printf("  SDT      : %s\n", m->phone);
    printf("  Ban      : %s\n", team_name(m->team));
    printf("  Chuc vu  : %s\n", member_role_name(m->role));
    printf("  Trang thai: %s\n",
           m->isActive == STATUS_ACTIVE ? "Active" : "Out CLB");
}

/* Remove element at index from members[] by shifting. */
static void removeMemberAt(AppDatabase *db, int idx) {
    for (int i = idx; i < db->memberCount - 1; i++) {
        db->members[i] = db->members[i + 1];
    }
    db->memberCount--;
}

/* Remove element at index from accounts[] by shifting. */
static void removeAccountAt(AppDatabase *db, int idx) {
    for (int i = idx; i < db->accountCount - 1; i++) {
        db->accounts[i] = db->accounts[i + 1];
    }
    db->accountCount--;
}

/* Remove ALL violations matching studentId by shifting.
 * Returns the number of violations removed. */
static int removeViolationsByStudentId(AppDatabase *db,
                                       const char *studentId) {
    int removed = 0;
    int i = 0;
    while (i < db->violationCount) {
        if (strcmp(db->violations[i].studentId, studentId) == 0) {
            /* Shift remaining elements left. */
            for (int j = i; j < db->violationCount - 1; j++) {
                db->violations[j] = db->violations[j + 1];
            }
            db->violationCount--;
            removed++;
            /* Do NOT increment i — new element shifted into position i. */
        } else {
            i++;
        }
    }
    return removed;
}

/* ============================================================
 * Story 2.3 — Delete Member (cascade)
 *
 * Data flow (from architecture.md):
 *   member_delete()
 *     → ask confirmation
 *     → if confirmed:
 *         remove from members[]           (shift array)
 *         remove all matching violations[] (shift array)
 *         remove matching from accounts[]  (shift array)
 *         → fileio_save_members()
 *         → fileio_save_violations()
 *         → fileio_save_accounts()
 * ============================================================ */

int member_delete(AppDatabase *db) {
    /* Permission check: only BCN can delete members. */
    Account *session = auth_get_session();
    if (session == NULL) {
        printf("[LOI] Chua dang nhap.\n");
        return -1;
    }
    if (session->role != ACCOUNT_ROLE_BCN) {
        printf("[LOI] Chi BCN moi co quyen xoa thanh vien.\n");
        return -1;
    }

    /* Prompt for target MSSV. */
    char targetId[MAX_MSSV_LEN];
    printf("\n=== XOA THANH VIEN ===\n");
    printf("Nhap MSSV can xoa: ");
    read_string(targetId, sizeof(targetId));

    /* Look up the member. */
    int memberIdx = member_find_by_id(db, targetId);
    if (memberIdx < 0) {
        printf("[LOI] Khong tim thay thanh vien voi MSSV [%s].\n", targetId);
        return -1;
    }

    /* Show member info for confirmation. */
    printf("\nThong tin thanh vien se bi xoa:\n");
    printMemberInfo(&db->members[memberIdx]);

    /* Ask for confirmation (architecture requires this). */
    char confirm[4];
    printf("\nBan co chac chan muon xoa? (y/n): ");
    read_string(confirm, sizeof(confirm));

    if (confirm[0] != 'y' && confirm[0] != 'Y') {
        printf("[OK] Da huy thao tac xoa.\n");
        return -1;
    }

    /* === CASCADE DELETE === */

    /* 1. Remove member from members[]. */
    removeMemberAt(db, memberIdx);

    /* 2. Remove all violations belonging to this studentId. */
    int violationsRemoved = removeViolationsByStudentId(db, targetId);

    /* 3. Remove matching account from accounts[]. */
    int accountIdx = findAccountIndex(db, targetId);
    if (accountIdx >= 0) {
        removeAccountAt(db, accountIdx);
    }

    /* 4. Persist all three data stores. */
    fileio_save_members(db);
    fileio_save_violations(db);
    fileio_save_accounts(db);

    printf("[OK] Da xoa thanh vien [%s].\n", targetId);
    printf("     Xoa %d vi pham lien quan.\n", violationsRemoved);
    return 0;
}

/* ============================================================
 * Stub implementations — to be completed in their own stories.
 * ============================================================ */

int member_add(AppDatabase *db) {
    (void)db;
    printf("[CANH BAO] Chuc nang them thanh vien chua duoc cai dat.\n");
    return -1;
}

int member_edit(AppDatabase *db) {
    (void)db;
    printf("[CANH BAO] Chuc nang sua thanh vien chua duoc cai dat.\n");
    return -1;
}

int member_view_profile(const AppDatabase *db) {
    (void)db;
    printf("[CANH BAO] Chuc nang xem ho so chua duoc cai dat.\n");
    return -1;
}

int member_list_all(const AppDatabase *db) {
    (void)db;
    printf("[CANH BAO] Chuc nang xem danh sach chua duoc cai dat.\n");
    return -1;
}
