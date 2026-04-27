#include "member.h"
#include "utils.h"
#include "fileio.h"
#include <stdio.h>
#include <string.h>

/* ============================================================
 * Story 2.1 — Add Member
 * ============================================================ */

int member_find_by_id(AppDatabase *db, const char *studentId) {
  if (db == NULL || studentId == NULL) return -1;

  for (int i = 0; i < db->memberCount; i++) {
    if (strcmp(db->members[i].studentId, studentId) == 0) {
      return i;
    }
  }

  return -1;
}

int member_validate_input(const Member *m, AppDatabase *db) {
  if (m == NULL || db == NULL) return -1;

  /* Check studentId is valid and not empty */
  if (!is_id_valid(m->studentId)) {
    printf("[LOI] MSSV khong hop le\n");
    return -1;
  }

  /* Check uniqueness */
  if (member_find_by_id(db, m->studentId) != -1) {
    printf("[LOI] MSSV da ton tai\n");
    return -1;
  }

  /* Check email format */
  if (!is_email_valid(m->email)) {
    printf("[LOI] Email khong hop le\n");
    return -1;
  }

  return 0;
}

int member_add(AppDatabase *db) {
  if (db == NULL) return -1;

  /* Check capacity */
  if (db->memberCount >= MAX_MEMBERS) {
    printf("[LOI] Da dat gioi han so luong thanh vien (%d)\n", MAX_MEMBERS);
    return -1;
  }

  printf("\nTHEM THANH VIEN MOI\n");

  Member new_member;
  memset(&new_member, 0, sizeof(Member));

  /* Input member details */
  printf("Nhap MSSV: ");
  read_string(new_member.studentId, MAX_MSSV_LEN);

  printf("Nhap ho va ten: ");
  read_string(new_member.fullName, MAX_NAME_LEN);

  printf("Nhap email: ");
  read_string(new_member.email, MAX_EMAIL_LEN);

  printf("Nhap so dien thoai: ");
  read_string(new_member.phone, MAX_PHONE_LEN);

  /* Select team */
  printf("Chon ban (0-Hoc thuat, 1-Ke hoach, 2-Nhan su, 3-Truyen thong): ");
  int team;
  if (read_int(&team) != 1) {
    printf("[LOI] Lua chon ban khong hop le\n");
    return -1;
  }
  if (team < TEAM_ACADEMIC || team > TEAM_MEDIA) {
    printf("[LOI] Ban khong hop le\n");
    return -1;
  }
  new_member.team = team;

  /* Select role */
  printf("Chon chuc vu (0-Thanh vien, 1-Truong nhom/Pho nhom, 2-Ban chu nhiem): ");
  int role;
  if (read_int(&role) != 1) {
    printf("[LOI] Lua chon chuc vu khong hop le\n");
    return -1;
  }
  if (role < MEMBER_ROLE_MEMBER || role > MEMBER_ROLE_BCN) {
    printf("[LOI] Chuc vu khong hop le\n");
    return -1;
  }
  new_member.role = role;

  /* Set default values */
  new_member.violationCount = 0;
  new_member.consecutiveAbsences = 0;
  new_member.totalFine = 0.0;
  new_member.isActive = STATUS_ACTIVE;

  /* Validate input */
  if (member_validate_input(&new_member, db) != 0) {
    return -1;
  }

  /* Add member to database */
  db->members[db->memberCount++] = new_member;

  /* Create account with default password = MSSV */
  Account new_account;
  memset(&new_account, 0, sizeof(Account));
  strncpy(new_account.studentId, new_member.studentId, MAX_MSSV_LEN - 1);
  strncpy(new_account.password, new_member.studentId, MAX_PASS_LEN - 1);
  new_account.role = ACCOUNT_ROLE_MEMBER;
  new_account.isLocked = 0;
  new_account.failCount = 0;
  db->accounts[db->accountCount++] = new_account;

  /* Save to files */
  if (fileio_save_members(db) != 0) {
    printf("[LOI] Khong the luu du lieu thanh vien\n");
    db->memberCount--;
    return -1;
  }

  if (fileio_save_accounts(db) != 0) {
    printf("[LOI] Khong the luu du lieu tai khoan\n");
    db->memberCount--;
    db->accountCount--;
    fileio_save_members(db);
    return -1;
  }

  printf("[OK] Them thanh vien thanh cong\n");
  printf("  MSSV: %s\n", new_member.studentId);
  printf("  Ten: %s\n", new_member.fullName);
  printf("  Ban: %s\n", team_name(new_member.team));
  printf("  Chuc vu: %s\n", member_role_name(new_member.role));
  printf("  Tai khoan da duoc tao voi mat khau mac dinh: %s\n", new_member.studentId);

  return 0;
}

/* ============================================================
 * Story 2.2 — Edit Member
 * ============================================================ */

int member_edit(AppDatabase *db) {
  (void)db;
  printf("[CANH BAO] Chua cai dat chuc nang sua thanh vien\n");
  return -1;
}

/* ============================================================
 * Story 2.3 — Delete Member
 * ============================================================ */

int member_delete(AppDatabase *db) {
  (void)db;
  printf("[CANH BAO] Chua cai dat chuc nang xoa thanh vien\n");
  return -1;
}

/* ============================================================
 * Story 2.4 — View Member List
 * ============================================================ */

void member_list_all(AppDatabase *db) {
  if (db == NULL) return;

  if (db->memberCount == 0) {
    printf("[THONG BAO] Chua co thanh vien nao trong du lieu\n");
    return;
  }

  printf("\nDANH SACH THANH VIEN\n");
  printf("+------+------------------+------------+-----------+\n");
  printf("| MSSV | Ho va ten        | Ban        | Chuc vu  |\n");
  printf("+------+------------------+------------+-----------+\n");

  for (int i = 0; i < db->memberCount; i++) {
    Member *m = &db->members[i];
    printf("| %-4s | %-16s | %-10s | %-9s |\n",
           m->studentId,
           m->fullName,
           team_name(m->team),
           member_role_name(m->role));
  }

  printf("+------+------------------+------------+-----------+\n");
  printf("Tong: %d thanh vien\n\n", db->memberCount);
}
