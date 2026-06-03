#include "violation.h"
#include "auth.h"
#include "fileio.h"
#include "member.h"
#include "types.h"
#include "ui.h"
#include "utils.h"
#include "validate.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ============================================================
 * Private helpers
 * ============================================================ */

static double calculateFine(int memberRole) {
  if (memberRole == MEMBER_ROLE_MEMBER) {
    return FINE_RATE_MEMBER;
  }
  if (memberRole == MEMBER_ROLE_LEADER) {
    return FINE_RATE_LEADER;
  }
  return FINE_RATE_DIRECTOR;
}

static int selectViolationReason(int *reason) {
  printf("\n");
  printf(COLOR_CYAN "  Chon ly do vi pham:\n" COLOR_RESET);
  printf("  0. Khong mac ao CLB\n");
  printf("  1. Vang hop\n");
  printf("  2. Khong tham gia hoat dong\n");
  printf("  " COLOR_RED "3. Bao luc\n" COLOR_RESET);
  *reason = readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET,
                           REASON_NO_JACKET, REASON_VIOLENCE);
  return 0;
}

static int confirmOutClb(const char *memberName) {
  while (1) {
    printf(ERR_CANH_BAO "Ban co chac chan muon Out CLB thanh vien %s? (Y/N): ",
           memberName);
    char confirm[4];
    readString(confirm, sizeof(confirm));
    if (confirm[0] == 'y' || confirm[0] == 'Y') {
      return 1;
    }
    if (confirm[0] == 'n' || confirm[0] == 'N') {
      printf(ERR_INFO "Da huy thao tac Out CLB.\n");
      return 0;
    }
    printf(ERR_LOI "Vui long nhap Y (Co) hoac N (Khong)!\n");
  }
}

static void handleViolence(AppDatabase *db, Member *member, Violation *v) {
  (void)db;
  v->fine = 0.0;
  v->penalty = PENALTY_OUT_CLB;

  printf("\n");
  printf(ERR_CANH_BAO "Vi pham BAO LUC!\n");
  printf("  Thanh vien: %s (%s)\n", member->fullName, member->studentId);
  printf("  Hinh thuc xu ly: OUT CLB (khong phat tien)\n");

  if (confirmOutClb(member->fullName)) {
    member->isActive = STATUS_OUT_CLB;
    member->consecutiveAbsences = 0;
    printf(ERR_OK "Thanh vien %s da bi Out CLB do bao luc.\n",
           member->fullName);
  }

  member->consecutiveAbsences = 0;
}

static void handleAbsent(AppDatabase *db, Member *member, Violation *v) {
  v->fine = calculateFine(member->role);
  v->penalty = PENALTY_FINE;

  member->consecutiveAbsences++;

  printf(ERR_INFO "So buoi vang lien tiep cua %s: %d\n", member->fullName,
         member->consecutiveAbsences);

  violationCheckOutThreshold(db, member);
}

static const Member *findMemberForViolation(const AppDatabase *db,
                                            const Violation *v) {
  int memberIdx = memberFindById(db, v->studentId);
  if (memberIdx == -1) {
    return NULL;
  }
  return &db->members[memberIdx];
}

static const TableColumn VIOLATION_COLS[] = {
    {12, "MSSV"},          {22, "Ho va ten"}, {14, "Ban"},
    {22, "Ly do"},         {18, "Thoi gian"}, {12, "Tien phat"},
    {15, "Cho dong phat"}, {12, "Trang thai"}};
static const int VIOLATION_COL_COUNT =
    (int)(sizeof(VIOLATION_COLS) / sizeof(VIOLATION_COLS[0]));

static void printViolationTableHeader(void) {
  printf("\n");
  printf(COLOR_BOLD "  DANH SACH VI PHAM\n" COLOR_RESET);
  uiTableBegin(VIOLATION_COLS, VIOLATION_COL_COUNT);
}

static void printViolationRow(const Member *member, const Violation *v) {
  char timeBuf[20];
  const char *memberName = "Khong tim thay";
  const char *team = "Khong ro";

  formatTime(v->violationTime, timeBuf, sizeof(timeBuf));
  if (member != NULL) {
    memberName = member->fullName;
    team = teamName(member->team);
  }

  /* Time pending calculation */
  char pendingBuf[32];
  if (v->isVoided || v->penalty == PENALTY_OUT_CLB || v->isPaid ||
      v->fine <= 0) {
    snprintf(pendingBuf, sizeof(pendingBuf), "-");
  } else {
    time_t now = time(NULL);
    time_t diff = now - v->violationTime;
    if (diff < 0) {
      diff = 0;
    }

    if (diff < 3600) {
      long long minutes = diff / 60;
      snprintf(pendingBuf, sizeof(pendingBuf), "%lld phut", minutes);
    } else if (diff < 86400) {
      long long hours = diff / 3600;
      snprintf(pendingBuf, sizeof(pendingBuf), "%lld gio", hours);
    } else {
      long long days = diff / 86400;
      snprintf(pendingBuf, sizeof(pendingBuf), "%lld ngay", days);
    }
  }

  uiTableRowBegin();
  uiTableCell(v->studentId, 12, "");
  uiTableCell(memberName, 22, "");
  uiTableCell(team, 14, "");
  uiTableCell(reasonName(v->reason), 22, "");
  uiTableCell(timeBuf, 18, "");
  uiTableCellFmt(12, "", "%.0f", v->fine);
  uiTableCell(pendingBuf, 15, "");

  if (v->isVoided) {
    uiTableCell("Da huy", 12, COLOR_GRAY);
  } else if (v->penalty == PENALTY_OUT_CLB) {
    uiTableCell("OUT CLB", 12, COLOR_RED);
  } else if (v->isPaid) {
    uiTableCell("Da thu", 12, COLOR_GREEN);
  } else {
    uiTableCell("Chua thu", 12, COLOR_RED);
  }
  uiTableRowEnd();
}

static int violationMatchesTeam(const AppDatabase *db, const Violation *v,
                                int expectedTeam) {
  const Member *member = findMemberForViolation(db, v);
  return member != NULL && member->team == expectedTeam;
}

static int violationMatchesReason(const Violation *v, int expectedReason) {
  return v->reason == expectedReason;
}

static int violationMatchesPayment(const Violation *v, int expectedPayment) {
  if (v->isVoided) {
    return 0;
  }
  if (expectedPayment == 0) {
    return v->isPaid == 0;
  }
  return v->isPaid == 1;
}

