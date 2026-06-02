#include "member.h"
#include "auth.h"
#include "fileio.h"
#include "types.h"
#include "ui.h"
#include "utils.h"
#include "validate.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ============================================================
 * FIND / SEARCH
 * ============================================================ */

static int memberIndexCompare(const void *a, const void *b) {
  return strcmp(((const MemberIndex *)a)->studentId,
                ((const MemberIndex *)b)->studentId);
}

void memberRebuildIndex(AppDatabase *db) {
  if (db == NULL) {
    return;
  }
  db->memberIndexCount = 0;
  for (int i = 0; i < db->memberCount; i++) {
    if (!db->members[i].isDeleted) {
      int idx = db->memberIndexCount;
      strncpy(db->memberIndexes[idx].studentId, db->members[i].studentId,
              MAX_MSSV_LEN - 1);
      db->memberIndexes[idx].studentId[MAX_MSSV_LEN - 1] = '\0';
      db->memberIndexes[idx].index = i;
      db->memberIndexCount++;
    }
  }
  if (db->memberIndexCount > 1) {
    qsort(db->memberIndexes, (size_t)db->memberIndexCount, sizeof(MemberIndex),
          memberIndexCompare);
  }
}

int memberFindById(const AppDatabase *db, const char *studentId) {
  if (db == NULL || studentId == NULL || db->memberIndexCount == 0) {
    return -1;
  }

  MemberIndex key;
  strncpy(key.studentId, studentId, MAX_MSSV_LEN - 1);
  key.studentId[MAX_MSSV_LEN - 1] = '\0';

  MemberIndex *res =
      bsearch(&key, db->memberIndexes, (size_t)db->memberIndexCount,
              sizeof(MemberIndex), memberIndexCompare);
  if (res != NULL) {
    return res->index;
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
    printf(ERR_LOI "Da dat gioi han so luong thanh vien (%d)!\n", MAX_MEMBERS);
    return -1;
  }

  uiClear();
  uiDrawBreadcrumb("[1] Quan ly thanh vien -> Them thanh vien moi");

  Member newMember;
  memset(&newMember, 0, sizeof(Member));

  /* MSSV with re-prompt */
  while (1) {
    printf(COLOR_CYAN "  Nhap MSSV (0 de quay lai): " COLOR_RESET);
    readString(newMember.studentId, MAX_MSSV_LEN);
    trimSpaces(newMember.studentId);
    mssvAutoUpper(newMember.studentId);
    if (strcmp(newMember.studentId, "0") == 0) {
      printf(ERR_INFO "Da huy thao tac.\n");
      return -1;
    }
    if (validateMSSV(newMember.studentId, db)) {
      break;
    }
  }

  /* Show MSSV decode */
  printf(ERR_INFO "MSSV: %s → Campus: %s | Khối: %s\n", newMember.studentId,
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
  newMember.role =
      readMenuChoice(COLOR_CYAN "  Chon chuc vu (0-Thanh vien, 1-Truong nhom, "
                                "2-Ban chu nhiem): " COLOR_RESET,
                     MEMBER_ROLE_MEMBER, MEMBER_ROLE_DIRECTOR);

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
  generateSalt(newAccount.salt, sizeof(newAccount.salt));
  hashPassword(newMember.studentId, newAccount.salt, newAccount.password);
  if (newMember.role == MEMBER_ROLE_DIRECTOR) {
    newAccount.role = ACCOUNT_ROLE_DIRECTOR;
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
  printf(COLOR_BOLD "  Email:   " COLOR_RESET "%s\n", newMember.email);
  printf(COLOR_BOLD "  Sdt:     " COLOR_RESET "%s\n", newMember.phone);
  printf(COLOR_BOLD "  Ban:     " COLOR_RESET "%s\n", teamName(newMember.team));
  printf(COLOR_BOLD "  Chuc vu: " COLOR_RESET "%s\n",
         memberRoleName(newMember.role));
  printf(COLOR_DIM
         "  Tai khoan da tao. Mat khau mac dinh = MSSV.\n"
         "  Thanh vien se phai doi mat khau khi dang nhap.\n" COLOR_RESET);

  Account *session = authGetSession();
  if (session != NULL) {
    logSystemAction(session->studentId, "Them thanh vien", newMember.studentId);
  }

  memberRebuildIndex(db);
  return 0;
}

/* ============================================================
 * EDIT MEMBER — with Enter=keep old + re-prompt validation
 * ============================================================ */

/* Edit helper: prompt with current value, Enter keeps old, re-prompt on error
 */
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

  uiClear();
  uiDrawBreadcrumb("[1] Quan ly thanh vien -> Sua thong tin");

  /* Find member by MSSV with re-prompt */
  char studentId[MAX_MSSV_LEN];
  int memberIndex = -1;
  while (1) {
    printf(COLOR_CYAN "  Nhap MSSV can sua (0 de quay lai): " COLOR_RESET);
    readString(studentId, MAX_MSSV_LEN);
    trimSpaces(studentId);
    mssvAutoUpper(studentId);
    if (strcmp(studentId, "0") == 0) {
      printf(ERR_INFO "Da huy thao tac.\n");
      return 0;
    }
    if (validateNotEmpty(studentId)) {
      memberIndex = memberFindById(db, studentId);
      if (memberIndex != -1) {
        break;
      }
      printf(ERR_LOI "Khong tim thay thanh vien voi MSSV: %s!\n", studentId);
    }
  }

  Member *m = &db->members[memberIndex];

  /* Show current info */
  printf("\n");
  printf(COLOR_BOLD "  Thong tin hien tai:\n" COLOR_RESET);
  printf(COLOR_DIM "  MSSV: %s [KHONG THE SUA]\n" COLOR_RESET, m->studentId);
  printf("  Ho ten:     %s\n", m->fullName);
  printf("  Email:      %s\n", m->email);
  printf("  SDT:        %s\n", m->phone);
  printf("  Ban:        %s\n", teamName(m->team));
  printf("  Chuc vu:    %s\n", memberRoleName(m->role));
  printf("  Trang thai: %s\n", m->isActive ? COLOR_GREEN "Hoat dong" COLOR_RESET
                                           : COLOR_RED "Out CLB" COLOR_RESET);

  printf("\n");
  printf(COLOR_DIM "  Nhap thong tin moi (Enter = giu nguyen):\n" COLOR_RESET);
  uiDrawSeparator();

  /* Edit fields — Enter keeps old, re-prompt on invalid */
  editField("Ho va ten     ", m->fullName, MAX_NAME_LEN, validateName);
  nameAutoFix(m->fullName);

  /* Email — re-prompt on validation failure */
  while (1) {
    printf(COLOR_CYAN "  Email         " COLOR_RESET "[%s]: ", m->email);
    char buffer[256];
    readString(buffer, sizeof(buffer));
    trimSpaces(buffer);
    if (strlen(buffer) == 0) {
      break;
    }
    for (int i = 0; buffer[i]; i++) {
      buffer[i] = (char)tolower((unsigned char)buffer[i]);
    }
    if (validateEmailUnique(buffer, db, m->studentId)) {
      strncpy(m->email, buffer, MAX_EMAIL_LEN - 1);
      m->email[MAX_EMAIL_LEN - 1] = '\0';
      break;
    }
  }

  /* Phone — re-prompt on validation failure */
  while (1) {
    printf(COLOR_CYAN "  So dien thoai" COLOR_RESET "[%s]: ", m->phone);
    char buffer[256];
    readString(buffer, sizeof(buffer));
    trimSpaces(buffer);
    if (strlen(buffer) == 0) {
      break;
    }
    char normalized[MAX_PHONE_LEN];
    strncpy(normalized, buffer, MAX_PHONE_LEN - 1);
    normalized[MAX_PHONE_LEN - 1] = '\0';
    phoneNormalize(normalized);
    if (validatePhoneUnique(normalized, db, m->studentId)) {
      strncpy(m->phone, normalized, MAX_PHONE_LEN - 1);
      m->phone[MAX_PHONE_LEN - 1] = '\0';
      break;
    }
  }

  /* Team selection — Enter to keep */
  while (1) {
    printf(COLOR_CYAN "  Ban moi (0-Hoc thuat, 1-Ke hoach, 2-Nhan su, 3-Truyen "
                      "thong)" COLOR_RESET COLOR_DIM " [%d]: " COLOR_RESET,
           m->team);
    char buf[32];
    readString(buf, sizeof(buf));
    trimSpaces(buf);
    if (strlen(buf) == 0) {
      break;
    }
    int val = 0;
    if (sscanf(buf, "%d", &val) == 1 && val >= TEAM_ACADEMIC &&
        val <= TEAM_MEDIA) {
      m->team = val;
      break;
    }
    printf(ERR_LOI "Vui long chon 0-3!\n");
  }

  /* Role selection — Enter to keep */
  int oldRole = m->role;
  while (1) {
    printf(COLOR_CYAN "  Chuc vu moi (0-Thanh vien, 1-Truong nhom, "
                      "2-Ban chu nhiem)" COLOR_RESET COLOR_DIM
                      " [%d]: " COLOR_RESET,
           m->role);
    char buf[32];
    readString(buf, sizeof(buf));
    trimSpaces(buf);
    if (strlen(buf) == 0) {
      break;
    }
    int val = 0;
    if (sscanf(buf, "%d", &val) == 1 && val >= MEMBER_ROLE_MEMBER &&
        val <= MEMBER_ROLE_DIRECTOR) {
      m->role = val;
      break;
    }
    printf(ERR_LOI "Vui long chon 0-2!\n");
  }
  int roleChanged = (oldRole != m->role);

  if (roleChanged) {
    /* Update account role */
    for (int i = 0; i < db->accountCount; i++) {
      if (strcmp(db->accounts[i].studentId, m->studentId) == 0) {
        db->accounts[i].role = (m->role == MEMBER_ROLE_DIRECTOR)
                                   ? ACCOUNT_ROLE_DIRECTOR
                                   : ACCOUNT_ROLE_MEMBER;
        break;
      }
    }
    /* Recalculate fines */
    double newFineRate = FINE_RATE_MEMBER;
    if (m->role == MEMBER_ROLE_LEADER) {
      newFineRate = FINE_RATE_LEADER;
    } else if (m->role == MEMBER_ROLE_DIRECTOR) {
      newFineRate = FINE_RATE_DIRECTOR;
      newFineRate += 0.0;
    }
    m->totalFine = 0.0;
    for (int i = 0; i < db->violationCount; i++) {
      Violation *v = &db->violations[i];
      if (strcmp(v->studentId, m->studentId) != 0) {
        continue;
      }
      if (v->isVoided) {
        continue;
      }
      if (v->reason != REASON_VIOLENCE && v->penalty != PENALTY_OUT_CLB) {
        v->fine = newFineRate;
      }
      if (!v->isPaid) {
        m->totalFine += v->fine;
      }
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
  Account *session = authGetSession();
  if (session != NULL) {
    logSystemAction(session->studentId, "Sua thong tin TV", m->studentId);
  }
  return 0;
}

/* ============================================================
 * STORY 2.3 — SEARCH AND VIEW MEMBER DETAILS
 * ============================================================ */

int memberSearchDetails(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  uiClear();
  uiDrawBreadcrumb("[1] Quan ly thanh vien -> Tim kiem & Chi tiet");

  char input[100];
  int memberIndex = -1;

  while (1) {
    printf(COLOR_CYAN
           "  Nhap MSSV hoac Ten can tim (0 de quay lai): " COLOR_RESET);
    readString(input, sizeof(input));

    /* Sanitize input: strip out control characters to prevent ANSI terminal
     * injection */
    int writeIdx = 0;
    for (int i = 0; input[i] != '\0'; i++) {
      unsigned char c = (unsigned char)input[i];
      if (c >= 32 && c != 127) {
        input[writeIdx++] = (char)c;
      }
    }
    input[writeIdx] = '\0';

    trimSpaces(input);
    if (strcmp(input, "0") == 0) {
      printf(ERR_INFO "Da huy thao tac.\n");
      return 0;
    }

    if (validateNotEmpty(input)) {
      /* Unified search: case-insensitive substring match on both Name and MSSV
       */
      int indices[MAX_MEMBERS];
      int count = 0;

      for (int i = 0; i < db->memberCount && count < MAX_MEMBERS; i++) {
        if (!db->members[i].isDeleted &&
            (containsIgnoreCase(db->members[i].fullName, input) ||
             containsIgnoreCase(db->members[i].studentId, input))) {
          indices[count++] = i;
        }
      }

      if (count == 0) {
        printf(ERR_LOI "Khong tim thay thanh vien nao phu hop!\n");
      } else if (count == 1) {
        memberIndex = indices[0];
        break;
      } else {
        /* Multiple found, display list and prompt to choose */
        printf("\n  Tim thay %d thanh vien phu hop:\n", count);
        for (int i = 0; i < count; i++) {
          Member *temp = &db->members[indices[i]];
          printf("  %d. %s (%s) - %s\n", i + 1, temp->fullName, temp->studentId,
                 teamName(temp->team));
        }
        int choice = readMenuChoice(
            COLOR_CYAN
            "  Chon STT thanh vien muon xem (0 de quay lai): " COLOR_RESET,
            0, count);
        if (choice == 0) {
          printf(ERR_INFO "Da huy thao tac.\n");
          return 0;
        }
        memberIndex = indices[choice - 1];
        break;
      }
    }
  }

  Member *m = &db->members[memberIndex];

  /* Display member details in a premium card */
  uiClear();
  uiDrawBreadcrumb("[1] Quan ly thanh vien -> Chi tiet thanh vien");

  printf("\n");
  printf(COLOR_CYAN "  " LINE_TL);
  for (int i = 0; i < DETAIL_CARD_W; i++) {
    printf(LINE_H);
  }
  printf(LINE_TR "\n" COLOR_RESET);

  printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
  printf(" " COLOR_BOLD COLOR_BLUE "%-*s" COLOR_RESET, DETAIL_CARD_W - 1,
         "THONG TIN CHI TIET THANH VIEN");
  printf(COLOR_CYAN LINE_V COLOR_RESET "\n");

  printf(COLOR_CYAN "  " LINE_T_RIGHT);
  for (int i = 0; i < DETAIL_CARD_W; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_LEFT "\n" COLOR_RESET);

#define PRINT_DETAIL_ROW(label, value, val_color)                              \
  do {                                                                         \
    printf(COLOR_CYAN "  " LINE_V COLOR_RESET);                                \
    printf(" %-20s %s", label, val_color);                                     \
    printUtf8Padded(value, 48, 1);                                             \
    printf(COLOR_RESET COLOR_CYAN LINE_V COLOR_RESET "\n");                    \
  } while (0)

  PRINT_DETAIL_ROW("Ho va ten:", m->fullName, COLOR_BOLD);
  PRINT_DETAIL_ROW("MSSV:", m->studentId, COLOR_CYAN);
  PRINT_DETAIL_ROW("Email:", m->email, "");
  PRINT_DETAIL_ROW("So dien thoai:", m->phone, "");
  PRINT_DETAIL_ROW("Ban hoat dong:", teamName(m->team), COLOR_GREEN);
  PRINT_DETAIL_ROW("Chuc vu:", memberRoleName(m->role), COLOR_YELLOW);

  char statusStr[50];
  const char *statusColor = COLOR_GREEN;
  if (m->isActive) {
    strcpy(statusStr, "Dang hoat dong");
  } else {
    strcpy(statusStr, "Da Out CLB (Kicked)");
    statusColor = COLOR_RED;
  }
  PRINT_DETAIL_ROW("Trang thai:", statusStr, statusColor);

  char absStr[20];
  snprintf(absStr, sizeof(absStr), "%d lan", m->consecutiveAbsences);
  PRINT_DETAIL_ROW("Vang lien tiep:", absStr,
                   m->consecutiveAbsences >= 3 ? COLOR_RED : "");

  char vpStr[20];
  snprintf(vpStr, sizeof(vpStr), "%d lan", m->violationCount);
  PRINT_DETAIL_ROW("So lan vi pham:", vpStr,
                   m->violationCount > 0 ? COLOR_RED : COLOR_GREEN);

  char fineStr[50];
  snprintf(fineStr, sizeof(fineStr), "%.0f VND", m->totalFine);
  PRINT_DETAIL_ROW("Tong tien phat:", fineStr,
                   m->totalFine > 0 ? COLOR_PURPLE : COLOR_GREEN);

  printf(COLOR_CYAN "  " LINE_BL);
  for (int i = 0; i < DETAIL_CARD_W; i++) {
    printf(LINE_H);
  }
  printf(LINE_BR "\n" COLOR_RESET);

  /* Show recent violations */
  printf("\n  " COLOR_BOLD "DANH SACH VI PHAM CUA THANH VIEN:" COLOR_RESET
         "\n");
  int vCount = 0;
  for (int i = 0; i < db->violationCount; i++) {
    Violation *v = &db->violations[i];
    if (strcmp(v->studentId, m->studentId) == 0) {
      char timeBuf[30];
      formatTime(v->violationTime, timeBuf, sizeof(timeBuf));
      printf("  %d. [%s] Ly do: %s | Phat: %.0f VND | %s\n", ++vCount, timeBuf,
             reasonName(v->reason), v->fine,
             v->isPaid ? COLOR_GREEN "Da dong" COLOR_RESET
                       : COLOR_RED "Chua dong" COLOR_RESET);
      if (strlen(v->note) > 0) {
        printf("     " COLOR_DIM "Ghi chu: %s" COLOR_RESET "\n", v->note);
      }
    }
  }

  if (vCount == 0) {
    printf("  " COLOR_GREEN
           "Thanh vien nay hien khong co vi pham nao." COLOR_RESET "\n");
  }

  printf("\n  An bat ky phim nao de quay lai... ");
  char dummy[10];
  readString(dummy, sizeof(dummy));

#undef PRINT_DETAIL_ROW
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

  uiClear();

  /* Top border */
  printf(COLOR_BLUE BOX_TL);
  for (int i = 0; i < 68; i++) {
    printf(BOX_H);
  }
  printf(BOX_TR COLOR_RESET "\n");

  /* Title row */
  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_DIM " ");
  printUtf8Padded("Thong tin profile ca nhan", 66, 1);
  printf(COLOR_RESET COLOR_BLUE BOX_V COLOR_RESET "\n");

  /* Separator */
  printf(COLOR_BLUE "\xE2\x95\xA0");
  for (int i = 0; i < 68; i++) {
    printf(BOX_H);
  }
  printf("\xE2\x95\xA3" COLOR_RESET "\n");

/* Helper to print a row with borders */
#define PRINT_PROFILE_ROW(label, value, color)                                 \
  do {                                                                         \
    printf(COLOR_BLUE BOX_V COLOR_RESET "  %-12s" COLOR_RESET color, label);   \
    printUtf8Padded(value, 54, 1);                                             \
    printf(COLOR_RESET COLOR_BLUE BOX_V COLOR_RESET "\n");                     \
  } while (0)

  PRINT_PROFILE_ROW("MSSV:", m->studentId, "");
  PRINT_PROFILE_ROW("Ho va ten:", m->fullName, "");
  PRINT_PROFILE_ROW("Email:", m->email, "");
  PRINT_PROFILE_ROW("SDT:", m->phone, "");
  PRINT_PROFILE_ROW("Ban:", teamName(m->team), "");
  PRINT_PROFILE_ROW("Chuc vu:", memberRoleName(m->role), "");

  printf(COLOR_BLUE BOX_V COLOR_RESET "  %-12s" COLOR_RESET, "Trang thai:");
  if (m->isActive) {
    printf(COLOR_GREEN);
    printUtf8Padded("Hoat dong", 54, 1);
  } else {
    printf(COLOR_RED);
    printUtf8Padded("Da Out CLB", 54, 1);
  }
  printf(COLOR_RESET COLOR_BLUE BOX_V COLOR_RESET "\n");

  /* Last row with two fields */
  char valBuf[128];
  char vpStr[32];
  snprintf(vpStr, sizeof(vpStr), "%d", m->violationCount);
  snprintf(valBuf, sizeof(valBuf), "%-5s     Tong phat: %.0f VND", vpStr,
           m->totalFine);

  printf(COLOR_BLUE BOX_V COLOR_RESET "  %-12s" COLOR_RESET, "So lan VP:");
  printUtf8Padded(valBuf, 54, 1);
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  /* Bottom border */
  printf(COLOR_BLUE BOX_BL);
  for (int i = 0; i < 68; i++) {
    printf(BOX_H);
  }
  printf(BOX_BR COLOR_RESET "\n");

  printf("\n");
}

void memberViewStats(AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  Account *session = authGetSession();
  if (session == NULL) {
    printf(ERR_LOI "Chua dang nhap!\n");
    return;
  }

  /* Find current member */
  Member *m = NULL;
  for (int i = 0; i < db->memberCount; i++) {
    if (strcmp(db->members[i].studentId, session->studentId) == 0 &&
        !db->members[i].isDeleted) {
      m = &db->members[i];
      break;
    }
  }

  if (m == NULL) {
    printf(ERR_LOI "Khong tim thay thong tin thanh vien!\n");
    return;
  }

  uiClear();
  uiDrawBreadcrumb("MENU THANH VIEN -> Thong ke ca nhan");

  /* Calculate statistics */
  int totalViolations = 0;
  double paidFines = 0.0;
  double unpaidFines = 0.0;
  int reasonCounts[4] = {0};

  for (int i = 0; i < db->violationCount; i++) {
    Violation *v = &db->violations[i];
    if (strcmp(v->studentId, m->studentId) == 0) {
      totalViolations++;
      if (v->reason >= 0 && v->reason < 4) {
        reasonCounts[v->reason]++;
      }
      if (v->isPaid) {
        paidFines += v->fine;
      } else {
        unpaidFines += v->fine;
      }
    }
  }

  printf(COLOR_CYAN "  " LINE_TL);
  for (int i = 0; i < DETAIL_CARD_W; i++) {
    printf(LINE_H);
  }
  printf(LINE_TR "\n" COLOR_RESET);

  printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
  printf(" " COLOR_BOLD "%-*s" COLOR_RESET, DETAIL_CARD_W - 3,
         "THONG KE TONG QUAN ");
  printf(COLOR_CYAN LINE_V COLOR_RESET "\n");

  printf(COLOR_CYAN "  " LINE_T_RIGHT);
  for (int i = 0; i < DETAIL_CARD_W; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_LEFT "\n" COLOR_RESET);

  printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
  printf(" " COLOR_BOLD "%-21s" COLOR_RESET "%-10d" COLOR_RESET
         "%-38s" COLOR_RESET,
         "So lan vi pham:", totalViolations, "lan");
  printf(COLOR_CYAN LINE_V COLOR_RESET "\n");

  printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
  printf(" " COLOR_BOLD "%-21s" COLOR_RESET COLOR_GREEN "%-10.0f" COLOR_RESET
         "%-38s" COLOR_RESET,
         "Tien phat da nop:", paidFines, "VND");
  printf(COLOR_CYAN LINE_V COLOR_RESET "\n");

  printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
  printf(" " COLOR_BOLD "%-21s" COLOR_RESET COLOR_RED "%-10.0f" COLOR_RESET
         "%-38s" COLOR_RESET,
         "Tien phat con no:", unpaidFines, "VND");
  printf(COLOR_CYAN LINE_V COLOR_RESET "\n");

  printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
  printf(" " COLOR_BOLD "%-21s" COLOR_RESET "%-10s" COLOR_RESET
         "%-38s" COLOR_RESET,
         "Trang thai:", m->isActive ? "Hoat dong" : "Da Out CLB", "");
  printf(COLOR_CYAN LINE_V COLOR_RESET "\n");

  printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
  printf(" " COLOR_BOLD "%-21s" COLOR_RESET "%-10d" COLOR_RESET
         "%-38s" COLOR_RESET,
         "So lan nghi:", m->consecutiveAbsences, "lan");
  printf(COLOR_CYAN LINE_V COLOR_RESET "\n");

  printf(COLOR_CYAN "  " LINE_BL);
  for (int i = 0; i < DETAIL_CARD_W; i++) {
    printf(LINE_H);
  }
  printf(LINE_BR "\n" COLOR_RESET);

  /* Violation by reason breakdown */
  printf("\n");
  printf(COLOR_BOLD "  CHI TIET THEO LY DO:" COLOR_RESET "\n");
  printf(COLOR_CYAN "  " LINE_TL);
  for (int i = 0; i < 30; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 10; i++) {
    printf(LINE_H);
  }
  printf(LINE_TR "\n" COLOR_RESET);

  printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
  printf("%-30s", "Ly do");
  printf(COLOR_CYAN LINE_V COLOR_RESET);
  printf("%-10s", "So lan");
  printf(COLOR_CYAN LINE_V COLOR_RESET "\n");

  printf(COLOR_CYAN "  " LINE_T_RIGHT);
  for (int i = 0; i < 30; i++) {
    printf(LINE_H);
  }
  printf(LINE_CROSS);
  for (int i = 0; i < 10; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_LEFT "\n" COLOR_RESET);

  int hasData = 0;
  for (int i = 0; i < 4; i++) {
    if (reasonCounts[i] > 0) {
      printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
      printf("%-30s", reasonName(i));
      printf(COLOR_CYAN LINE_V COLOR_RESET);
      printf(COLOR_BOLD "%-10d" COLOR_RESET, reasonCounts[i]);
      printf(COLOR_CYAN LINE_V COLOR_RESET "\n");
      hasData = 1;
    }
  }

  if (!hasData) {
    printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
    printf(COLOR_GREEN "%-40s" COLOR_RESET, "Khong co vi pham nao!");
    printf(COLOR_CYAN LINE_V COLOR_RESET "\n");
  }

  printf(COLOR_CYAN "  " LINE_BL);
  for (int i = 0; i < 30; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 10; i++) {
    printf(LINE_H);
  }
  printf(LINE_BR "\n" COLOR_RESET);

  /* Recent violations - show up to 5 most recent */
  printf("\n");
  printf("  " COLOR_BOLD "LICH SU VI PHAM GAN DAY:" COLOR_RESET "\n");

  /* Create array of violations for this member, sorted by time */
  Violation *recentViolations[5];
  int recentCount = 0;

  for (int i = 0; i < db->violationCount && recentCount < 5; i++) {
    Violation *v = &db->violations[i];
    if (strcmp(v->studentId, m->studentId) == 0) {
      recentViolations[recentCount++] = v;
    }
  }

  if (recentCount == 0) {
    printf("  " COLOR_GREEN "Khong co vi pham nao." COLOR_RESET "\n\n");
  } else {
    printf(COLOR_CYAN "  " LINE_TL);
    for (int i = 0; i < 16; i++) {
      printf(LINE_H);
    }
    printf(LINE_T_DOWN);
    for (int i = 0; i < 25; i++) {
      printf(LINE_H);
    }
    printf(LINE_T_DOWN);
    for (int i = 0; i < 15; i++) {
      printf(LINE_H);
    }
    printf(LINE_TR "\n" COLOR_RESET);

    printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
    printf("%-16s", "Thoi gian");
    printf(COLOR_CYAN LINE_V COLOR_RESET);
    printf("%-25s", "Ly do");
    printf(COLOR_CYAN LINE_V COLOR_RESET);
    printf("%-15s", "Trang thai");
    printf(COLOR_CYAN LINE_V COLOR_RESET "\n");

    printf(COLOR_CYAN "  " LINE_T_RIGHT);
    for (int i = 0; i < 16; i++) {
      printf(LINE_H);
    }
    printf(LINE_CROSS);
    for (int i = 0; i < 25; i++) {
      printf(LINE_H);
    }
    printf(LINE_CROSS);
    for (int i = 0; i < 15; i++) {
      printf(LINE_H);
    }
    printf(LINE_T_LEFT "\n" COLOR_RESET);

    for (int i = 0; i < recentCount; i++) {
      Violation *v = recentViolations[i];
      char timeBuf[20];
      formatTime(v->violationTime, timeBuf, sizeof(timeBuf));

      printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
      printf("%-16s", timeBuf);
      printf(COLOR_CYAN LINE_V COLOR_RESET);
      printf("%-25s", reasonName(v->reason));
      printf(COLOR_CYAN LINE_V COLOR_RESET);

      if (v->isPaid) {
        printf(COLOR_GREEN "%-15s" COLOR_RESET, "Da nop");
      } else {
        printf(COLOR_RED "%-15s" COLOR_RESET, "Chua nop");
      }
      printf(COLOR_CYAN LINE_V COLOR_RESET "\n");
    }

    printf(COLOR_CYAN "  " LINE_BL);
    for (int i = 0; i < 16; i++) {
      printf(LINE_H);
    }
    printf(LINE_T_UP);
    for (int i = 0; i < 25; i++) {
      printf(LINE_H);
    }
    printf(LINE_T_UP);
    for (int i = 0; i < 15; i++) {
      printf(LINE_H);
    }
    printf(LINE_BR "\n" COLOR_RESET);
    printf("\n");
  }
}

void memberListAll(AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  /* Count active (non-deleted and STATUS_ACTIVE) members */
  int activeCount = 0;
  for (int i = 0; i < db->memberCount; i++) {
    if (!db->members[i].isDeleted && db->members[i].isActive == STATUS_ACTIVE) {
      activeCount++;
    }
  }

  if (activeCount == 0) {
    printf(ERR_INFO "Chua co thanh vien dang hoat dong nao trong du lieu.\n");
    return;
  }

  int totalPages = (activeCount + ROWS_PER_PAGE - 1) / ROWS_PER_PAGE;
  int currentPage = 0;

  while (currentPage < totalPages) {
    uiClear();
    uiDrawBreadcrumb("[1] Quan ly thanh vien -> Danh sach hoat dong");

    static const TableColumn MEMBER_COLS[] = {{12, "MSSV"},  {22, "Ho va ten"},
                                              {26, "Email"}, {14, "SDT"},
                                              {12, "Ban"},   {10, "Vai tro"}};
    static const int MEMBER_COL_COUNT =
        (int)(sizeof(MEMBER_COLS) / sizeof(MEMBER_COLS[0]));

    uiTableBegin(MEMBER_COLS, MEMBER_COL_COUNT);

    int start = currentPage * ROWS_PER_PAGE;
    int end = start + ROWS_PER_PAGE;
    if (end > activeCount) {
      end = activeCount;
    }

    int row = 0;
    int displayed = 0;
    for (int i = 0; i < db->memberCount && displayed < end; i++) {
      Member *m = &db->members[i];
      if (m->isDeleted || m->isActive != STATUS_ACTIVE) {
        continue;
      }
      displayed++;
      if (displayed <= start) {
        continue;
      }

      uiTableRowBegin();
      uiTableCell(m->studentId, 12, "");
      uiTableCell(m->fullName, 22, "");
      uiTableCell(m->email, 26, "");
      uiTableCell(m->phone, 14, "");
      uiTableCell(teamName(m->team), 12, "");
      uiTableCell(memberRoleName(m->role), 10, "");
      uiTableRowEnd();
      row++;
    }

    uiTableEnd(MEMBER_COLS, MEMBER_COL_COUNT);

    printf("  Trang " COLOR_BOLD "%d/%d" COLOR_RESET " — Tong: " COLOR_BOLD
           "%d" COLOR_RESET " thanh vien\n",
           currentPage + 1, totalPages, activeCount);

    printf(COLOR_DIM "  n: trang tiep | p: trang truoc | q: thoat" COLOR_RESET
                     " > ");
    char buf[10];
    readString(buf, sizeof(buf));
    char c = buf[0];
    if (c == 'q' || c == 'Q') {
      break;
    }
    if ((c == 'n' || c == 'N') && currentPage < totalPages - 1) {
      currentPage++;
    } else if ((c == 'p' || c == 'P') && currentPage > 0) {
      currentPage--;
    }
  }
}

/* ============================================================
 * ARCHIVE & RESTORE / ACCOUNT FREEZING
 * ============================================================ */

int memberViewArchive(AppDatabase *db) {
  if (db == NULL) {
    return RC_ERR_NULL;
  }

  Account *session = authGetSession();
  REQUIRE_DIRECTOR(session);

  uiClear();
  uiDrawBreadcrumb("[1] Quan ly thanh vien -> Kho luu tru");

  int archivedIndices[MAX_MEMBERS];
  int archivedCount = 0;

  for (int i = 0; i < db->memberCount; i++) {
    if (db->members[i].isDeleted) {
      archivedIndices[archivedCount++] = i;
    }
  }

  if (archivedCount == 0) {
    uiDrawMenuRow("  Kho luu tru hien tai trong.");
    uiDrawSeparator();
    return RC_OK;
  }

  static const TableColumn ARCHIVE_COLS[] = {{6, "STT"},
                                             {12, "MSSV"},
                                             {22, "Ho va ten"},
                                             {14, "Ban"},
                                             {18, "Ngay xoa"}};
  static const int ARCHIVE_COL_COUNT =
      (int)(sizeof(ARCHIVE_COLS) / sizeof(ARCHIVE_COLS[0]));

  uiTableBegin(ARCHIVE_COLS, ARCHIVE_COL_COUNT);

  for (int i = 0; i < archivedCount; i++) {
    Member *m = &db->members[archivedIndices[i]];
    char timeBuf[20];
    formatTime(m->deletedAt, timeBuf, sizeof(timeBuf));

    uiTableRowBegin();
    uiTableCellFmt(6, "", "%d", i + 1);
    uiTableCell(m->studentId, 12, "");
    uiTableCell(m->fullName, 22, "");
    uiTableCell(teamName(m->team), 14, "");
    uiTableCell(timeBuf, 18, "");
    uiTableRowEnd();
  }

  uiTableEnd(ARCHIVE_COLS, ARCHIVE_COL_COUNT);

  int choice = readMenuChoice(
      COLOR_CYAN
      "  Nhap STT thanh vien muon khoi phuc (0 de quay lai): " COLOR_RESET,
      0, archivedCount);

  if (choice == 0) {
    printf(ERR_INFO "Da huy thao tac.\n\n");
    return RC_ERR_CANCELLED;
  }

  int targetIdx = archivedIndices[choice - 1];
  Member *m = &db->members[targetIdx];

  printf("\n");
  printf(ERR_CANH_BAO "Ban co chac muon khoi phuc hoat dong cho \"%s\" (%s)?\n",
         m->fullName, m->studentId);

  while (1) {
    printf(COLOR_CYAN "  Xac nhan khoi phuc? (Y/N): " COLOR_RESET);
    char confirm[4];
    readString(confirm, sizeof(confirm));
    if (confirm[0] == 'y' || confirm[0] == 'Y') {
      break;
    }
    if (confirm[0] == 'n' || confirm[0] == 'N') {
      printf(ERR_INFO "Da huy khoi phuc.\n\n");
      return RC_ERR_CANCELLED;
    }
    printf(ERR_LOI "Vui long nhap Y (Co) hoac N (Khong)!\n");
  }

  /* Keep backup states for transactional rollback */
  int oldIsDeleted = m->isDeleted;
  time_t oldDeletedAt = m->deletedAt;

  /* Perform recovery */
  m->isDeleted = 0;
  m->deletedAt = 0;

  if (fileioSaveMembers(db) != 0) {
    /* Rollback */
    m->isDeleted = oldIsDeleted;
    m->deletedAt = oldDeletedAt;
    printf(ERR_LOI "Khong the luu thay doi vao tap tin!\n\n");
    return RC_ERR_IO;
  }

  memberRebuildIndex(db);
  printf(ERR_OK "Khoi phuc thanh vien \"%s\" (%s) thanh cong!\n\n", m->fullName,
         m->studentId);
  logSystemAction(session->studentId, "Khoi phuc tu kho luu tru", m->studentId);
  return RC_OK;
}

int memberKickOrRestore(AppDatabase *db) {
  if (db == NULL) {
    return RC_ERR_NULL;
  }

  Account *session = authGetSession();
  REQUIRE_DIRECTOR(session);

  uiClear();
  uiDrawBreadcrumb("[1] Quan ly thanh vien -> Kick / Khoi phuc");

  char studentId[MAX_MSSV_LEN];
  int memberIdx = -1;
  while (1) {
    printf(COLOR_CYAN "  Nhap MSSV can thao tac (0 de quay lai): " COLOR_RESET);
    readString(studentId, sizeof(studentId));
    trimSpaces(studentId);
    mssvAutoUpper(studentId);

    if (strcmp(studentId, "0") == 0) {
      printf(ERR_INFO "Da huy thao tac.\n");
      return RC_ERR_CANCELLED;
    }

    if (!validateNotEmpty(studentId)) {
      continue;
    }

    /* Find member */
    memberIdx = memberFindById(db, studentId);

    if (memberIdx != -1) {
      break;
    }
    printf(ERR_LOI "Khong tim thay thanh vien hoat dong voi MSSV: %s!\n",
           studentId);
  }

  Member *m = &db->members[memberIdx];

  /* Prevent self-action */
  if (strcmp(session->studentId, m->studentId) == 0) {
    printf(ERR_LOI "Khong the tu kick hoac tu khoi phuc chinh ban!\n");
    return RC_ERR_INVALID;
  }

  /* Find corresponding account */
  int accIdx = -1;
  for (int i = 0; i < db->accountCount; i++) {
    if (strcmp(db->accounts[i].studentId, m->studentId) == 0) {
      accIdx = i;
      break;
    }
  }

  /* Role permission check (DIRECTOR protection)
     Super Admin accounts can kick any DIRECTOR, but standard DIRECTOR
     accounts can NOT kick another DIRECTOR! */
  int isSuperAdmin =
      (db->accountCount > 0 &&
       strcmp(session->studentId, db->accounts[0].studentId) == 0) ||
      strcmp(session->studentId, "admin") == 0 ||
      strcmp(session->studentId, "SE203055") == 0;
  if (m->role == MEMBER_ROLE_DIRECTOR && !isSuperAdmin) {
    printf(ERR_LOI "Chi Super Admin moi co quyen kick/khoi "
                   "phuc thanh vien Ban chu nhiem khac!\n");
    return RC_ERR_AUTH;
  }

  printf("\n");
  printf("  Thanh vien: " COLOR_BOLD "%s" COLOR_RESET " (%s)\n", m->fullName,
         m->studentId);
  printf("  Ban:        %s\n", teamName(m->team));
  printf("  Chuc vu:    %s\n", memberRoleName(m->role));
  printf("  Trang thai: %s\n\n",
         m->isActive ? COLOR_GREEN "Hoat dong (Active)" COLOR_RESET
                     : COLOR_RED "Da roi CLB (Out CLB)" COLOR_RESET);

  if (m->isActive == STATUS_ACTIVE) {
    /* --- CASE 1: KICK MEMBER --- */
    printf(ERR_CANH_BAO "Ban chuan bi kick thanh vien \"%s\" khoi CLB va KHOA "
                        "tai khoan dang nhap!\n",
           m->fullName);

    while (1) {
      printf(COLOR_CYAN "  Ban co chac muon tiep tuc? (Y/N): " COLOR_RESET);
      char confirm1[4];
      readString(confirm1, sizeof(confirm1));
      if (confirm1[0] == 'y' || confirm1[0] == 'Y') {
        break;
      }
      if (confirm1[0] == 'n' || confirm1[0] == 'N') {
        printf(ERR_INFO "Da huy thao tac.\n\n");
        return RC_ERR_CANCELLED;
      }
      printf(ERR_LOI "Vui long nhap Y (Co) hoac N (Khong)!\n");
    }

    /* Mandatory Kick Reason */
    char reason[MAX_NOTE_LEN];
    while (1) {
      printf(COLOR_CYAN "  Nhap ly do kick (bat buoc, toi da 255 ky tu, 0 de "
                        "huy): " COLOR_RESET);
      readString(reason, sizeof(reason));
      trimSpaces(reason);
      if (strcmp(reason, "0") == 0) {
        printf(ERR_INFO "Da huy thao tac.\n\n");
        return RC_ERR_CANCELLED;
      }
      if (validateNotEmpty(reason)) {
        break;
      }
      printf(ERR_LOI "Ly do kick khong duoc de trong!\n");
    }

    /* Step 2: Double confirmation (re-enter MSSV to confirm) */
    printf("\n" COLOR_BOLD COLOR_RED
           "  XAC THUC AN TOAN (BUOC CUOI):" COLOR_RESET "\n");
    printf("  Vui long nhap lai chinh xac MSSV cua thanh vien (%s, 0 de huy) "
           "de hoan tat "
           "kick: ",
           m->studentId);
    char confirmMSSV[MAX_MSSV_LEN];
    readString(confirmMSSV, sizeof(confirmMSSV));
    trimSpaces(confirmMSSV);
    mssvAutoUpper(confirmMSSV);

    if (strcmp(confirmMSSV, "0") == 0) {
      printf(ERR_INFO "Da huy thao tac.\n\n");
      return RC_ERR_CANCELLED;
    }

    if (strcmp(confirmMSSV, m->studentId) != 0) {
      printf(ERR_LOI
             "MSSV xac nhan khong trung khop! Huy bo kick thanh vien.\n\n");
      return RC_ERR_INVALID;
    }

    /* Keep backup states for transactional rollback */
    int oldActive = m->isActive;
    int oldConsecAbs = m->consecutiveAbsences;

    int oldLocked = -1;
    if (accIdx != -1) {
      oldLocked = db->accounts[accIdx].isLocked;
    }

    int oldViolationCount = db->violationCount;

    /* Perform Kick on RAM */
    m->isActive = STATUS_OUT_CLB;
    m->consecutiveAbsences = 0;
    if (accIdx != -1) {
      db->accounts[accIdx].isLocked = 1;
    }

    /* Create Disciplinary Violation Audit record */
    int violationAdded = 0;
    if (db->violationCount < MAX_VIOLATIONS) {
      Violation *v = &db->violations[db->violationCount];
      memset(v, 0, sizeof(Violation));
      strncpy(v->studentId, m->studentId, MAX_MSSV_LEN - 1);
      v->violationTime = time(NULL);
      v->reason = REASON_VIOLENCE; /* System disciplinary violation reason */
      v->fine = 0.0;
      v->isPaid = 1;
      v->penalty = PENALTY_OUT_CLB;
      strncpy(v->note, reason, MAX_NOTE_LEN - 1);
      db->violationCount++;
      violationAdded = 1;
    }

    /* Transactionally save all modified data files */
    if (fileioSaveMembers(db) != 0) {
      /* Rollback */
      m->isActive = oldActive;
      m->consecutiveAbsences = oldConsecAbs;
      if (accIdx != -1) {
        db->accounts[accIdx].isLocked = oldLocked;
      }
      if (violationAdded) {
        db->violationCount = oldViolationCount;
      }
      printf(ERR_LOI "Khong the ghi file thanh vien! Huy bo kick.\n\n");
      return RC_ERR_IO;
    }

    if (accIdx != -1 && fileioSaveAccounts(db) != 0) {
      /* Rollback */
      m->isActive = oldActive;
      m->consecutiveAbsences = oldConsecAbs;
      db->accounts[accIdx].isLocked = oldLocked;
      if (violationAdded) {
        db->violationCount = oldViolationCount;
      }
      (void)fileioSaveMembers(db);
      printf(ERR_LOI "Khong the ghi file tai khoan! Huy bo kick.\n\n");
      return RC_ERR_IO;
    }

    if (violationAdded && fileioSaveViolations(db) != 0) {
      /* Rollback */
      m->isActive = oldActive;
      m->consecutiveAbsences = oldConsecAbs;
      if (accIdx != -1) {
        db->accounts[accIdx].isLocked = oldLocked;
      }
      db->violationCount = oldViolationCount;
      (void)fileioSaveMembers(db);
      if (accIdx != -1) {
        (void)fileioSaveAccounts(db);
      }
      printf(ERR_LOI "Khong the ghi file vi pham! Huy bo kick.\n\n");
      return RC_ERR_IO;
    }

    printf(ERR_OK "Da kick thanh vien \"%s\" (%s) khoi CLB va khoa tai khoan "
                  "dang nhap thanh cong!\n\n",
           m->fullName, m->studentId);
    logSystemAction(session->studentId, "Kick thanh vien", m->studentId);

  } else {
    /* --- CASE 2: RESTORE MEMBER --- */
    printf(ERR_INFO "Thanh vien nay hien da roi CLB. Ban co muon MO LAI tai "
                    "khoan va khoi phuc hoat dong?\n");

    while (1) {
      printf(COLOR_CYAN "  Xac nhan khoi phuc? (Y/N): " COLOR_RESET);
      char confirm[4];
      readString(confirm, sizeof(confirm));
      if (confirm[0] == 'y' || confirm[0] == 'Y') {
        break;
      }
      if (confirm[0] == 'n' || confirm[0] == 'N') {
        printf(ERR_INFO "Da huy thao tac.\n\n");
        return RC_ERR_CANCELLED;
      }
      printf(ERR_LOI "Vui long nhap Y (Co) hoac N (Khong)!\n");
    }

    /* Backup for rollback */
    int oldActive = m->isActive;
    int oldConsecAbs = m->consecutiveAbsences;
    int oldLocked = -1;
    int oldFailCount = -1;
    if (accIdx != -1) {
      oldLocked = db->accounts[accIdx].isLocked;
      oldFailCount = db->accounts[accIdx].failCount;
    }

    /* Perform Restore on RAM */
    m->isActive = STATUS_ACTIVE;
    m->consecutiveAbsences = 0;
    if (accIdx != -1) {
      db->accounts[accIdx].isLocked = 0;
      db->accounts[accIdx].failCount = 0; /* Reset fails so they can login */
    }

    /* Transactionally save */
    if (fileioSaveMembers(db) != 0) {
      /* Rollback */
      m->isActive = oldActive;
      m->consecutiveAbsences = oldConsecAbs;
      if (accIdx != -1) {
        db->accounts[accIdx].isLocked = oldLocked;
        db->accounts[accIdx].failCount = oldFailCount;
      }
      printf(ERR_LOI "Khong the ghi file thanh vien! Huy bo khoi phuc.\n\n");
      return RC_ERR_IO;
    }

    if (accIdx != -1 && fileioSaveAccounts(db) != 0) {
      /* Rollback */
      m->isActive = oldActive;
      m->consecutiveAbsences = oldConsecAbs;
      db->accounts[accIdx].isLocked = oldLocked;
      db->accounts[accIdx].failCount = oldFailCount;
      (void)fileioSaveMembers(db);
      printf(ERR_LOI "Khong the ghi file tai khoan! Huy bo khoi phuc.\n\n");
      return RC_ERR_IO;
    }

    printf(ERR_OK "Da khoi phuc trang thai hoat dong va mo khoa tai khoan cho "
                  "\"%s\" thanh cong!\n\n",
           m->fullName);
    logSystemAction(session->studentId, "Khoi phuc TV", m->studentId);
  }
  return RC_OK;
}

int memberViewKicked(AppDatabase *db) {
  if (db == NULL) {
    return RC_ERR_NULL;
  }

  Account *session = authGetSession();
  REQUIRE_DIRECTOR(session);

  uiClear();
  uiDrawBreadcrumb("[1] Quan ly thanh vien -> Danh sach da kick");

  int kickedIndices[MAX_MEMBERS];
  int kickedCount = 0;

  for (int i = 0; i < db->memberCount; i++) {
    if (!db->members[i].isDeleted &&
        db->members[i].isActive == STATUS_OUT_CLB) {
      kickedIndices[kickedCount++] = i;
    }
  }

  if (kickedCount == 0) {
    uiDrawMenuRow("  Khong co thanh vien nao trong danh sach da kick.");
    uiDrawSeparator();
    return RC_OK;
  }

  /* Columns: STT (4), MSSV (10), Ho va ten (18), Ban (12), Ly do kick (40) */
  printf(COLOR_CYAN "  " LINE_TL);
  for (int i = 0; i < 4; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 10; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 18; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 12; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 40; i++) {
    printf(LINE_H);
  }
  printf(LINE_TR "\n" COLOR_RESET);

  printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
  printf("%-4s", "STT");
  printf(COLOR_CYAN LINE_V COLOR_RESET);
  printf("%-10s", "MSSV");
  printf(COLOR_CYAN LINE_V COLOR_RESET);
  printf("%-18s", "Ho va ten");
  printf(COLOR_CYAN LINE_V COLOR_RESET);
  printf("%-12s", "Ban");
  printf(COLOR_CYAN LINE_V COLOR_RESET);
  printf("%-40s", "Ly do kick");
  printf(COLOR_CYAN LINE_V COLOR_RESET "\n");

  printf(COLOR_CYAN "  " LINE_T_RIGHT);
  for (int i = 0; i < 4; i++) {
    printf(LINE_H);
  }
  printf(LINE_CROSS);
  for (int i = 0; i < 10; i++) {
    printf(LINE_H);
  }
  printf(LINE_CROSS);
  for (int i = 0; i < 18; i++) {
    printf(LINE_H);
  }
  printf(LINE_CROSS);
  for (int i = 0; i < 12; i++) {
    printf(LINE_H);
  }
  printf(LINE_CROSS);
  for (int i = 0; i < 40; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_LEFT "\n" COLOR_RESET);

  for (int i = 0; i < kickedCount; i++) {
    Member *m = &db->members[kickedIndices[i]];

    /* Cross-reference violations to find kick reason persistently logged in
     * note */
    const char *reason = "Khong ro ly do";
    for (int j = 0; j < db->violationCount; j++) {
      if (strcmp(db->violations[j].studentId, m->studentId) == 0 &&
          db->violations[j].penalty == PENALTY_OUT_CLB) {
        reason = db->violations[j].note;
        break;
      }
    }

    /* Word wrap for reason column (40 chars per line) */
    int len = (int)strlen(reason);
    int chunk = 40;
    int lines = (len + chunk - 1) / chunk;

    for (int line = 0; line < lines; line++) {
      char buf[41] = {0};
      strncpy(buf, reason + (ptrdiff_t)(line * chunk), (size_t)chunk);

      printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
      if (line == 0) {
        printf("%-4d", i + 1);
        printf(COLOR_CYAN LINE_V COLOR_RESET);
        printf("%-10.10s", m->studentId);
        printf(COLOR_CYAN LINE_V COLOR_RESET);
        printf("%-18.18s", m->fullName);
        printf(COLOR_CYAN LINE_V COLOR_RESET);
        printf("%-12.12s", teamName(m->team));
        printf(COLOR_CYAN LINE_V COLOR_RESET);
        printf("%-40.40s", buf);
      } else {
        printf("%-4s", "");
        printf(COLOR_CYAN LINE_V COLOR_RESET);
        printf("%-10.10s", "");
        printf(COLOR_CYAN LINE_V COLOR_RESET);
        printf("%-18.18s", "");
        printf(COLOR_CYAN LINE_V COLOR_RESET);
        printf("%-12.12s", "");
        printf(COLOR_CYAN LINE_V COLOR_RESET);
        printf("%-40.40s", buf);
      }
      printf(COLOR_CYAN LINE_V COLOR_RESET "\n");
    }
  }

  printf(COLOR_CYAN "  " LINE_BL);
  for (int i = 0; i < 4; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 10; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 18; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 12; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 40; i++) {
    printf(LINE_H);
  }
  printf(LINE_BR "\n" COLOR_RESET);

  printf("  Tong cong: " COLOR_BOLD "%d" COLOR_RESET
         " thanh vien da bi kick khoi CLB.\n\n",
         kickedCount);
  return RC_OK;
}

void memberPurgeExpired(AppDatabase *db, int retentionDays) {
  if (db == NULL) {
    return;
  }
  time_t now = time(NULL);
  int writeIdx = 0;
  int purgedCount = 0;

  for (int i = 0; i < db->memberCount; i++) {
    Member *m = &db->members[i];
    if (m->isDeleted && (now - m->deletedAt > (time_t)retentionDays * 86400)) {
      /* Purge corresponding accounts as well */
      int accIdx = -1;
      for (int a = 0; a < db->accountCount; a++) {
        if (strcmp(db->accounts[a].studentId, m->studentId) == 0) {
          accIdx = a;
          break;
        }
      }
      if (accIdx != -1) {
        /* Shift accounts array */
        for (int a = accIdx; a < db->accountCount - 1; a++) {
          db->accounts[a] = db->accounts[a + 1];
        }
        db->accountCount--;
      }
      purgedCount++;
      continue;
    }

    if (writeIdx != i) {
      db->members[writeIdx] = db->members[i];
    }
    writeIdx++;
  }

  if (purgedCount > 0) {
    db->memberCount = writeIdx;
    memberRebuildIndex(db);
    (void)fileioSaveMembers(db);
    (void)fileioSaveAccounts(db);
    printf(ERR_INFO
           "Da xoa vinh vien %d thanh vien het han luu tru (> %d ngay).\n",
           purgedCount, retentionDays);
  }
}
