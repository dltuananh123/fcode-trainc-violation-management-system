#include "member.h"
#include "auth.h"
#include "fileio.h"
#include "types.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

/* ============================================================
 * Story 2.1 — Add Member
 * ============================================================ */

int memberFindById(const AppDatabase *db, const char *studentId) {
  if (db == NULL || studentId == NULL) {
    return -1;
  }

  for (int i = 0; i < db->memberCount; i++) {
    if (strcmp(db->members[i].studentId, studentId) == 0) {
      return i;
    }
  }

  return -1;
}

int memberValidateInput(const Member *m, const AppDatabase *db) {
  if (m == NULL || db == NULL) {
    return -1;
  }

  /* Check name is not empty */
  if (strlen(m->fullName) == 0) {
    printf("[LOI] Ho va ten khong duoc de trong\n");
    return -1;
  }

  /* Check email format */
  if (!is_email_valid(m->email)) {
    printf("[LOI] Email khong hop le\n");
    return -1;
  }

  /* Check phone is not empty */
  if (strlen(m->phone) == 0) {
    printf("[LOI] So dien thoai khong duoc de trong\n");
    return -1;
  }

  return 0;
}

int memberAdd(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  /* Check capacity */
  if (db->memberCount >= MAX_MEMBERS) {
    printf("[LOI] Da dat gioi han so luong thanh vien (%d)\n", MAX_MEMBERS);
    return -1;
  }

  printf("\nTHEM THANH VIEN MOI\n");

  Member newMember;
  memset(&newMember, 0, sizeof(Member));

  /* Input MSSV and validate immediately (Fix #5) */
  printf("Nhap MSSV: ");
  read_string(newMember.studentId, MAX_MSSV_LEN);
  if (!is_id_valid(newMember.studentId)) {
    printf("[LOI] MSSV khong hop le\n");
    return -1;
  }
  if (memberFindById(db, newMember.studentId) != -1) {
    printf("[LOI] MSSV da ton tai\n");
    return -1;
  }

  /* Input rest of member details */
  printf("Nhap ho va ten: ");
  read_string(newMember.fullName, MAX_NAME_LEN);

  printf("Nhap email: ");
  read_string(newMember.email, MAX_EMAIL_LEN);

  printf("Nhap so dien thoai: ");
  read_string(newMember.phone, MAX_PHONE_LEN);

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
  newMember.team = team;

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
  newMember.role = role;

  /* Set default values */
  newMember.violationCount = 0;
  newMember.consecutiveAbsences = 0;
  newMember.totalFine = 0.0;
  newMember.isActive = STATUS_ACTIVE;

  /* Validate remaining input (name, email, phone) */
  if (memberValidateInput(&newMember, db) != 0) {
    return -1;
  }

  /* Add member to database */
  db->members[db->memberCount++] = newMember;

  /* Create account with default password = MSSV */
  Account newAccount;
  memset(&newAccount, 0, sizeof(Account));
  strncpy(newAccount.studentId, newMember.studentId, MAX_MSSV_LEN - 1);
  newAccount.studentId[MAX_MSSV_LEN - 1] = '\0';
  strncpy(newAccount.password, newMember.studentId, MAX_PASS_LEN - 1);
  newAccount.password[MAX_PASS_LEN - 1] = '\0';
  if (newMember.role == MEMBER_ROLE_BCN) {
    newAccount.role = ACCOUNT_ROLE_BCN;
  } else {
    newAccount.role = ACCOUNT_ROLE_MEMBER;
  }
  newAccount.isLocked = 0;
  newAccount.failCount = 0;
  db->accounts[db->accountCount++] = newAccount;

  /* Save to files */
  if (fileioSaveMembers(db) != 0) {
    printf("[LOI] Khong the luu du lieu thanh vien\n");
    db->memberCount--;
    return -1;
  }

  if (fileioSaveAccounts(db) != 0) {
    printf("[LOI] Khong the luu du lieu tai khoan\n");
    db->memberCount--;
    db->accountCount--;
    if (fileioSaveMembers(db) != 0) {
      printf("[LOI] Khong the hoan tac du lieu thanh vien\n");
    }
    return -1;
  }

  printf("[OK] Them thanh vien thanh cong\n");
  printf("  MSSV: %s\n", newMember.studentId);
  printf("  Ten: %s\n", newMember.fullName);
  printf("  Ban: %s\n", team_name(newMember.team));
  printf("  Chuc vu: %s\n", member_role_name(newMember.role));
  printf("  Tai khoan da duoc tao voi mat khau mac dinh: %s\n",
         newMember.studentId);

  return 0;
}

