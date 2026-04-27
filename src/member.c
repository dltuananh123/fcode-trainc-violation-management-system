#include "member.h"
#include "fileio.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

/* ============================================================
 * Story 2.1 — Add Member
 * ============================================================ */

int member_find_by_id(AppDatabase *db, const char *studentId) {
  if (db == NULL || studentId == NULL)
    return -1;

  for (int i = 0; i < db->memberCount; i++) {
    if (strcmp(db->members[i].studentId, studentId) == 0) {
      return i;
    }
  }

  return -1;
}

int member_validate_input(const Member *m, AppDatabase *db) {
  if (m == NULL || db == NULL)
    return -1;

  /* Check studentId not empty */
  if (strlen(m->studentId) == 0) {
    printf("[LOI] MSSV khong duoc de trong\n");
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
  if (db == NULL)
    return -1;

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
  printf(
      "Chon chuc vu (0-Thanh vien, 1-Truong nhom/Pho nhom, 2-Ban chu nhiem): ");
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
  printf("  Tai khoan da duoc tao voi mat khau mac dinh: %s\n",
         new_member.studentId);

  return 0;
}

/* ============================================================
 * Story 2.2 — Edit Member
 * ============================================================ */

int member_edit(AppDatabase *db) {
  if (db == NULL)
    return -1;

  char studentId[MAX_MSSV_LEN];
  printf("\nSUA THONG TIN THANH VIEN\n");
  printf("Nhap MSSV can sua: ");
  read_string(studentId, MAX_MSSV_LEN);

  int memberIndex = member_find_by_id(db, studentId);
  if (memberIndex == -1) {
    printf("[LOI] Khong tim thay thanh vien voi MSSV: %s\n", studentId);
    return -1;
  }

  Member *m = &db->members[memberIndex];
  char buffer[MAX_NAME_LEN];

  printf("\nTHONG TIN HIEN TAI:\n");
  printf("  Ho va ten: %s\n", m->fullName);
  printf("  Email: %s\n", m->email);
  printf("  So dien thoai: %s\n", m->phone);
  printf("  Ban: %s\n", team_name(m->team));
  printf("  Chuc vu: %s\n", member_role_name(m->role));
  printf("  Trang thai: %s\n", m->isActive ? "Hoat dong" : "Da Out CLB");

  printf("\nNHAP THONG TIN MOI (Nhan Enter de giu nguyen):\n");

  printf("Ho va ten moi: ");
  read_string(buffer, MAX_NAME_LEN);
  if (strlen(buffer) > 0) {
    strncpy(m->fullName, buffer, MAX_NAME_LEN - 1);
    m->fullName[MAX_NAME_LEN - 1] = '\0';
  }

  printf("Email moi: ");
  read_string(buffer, MAX_EMAIL_LEN);
  if (strlen(buffer) > 0) {
    if (!is_email_valid(buffer)) {
      printf("[LOI] Email khong hop le, giu nguyen email cu\n");
    } else {
      strncpy(m->email, buffer, MAX_EMAIL_LEN - 1);
      m->email[MAX_EMAIL_LEN - 1] = '\0';
    }
  }

  printf("So dien thoai moi: ");
  read_string(buffer, MAX_PHONE_LEN);
  if (strlen(buffer) > 0) {
    strncpy(m->phone, buffer, MAX_PHONE_LEN - 1);
    m->phone[MAX_PHONE_LEN - 1] = '\0';
  }

  printf("Ban moi (0-Hoc thuat, 1-Ke hoach, 2-Nhan su, 3-Truyen thong) [-1 de "
         "giu nguyen]: ");
  int newTeam;
  if (read_int(&newTeam)) {
    if (newTeam >= TEAM_ACADEMIC && newTeam <= TEAM_MEDIA) {
      m->team = newTeam;
    } else if (newTeam != -1) {
      printf("[LOI] Ban khong hop le, giu nguyen ban cu\n");
    }
  }

  printf("Chuc vu moi (0-Thanh vien, 1-Truong nhom/Pho nhom, 2-Ban chu nhiem) "
         "[-1 de giu nguyen]: ");
  int newRole;
  int roleChanged = 0;
  if (read_int(&newRole)) {
    if (newRole >= MEMBER_ROLE_MEMBER && newRole <= MEMBER_ROLE_BCN) {
      if (m->role != newRole) {
        m->role = newRole;
        roleChanged = 1;
      }
    } else if (newRole != -1) {
      printf("[LOI] Chuc vu khong hop le, giu nguyen chuc vu cu\n");
    }
  }

  printf("Trang thai (1-Hoat dong, 0-Da Out CLB) [-1 de giu nguyen]: ");
  int newStatus;
  if (read_int(&newStatus)) {
    if (newStatus == STATUS_ACTIVE || newStatus == STATUS_OUT_CLB) {
      m->isActive = newStatus;
    } else if (newStatus != -1) {
      printf("[LOI] Trang thai khong hop le, giu nguyen trang thai cu\n");
    }
  }

  if (roleChanged) {
    double newFineRate = (m->role == MEMBER_ROLE_MEMBER) ? 20000.0 : 50000.0;
    m->totalFine = 0.0;

    for (int i = 0; i < db->violationCount; i++) {
      Violation *v = &db->violations[i];
      if (strcmp(v->studentId, m->studentId) == 0 && v->isPaid == 0) {
        if (v->reason != REASON_VIOLENCE) {
          v->fine = newFineRate;
        }
        m->totalFine += v->fine;
      }
    }

    if (fileio_save_violations(db) != 0) {
      printf(
          "[LOI] Khong the luu du lieu vi pham sau khi tinh lai tien phat\n");
      return -1;
    }
    printf("[THONG BAO] Da tinh lai tien phat cho cac vi pham chua dong do "
           "thay doi chuc vu\n");
  }

  if (fileio_save_members(db) != 0) {
    printf("[LOI] Khong the luu du lieu thanh vien\n");
    return -1;
  }

  printf("[OK] Sua thong tin thanh vien thanh cong\n");
  return 0;
}

/* ============================================================
 * Story 2.3 — Delete Member
 * ============================================================ */

int member_delete(AppDatabase *db) {
  if (db == NULL)
    return -1;

  char studentId[MAX_MSSV_LEN];
  printf("\nXOA THANH VIEN\n");
  printf("Nhap MSSV can xoa: ");
  read_string(studentId, MAX_MSSV_LEN);

  int memberIndex = member_find_by_id(db, studentId);
  if (memberIndex == -1) {
    printf("[LOI] Khong tim thay thanh vien voi MSSV: %s\n", studentId);
    return -1;
  }

  Member *m = &db->members[memberIndex];
  printf("\nTHONG TIN THANH VIEN:\n");
  printf("  Ho va ten: %s\n", m->fullName);
  printf("  Ban: %s\n", team_name(m->team));
  printf("  Chuc vu: %s\n", member_role_name(m->role));

  printf("\nXac nhan xoa thanh vien nay va toan bo du lieu lien quan? (1: "
         "Co, 0: Khong): ");
  int confirm;
  if (read_int(&confirm) != 1 || confirm != 1) {
    printf("[THONG BAO] Da huy xoa thanh vien.\n");
    return 0;
  }

  /* 1. Xoa thanh vien */
  for (int i = memberIndex; i < db->memberCount - 1; i++) {
    db->members[i] = db->members[i + 1];
  }
  db->memberCount--;

  /* 2. Xoa vi pham lien quan */
  int v_index = 0;
  while (v_index < db->violationCount) {
    if (strcmp(db->violations[v_index].studentId, studentId) == 0) {
      for (int j = v_index; j < db->violationCount - 1; j++) {
        db->violations[j] = db->violations[j + 1];
      }
      db->violationCount--;
    } else {
      v_index++;
    }
  }

  /* 3. Xoa tai khoan lien quan */
  int a_index = 0;
  while (a_index < db->accountCount) {
    if (strcmp(db->accounts[a_index].studentId, studentId) == 0) {
      for (int j = a_index; j < db->accountCount - 1; j++) {
        db->accounts[j] = db->accounts[j + 1];
      }
      db->accountCount--;
    } else {
      a_index++;
    }
  }

  /* 4. Luu vao file */
  if (fileio_save_members(db) != 0) {
    printf("[LOI] Khong the luu du lieu thanh vien sau khi xoa\n");
    return -1;
  }
  if (fileio_save_violations(db) != 0) {
    printf("[LOI] Khong the luu du lieu vi pham sau khi xoa\n");
    return -1;
  }
  if (fileio_save_accounts(db) != 0) {
    printf("[LOI] Khong the luu du lieu tai khoan sau khi xoa\n");
    return -1;
  }

  printf("[OK] Xoa thanh vien thanh cong.\n");
  return 0;
}

/* ============================================================
 * Story 2.4 — View Member List
 * ============================================================ */

void member_list_all(AppDatabase *db) {
  if (db == NULL)
    return;

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
    printf("| %-4s | %-16s | %-10s | %-9s |\n", m->studentId, m->fullName,
           team_name(m->team), member_role_name(m->role));
  }

  printf("+------+------------------+------------+-----------+\n");
  printf("Tong: %d thanh vien\n\n", db->memberCount);
}
