#include "member.h"
#include "auth.h"
#include "fileio.h"
#include "types.h"
#include "ui.h"
#include "utils.h"
#include "validate.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

/* ============================================================
 * FIND / SEARCH
 * ============================================================ */

int memberFindById(const AppDatabase *db, const char *studentId) {
  if (db == NULL || studentId == NULL) {
    return -1;
  }

  for (int i = 0; i < db->memberCount; i++) {
    if (!db->members[i].isDeleted &&
        strcmp(db->members[i].studentId, studentId) == 0) {
      return i;
    }
  }

  return -1;
}

/* Case-insensitive substring search */
static int containsIgnoreCase(const char *haystack, const char *needle) {
  if (needle[0] == '\0') {
    return 1;
  }

  size_t hLen = strlen(haystack);
  size_t nLen = strlen(needle);

  if (nLen > hLen) {
    return 0;
  }

  for (size_t i = 0; i <= hLen - nLen; i++) {
    int match = 1;
    for (size_t j = 0; j < nLen; j++) {
      if (tolower((unsigned char)haystack[i + j]) !=
          tolower((unsigned char)needle[j])) {
        match = 0;
        break;
      }
    }
    if (match) {
      return 1;
    }
  }

  return 0;
}

int memberSearchByName(const AppDatabase *db, const char *keyword,
                       int *outIndices, int maxResults) {
  if (db == NULL || keyword == NULL || outIndices == NULL) {
    return 0;
  }

  int count = 0;
  for (int i = 0; i < db->memberCount && count < maxResults; i++) {
    if (!db->members[i].isDeleted &&
        containsIgnoreCase(db->members[i].fullName, keyword)) {
      outIndices[count++] = i;
    }
  }

  return count;
}

int memberValidateInput(const Member *m, const AppDatabase *db) {
  if (m == NULL || db == NULL) {
    return -1;
  }

  int errors = 0;

  if (!validateName(m->fullName)) {
    errors++;
  }
  if (!validateEmailUnique(m->email, db, m->studentId)) {
    errors++;
  }
  if (!validatePhoneUnique(m->phone, db, m->studentId)) {
    errors++;
  }

  /* Warning if both email and phone are empty */
  if (strlen(m->email) == 0 && strlen(m->phone) == 0) {
    printf(ERR_CANH_BAO "Khuyen nghi nhap it nhat email hoac SDT "
           "de lien he!\n");
  }

  return errors > 0 ? -1 : 0;
}

/* ============================================================
 * ADD MEMBER — with comprehensive validation + re-prompt
 * ============================================================ */