/* ============================================================
 * Story 2.2 — Edit Member (helpers)
 * ============================================================ */

static int isBlankString(const char *s) {
  while (*s) {
    if (!isspace((unsigned char)*s)) {
      return 0;
    }
    s++;
  }
  return 1;
}

/* Edit member's full name. Skips if user presses Enter. */
static void editName(Member *m) {
  char buffer[MAX_NAME_LEN];
  printf("Ho va ten moi: ");
  read_string(buffer, MAX_NAME_LEN);
  if (strlen(buffer) > 0 && !isBlankString(buffer)) {
    strncpy(m->fullName, buffer, MAX_NAME_LEN - 1);
    m->fullName[MAX_NAME_LEN - 1] = '\0';
  }
}

/* Edit member's email with validation. Skips if user presses Enter. */
static void editEmail(Member *m) {
  char buffer[MAX_EMAIL_LEN];
  printf("Email moi: ");
  read_string(buffer, MAX_EMAIL_LEN);
  if (strlen(buffer) == 0) {
    return;
  }
  if (!is_email_valid(buffer)) {
    printf("[LOI] Email khong hop le, giu nguyen email cu\n");
    return;
  }
  strncpy(m->email, buffer, MAX_EMAIL_LEN - 1);
  m->email[MAX_EMAIL_LEN - 1] = '\0';
}

/* Edit member's phone number. Skips if user presses Enter. */
static void editPhone(Member *m) {
  char buffer[MAX_PHONE_LEN];
  printf("So dien thoai moi: ");
  read_string(buffer, MAX_PHONE_LEN);
  if (strlen(buffer) > 0 && !isBlankString(buffer)) {
    strncpy(m->phone, buffer, MAX_PHONE_LEN - 1);
    m->phone[MAX_PHONE_LEN - 1] = '\0';
  }
}

/* Edit member's team. Enter -1 to keep current value. */
static void editTeam(Member *m) {
  printf("Ban moi (0-Hoc thuat, 1-Ke hoach, 2-Nhan su, 3-Truyen thong) [-1 de "
         "giu nguyen]: ");
  int newTeam;
  if (!read_int(&newTeam) || newTeam == -1) {
    return;
  }
  if (newTeam >= TEAM_ACADEMIC && newTeam <= TEAM_MEDIA) {
    m->team = newTeam;
  } else {
    printf("[LOI] Ban khong hop le, giu nguyen ban cu\n");
  }
}

/* Edit member's role. Returns 1 if role changed, 0 otherwise. */
static int editRole(Member *m) {
  printf("Chuc vu moi (0-Thanh vien, 1-Truong nhom/Pho nhom, 2-Ban chu nhiem) "
         "[-1 de giu nguyen]: ");
  int newRole;
  if (!read_int(&newRole) || newRole == -1) {
    return 0;
  }
  if (newRole < MEMBER_ROLE_MEMBER || newRole > MEMBER_ROLE_BCN) {
    printf("[LOI] Chuc vu khong hop le, giu nguyen chuc vu cu\n");
    return 0;
  }
  if (m->role == newRole) {
    return 0;
  }
  m->role = newRole;
  return 1;
}

