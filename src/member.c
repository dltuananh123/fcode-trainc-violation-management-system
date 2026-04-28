#include "member.h"
#include "auth.h"
#include "fileio.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

/* ============================================================
 * Story 2.1 — Add Member
 * ============================================================ */

int memberFindById(AppDatabase *db, const char *studentId) {
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

int memberValidateInput(const Member *m, AppDatabase *db) {
  if (m == NULL || db == NULL) {
    return -1;
  }

  /* Check studentId is valid and not empty */
  if (!isIdValid(m->studentId)) {
    printf("[LOI] MSSV khong hop le\n");
    return -1;
  }

  /* Check uniqueness */
  if (memberFindById(db, m->studentId) != -1) {
    printf("[LOI] MSSV da ton tai\n");
    return -1;
  }

  /* Check email format */
  if (!isEmailValid(m->email)) {
    printf("[LOI] Email khong hop le\n");
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

  /* Input member details */
  printf("Nhap MSSV: ");
  readString(newMember.studentId, MAX_MSSV_LEN);

  printf("Nhap ho va ten: ");
  readString(newMember.fullName, MAX_NAME_LEN);

  printf("Nhap email: ");
  readString(newMember.email, MAX_EMAIL_LEN);

  printf("Nhap so dien thoai: ");
  readString(newMember.phone, MAX_PHONE_LEN);

  /* Select team */
  printf("Chon ban (0-Hoc thuat, 1-Ke hoach, 2-Nhan su, 3-Truyen thong): ");
  int team;
  if (readInt(&team) != 1) {
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
  if (readInt(&role) != 1) {
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

  /* Validate input */
  if (memberValidateInput(&newMember, db) != 0) {
    return -1;
  }

  /* Add member to database */
  db->members[db->memberCount++] = newMember;

  /* Create account with default password = MSSV */
  Account newAccount;
  memset(&newAccount, 0, sizeof(Account));
  strncpy(newAccount.studentId, newMember.studentId, MAX_MSSV_LEN - 1);
  strncpy(newAccount.password, newMember.studentId, MAX_PASS_LEN - 1);
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
    fileioSaveMembers(db);
    return -1;
  }

  printf("[OK] Them thanh vien thanh cong\n");
  printf("  MSSV: %s\n", newMember.studentId);
  printf("  Ten: %s\n", newMember.fullName);
  printf("  Ban: %s\n", teamName(newMember.team));
  printf("  Chuc vu: %s\n", memberRoleName(newMember.role));
  printf("  Tai khoan da duoc tao voi mat khau mac dinh: %s\n",
         newMember.studentId);

  return 0;
}

/* ============================================================
 * Story 2.2 — Edit Member
 * ============================================================ */

int memberEdit(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  char studentId[MAX_MSSV_LEN];
  printf("\nSUA THONG TIN THANH VIEN\n");
  printf("Nhap MSSV can sua: ");
  readString(studentId, MAX_MSSV_LEN);

  int memberIndex = memberFindById(db, studentId);
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
  printf("  Ban: %s\n", teamName(m->team));
  printf("  Chuc vu: %s\n", memberRoleName(m->role));
  printf("  Trang thai: %s\n", m->isActive ? "Hoat dong" : "Da Out CLB");

  printf("\nNHAP THONG TIN MOI (Nhan Enter de giu nguyen):\n");

  printf("Ho va ten moi: ");
  readString(buffer, MAX_NAME_LEN);
  if (strlen(buffer) > 0) {
    strncpy(m->fullName, buffer, MAX_NAME_LEN - 1);
    m->fullName[MAX_NAME_LEN - 1] = '\0';
  }

  printf("Email moi: ");
  readString(buffer, MAX_EMAIL_LEN);
  if (strlen(buffer) > 0) {
    if (!isEmailValid(buffer)) {
      printf("[LOI] Email khong hop le, giu nguyen email cu\n");
    } else {
      strncpy(m->email, buffer, MAX_EMAIL_LEN - 1);
      m->email[MAX_EMAIL_LEN - 1] = '\0';
    }
  }

  printf("So dien thoai moi: ");
  readString(buffer, MAX_PHONE_LEN);
  if (strlen(buffer) > 0) {
    strncpy(m->phone, buffer, MAX_PHONE_LEN - 1);
    m->phone[MAX_PHONE_LEN - 1] = '\0';
  }

  printf("Ban moi (0-Hoc thuat, 1-Ke hoach, 2-Nhan su, 3-Truyen thong) [-1 de "
         "giu nguyen]: ");
  int newTeam;
  if (readInt(&newTeam)) {
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
  if (readInt(&newRole)) {
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
  if (readInt(&newStatus)) {
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

    if (fileioSaveViolations(db) != 0) {
      printf(
          "[LOI] Khong the luu du lieu vi pham sau khi tinh lai tien phat\n");
      return -1;
    }
    printf("[THONG BAO] Da tinh lai tien phat cho cac vi pham chua dong do "
           "thay doi chuc vu\n");
  }

  if (fileioSaveMembers(db) != 0) {
    printf("[LOI] Khong the luu du lieu thanh vien\n");
    return -1;
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
  readString(studentId, MAX_MSSV_LEN);

  int memberIndex = memberFindById(db, studentId);
  if (memberIndex == -1) {
    printf("[LOI] Khong tim thay thanh vien voi MSSV: %s\n", studentId);
    return -1;
  }

  Member *m = &db->members[memberIndex];
  printf("\nTHONG TIN THANH VIEN:\n");
  printf("  Ho va ten: %s\n", m->fullName);
  printf("  Ban: %s\n", teamName(m->team));
  printf("  Chuc vu: %s\n", memberRoleName(m->role));

  printf("\nXac nhan xoa thanh vien nay va toan bo du lieu lien quan? (1: "
         "Co, 0: Khong): ");
  int confirm;
  if (readInt(&confirm) != 1 || confirm != 1) {
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
  printf("Ban            : %s\n", teamName(m->team));
  printf("Chuc vu        : %s\n", memberRoleName(m->role));
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
           teamName(m->team), memberRoleName(m->role));

    if ((i + 1) % 20 == 0 && (i + 1) < db->memberCount) {
      printf("\n[Nhan Enter de xem trang tiep theo hoac nhap 'q' roi Enter de "
             "thoat]: ");
      char buf[10];
      readString(buf, sizeof(buf));
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