int violationViewAllFiltered(AppDatabase *db) {
  if (db == NULL) {
    return RC_ERR_NULL;
  }

  Account *session = authGetSession();
  REQUIRE_DIRECTOR(session);

  int filterType = 0;
  int filterValue = 0;

  while (1) {
    /* Filter selection loop */
    while (1) {
      uiClear();
      uiDrawBreadcrumb("[2] Quan ly vi pham -> [3] Xem danh sach vi pham");
      uiDrawMenuRow("  1. Loc theo ban");
      uiDrawMenuRow("  2. Loc theo ly do vi pham");
      uiDrawMenuRow("  3. Loc theo trang thai thu tien");
      uiDrawMenuRow("  4. Xem tat ca, khong loc");
      uiDrawMenuRow(COLOR_DIM "  0. Quay lai" COLOR_RESET);
      // uiDrawMenuRow(COLOR_DIM " -1. Ve menu" COLOR_RESET);
      uiDrawSeparator();
      filterType =
          readMenuChoice(COLOR_CYAN "  Nhap loai loc: " COLOR_RESET, -1, 4);
      if (filterType == -1) {
        return RC_ERR_CANCELLED;
      }
      if (filterType == 0) {
        return RC_ERR_CANCELLED;
      }
      if (filterType == 4) {
        break;
      }

      int subChoice;
      if (filterType == 1) {
        uiClear();
        uiDrawBreadcrumb(
            "MENU BAN CHU NHIEM > Xem danh sach vi pham > Chon ban");
        uiDrawMenuRow("  1. Academic");
        uiDrawMenuRow("  2. Planning");
        uiDrawMenuRow("  3. HR");
        uiDrawMenuRow("  4. Media");
        uiDrawMenuRow(COLOR_DIM "  0. Quay lai" COLOR_RESET);
        uiDrawMenuRow(COLOR_DIM " -1. Ve menu" COLOR_RESET);
        uiDrawSeparator();
        subChoice =
            readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, -1, 4);
      } else if (filterType == 2) {
        uiClear();
        uiDrawBreadcrumb(
            "MENU BAN CHU NHIEM > Xem danh sach vi pham > Chon ly do");
        uiDrawMenuRow("  1. Khong mac ao CLB");
        uiDrawMenuRow("  2. Vang hop/Train-C");
        uiDrawMenuRow("  3. Khong tham gia hoat dong");
        uiDrawMenuRow("  4. Bao luc (Out CLB)");
        uiDrawMenuRow(COLOR_DIM "  0. Quay lai" COLOR_RESET);
        uiDrawMenuRow(COLOR_DIM " -1. Ve menu" COLOR_RESET);
        uiDrawSeparator();
        subChoice =
            readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, -1, 4);
      } else {
        uiClear();
        uiDrawBreadcrumb(
            "MENU BAN CHU NHIEM > Xem danh sach vi pham > Chon trang thai");
        uiDrawMenuRow("  1. Chua thu tien");
        uiDrawMenuRow("  2. Da thu tien");
        uiDrawMenuRow(COLOR_DIM "  0. Quay lai" COLOR_RESET);
        uiDrawMenuRow(COLOR_DIM " -1. Ve menu" COLOR_RESET);
        uiDrawSeparator();
        subChoice =
            readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, -1, 2);
      }

      if (subChoice == -1) {
        return RC_ERR_CANCELLED;
      }
      if (subChoice == 0) {
        continue;
      }
      filterValue = subChoice - 1;
      break;
    }

    /* Collect matching violation indices */
    int matchIdx[MAX_VIOLATIONS];
    int found = 0;
    for (int i = 0; i < db->violationCount; i++) {
      const Violation *v = &db->violations[i];
      int match = 0;
      switch (filterType) {
      case 4:
        match = 1;
        break;
      case 1:
        match = violationMatchesTeam(db, v, filterValue);
        break;
      case 2:
        match = violationMatchesReason(v, filterValue);
        break;
      case 3:
        match = violationMatchesPayment(v, filterValue);
        break;
      default:
        break;
      }
      if (match) {
        matchIdx[found++] = i;
      }
    }

    if (found == 0) {
      printf("  Khong co vi pham nao\n");
      printf("\n  Nhan Enter de tiep tuc...");
      while (getchar() != '\n' && getchar() != EOF) {
        ;
      }
      continue;
    }

    int totalPages = (found + ROWS_PER_PAGE - 1) / ROWS_PER_PAGE;
    int currentPage = 0;
    int exitPagination = 0;

    while (1) {
      uiClear();
      printViolationTableHeader();

      int start = currentPage * ROWS_PER_PAGE;
      int end = start + ROWS_PER_PAGE;
      if (end > found) {
        end = found;
      }

      for (int i = start; i < end; i++) {
        const Violation *v = &db->violations[matchIdx[i]];
        printViolationRow(findMemberForViolation(db, v), v);
      }

      uiTableEnd(VIOLATION_COLS, VIOLATION_COL_COUNT);

      printf("  Trang " COLOR_BOLD "%d/%d" COLOR_RESET " — Tong: " COLOR_BOLD
             "%d" COLOR_RESET " vi pham\n",
             currentPage + 1, totalPages, found);

      if (totalPages > 1) {
        printf(COLOR_DIM
               "  n: trang tiep | p: trang truoc | q: thoat" COLOR_RESET " > ");
        char buf[10];
        readString(buf, sizeof(buf));
        char c = buf[0];
        if (c == 'q' || c == 'Q') {
          exitPagination = 1;
          break;
        }
        if ((c == 'n' || c == 'N') && currentPage < totalPages - 1) {
          currentPage++;
        } else if ((c == 'p' || c == 'P') && currentPage > 0) {
          currentPage--;
        }
      } else {
        printf("\n  Nhan Enter de tiep tuc...");
        while (getchar() != '\n' && getchar() != EOF) {
          ;
        }
        exitPagination = 1;
        break;
      }
    }

    if (exitPagination) {
      continue;
    }
  }
  return RC_OK;
}

/* ============================================================
 * Story 3.1 - Record Violation
 * ============================================================ */

int violationRecord(AppDatabase *db) {
  if (db == NULL) {
    return RC_ERR_NULL;
  }

  Account *session = authGetSession();
  REQUIRE_DIRECTOR(session);

  if (db->violationCount >= MAX_VIOLATIONS) {
    printf(ERR_LOI "Da dat gioi han so luong vi pham (%d)!\n", MAX_VIOLATIONS);
    return RC_ERR_FULL;
  }

  uiClear();
  uiDrawBreadcrumb("[2] Quan ly vi pham -> [1] Ghi nhan vi pham moi");

  /* Search member by MSSV or name with re-prompt */
  char input[MAX_NAME_LEN];
  int memberIdx = -1;
  while (1) {
    printf(COLOR_CYAN
           "  Nhap MSSV hoac ten thanh vien (0 de quay lai): " COLOR_RESET);
    readString(input, sizeof(input));
    trimSpaces(input);
    if (strcmp(input, "0") == 0) {
      printf(ERR_INFO "Da huy thao tac.\n");
      return RC_ERR_CANCELLED;
    }
    if (!validateNotEmpty(input)) {
      continue;
    }
    if (!sanitizeInput(input)) {
      continue;
    }

    /* Try exact MSSV match first */
    memberIdx = memberFindById(db, input);

    if (memberIdx == -1) {
      /* Search by name (fuzzy) */
      int indices[MAX_MEMBERS];
      int count = memberSearchByName(db, input, indices, MAX_MEMBERS);

      if (count == 0) {
        printf(ERR_INFO "Khong tim thay thanh vien nao! Vui long thu lai.\n");
        continue;
      }

      if (count == 1) {
        memberIdx = indices[0];
      } else {
        printf("\n" COLOR_BOLD "  Tim thay %d thanh vien:\n" COLOR_RESET,
               count);
        for (int i = 0; i < count; i++) {
          Member *m = &db->members[indices[i]];
          printf("  %d. %s - %s (%s)\n", i + 1, m->studentId, m->fullName,
                 teamName(m->team));
        }
        int choice =
            readMenuChoice(COLOR_CYAN "  Chon STT: " COLOR_RESET, 1, count);
        memberIdx = indices[choice - 1];
      }
    }
    if (memberIdx != -1) {
      break;
    }
  }

  Member *member = &db->members[memberIdx];
  Member oldMemberState = *member;
  int oldViolationCount = db->violationCount;

  /* Check member is active */
  if (member->isActive != STATUS_ACTIVE) {
    printf(ERR_LOI "Thanh vien \"%s\" da thoat CLB! Khong the ghi vi pham.\n",
           member->fullName);
    return -1;
  }

  /* Show member info */
  printf("\n");
  printf(COLOR_BOLD "  Thanh vien: " COLOR_RESET "%s (%s)\n", member->fullName,
         member->studentId);
  printf(COLOR_BOLD "  Ban:        " COLOR_RESET "%s\n",
         teamName(member->team));
  printf(COLOR_BOLD "  Chuc vu:    " COLOR_RESET "%s\n",
         memberRoleName(member->role));

  /* Select reason */
  int reason;
  if (selectViolationReason(&reason) != 0) {
    return -1;
  }

  /* Create violation */
  Violation newViolation;
  memset(&newViolation, 0, sizeof(Violation));

  newViolation.id = db->nextViolationId++;
  strncpy(newViolation.studentId, member->studentId, MAX_MSSV_LEN - 1);
  newViolation.studentId[MAX_MSSV_LEN - 1] = '\0';
  newViolation.reason = reason;
  newViolation.violationTime = time(NULL);
  newViolation.isPaid = 0;
  newViolation.paidAt = 0;
  newViolation.isVoided = 0;
  newViolation.voidedAt = 0;

  switch (reason) {
  case REASON_VIOLENCE:
    handleViolence(db, member, &newViolation);
    break;
  case REASON_ABSENT:
    handleAbsent(db, member, &newViolation);
    break;
  case REASON_NO_JACKET:
  case REASON_NO_ACTIVITY:
  default:
    newViolation.fine = calculateFine(member->role);
    newViolation.penalty = PENALTY_FINE;
    member->consecutiveAbsences = 0;
    break;
  }

  /* Note with validation */
  while (1) {
    printf(COLOR_CYAN "  Ghi chu (Enter de bo qua): " COLOR_RESET);
    readString(newViolation.note, MAX_NOTE_LEN);
    trimSpaces(newViolation.note);
    if (strlen(newViolation.note) == 0) {
      break;
    }
    if (sanitizeInput(newViolation.note)) {
      break;
    }
  }

  /* PREVIEW before saving */
  printf("\n");
  uiDrawSeparator();
  printf(COLOR_BOLD "  XAC NHAN VI PHAM\n" COLOR_RESET);
  printf("  Thanh vien:  %s (%s)\n", member->fullName, member->studentId);
  printf("  Loi:         %s\n", reasonName(newViolation.reason));
  printf("  Ngay:        ");
  char timeBuf[20];
  formatTime(newViolation.violationTime, timeBuf, sizeof(timeBuf));
  printf("%s\n", timeBuf);
  if (newViolation.penalty == PENALTY_OUT_CLB) {
    printf("  Xu ly:      " COLOR_RED "OUT CLB" COLOR_RESET "\n");
  } else {
    printf("  Tien phat:  " COLOR_PURPLE "%.0f VND" COLOR_RESET "\n",
           newViolation.fine);
  }
  printf("  Tong VP chua dong: %d → %d\n", member->violationCount,
         member->violationCount + 1);
  if (newViolation.fine > 0) {
    printf("  Tong no:    %.0f → %.0f VND\n", member->totalFine,
           member->totalFine + newViolation.fine);
  }
  uiDrawSeparator();

  /* Confirm */
  while (1) {
    printf(COLOR_CYAN "  Xac nhan ghi vi pham? (Y/N): " COLOR_RESET);
    char confirm[4];
    readString(confirm, sizeof(confirm));
    if (confirm[0] == 'y' || confirm[0] == 'Y') {
      break;
    }
    if (confirm[0] == 'n' || confirm[0] == 'N') {
      *member = oldMemberState;
      printf(ERR_INFO "Da huy ghi nhan vi pham.\n");
      return RC_ERR_CANCELLED;
    }
    printf(ERR_LOI "Vui long nhap Y (Co) hoac N (Khong)!\n");
  }

  /* Save */
  db->violations[db->violationCount++] = newViolation;
  member->violationCount++;
  member->totalFine += newViolation.fine;

  if (fileioSaveViolations(db) != 0) {
    printf(ERR_LOI "Khong the luu du lieu vi pham!\n");
    db->violationCount = oldViolationCount;
    *member = oldMemberState;
    return RC_ERR_IO;
  }

  if (fileioSaveMembers(db) != 0) {
    db->violationCount = oldViolationCount;
    *member = oldMemberState;
    (void)fileioSaveViolations(db);
    printf(ERR_LOI "Khong the luu du lieu thanh vien!\n");
    return RC_ERR_IO;
  }

  printf(ERR_OK "Ghi nhan vi pham thanh cong!\n");
  if (session != NULL) {
    logSystemAction(session->studentId, "Ghi nhan vi pham", member->studentId);
  }
  return RC_OK;
}