/* Edit member's active status. Enter -1 to keep current value. */
static void editStatus(Member *m) {
  printf("Trang thai (1-Hoat dong, 0-Da Out CLB) [-1 de giu nguyen]: ");
  int newStatus;
  if (!read_int(&newStatus) || newStatus == -1) {
    return;
  }
  if (newStatus == STATUS_ACTIVE || newStatus == STATUS_OUT_CLB) {
    m->isActive = newStatus;
  } else {
    printf("[LOI] Trang thai khong hop le, giu nguyen trang thai cu\n");
  }
}

/* Recalculate fines for all unpaid violations after a role change.
 * Only updates memory — caller is responsible for persisting. */
static void recalcFines(AppDatabase *db, Member *m) {
  double newFineRate = (m->role == MEMBER_ROLE_MEMBER) ? 20000.0 : 50000.0;
  m->totalFine = 0.0;

  for (int i = 0; i < db->violationCount; i++) {
    Violation *v = &db->violations[i];
    if (strcmp(v->studentId, m->studentId) != 0 || v->isPaid != 0) {
      continue;
    }
    if (v->reason != REASON_VIOLENCE) {
      v->fine = newFineRate;
    }
    m->totalFine += v->fine;
  }

  printf("[THONG BAO] Da tinh lai tien phat cho cac vi pham chua dong do "
         "thay doi chuc vu\n");
}

/* Display current member information. */
static void displayMemberInfo(const Member *m) {
  printf("\nTHONG TIN HIEN TAI:\n");
  printf("  Ho va ten: %s\n", m->fullName);
  printf("  Email: %s\n", m->email);
  printf("  So dien thoai: %s\n", m->phone);
  printf("  Ban: %s\n", team_name(m->team));
  printf("  Chuc vu: %s\n", member_role_name(m->role));
  printf("  Trang thai: %s\n", m->isActive ? "Hoat dong" : "Da Out CLB");
}

int memberEdit(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  char studentId[MAX_MSSV_LEN];
  printf("\nSUA THONG TIN THANH VIEN\n");
  printf("Nhap MSSV can sua: ");
  read_string(studentId, MAX_MSSV_LEN);

  int memberIndex = memberFindById(db, studentId);
  if (memberIndex == -1) {
    printf("[LOI] Khong tim thay thanh vien voi MSSV: %s\n", studentId);
    return -1;
  }

  Member *m = &db->members[memberIndex];

  displayMemberInfo(m);
  printf("\nNHAP THONG TIN MOI (Nhan Enter de giu nguyen):\n");

  editName(m);
  editEmail(m);
  editPhone(m);
  editTeam(m);

  int roleChanged = editRole(m);
  editStatus(m);

  if (roleChanged) {
    /* Update corresponding Account.role to match new Member.role */
    for (int i = 0; i < db->accountCount; i++) {
      if (strcmp(db->accounts[i].studentId, m->studentId) == 0) {
        db->accounts[i].role =
            (m->role == MEMBER_ROLE_BCN) ? ACCOUNT_ROLE_BCN : ACCOUNT_ROLE_MEMBER;
        break;
      }
    }

    /* Recalculate unpaid violation fines in memory */
    recalcFines(db, m);
  }

  /* Save members first (has updated totalFine), then violations, then accounts */
  if (fileioSaveMembers(db) != 0) {
    printf("[LOI] Khong the luu du lieu thanh vien\n");
    return -1;
  }

  if (roleChanged) {
    if (fileioSaveViolations(db) != 0) {
      printf("[LOI] Khong the luu du lieu vi pham\n");
      return -1;
    }
    if (fileioSaveAccounts(db) != 0) {
      printf("[LOI] Khong the cap nhat quyen tai khoan\n");
      return -1;
    }
  }

  printf("[OK] Sua thong tin thanh vien thanh cong\n");
  return 0;
}

/* ============================================================
 * Story 2.3 — Delete Member
 * ============================================================ */