int memberAdd(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  if (db->memberCount >= MAX_MEMBERS) {
    printf(ERR_LOI "Da dat gioi han so luong thanh vien (%d)!\n",
           MAX_MEMBERS);
    return -1;
  }

  printf("\n");
  uiDrawSeparator();
  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD COLOR_CYAN "  THEM THANH VIEN MOI");
    printf("                          ");
  printf(COLOR_RESET COLOR_BLUE BOX_V COLOR_RESET "\n");
  uiDrawSeparator();

  Member newMember;
  memset(&newMember, 0, sizeof(Member));

  /* MSSV with re-prompt */
  while (1) {
    printf(COLOR_CYAN "  Nhap MSSV: " COLOR_RESET);
    readString(newMember.studentId, MAX_MSSV_LEN);
    trimSpaces(newMember.studentId);
    mssvAutoUpper(newMember.studentId);
    if (validateMSSV(newMember.studentId, db)) {
      break;
    }
  }

  /* Show MSSV decode */
  printf(ERR_INFO "MSSV: %s → Campus: %s | Khối: %s\n",
         newMember.studentId,
         mssvCampusName(newMember.studentId[0]),
         mssvDivisionName(newMember.studentId[1]));

  /* Full name with re-prompt */
  while (1) {
    printf(COLOR_CYAN "  Nhap ho va ten: " COLOR_RESET);
    readString(newMember.fullName, MAX_NAME_LEN);
    trimSpaces(newMember.fullName);
    nameAutoFix(newMember.fullName);
    if (validateName(newMember.fullName)) {
      break;
    }
  }

  /* Email with re-prompt + uniqueness */
  while (1) {
    printf(COLOR_CYAN "  Nhap email: " COLOR_RESET);
    readString(newMember.email, MAX_EMAIL_LEN);
    trimSpaces(newMember.email);
    /* Auto-lowercase */
    for (int i = 0; newMember.email[i]; i++) {
      newMember.email[i] = (char)tolower((unsigned char)newMember.email[i]);
    }
    if (validateEmailUnique(newMember.email, db, NULL)) {
      break;
    }
  }

  /* Phone with re-prompt + uniqueness + auto-normalize */
  while (1) {
    printf(COLOR_CYAN "  Nhap so dien thoai: " COLOR_RESET);
    readString(newMember.phone, MAX_PHONE_LEN);
    trimSpaces(newMember.phone);
    phoneNormalize(newMember.phone);
    if (validatePhoneUnique(newMember.phone, db, NULL)) {
      printf(ERR_INFO "Nha mang: %s\n", phoneCarrier(newMember.phone));
      break;
    }
  }

  /* Team selection with re-prompt */
  newMember.team = readMenuChoice(
      COLOR_CYAN "  Chon ban (0-Hoc thuat, 1-Ke hoach, 2-Nhan su, "
      "3-Truyen thong): " COLOR_RESET,
      TEAM_ACADEMIC, TEAM_MEDIA);

  /* Role selection with re-prompt */
  newMember.role = readMenuChoice(
      COLOR_CYAN "  Chon chuc vu (0-Thanh vien, 1-Truong nhom, "
      "2-BCN): " COLOR_RESET,
      MEMBER_ROLE_MEMBER, MEMBER_ROLE_BCN);

  /* Set defaults */
  newMember.violationCount = 0;
  newMember.consecutiveAbsences = 0;
  newMember.totalFine = 0.0;
  newMember.isActive = STATUS_ACTIVE;
  newMember.isDeleted = 0;
  newMember.deletedAt = 0;

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
  newAccount.isDefaultPassword = 1;
  db->accounts[db->accountCount++] = newAccount;

  /* Save to files */
  if (fileioSaveMembers(db) != 0) {
    printf(ERR_LOI "Khong the luu du lieu thanh vien!\n");
    db->memberCount--;
    return -1;
  }

  if (fileioSaveAccounts(db) != 0) {
    printf(ERR_LOI "Khong the luu du lieu tai khoan!\n");
    db->memberCount--;
    db->accountCount--;
    (void)fileioSaveMembers(db);
    return -1;
  }

  printf("\n");
  printf(ERR_OK "Them thanh vien thanh cong!\n");
  printf(COLOR_BOLD "  MSSV:    " COLOR_RESET "%s\n", newMember.studentId);
  printf(COLOR_BOLD "  Ten:     " COLOR_RESET "%s\n", newMember.fullName);
  printf(COLOR_BOLD "  Ban:     " COLOR_RESET "%s\n",
         teamName(newMember.team));
  printf(COLOR_BOLD "  Chuc vu: " COLOR_RESET "%s\n",
         memberRoleName(newMember.role));
  printf(COLOR_DIM "  Tai khoan da tao. Mat khau mac dinh = MSSV.\n"
         "  Thanh vien se phai doi mat khau khi dang nhap.\n" COLOR_RESET);

  return 0;
}

/* ============================================================
 * EDIT MEMBER — with Enter=keep old + re-prompt validation
 * ============================================================ */

/* Edit helper: prompt with current value, Enter keeps old, re-prompt on error */
static void editField(const char *label, char *current, size_t maxLen,
                      int (*validator)(const char *)) {
  while (1) {
    printf(COLOR_CYAN "  %s" COLOR_RESET "[%s]: ", label, current);
    char buffer[256];
    readString(buffer, sizeof(buffer));
    trimSpaces(buffer);

    /* Enter = keep old value */
    if (strlen(buffer) == 0) {
      return;
    }

    /* Validate new value */
    if (validator != NULL && !validator(buffer)) {
      continue; /* re-prompt */
    }

    strncpy(current, buffer, maxLen - 1);
    current[maxLen - 1] = '\0';
    return;
  }
}