/* ============================================================
 * Story 3.2 - Out CLB Warning & Enforcement
 * ============================================================ */

int violationCheckOutThreshold(AppDatabase *db, Member *member) {
  if (db == NULL || member == NULL) {
    return -1;
  }

  if (member->consecutiveAbsences == 3) {
    printf("\n" ERR_CANH_BAO "Thanh vien %s da vang 3 buoi lien tiep!\n",
           member->fullName);
    printf("  Neu vang them 1 buoi nua se bi Out CLB.\n");
    return 0;
  }

  if (member->consecutiveAbsences >= 4) {
    printf("\n" ERR_CANH_BAO
           "Thanh vien %s da vang qua 3 buoi lien tiep -> Out "
           "CLB!\n",
           member->fullName);
    printf("  So buoi vang lien tiep: %d\n", member->consecutiveAbsences);

    if (confirmOutClb(member->fullName)) {
      member->isActive = STATUS_OUT_CLB;
      printf(ERR_OK "Thanh vien %s da bi Out CLB.\n", member->fullName);
      return 1;
    }
  }

  return 0;
}

int violationCheckAllOutClb(AppDatabase *db) {
  if (db == NULL) {
    return RC_ERR_NULL;
  }

  Account *session = authGetSession();
  REQUIRE_DIRECTOR(session);

  printf("\nKIEM TRA NGUONG OUT CLB\n");
  printf("+------------+----------------------+-----------+------------+\n");
  printf("| MSSV       | Ho va ten            | Vang LT   | Trang thai |\n");
  printf("+------------+----------------------+-----------+------------+\n");

  int found = 0;

  for (int i = 0; i < db->memberCount; i++) {
    Member *m = &db->members[i];

    if (m->consecutiveAbsences >= 2 || m->isActive == STATUS_OUT_CLB) {
      const char *status;
      if (m->isActive == STATUS_OUT_CLB) {
        status = "Out CLB";
      } else if (m->consecutiveAbsences >= 4) {
        status = "QUA NGUONG";
      } else if (m->consecutiveAbsences == 3) {
        status = "CANH BAO";
      } else {
        status = "Theo doi";
      }

      printf("| %-10.10s | %-20.20s | %-9d | %-10.10s |\n", m->studentId,
             m->fullName, m->consecutiveAbsences, status);
      found++;
    }
  }

  printf("+------------+----------------------+-----------+------------+\n");

  if (found == 0) {
    printf("[THONG BAO] Khong co thanh vien nao gan nguong Out CLB\n");
  } else {
    printf("Tong: %d thanh vien can chu y\n", found);
  }

  printf("\nChu thich:\n");
  printf("  Theo doi  : Vang 2 buoi lien tiep\n");
  printf("  CANH BAO  : Vang 3 buoi lien tiep (them 1 buoi -> Out)\n");
  printf("  QUA NGUONG: Vang qua 3 buoi, cho Ban chu nhiem xu ly\n");
  printf("  Out CLB   : Da bi Out CLB\n\n");
  return RC_OK;
}

/* ============================================================
 * Story 3.3 - Mark Paid & View Own Violations/Fines
 * ============================================================ */