int memberDelete(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  char studentId[MAX_MSSV_LEN];
  printf("\nXOA THANH VIEN\n");
  printf("Nhap MSSV can xoa: ");
  read_string(studentId, MAX_MSSV_LEN);

  int memberIndex = memberFindById(db, studentId);
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

  /* 1. Remove member from array */
  for (int i = memberIndex; i < db->memberCount - 1; i++) {
    db->members[i] = db->members[i + 1];
  }
  db->memberCount--;

  /* 2. Remove related violations */
  int vIndex = 0;
  while (vIndex < db->violationCount) {
    if (strcmp(db->violations[vIndex].studentId, studentId) == 0) {
      for (int j = vIndex; j < db->violationCount - 1; j++) {
        db->violations[j] = db->violations[j + 1];
      }
      db->violationCount--;
    } else {
      vIndex++;
    }
  }

  /* 3. Remove related account */
  int aIndex = 0;
  while (aIndex < db->accountCount) {
    if (strcmp(db->accounts[aIndex].studentId, studentId) == 0) {
      for (int j = aIndex; j < db->accountCount - 1; j++) {
        db->accounts[j] = db->accounts[j + 1];
      }
      db->accountCount--;
    } else {
      aIndex++;
    }
  }

  /* 4. Persist to files */
  if (fileioSaveMembers(db) != 0) {
    printf("[LOI] Khong the luu du lieu thanh vien sau khi xoa\n");
    return -1;
  }
  if (fileioSaveViolations(db) != 0) {
    printf("[LOI] Khong the luu du lieu vi pham sau khi xoa\n");
    return -1;
  }
  if (fileioSaveAccounts(db) != 0) {
    printf("[LOI] Khong the luu du lieu tai khoan sau khi xoa\n");
    return -1;
  }

  printf("[OK] Xoa thanh vien thanh cong.\n");
  return 0;
}

/* ============================================================
 * Story 2.4 — View Member List & Profile
 * ============================================================ */

void memberViewProfile(AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  Account *session = authGetSession();
  if (session == NULL) {
    printf("[LOI] Ban phai dang nhap de xem profile\n");
    return;
  }

  int idx = memberFindById(db, session->studentId);
  if (idx == -1) {
    printf("[LOI] Khong tim thay thong tin thanh vien\n");
    return;
  }

  Member *m = &db->members[idx];

  printf("\nTHONG TIN CA NHAN\n");
  printf("----------------------------------------\n");
  printf("MSSV           : %s\n", m->studentId);
  printf("Ho va ten      : %s\n", m->fullName);
  printf("Email          : %s\n", m->email);
  printf("So dien thoai  : %s\n", m->phone);
  printf("Ban            : %s\n", team_name(m->team));
  printf("Chuc vu        : %s\n", member_role_name(m->role));
  printf("Trang thai     : %s\n", m->isActive ? "Active" : "Out CLB");
  printf("So lan vi pham : %d\n", m->violationCount);
  printf("Tong tien phat : %.0f VND\n", m->totalFine);
  printf("----------------------------------------\n\n");
}

void memberListAll(AppDatabase *db) {
  if (db == NULL) {
    return;
  }

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

    if ((i + 1) % 20 == 0 && (i + 1) < db->memberCount) {
      printf("\n[Nhan Enter de xem trang tiep theo hoac nhap 'q' roi Enter de "
             "thoat]: ");
      char buf[10];
      read_string(buf, sizeof(buf));
      if (buf[0] == 'q' || buf[0] == 'Q') {
        break;
      }
      printf("\n+------+------------------+------------+-----------+\n");
      printf("| MSSV | Ho va ten        | Ban        | Chuc vu  |\n");
      printf("+------+------------------+------------+-----------+\n");
    }
  }

  printf("+------+------------------+------------+-----------+\n");
  printf("Tong: %d thanh vien\n\n", db->memberCount);
}