int memberEdit(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  printf("\n");
  uiDrawSeparator();
  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD COLOR_CYAN "  CHINH SUA THONG TIN THANH VIEN");
  printf("                  ");
  printf(COLOR_RESET COLOR_BLUE BOX_V COLOR_RESET "\n");
  uiDrawSeparator();

  /* Find member by MSSV with re-prompt */
  char studentId[MAX_MSSV_LEN];
  int memberIndex = -1;
  while (1) {
    printf(COLOR_CYAN "  Nhap MSSV can sua: " COLOR_RESET);
    readString(studentId, MAX_MSSV_LEN);
    trimSpaces(studentId);
    mssvAutoUpper(studentId);
    if (validateNotEmpty(studentId)) {
      memberIndex = memberFindById(db, studentId);
      if (memberIndex != -1) {
        break;
      }
      printf(ERR_LOI "Khong tim thay thanh vien voi MSSV: %s!\n",
             studentId);
    }
  }

  Member *m = &db->members[memberIndex];

  /* Show current info */
  printf("\n");
  printf(COLOR_BOLD "  Thong tin hien tai:\n" COLOR_RESET);
  printf(COLOR_DIM "  MSSV: %s [KHONG THE SUA]\n" COLOR_RESET,
         m->studentId);
  printf("  Ho ten:     %s\n", m->fullName);
  printf("  Email:      %s\n", m->email);
  printf("  SDT:        %s\n", m->phone);
  printf("  Ban:        %s\n", teamName(m->team));
  printf("  Chuc vu:    %s\n", memberRoleName(m->role));
  printf("  Trang thai: %s\n",
         m->isActive ? COLOR_GREEN "Hoat dong" COLOR_RESET
                     : COLOR_RED "Out CLB" COLOR_RESET);

  printf("\n");
  printf(COLOR_DIM "  Nhap thong tin moi (Enter = giu nguyen):\n" COLOR_RESET);
  uiDrawSeparator();

  /* Edit fields — Enter keeps old, re-prompt on invalid */
  editField("Ho va ten     ", m->fullName, MAX_NAME_LEN, validateName);
  nameAutoFix(m->fullName);

  editField("Email         ", m->email, MAX_EMAIL_LEN, NULL);
  /* Lowercase email */
  for (int i = 0; m->email[i]; i++) {
    m->email[i] = (char)tolower((unsigned char)m->email[i]);
  }
  /* Validate email format + uniqueness */
  if (!validateEmailUnique(m->email, db, m->studentId)) {
    printf(ERR_LOI "Email khong hop le, giu nguyen email cu!\n");
  }

  editField("So dien thoai ", m->phone, MAX_PHONE_LEN, NULL);
  phoneNormalize(m->phone);
  if (!validatePhoneUnique(m->phone, db, m->studentId)) {
    printf(ERR_LOI "SDT khong hop le, giu nguyen SDT cu!\n");
  }

  /* Team selection */
  m->team = readMenuChoice(
      COLOR_CYAN "  Ban moi (0-Hoc thuat, 1-Ke hoach, "
      "2-Nhan su, 3-Truyen thong): " COLOR_RESET,
      TEAM_ACADEMIC, TEAM_MEDIA);

  /* Role selection */
  int oldRole = m->role;
  m->role = readMenuChoice(
      COLOR_CYAN "  Chuc vu moi (0-Thanh vien, 1-Truong nhom, 2-BCN): "
      COLOR_RESET,
      MEMBER_ROLE_MEMBER, MEMBER_ROLE_BCN);
  int roleChanged = (oldRole != m->role);

  if (roleChanged) {
    /* Update account role */
    for (int i = 0; i < db->accountCount; i++) {
      if (strcmp(db->accounts[i].studentId, m->studentId) == 0) {
        db->accounts[i].role = (m->role == MEMBER_ROLE_BCN)
                                   ? ACCOUNT_ROLE_BCN
                                   : ACCOUNT_ROLE_MEMBER;
        break;
      }
    }
    /* Recalculate fines */
    double newFineRate =
        (m->role == MEMBER_ROLE_MEMBER) ? 20000.0 : 50000.0;
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
    printf(ERR_INFO "Da tinh lai tien phat do thay doi chuc vu.\n");
  }

  /* Save */
  if (fileioSaveMembers(db) != 0) {
    printf(ERR_LOI "Khong the luu du lieu thanh vien!\n");
    return -1;
  }
  if (roleChanged) {
    (void)fileioSaveViolations(db);
    (void)fileioSaveAccounts(db);
  }

  printf(ERR_OK "Sua thong tin thanh vien thanh cong!\n");
  return 0;
}