void violationViewOwn(AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  Account *session = authGetSession();
  if (session == NULL) {
    printf(ERR_LOI "Chua dang nhap!\n");
    return;
  }

  uiClear();
  uiDrawBreadcrumb("MENU THANH VIEN -> Danh sach vi pham");
  printf(COLOR_CYAN "  " LINE_TL);
  for (int i = 0; i < 16; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 25; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 10; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 13; i++) {
    printf(LINE_H);
  }
  printf(LINE_TR "\n" COLOR_RESET);
  printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
  printf("%-16s", "Thoi gian");
  printf(COLOR_CYAN LINE_V COLOR_RESET);
  printf("%-25s", "Ly do");
  printf(COLOR_CYAN LINE_V COLOR_RESET);
  printf("%-10s", "Tien phat");
  printf(COLOR_CYAN LINE_V COLOR_RESET);
  printf("%-13s", "Trang thai");
  printf(COLOR_CYAN LINE_V COLOR_RESET "\n");
  printf(COLOR_CYAN "  " LINE_T_RIGHT);
  for (int i = 0; i < 16; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 25; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 10; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 13; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_LEFT "\n" COLOR_RESET);

  int found = 0;
  for (int i = 0; i < db->violationCount; i++) {
    Violation *v = &db->violations[i];
    if (strcmp(v->studentId, session->studentId) == 0) {
      char timeBuf[20];
      formatTime(v->violationTime, timeBuf, sizeof(timeBuf));
      printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
      printf("%-16s", timeBuf);
      printf(COLOR_CYAN LINE_V COLOR_RESET);
      printf("%-25s", reasonName(v->reason));
      printf(COLOR_CYAN LINE_V COLOR_RESET);
      printf("%-10.0f", v->fine);
      printf(COLOR_CYAN LINE_V COLOR_RESET);
      if (v->isVoided) {
        printf(COLOR_GRAY "%-13s" COLOR_RESET, "Da huy");
      } else if (v->penalty == PENALTY_OUT_CLB) {
        printf(COLOR_RED "%-13s" COLOR_RESET, "OUT CLB");
      } else if (v->isPaid) {
        printf(COLOR_GREEN "%-13s" COLOR_RESET, "Da thu");
      } else {
        printf(COLOR_RED "%-13s" COLOR_RESET, "Chua thu");
      }
      printf(COLOR_CYAN LINE_V COLOR_RESET "\n");
      found++;
    }
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
  for (int i = 0; i < 10; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 13; i++) {
    printf(LINE_H);
  }
  printf(LINE_BR "\n" COLOR_RESET);

  if (found == 0) {
    printf(ERR_INFO "Ban khong co vi pham nao.\n");
  } else {
    printf("  Tong cong: " COLOR_BOLD "%d" COLOR_RESET " vi pham.\n\n", found);
  }
}

void violationViewFines(AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  Account *session = authGetSession();
  if (session == NULL) {
    printf(ERR_LOI "Chua dang nhap!\n");
    return;
  }

  uiClear();
  uiDrawBreadcrumb("MENU THANH VIEN -> Cac khoan phat chua dong");
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
  printf("%-15s", "So tien (VND)");
  printf(COLOR_CYAN LINE_V COLOR_RESET "\n");
  printf(COLOR_CYAN "  " LINE_T_RIGHT);
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
  printf(LINE_T_LEFT "\n" COLOR_RESET);

  double total = 0.0;
  int found = 0;
  for (int i = 0; i < db->violationCount; i++) {
    Violation *v = &db->violations[i];
    if (strcmp(v->studentId, session->studentId) == 0 && v->isPaid == 0 &&
        v->fine > 0 && v->isVoided == 0) {
      char timeBuf[20];
      formatTime(v->violationTime, timeBuf, sizeof(timeBuf));
      printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
      printf("%-16s", timeBuf);
      printf(COLOR_CYAN LINE_V COLOR_RESET);
      printf("%-25s", reasonName(v->reason));
      printf(COLOR_CYAN LINE_V COLOR_RESET);
      printf(COLOR_RED "%-15.0f" COLOR_RESET, v->fine);
      printf(COLOR_CYAN LINE_V COLOR_RESET "\n");
      total += v->fine;
      found++;
    }
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

  if (found == 0) {
    printf(ERR_OK "Tuyet voi! Ban khong no khoan phat nao.\n\n");
  } else {
    printf("  Tong cong: " COLOR_BOLD "%d" COLOR_RESET
           " khoan phat chua dong.\n",
           found);
    printf("  TONG SO TIEN CAN DONG: " COLOR_BOLD COLOR_PURPLE
           "%.0f VND" COLOR_RESET "\n\n",
           total);
  }
}

void violationViewPaymentHistory(AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  Account *session = authGetSession();
  if (session == NULL) {
    printf(ERR_LOI "Chua dang nhap!\n");
    return;
  }

  uiClear();
  uiDrawBreadcrumb("MENU THANH VIEN -> Lich su nop tien phat");

  printf(COLOR_CYAN "  " LINE_TL);
  for (int i = 0; i < 16; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 20; i++) {
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
  printf("%-20s", "Ly do");
  printf(COLOR_CYAN LINE_V COLOR_RESET);
  printf("%-15s", "So tien (VND)");
  printf(COLOR_CYAN LINE_V COLOR_RESET "\n");

  printf(COLOR_CYAN "  " LINE_T_RIGHT);
  for (int i = 0; i < 16; i++) {
    printf(LINE_H);
  }
  printf(LINE_CROSS);
  for (int i = 0; i < 20; i++) {
    printf(LINE_H);
  }
  printf(LINE_CROSS);
  for (int i = 0; i < 15; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_LEFT "\n" COLOR_RESET);

  double totalPaid = 0.0;
  int found = 0;
  for (int i = 0; i < db->violationCount; i++) {
    Violation *v = &db->violations[i];
    if (strcmp(v->studentId, session->studentId) == 0 && v->isPaid == 1 &&
        v->fine > 0 && v->isVoided == 0) {
      char timeBuf[20];
      formatTime(v->violationTime, timeBuf, sizeof(timeBuf));
      printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
      printf("%-16s", timeBuf);
      printf(COLOR_CYAN LINE_V COLOR_RESET);
      printf("%-20s", reasonName(v->reason));
      printf(COLOR_CYAN LINE_V COLOR_RESET);
      printf(COLOR_GREEN "%-15.0f" COLOR_RESET, v->fine);
      printf(COLOR_CYAN LINE_V COLOR_RESET "\n");
      totalPaid += v->fine;
      found++;
    }
  }

  printf(COLOR_CYAN "  " LINE_BL);
  for (int i = 0; i < 16; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 20; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 15; i++) {
    printf(LINE_H);
  }
  printf(LINE_BR "\n" COLOR_RESET);

  if (found == 0) {
    printf(ERR_INFO "Ban chua nop khoan phat nao.\n");
  } else {
    printf("  Tong cong: " COLOR_BOLD "%d" COLOR_RESET " lan nop.\n", found);
    printf("  TONG SO TIEN DA NOP: " COLOR_BOLD COLOR_GREEN
           "%.0f VND" COLOR_RESET "\n\n",
           totalPaid);
  }
}

int violationMarkPaid(AppDatabase *db) {
  if (db == NULL) {
    return RC_ERR_NULL;
  }

  Account *session = authGetSession();
  REQUIRE_DIRECTOR(session);

  uiClear();
  uiDrawBreadcrumb("[2] Quan ly vi pham -> [2] Danh dau da thu tien phat ");

  /* Search member by MSSV or name with re-prompt */
  char input[MAX_NAME_LEN];
  int memberIdx = -1;
  while (1) {
    printf(COLOR_CYAN
           "  Nhap MSSV hoac ten thanh vien (0 de quay lai): " COLOR_RESET);
    readString(input, sizeof(input));
    trimSpaces(input);
    if (strcmp(input, "0") == 0) {
      printf(ERR_INFO "Da huy thao tac.\n");
      return RC_ERR_CANCELLED;
    }
    if (!validateNotEmpty(input)) {
      continue;
    }
    if (!sanitizeInput(input)) {
      continue;
    }

    /* Try exact MSSV match first */
    memberIdx = memberFindById(db, input);

    if (memberIdx == -1) {
      /* Search by name (fuzzy) */
      int indices[MAX_MEMBERS];
      int count = memberSearchByName(db, input, indices, MAX_MEMBERS);

      if (count == 0) {
        printf(ERR_INFO "Khong tim thay thanh vien nao! Vui long thu lai.\n");
        continue;
      }

      if (count == 1) {
        memberIdx = indices[0];
      } else {
        printf("\n" COLOR_BOLD "  Tim thay %d thanh vien:\n" COLOR_RESET,
               count);
        for (int i = 0; i < count; i++) {
          Member *m = &db->members[indices[i]];
          printf("  %d. %s - %s (%s)\n", i + 1, m->studentId, m->fullName,
                 teamName(m->team));
        }
        int choice =
            readMenuChoice(COLOR_CYAN "  Chon STT: " COLOR_RESET, 1, count);
        memberIdx = indices[choice - 1];
      }
    }
    if (memberIdx != -1) {
      break;
    }
  }

  Member *m = &db->members[memberIdx];
  int unpaidIds[MAX_VIOLATIONS];
  int unpaidCount = 0;

  printf("\n");
  printf(COLOR_BOLD "  Danh sach vi pham chua dong phat cua %s:\n" COLOR_RESET,
         m->fullName);
  printf(COLOR_CYAN "  " LINE_TL);
  for (int i = 0; i < 6; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 18; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 25; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_TR "\n" COLOR_RESET);
  printf(COLOR_CYAN "  " LINE_V COLOR_RESET
                    " STT  " COLOR_CYAN LINE_V COLOR_RESET
                    " Thoi gian        " COLOR_CYAN LINE_V COLOR_RESET
                    " Ly do                   " COLOR_CYAN LINE_V COLOR_RESET
                    " Tien phat (VND) " COLOR_CYAN LINE_V COLOR_RESET "\n");
  printf(COLOR_CYAN "  " LINE_T_RIGHT);
  for (int i = 0; i < 6; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 18; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 25; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_LEFT "\n" COLOR_RESET);

  for (int i = 0; i < db->violationCount; i++) {
    Violation *v = &db->violations[i];
    if (strcmp(v->studentId, m->studentId) == 0 && v->isPaid == 0 &&
        v->fine > 0 && v->isVoided == 0) {
      unpaidIds[unpaidCount] = v->id;
      char timeBuf[20];
      formatTime(v->violationTime, timeBuf, sizeof(timeBuf));
      printf(COLOR_CYAN "  " LINE_V COLOR_RESET " %-4d ", unpaidCount + 1);
      printf(COLOR_CYAN LINE_V COLOR_RESET " %-16s ", timeBuf);
      printf(COLOR_CYAN LINE_V COLOR_RESET " %-23s ", reasonName(v->reason));
      printf(COLOR_CYAN LINE_V COLOR_RESET " " COLOR_RED "%-15.0f" COLOR_RESET
                                           " " COLOR_CYAN LINE_V COLOR_RESET
                                           "\n",
             v->fine);
      unpaidCount++;
    }
  }

  printf(COLOR_CYAN "  " LINE_BL);
  for (int i = 0; i < 6; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 18; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 25; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_BR "\n" COLOR_RESET);

  if (unpaidCount == 0) {
    printf(ERR_OK "Thanh vien nay khong co khoan phat nao can thu.\n\n");
    return 0;
  }

  char choiceStr[128];
  int selectedIds[MAX_VIOLATIONS];
  int selectedCount = 0;

  while (1) {
    printf(COLOR_CYAN "  Chon STT de thu (vd: 1,3 hoac 99 de thu tat ca, 0 de "
                      "huy): " COLOR_RESET);
    readString(choiceStr, sizeof(choiceStr));
    trimSpaces(choiceStr);

    if (strcmp(choiceStr, "0") == 0) {
      printf(ERR_INFO "Da huy thao tac.\n\n");
      return 0;
    }

    if (strcmp(choiceStr, "99") == 0) {
      selectedCount = unpaidCount;
      for (int i = 0; i < unpaidCount; i++) {
        selectedIds[i] = unpaidIds[i];
      }
      break;
    }

    /* Parse comma-separated list of STTs */
    int valid = 1;
    selectedCount = 0;
    char tempStr[128];
    strncpy(tempStr, choiceStr, sizeof(tempStr));
    tempStr[sizeof(tempStr) - 1] = '\0';

    char *token = strtok(tempStr, ",");
    while (token != NULL) {
      /* Trim spaces from token */
      while (*token == ' ') {
        token++;
      }
      int len = (int)strlen(token);
      while (len > 0 && token[len - 1] == ' ') {
        token[len - 1] = '\0';
        len--;
      }

      if (len == 0) {
        token = strtok(NULL, ",");
        continue;
      }

      /* Check if token is a valid number */
      int isNum = 1;
      for (int i = 0; i < len; i++) {
        if (!isdigit((unsigned char)token[i])) {
          isNum = 0;
          break;
        }
      }

      if (!isNum) {
        printf(ERR_LOI "Vi tri \"%s\" khong phai la so hop le!\n", token);
        valid = 0;
        break;
      }

      int val = atoi(token);
      if (val < 1 || val > unpaidCount) {
        printf(ERR_LOI "STT %d nam ngoai pham vi (1-%d)!\n", val, unpaidCount);
        valid = 0;
        break;
      }

      /* Check for duplicates in selectedIds */
      int dup = 0;
      int realIdVal = unpaidIds[val - 1];
      for (int i = 0; i < selectedCount; i++) {
        if (selectedIds[i] == realIdVal) {
          dup = 1;
          break;
        }
      }

      if (!dup) {
        if (selectedCount < MAX_VIOLATIONS) {
          selectedIds[selectedCount++] = realIdVal;
        }
      }

      token = strtok(NULL, ",");
    }

    if (valid && selectedCount > 0) {
      break;
    }
    if (valid && selectedCount == 0) {
      printf(ERR_LOI "Vui long chon it nhat mot STT hoac nhap 0 de huy!\n");
    }
  }

  /* Save old states for restoration in case of save failure */
  double oldTotalFine = m->totalFine;
  int oldConsecutiveAbsences = m->consecutiveAbsences;
  int oldPaidStates[MAX_VIOLATIONS];
  time_t oldPaidAt[MAX_VIOLATIONS];
  for (int i = 0; i < db->violationCount; i++) {
    oldPaidStates[i] = db->violations[i].isPaid;
    oldPaidAt[i] = db->violations[i].paidAt;
  }

  /* Mark selected violations as paid */
  time_t paidTime = time(NULL);
  for (int i = 0; i < selectedCount; i++) {
    int targetId = selectedIds[i];
    for (int j = 0; j < db->violationCount; j++) {
      if (db->violations[j].id == targetId) {
        db->violations[j].isPaid = 1;
        db->violations[j].paidAt = paidTime;
        break;
      }
    }
  }

  /* Recalculate total fine */
  double newTotal = 0.0;
  for (int i = 0; i < db->violationCount; i++) {
    Violation *v = &db->violations[i];
    if (strcmp(v->studentId, m->studentId) == 0 && v->isPaid == 0 &&
        v->isVoided == 0) {
      newTotal += v->fine;
    }
  }
  m->totalFine = newTotal;

  /* Reset consecutive absences if all absent violations are paid/voided */
  int hasUnpaidAbsent = 0;
  for (int i = 0; i < db->violationCount; i++) {
    Violation *v = &db->violations[i];
    if (strcmp(v->studentId, m->studentId) == 0 && v->reason == REASON_ABSENT &&
        v->isPaid == 0 && v->isVoided == 0) {
      hasUnpaidAbsent = 1;
      break;
    }
  }
  if (!hasUnpaidAbsent) {
    m->consecutiveAbsences = 0;
  }

  if (fileioSaveViolations(db) != 0) {
    /* Restore old states */
    for (int i = 0; i < db->violationCount; i++) {
      db->violations[i].isPaid = oldPaidStates[i];
      db->violations[i].paidAt = oldPaidAt[i];
    }
    m->totalFine = oldTotalFine;
    m->consecutiveAbsences = oldConsecutiveAbsences;
    printf(ERR_LOI "Khong the luu du lieu sau khi thu tien!\n");
    return RC_ERR_IO;
  }

  if (fileioSaveMembers(db) != 0) {
    /* Restore old states */
    for (int i = 0; i < db->violationCount; i++) {
      db->violations[i].isPaid = oldPaidStates[i];
      db->violations[i].paidAt = oldPaidAt[i];
    }
    m->totalFine = oldTotalFine;
    m->consecutiveAbsences = oldConsecutiveAbsences;
    (void)fileioSaveViolations(db);
    printf(ERR_LOI "Khong the luu du lieu sau khi thu tien!\n");
    return RC_ERR_IO;
  }

  if (selectedCount == unpaidCount) {
    printf(ERR_OK "Da thu TOAN BO tien phat thanh cong!\n\n");
    logSystemAction(session->studentId, "Thu toan bo tien phat", m->studentId);
  } else {
    printf(ERR_OK "Da thu tien phat cua cac STT da chon thanh cong! Tong no "
                  "con lai: " COLOR_BOLD COLOR_GREEN "%.0f VND" COLOR_RESET
                  "\n\n",
           m->totalFine);
    logSystemAction(session->studentId, "Thu mot phan tien phat", m->studentId);
  }
  return RC_OK;
}

/* ============================================================
 * Story 17 - View Violation History by Member (BCN)
 * ============================================================ */

int violationViewByMSSV(AppDatabase *db) {
  if (db == NULL) {
    return RC_ERR_NULL;
  }

  Account *session = authGetSession();
  REQUIRE_DIRECTOR(session);

  char input[MAX_NAME_LEN];
  int memberIdx = -1;
  while (1) {
    uiClear();
    uiDrawBreadcrumb(
        "[2] Quan ly vi pham -> [4] Xem lich su vi pham theo thanh vien");

    printf(COLOR_CYAN
           "  Nhap MSSV hoac ten thanh vien (0 de quay lai): " COLOR_RESET);
    readString(input, sizeof(input));
    trimSpaces(input);
    if (strcmp(input, "0") == 0) {
      printf(ERR_INFO "Da huy thao tac.\n");
      return RC_ERR_CANCELLED;
    }
    if (!validateNotEmpty(input)) {
      continue;
    }

    memberIdx = memberFindById(db, input);
    if (memberIdx == -1) {
      int indices[MAX_MEMBERS];
      int count = memberSearchByName(db, input, indices, MAX_MEMBERS);
      if (count == 0) {
        printf(ERR_INFO "Khong tim thay thanh vien! Vui long thu lai.\n");
        uiPause();
        continue;
      }
      if (count == 1) {
        memberIdx = indices[0];
      } else {
        printf("\n" COLOR_BOLD "  Tim thay %d thanh vien:\n" COLOR_RESET,
               count);
        for (int i = 0; i < count; i++) {
          Member *m = &db->members[indices[i]];
          printf("  %d. %s - %s (%s)\n", i + 1, m->studentId, m->fullName,
                 teamName(m->team));
        }
        int choice =
            readMenuChoice(COLOR_CYAN "  Chon STT: " COLOR_RESET, 1, count);
        memberIdx = indices[choice - 1];
      }
    }
    if (memberIdx != -1) {
      break;
    }
  }

  Member *m = &db->members[memberIdx];
  int matchIdx[MAX_VIOLATIONS];
  int found = 0;
  for (int i = 0; i < db->violationCount; i++) {
    if (strcmp(db->violations[i].studentId, m->studentId) == 0) {
      matchIdx[found++] = i;
    }
  }

  if (found == 0) {
    printf(ERR_INFO "Thanh vien \"%s\" (%s) khong co lich su vi pham.\n\n",
           m->fullName, m->studentId);
    return RC_OK;
  }

  int totalPages = (found + ROWS_PER_PAGE - 1) / ROWS_PER_PAGE;
  int currentPage = 0;

  while (1) {
    uiClear();
    printf(COLOR_BOLD "  LICH SU VI PHAM CUA: " COLOR_RESET "%s (%s) - %s\n\n",
           m->fullName, m->studentId, teamName(m->team));

    printViolationTableHeader();

    int start = currentPage * ROWS_PER_PAGE;
    int end = start + ROWS_PER_PAGE;
    if (end > found) {
      end = found;
    }

    for (int i = start; i < end; i++) {
      printViolationRow(m, &db->violations[matchIdx[i]]);
    }

    printf(COLOR_CYAN "  " LINE_BL);
    for (int i = 0; i < 12; i++) {
      printf(LINE_H);
    }
    printf(LINE_T_UP);
    for (int i = 0; i < 22; i++) {
      printf(LINE_H);
    }
    printf(LINE_T_UP);
    for (int i = 0; i < 14; i++) {
      printf(LINE_H);
    }
    printf(LINE_T_UP);
    for (int i = 0; i < 22; i++) {
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
    for (int i = 0; i < 15; i++) {
      printf(LINE_H);
    }
    printf(LINE_T_UP);
    for (int i = 0; i < 12; i++) {
      printf(LINE_H);
    }
    printf(LINE_BR "\n" COLOR_RESET);

    printf("  Trang " COLOR_BOLD "%d/%d" COLOR_RESET " — Tong: " COLOR_BOLD
           "%d" COLOR_RESET " vi pham\n",
           currentPage + 1, totalPages, found);

    if (totalPages > 1) {
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
    } else {
      break;
    }
  }
  return RC_OK;
}

/* ============================================================
 * Story 4.4 - Search Violations by Date Range
 * ============================================================ */

int violationSearchByDate(AppDatabase *db) {
  if (db == NULL) {
    return RC_ERR_NULL;
  }

  Account *session = authGetSession();
  REQUIRE_DIRECTOR(session);

  if (db->violationCount == 0) {
    printf(ERR_INFO "Khong co vi pham nao trong du lieu.\n");
    return RC_OK;
  }

  char startBuf[16];
  char endBuf[16];
  time_t start;
  time_t end;

  uiClear();
  uiDrawBreadcrumb("[2] Quan ly vi pham -> [5] Tim kiem vi pham theo ngay");

date_input:
  while (1) {
    while (1) {
      printf(COLOR_CYAN
             "  Nhap ngay bat dau (dd/mm/yyyy, 0 de quay lai): " COLOR_RESET);
      readString(startBuf, sizeof(startBuf));
      trimSpaces(startBuf);
      if (strcmp(startBuf, "0") == 0) {
        printf(ERR_INFO "Da huy thao tac.\n");
        return RC_ERR_CANCELLED;
      }
      if (validateDate(startBuf)) {
        parseDate(startBuf, &start, 0);
        break;
      }
    }

    while (1) {
      printf(COLOR_CYAN
             "  Nhap ngay ket thuc (dd/mm/yyyy, 0 de quay lai): " COLOR_RESET);
      readString(endBuf, sizeof(endBuf));
      trimSpaces(endBuf);
      if (strcmp(endBuf, "0") == 0) {
        printf(ERR_INFO "Da huy thao tac.\n");
        return RC_ERR_CANCELLED;
      }
      if (validateDate(endBuf)) {
        parseDate(endBuf, &end, 1);
        break;
      }
    }

    if (start <= end) {
      break;
    }
    printf(ERR_LOI "Ngay bat dau phai truoc hoac bang ngay ket thuc! Vui long "
                   "nhap lai.\n");
  }

  /* Collect matching violation indices */
  int matchIdx[MAX_VIOLATIONS];
  int found = 0;
  for (int i = 0; i < db->violationCount; i++) {
    const Violation *v = &db->violations[i];
    if (v->violationTime >= start && v->violationTime <= end) {
      matchIdx[found++] = i;
    }
  }

  if (found == 0) {
    printf("  Khong co vi pham nao trong khoang ngay nay\n");
    return RC_OK;
  }

  int totalPages = (found + ROWS_PER_PAGE - 1) / ROWS_PER_PAGE;
  int currentPage = 0;

  while (1) {
    uiClear();
    printViolationTableHeader();

    int startIdx = currentPage * ROWS_PER_PAGE;
    int endIdx = startIdx + ROWS_PER_PAGE;
    if (endIdx > found) {
      endIdx = found;
    }

    for (int i = startIdx; i < endIdx; i++) {
      const Violation *v = &db->violations[matchIdx[i]];
      printViolationRow(findMemberForViolation(db, v), v);
    }

    uiTableEnd(VIOLATION_COLS, VIOLATION_COL_COUNT);

    printf("  Trang " COLOR_BOLD "%d/%d" COLOR_RESET " — Tong: " COLOR_BOLD
           "%d" COLOR_RESET " vi pham\n",
           currentPage + 1, totalPages, found);

    if (totalPages > 1) {
      printf(COLOR_DIM "  n: trang tiep | p: trang truoc | q: thoat" COLOR_RESET
                       " > ");
      char buf[10];
      readString(buf, sizeof(buf));
      char c = buf[0];
      if (c == 'q' || c == 'Q') {
        goto date_input;
      }
      if ((c == 'n' || c == 'N') && currentPage < totalPages - 1) {
        currentPage++;
      } else if ((c == 'p' || c == 'P') && currentPage > 0) {
        currentPage--;
      }
    } else {
      printf("\n  Nhan Enter de tiep tuc...");
      while (getchar() != '\n' && getchar() != EOF) {
        ;
      }
      goto date_input;
    }
  }
}

int violationVoid(AppDatabase *db) {
  if (db == NULL) {
    return RC_ERR_NULL;
  }

  Account *session = authGetSession();
  REQUIRE_DIRECTOR(session);

  uiClear();
  uiDrawBreadcrumb("[2] Quan ly vi pham -> Huy vi pham");

  /* Search member by MSSV or name with re-prompt */
  char input[MAX_NAME_LEN];
  int memberIdx = -1;
  while (1) {
    printf(COLOR_CYAN
           "  Nhap MSSV hoac ten thanh vien (0 de quay lai): " COLOR_RESET);
    readString(input, sizeof(input));
    trimSpaces(input);
    if (strcmp(input, "0") == 0) {
      printf(ERR_INFO "Da huy thao tac.\n");
      return RC_ERR_CANCELLED;
    }
    if (!validateNotEmpty(input)) {
      continue;
    }
    if (!sanitizeInput(input)) {
      continue;
    }

    memberIdx = memberFindById(db, input);
    if (memberIdx == -1) {
      int indices[MAX_MEMBERS];
      int count = memberSearchByName(db, input, indices, MAX_MEMBERS);
      if (count == 0) {
        printf(ERR_INFO "Khong tim thay thanh vien! Vui long thu lai.\n");
        continue;
      }
      if (count == 1) {
        memberIdx = indices[0];
      } else {
        printf("\n" COLOR_BOLD "  Tim thay %d thanh vien:\n" COLOR_RESET,
               count);
        for (int i = 0; i < count; i++) {
          Member *m = &db->members[indices[i]];
          printf("  %d. %s - %s (%s)\n", i + 1, m->studentId, m->fullName,
                 teamName(m->team));
        }
        int choice =
            readMenuChoice(COLOR_CYAN "  Chon STT: " COLOR_RESET, 1, count);
        memberIdx = indices[choice - 1];
      }
    }
    if (memberIdx != -1) {
      break;
    }
  }

  Member *m = &db->members[memberIdx];
  int activeViolIndices[MAX_VIOLATIONS];
  int activeViolCount = 0;

  printf("\n");
  printf(COLOR_BOLD "  Danh sach vi pham cua %s:\n" COLOR_RESET, m->fullName);
  printf(COLOR_CYAN "  " LINE_TL);
  for (int i = 0; i < 6; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 18; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 25; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 12; i++) {
    printf(LINE_H);
  }
  printf(LINE_TR "\n" COLOR_RESET);

  printf(COLOR_CYAN "  " LINE_V COLOR_RESET
                    " STT  " COLOR_CYAN LINE_V COLOR_RESET
                    " Thoi gian        " COLOR_CYAN LINE_V COLOR_RESET
                    " Ly do                   " COLOR_CYAN LINE_V COLOR_RESET
                    " Tien phat (VND) " COLOR_CYAN LINE_V COLOR_RESET
                    " Trang thai " COLOR_CYAN LINE_V COLOR_RESET "\n");

  printf(COLOR_CYAN "  " LINE_T_RIGHT);
  for (int i = 0; i < 6; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 18; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 25; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 12; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_LEFT "\n" COLOR_RESET);

  for (int i = 0; i < db->violationCount; i++) {
    Violation *v = &db->violations[i];
    if (strcmp(v->studentId, m->studentId) == 0 && v->isVoided == 0) {
      activeViolIndices[activeViolCount] = i;
      char timeBuf[20];
      formatTime(v->violationTime, timeBuf, sizeof(timeBuf));
      printf(COLOR_CYAN "  " LINE_V COLOR_RESET " %-4d ", activeViolCount + 1);
      printf(COLOR_CYAN LINE_V COLOR_RESET " %-16s ", timeBuf);
      printf(COLOR_CYAN LINE_V COLOR_RESET " %-23s ", reasonName(v->reason));
      printf(COLOR_CYAN LINE_V COLOR_RESET " %-15.0f ", v->fine);

      if (v->penalty == PENALTY_OUT_CLB) {
        printf(COLOR_CYAN LINE_V COLOR_RESET
               " " COLOR_RED "OUT CLB    " COLOR_RESET
               " " COLOR_CYAN LINE_V COLOR_RESET "\n");
      } else if (v->isPaid) {
        printf(COLOR_CYAN LINE_V COLOR_RESET
               " " COLOR_GREEN "Da thu     " COLOR_RESET
               " " COLOR_CYAN LINE_V COLOR_RESET "\n");
      } else {
        printf(COLOR_CYAN LINE_V COLOR_RESET
               " " COLOR_RED "Chua thu   " COLOR_RESET
               " " COLOR_CYAN LINE_V COLOR_RESET "\n");
      }
      activeViolCount++;
    }
  }

  printf(COLOR_CYAN "  " LINE_BL);
  for (int i = 0; i < 6; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 18; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 25; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 12; i++) {
    printf(LINE_H);
  }
  printf(LINE_BR "\n" COLOR_RESET);

  if (activeViolCount == 0) {
    printf(ERR_OK "Thanh vien nay khong co vi pham nao de huy.\n\n");
    return RC_OK;
  }

  int choice = readMenuChoice(
      COLOR_CYAN "  Chon STT vi pham can huy (0 de quay lai): " COLOR_RESET, 0,
      activeViolCount);
  if (choice == 0) {
    printf(ERR_INFO "Da huy thao tac.\n\n");
    return RC_ERR_CANCELLED;
  }

  int selectedIdx = activeViolIndices[choice - 1];
  Violation *target = &db->violations[selectedIdx];

  char reason[MAX_NOTE_LEN];
  while (1) {
    printf(COLOR_CYAN
           "  Nhap ly do huy vi pham (bat buoc, 0 de huy): " COLOR_RESET);
    readString(reason, sizeof(reason));
    trimSpaces(reason);
    if (strcmp(reason, "0") == 0) {
      printf(ERR_INFO "Da huy thao tac.\n\n");
      return RC_ERR_CANCELLED;
    }
    if (strlen(reason) == 0) {
      printf(ERR_LOI "Ly do huy khong duoc de trong!\n");
      continue;
    }
    if (sanitizeInput(reason)) {
      break;
    }
  }

  /* Confirm voiding */
  printf("\n");
  uiDrawSeparator();
  printf(COLOR_BOLD "  XAC NHAN HUY VI PHAM\n" COLOR_RESET);
  printf("  Thanh vien:  %s (%s)\n", m->fullName, m->studentId);
  printf("  ID vi pham:  #%d\n", target->id);
  printf("  Loi vi pham: %s\n", reasonName(target->reason));
  printf("  Ly do huy:   %s\n", reason);
  uiDrawSeparator();

  while (1) {
    printf(COLOR_CYAN
           "  Ban co chac chan muon huy vi pham nay? (Y/N): " COLOR_RESET);
    char confirm[4];
    readString(confirm, sizeof(confirm));
    if (confirm[0] == 'y' || confirm[0] == 'Y') {
      break;
    }
    if (confirm[0] == 'n' || confirm[0] == 'N') {
      printf(ERR_INFO "Da huy thao tac.\n");
      return RC_ERR_CANCELLED;
    }
    printf(ERR_LOI "Vui long nhap Y (Co) hoac N (Khong)!\n");
  }

  /* Backup states for restoration on failure */
  int oldIsVoided = target->isVoided;
  time_t oldVoidedAt = target->voidedAt;
  char oldVoidReason[MAX_NOTE_LEN];
  strncpy(oldVoidReason, target->voidReason, sizeof(oldVoidReason) - 1);
  oldVoidReason[sizeof(oldVoidReason) - 1] = '\0';

  double oldTotalFine = m->totalFine;
  int oldViolationCount = m->violationCount;
  int oldConsecutiveAbsences = m->consecutiveAbsences;

  /* Apply void */
  target->isVoided = 1;
  target->voidedAt = time(NULL);
  strncpy(target->voidReason, reason, MAX_NOTE_LEN - 1);
  target->voidReason[MAX_NOTE_LEN - 1] = '\0';

  /* Recalculate member states */
  double newTotal = 0.0;
  for (int i = 0; i < db->violationCount; i++) {
    Violation *v = &db->violations[i];
    if (strcmp(v->studentId, m->studentId) == 0 && v->isPaid == 0 &&
        v->isVoided == 0) {
      newTotal += v->fine;
    }
  }
  m->totalFine = newTotal;

  int newViolCount = 0;
  for (int i = 0; i < db->violationCount; i++) {
    Violation *v = &db->violations[i];
    if (strcmp(v->studentId, m->studentId) == 0 && v->isVoided == 0) {
      newViolCount++;
    }
  }
  m->violationCount = newViolCount;

  /* Recalculate consecutive absences if target was an absent violation */
  if (target->reason == REASON_ABSENT) {
    int hasUnpaidAbsent = 0;
    for (int i = 0; i < db->violationCount; i++) {
      Violation *v = &db->violations[i];
      if (strcmp(v->studentId, m->studentId) == 0 &&
          v->reason == REASON_ABSENT && v->isPaid == 0 && v->isVoided == 0) {
        hasUnpaidAbsent = 1;
        break;
      }
    }
    if (!hasUnpaidAbsent) {
      m->consecutiveAbsences = 0;
    }
  }

  /* Save to files */
  if (fileioSaveViolations(db) != 0) {
    /* Restore */
    target->isVoided = oldIsVoided;
    target->voidedAt = oldVoidedAt;
    strncpy(target->voidReason, oldVoidReason, sizeof(target->voidReason) - 1);
    target->voidReason[sizeof(target->voidReason) - 1] = '\0';
    m->totalFine = oldTotalFine;
    m->violationCount = oldViolationCount;
    m->consecutiveAbsences = oldConsecutiveAbsences;
    printf(ERR_LOI "Khong the luu du lieu vi pham!\n");
    return RC_ERR_IO;
  }

  if (fileioSaveMembers(db) != 0) {
    /* Restore */
    target->isVoided = oldIsVoided;
    target->voidedAt = oldVoidedAt;
    strncpy(target->voidReason, oldVoidReason, sizeof(target->voidReason) - 1);
    target->voidReason[sizeof(target->voidReason) - 1] = '\0';
    m->totalFine = oldTotalFine;
    m->violationCount = oldViolationCount;
    m->consecutiveAbsences = oldConsecutiveAbsences;
    (void)fileioSaveViolations(db);
    printf(ERR_LOI "Khong the luu du lieu thanh vien!\n");
    return RC_ERR_IO;
  }

  printf(ERR_OK "Da huy vi pham #%d thanh cong!\n\n", target->id);
  char logMsg[512];
  snprintf(logMsg, sizeof(logMsg), "Huy vi pham #%d, ly do: %s", target->id,
           reason);
  logSystemAction(session->studentId, logMsg, m->studentId);

  return RC_OK;
}

int violationImportCsv(AppDatabase *db) {
  if (db == NULL) {
    return RC_ERR_NULL;
  }

  Account *session = authGetSession();
  REQUIRE_DIRECTOR(session);

  uiClear();
  uiDrawBreadcrumb("[2] Quan ly vi pham -> Import vi pham tu file CSV");

  char filepath[512];
  FILE *fp = NULL;
  while (1) {
    printf(COLOR_CYAN "  Nhap duong dan file CSV (vd: template.csv, 0 de quay lai): " COLOR_RESET);
    readString(filepath, sizeof(filepath));
    trimSpaces(filepath);
    
    if (strcmp(filepath, "0") == 0 || strlen(filepath) == 0) {
      printf(ERR_INFO "Da huy thao tac.\n");
      return RC_ERR_CANCELLED;
    }

    /* Check for malicious characters in path to prevent OS command injection or directory issues */
    int is_valid = 1;
    for (int i = 0; filepath[i] != '\0'; i++) {
      char c = filepath[i];
      if (c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
        is_valid = 0;
        break;
      }
    }
    if (!is_valid) {
      printf(ERR_LOI "Duong dan file chua ky tu khong hop le (*, ?, \", <, >, |)!\n");
      continue;
    }

    fp = fopen(filepath, "r");
    if (fp == NULL) {
      printf(ERR_LOI "Khong tim thay hoac khong the mo file: %s!\n", filepath);
      continue;
    }
    break;
  }

  /* Dry-run validation records */
  typedef struct {
    char studentId[MAX_MSSV_LEN];
    int reasonCode;
    char notes[MAX_NOTE_LEN];
    int isValid;
    char errorMsg[128];
    char memberName[MAX_NAME_LEN];
  } CsvRecord;

  CsvRecord *records = malloc(sizeof(CsvRecord) * 1000);
  if (records == NULL) {
    printf(ERR_LOI "Loi cap phat bo nho!\n");
    fclose(fp);
    return RC_ERR_NULL;
  }

  char line[1024];
  int count = 0;
  int validCount = 0;
  int invalidCount = 0;

  /* Check and skip header row if present */
  if (fgets(line, sizeof(line), fp) != NULL) {
    if (strstr(line, "studentId") == NULL && strstr(line, "mssv") == NULL) {
      fseek(fp, 0, SEEK_SET);
    }
  }

  while (fgets(line, sizeof(line), fp) != NULL && count < 1000) {
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
      line[len - 1] = '\0';
      len--;
    }

    if (len == 0) {
      continue;
    }

    CsvRecord *rec = &records[count];
    memset(rec, 0, sizeof(CsvRecord));
    rec->isValid = 1;

    char lineCopy[1024];
    strncpy(lineCopy, line, sizeof(lineCopy) - 1);
    lineCopy[sizeof(lineCopy) - 1] = '\0';

    char *token = lineCopy;
    char *comma1 = strchr(token, ',');
    if (comma1 == NULL) {
      rec->isValid = 0;
      strncpy(rec->errorMsg, "Thieu thong tin loi vi pham", sizeof(rec->errorMsg) - 1);
    } else {
      *comma1 = '\0';
      strncpy(rec->studentId, token, MAX_MSSV_LEN - 1);
      trimSpaces(rec->studentId);
      mssvAutoUpper(rec->studentId);

      char *reasonPart = comma1 + 1;
      char *comma2 = strchr(reasonPart, ',');
      if (comma2 != NULL) {
        *comma2 = '\0';
        strncpy(rec->notes, comma2 + 1, MAX_NOTE_LEN - 1);
        trimSpaces(rec->notes);
      }
      trimSpaces(reasonPart);

      int reasonVal = -1;
      if (sscanf(reasonPart, "%d", &reasonVal) != 1) {
        rec->isValid = 0;
        strncpy(rec->errorMsg, "Ma loi khong phai la so", sizeof(rec->errorMsg) - 1);
      } else if (reasonVal < 0 || reasonVal > 3) {
        rec->isValid = 0;
        strncpy(rec->errorMsg, "Ma loi ngoai pham vi [0-3]", sizeof(rec->errorMsg) - 1);
      } else {
        rec->reasonCode = reasonVal;
      }
    }

    if (rec->isValid) {
      int memberIdx = memberFindById(db, rec->studentId);
      if (memberIdx == -1) {
        rec->isValid = 0;
        strncpy(rec->errorMsg, "MSSV khong ton tai", sizeof(rec->errorMsg) - 1);
      } else {
        Member *m = &db->members[memberIdx];
        if (m->isDeleted) {
          rec->isValid = 0;
          strncpy(rec->errorMsg, "Thanh vien da bi xoa", sizeof(rec->errorMsg) - 1);
        } else if (m->isActive == STATUS_OUT_CLB) {
          rec->isValid = 0;
          strncpy(rec->errorMsg, "Thanh vien da ra khoi CLB", sizeof(rec->errorMsg) - 1);
        } else {
          strncpy(rec->memberName, m->fullName, MAX_NAME_LEN - 1);
        }
      }
    }

    if (rec->isValid) {
      validCount++;
    } else {
      invalidCount++;
    }
    count++;
  }
  fclose(fp);

  if (count == 0) {
    printf(ERR_INFO "File CSV khong co du lieu!\n");
    free(records);
    uiPause();
    return RC_OK;
  }

  int currentPage = 0;
  int pageSize = 10;
  int totalPages = (count + pageSize - 1) / pageSize;

  while (1) {
    uiClear();
    uiDrawBreadcrumb("[2] Quan ly vi pham -> Preview Import CSV");

    printf("\n" COLOR_BOLD "  XEM TRUOC DU LIEU IMPORT (PREVIEW)" COLOR_RESET "\n");
    printf(COLOR_CYAN "  " LINE_TL);
    for (int i = 0; i < 5; i++) printf(LINE_H);
    printf(LINE_T_DOWN);
    for (int i = 0; i < 12; i++) printf(LINE_H);
    printf(LINE_T_DOWN);
    for (int i = 0; i < 12; i++) printf(LINE_H);
    printf(LINE_T_DOWN);
    for (int i = 0; i < 22; i++) printf(LINE_H);
    printf(LINE_T_DOWN);
    for (int i = 0; i < 20; i++) printf(LINE_H);
    printf(LINE_T_DOWN);
    for (int i = 0; i < 22; i++) printf(LINE_H);
    printf(LINE_TR "\n" COLOR_RESET);

    printf(COLOR_CYAN "  " LINE_V COLOR_RESET
                      " STT " COLOR_CYAN LINE_V COLOR_RESET
                      " Trang thai " COLOR_CYAN LINE_V COLOR_RESET
                      " MSSV       " COLOR_CYAN LINE_V COLOR_RESET
                      " Ho va ten            " COLOR_CYAN LINE_V COLOR_RESET
                      " Loi vi pham        " COLOR_CYAN LINE_V COLOR_RESET
                      " Ghi chu / Ghi chu loi " COLOR_CYAN LINE_V COLOR_RESET "\n");

    printf(COLOR_CYAN "  " LINE_T_RIGHT);
    for (int i = 0; i < 5; i++) printf(LINE_H);
    printf(LINE_T_DOWN);
    for (int i = 0; i < 12; i++) printf(LINE_H);
    printf(LINE_T_DOWN);
    for (int i = 0; i < 12; i++) printf(LINE_H);
    printf(LINE_T_DOWN);
    for (int i = 0; i < 22; i++) printf(LINE_H);
    printf(LINE_T_DOWN);
    for (int i = 0; i < 20; i++) printf(LINE_H);
    printf(LINE_T_DOWN);
    for (int i = 0; i < 22; i++) printf(LINE_H);
    printf(LINE_T_LEFT "\n" COLOR_RESET);

    int startIdx = currentPage * pageSize;
    int endIdx = startIdx + pageSize;
    if (endIdx > count) endIdx = count;

    for (int i = startIdx; i < endIdx; i++) {
      CsvRecord *rec = &records[i];
      printf(COLOR_CYAN "  " LINE_V COLOR_RESET " %-3d ", i + 1);
      
      if (rec->isValid) {
        printf(COLOR_CYAN LINE_V COLOR_RESET " " COLOR_GREEN "OK        " COLOR_RESET " ");
        printf(COLOR_CYAN LINE_V COLOR_RESET " %-10s ", rec->studentId);
        printf(COLOR_CYAN LINE_V COLOR_RESET " %-20s ", rec->memberName);
        printf(COLOR_CYAN LINE_V COLOR_RESET " %-18s ", reasonName(rec->reasonCode));
        printf(COLOR_CYAN LINE_V COLOR_RESET " %-20s ", rec->notes);
      } else {
        printf(COLOR_CYAN LINE_V COLOR_RESET " " COLOR_RED "ERROR     " COLOR_RESET " ");
        printf(COLOR_CYAN LINE_V COLOR_RESET " %-10s ", rec->studentId);
        printf(COLOR_CYAN LINE_V COLOR_RESET " %-20s ", "(Khong ro)");
        printf(COLOR_CYAN LINE_V COLOR_RESET " %-18s ", "(Loi)");
        printf(COLOR_CYAN LINE_V COLOR_RESET " " COLOR_RED "%-20s" COLOR_RESET " ", rec->errorMsg);
      }
      printf(COLOR_CYAN LINE_V COLOR_RESET "\n");
    }

    printf(COLOR_CYAN "  " LINE_BL);
    for (int i = 0; i < 5; i++) printf(LINE_H);
    printf(LINE_T_UP);
    for (int i = 0; i < 12; i++) printf(LINE_H);
    printf(LINE_T_UP);
    for (int i = 0; i < 12; i++) printf(LINE_H);
    printf(LINE_T_UP);
    for (int i = 0; i < 22; i++) printf(LINE_H);
    printf(LINE_T_UP);
    for (int i = 0; i < 20; i++) printf(LINE_H);
    printf(LINE_T_UP);
    for (int i = 0; i < 22; i++) printf(LINE_H);
    printf(LINE_BR "\n" COLOR_RESET);

    printf("  Trang " COLOR_BOLD "%d/%d" COLOR_RESET " — Tong: %d | Hop le: %d | Loi: %d (Se bi bo qua)\n",
           currentPage + 1, totalPages, count, validCount, invalidCount);

    if (totalPages > 1) {
      printf(COLOR_DIM "  n: trang tiep | p: trang truoc | y: xac nhan import | q: huy > " COLOR_RESET);
      char buf[10];
      readString(buf, sizeof(buf));
      trimSpaces(buf);
      if (strcmp(buf, "n") == 0 || strcmp(buf, "N") == 0) {
        if (currentPage < totalPages - 1) currentPage++;
      } else if (strcmp(buf, "p") == 0 || strcmp(buf, "P") == 0) {
        if (currentPage > 0) currentPage--;
      } else if (strcmp(buf, "y") == 0 || strcmp(buf, "Y") == 0) {
        break;
      } else if (strcmp(buf, "q") == 0 || strcmp(buf, "Q") == 0) {
        printf(ERR_INFO "Da huy thao tac.\n");
        free(records);
        return RC_ERR_CANCELLED;
      }
    } else {
      printf(COLOR_CYAN "  Xac nhan ghi nhan %d vi pham hop le? (Y/N): " COLOR_RESET, validCount);
      char confirm[4];
      readString(confirm, sizeof(confirm));
      if (confirm[0] == 'y' || confirm[0] == 'Y') {
        break;
      } else {
        printf(ERR_INFO "Da huy thao tac.\n");
        free(records);
        return RC_ERR_CANCELLED;
      }
    }
  }

  if (validCount == 0) {
    printf(ERR_INFO "Khong co dong nao hop le de import!\n");
    free(records);
    uiPause();
    return RC_OK;
  }

  time_t currentTime = time(NULL);
  int importedCount = 0;

  for (int i = 0; i < count; i++) {
    CsvRecord *rec = &records[i];
    if (!rec->isValid) {
      continue;
    }

    if (db->violationCount >= MAX_VIOLATIONS) {
      printf(ERR_LOI "Da dat gioi han toi da so luong vi pham! Chi ghi nhan %d vi pham.\n", importedCount);
      break;
    }

    int memberIdx = memberFindById(db, rec->studentId);
    if (memberIdx == -1) continue;
    Member *m = &db->members[memberIdx];

    Violation newViolation;
    memset(&newViolation, 0, sizeof(Violation));
    newViolation.id = db->nextViolationId++;
    strncpy(newViolation.studentId, rec->studentId, MAX_MSSV_LEN - 1);
    newViolation.studentId[MAX_MSSV_LEN - 1] = '\0';
    newViolation.reason = rec->reasonCode;
    newViolation.violationTime = currentTime;
    newViolation.isPaid = 0;
    newViolation.isVoided = 0;
    strncpy(newViolation.note, rec->notes, MAX_NOTE_LEN - 1);
    newViolation.note[MAX_NOTE_LEN - 1] = '\0';

    double rate = FINE_RATE_MEMBER;
    if (m->role == MEMBER_ROLE_LEADER) rate = FINE_RATE_LEADER;
    else if (m->role == MEMBER_ROLE_DIRECTOR) rate = FINE_RATE_DIRECTOR;
    
    newViolation.fine = rate;
    newViolation.penalty = PENALTY_FINE;

    db->violations[db->violationCount++] = newViolation;

    m->violationCount++;
    m->totalFine += rate;
    if (rec->reasonCode == REASON_ABSENT) {
      m->consecutiveAbsences++;
    } else {
      m->consecutiveAbsences = 0;
    }

    (void)violationCheckOutThreshold(db, m);
    importedCount++;
  }

  /* Automatic reset for active members not absent in this CSV batch */
  int resetCount = 0;
  for (int i = 0; i < db->memberCount; i++) {
    Member *m = &db->members[i];
    if (m->isDeleted || m->isActive != STATUS_ACTIVE) {
      continue;
    }

    /* Check if this member is in the CSV with REASON_ABSENT */
    int isAbsentInCsv = 0;
    for (int j = 0; j < count; j++) {
      CsvRecord *rec = &records[j];
      if (rec->isValid && strcmp(rec->studentId, m->studentId) == 0 && rec->reasonCode == REASON_ABSENT) {
        isAbsentInCsv = 1;
        break;
      }
    }

    /* If they are not absent in the CSV and had consecutive absences, reset them */
    if (!isAbsentInCsv && m->consecutiveAbsences > 0) {
      int oldAbs = m->consecutiveAbsences;
      m->consecutiveAbsences = 0;
      resetCount++;

      /* Log action */
      char descBuf[256];
      snprintf(descBuf, sizeof(descBuf), "Tu dong reset vang khi import CSV (%d -> 0)", oldAbs);
      logSystemAction(session->studentId, descBuf, m->studentId);
    }
  }

  (void)fileioSaveViolations(db);
  (void)fileioSaveMembers(db);

  printf(ERR_OK "Import thanh cong %d vi pham vao database!\n", importedCount);
  if (resetCount > 0) {
    printf(ERR_OK "Da tu dong reset vang lien tiep ve 0 cho %d thanh vien di hop day du!\n", resetCount);
  }
  logSystemAction(session->studentId, "Import CSV vi pham", filepath);

  free(records);
  uiPause();
  return RC_OK;
}