/* ============================================================
 * DELETE MEMBER — with soft delete + confirmation
 * ============================================================ */

int memberDelete(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  printf("\n");
  uiDrawSeparator();
  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD COLOR_RED "  XOA THANH VIEN");
  printf("                                  ");
  printf(COLOR_RESET COLOR_BLUE BOX_V COLOR_RESET "\n");
  uiDrawSeparator();

  /* Find member */
  char studentId[MAX_MSSV_LEN];
  int memberIndex = -1;
  while (1) {
    printf(COLOR_CYAN "  Nhap MSSV can xoa: " COLOR_RESET);
    readString(studentId, MAX_MSSV_LEN);
    trimSpaces(studentId);
    mssvAutoUpper(studentId);
    if (validateNotEmpty(studentId)) {
      memberIndex = memberFindById(db, studentId);
      if (memberIndex != -1) {
        break;
      }
      printf(ERR_LOI "Khong tim thay thanh vien voi MSSV: %s!\n",
             studentId);
    }
  }

  Member *m = &db->members[memberIndex];

  /* Prevent self-deletion */
  Account *session = authGetSession();
  if (session != NULL && strcmp(session->studentId, studentId) == 0) {
    printf(ERR_LOI "Khong the xoa tai khoan cua chinh ban!\n");
    return -1;
  }

  /* Check for unpaid violations */
  int unpaidCount = 0;
  for (int i = 0; i < db->violationCount; i++) {
    if (strcmp(db->violations[i].studentId, studentId) == 0 &&
        !db->violations[i].isPaid) {
      unpaidCount++;
    }
  }
  if (unpaidCount > 0) {
    printf(ERR_LOI "Thanh vien con %d vi pham chua dong phat! "
           "Khong the xoa.\n", unpaidCount);
    return -1;
  }

  /* Show member info */
  printf("\n");
  printf(COLOR_BOLD "  THONG TIN THANH VIEN:\n" COLOR_RESET);
  printf("  Ho ten:      %s\n", m->fullName);
  printf("  MSSV:        %s\n", m->studentId);
  printf("  Email:       %s\n", m->email);
  printf("  SDT:         %s\n", m->phone);
  printf("  Ban:         %s\n", teamName(m->team));
  printf("  Chuc vu:     %s\n", memberRoleName(m->role));
  printf("  Trang thai:  %s\n",
         m->isActive ? "Hoat dong" : "Out CLB");
  printf("  So lan VP:   %d\n", m->violationCount);
  printf("  Tong phat:   %.0f VND\n", m->totalFine);

  /* Confirm */
  printf("\n");
  printf(ERR_CANH_BAO "Ban sap xoa thanh vien \"%s\" (%s) "
         "va toan bo du lieu lien quan!\n", m->fullName, m->studentId);
  int confirm = readMenuChoice(
      COLOR_RED "  Xac nhan xoa? (1=Co, 0=Khong): " COLOR_RESET, 0, 1);
  if (confirm != 1) {
    printf(ERR_INFO "Da huy xoa thanh vien.\n");
    return 0;
  }

  /* Soft delete: mark as deleted */
  m->isDeleted = 1;
  m->deletedAt = time(NULL);

  /* Save */
  if (fileioSaveMembers(db) != 0) {
    printf(ERR_LOI "Khong the luu du lieu!\n");
    return -1;
  }

  printf(ERR_OK "Xoa thanh vien thanh cong! (Du lieu da duoc an)\n");
  return 0;
}

/* ============================================================
 * VIEW MEMBER LIST & PROFILE
 * ============================================================ */

void memberViewProfile(AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  Account *session = authGetSession();
  if (session == NULL) {
    printf(ERR_LOI "Ban phai dang nhap de xem profile!\n");
    return;
  }

  int idx = memberFindById(db, session->studentId);
  if (idx == -1) {
    printf(ERR_LOI "Khong tim thay thong tin thanh vien!\n");
    return;
  }

  Member *m = &db->members[idx];

  printf("\n");
  uiDrawSeparator();
  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD COLOR_CYAN "  THONG TIN CA NHAN");
  printf("                            ");
  printf(COLOR_RESET COLOR_BLUE BOX_V COLOR_RESET "\n");
  uiDrawSeparator();

  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD "  MSSV:       " COLOR_RESET "%-39s",
         m->studentId);
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD "  Ho va ten:  " COLOR_RESET "%-39s",
         m->fullName);
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD "  Email:      " COLOR_RESET "%-39s",
         m->email);
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD "  SDT:        " COLOR_RESET "%-39s",
         m->phone);
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD "  Ban:        " COLOR_RESET "%-39s",
         teamName(m->team));
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD "  Chuc vu:    " COLOR_RESET "%-39s",
         memberRoleName(m->role));
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD "  Trang thai: " COLOR_RESET);
  if (m->isActive) {
    printf(COLOR_GREEN "Hoat dong");
    printf("                                  ");
    printf(COLOR_RESET);
  } else {
    printf(COLOR_RED "Da Out CLB");
    printf("                                    ");
    printf(COLOR_RESET);
  }
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD "  So lan VP:  " COLOR_RESET "%-5d", m->violationCount);
  printf("     ");
  printf(COLOR_BOLD "Tong phat: " COLOR_RESET COLOR_PURPLE "%-14.0f",
         m->totalFine);
  printf(COLOR_RESET " ");
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  uiDrawSeparator();
  printf("\n");
}

void memberListAll(AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  /* Count active (non-deleted) members */
  int activeCount = 0;
  for (int i = 0; i < db->memberCount; i++) {
    if (!db->members[i].isDeleted) {
      activeCount++;
    }
  }

  if (activeCount == 0) {
    printf(ERR_INFO "Chua co thanh vien nao trong du lieu.\n");
    return;
  }

  printf("\n");
  printf(COLOR_BOLD "  DANH SACH THANH VIEN (%d)\n" COLOR_RESET,
         activeCount);
  printf(COLOR_CYAN
         "  " LINE_TL LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_cross LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_cross LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_cross LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_TR "\n" COLOR_RESET);
  printf(COLOR_CYAN "  " LINE_V COLOR_RESET
         " MSSV       " COLOR_CYAN LINE_V COLOR_RESET
         " Ho va ten            " COLOR_CYAN LINE_V COLOR_RESET
         " Ban          " COLOR_CYAN LINE_V COLOR_RESET
         " Trang thai " COLOR_CYAN LINE_V COLOR_RESET "\n");
  printf(COLOR_CYAN
         "  " LINE_TL LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_cross LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_cross LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_cross LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_TR "\n" COLOR_RESET);

  int active = 0, outClb = 0;
  int displayed = 0;
  for (int i = 0; i < db->memberCount; i++) {
    Member *m = &db->members[i];
    if (m->isDeleted) {
      continue;
    }

    if (m->isActive) {
      active++;
    } else {
      outClb++;
    }

    printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
    printf(" %-10s ", m->studentId);
    printf(COLOR_CYAN LINE_V COLOR_RESET);
    printf(" %-20s ", m->fullName);
    printf(COLOR_CYAN LINE_V COLOR_RESET);
    printf(" %-12s ", teamName(m->team));
    printf(COLOR_CYAN LINE_V COLOR_RESET);
    if (m->isActive) {
      printf(COLOR_GREEN " Hoat dong  " COLOR_RESET);
    } else {
      printf(COLOR_RED " Out CLB    " COLOR_RESET);
    }
    printf(COLOR_CYAN LINE_V COLOR_RESET "\n");

    displayed++;
    if (displayed % 20 == 0 && displayed < activeCount) {
      printf("\n  [Enter = tiep tuc | q = thoat]: ");
      char buf[10];
      readString(buf, sizeof(buf));
      if (buf[0] == 'q' || buf[0] == 'Q') {
        break;
      }
    }
  }

  printf(COLOR_CYAN
         "  " LINE_BL LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_cross LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_cross LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_cross LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_H LINE_H LINE_H LINE_H LINE_H LINE_H
         LINE_BR "\n" COLOR_RESET);
  printf("  Tong: " COLOR_BOLD "%d" COLOR_RESET
         " thanh vien (" COLOR_GREEN "Hoat dong: %d" COLOR_RESET
         " | " COLOR_RED "Out CLB: %d" COLOR_RESET ")\n\n",
         activeCount, active, outClb);
}
