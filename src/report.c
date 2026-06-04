#include "report.h"
#include "member.h"
#include "types.h"
#include "ui.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void aggregateTeamTotals(const AppDatabase *db, double collected[],
                                double outstanding[]) {
  for (int team = TEAM_ACADEMIC; team <= TEAM_MEDIA; team++) {
    collected[team] = 0.0;
    outstanding[team] = 0.0;
  }

  for (int i = 0; i < db->violationCount; i++) {
    const Violation *v = &db->violations[i];
    if (v->isVoided) {
      continue;
    }
    int team = -1;

    int memberIdx = memberFindById(db, v->studentId);
    if (memberIdx != -1) {
      team = db->members[memberIdx].team;
    }

    if (team >= TEAM_ACADEMIC && team <= TEAM_MEDIA) {
      if (v->isPaid) {
        collected[team] += v->fine;
      } else {
        outstanding[team] += v->fine;
      }
    }
  }
}

static int countMemberViolations(const AppDatabase *db, const char *studentId) {
  if (db == NULL || studentId == NULL) {
    return 0;
  }
  int count = 0;

  for (int i = 0; i < db->violationCount; i++) {
    if (strcmp(db->violations[i].studentId, studentId) == 0 &&
        db->violations[i].isVoided == 0) {
      count++;
    }
  }

  return count;
}

typedef struct {
  const Member *member;
  int violationCount;
} MemberViolationCount;

static int compareAscending(const void *a, const void *b) {
  const MemberViolationCount *ma = (const MemberViolationCount *)a;
  const MemberViolationCount *mb = (const MemberViolationCount *)b;
  return ma->violationCount - mb->violationCount;
}

static int compareDescending(const void *a, const void *b) {
  const MemberViolationCount *ma = (const MemberViolationCount *)a;
  const MemberViolationCount *mb = (const MemberViolationCount *)b;
  return mb->violationCount - ma->violationCount;
}

static void sortMemberPointersByViolationCount(const AppDatabase *db,
                                               const Member *sorted[],
                                               int count, int ascending) {
  if (count <= 0) {
    return;
  }
  MemberViolationCount *mvc =
      malloc((size_t)count * sizeof(MemberViolationCount));
  if (!mvc) {
    return;
  }
  for (int i = 0; i < count; i++) {
    mvc[i].member = sorted[i];
    mvc[i].violationCount = countMemberViolations(db, sorted[i]->studentId);
  }
  qsort(mvc, (size_t)count, sizeof(MemberViolationCount),
        ascending ? compareAscending : compareDescending);
  for (int i = 0; i < count; i++) {
    sorted[i] = mvc[i].member;
  }
  free(mvc);
}

/* ============================================================
 * REPORT EXPORT FUNCTIONS
 * ============================================================ */

void reportTeamStats(const AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  double collected[4] = {0.0, 0.0, 0.0, 0.0};
  double outstanding[4] = {0.0, 0.0, 0.0, 0.0};
  aggregateTeamTotals(db, collected, outstanding);

  uiClear();
  uiDrawBreadcrumb("[3] Quan ly bao cao -> [1] Thong ke tien phat theo ban");
  printf(COLOR_CYAN "  " LINE_TL);
  for (int i = 0; i < 16; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_TR "\n" COLOR_RESET);

  printf(COLOR_CYAN "  " LINE_V COLOR_RESET
                    " Ban            " COLOR_CYAN LINE_V COLOR_RESET
                    " Da thu (VND)    " COLOR_CYAN LINE_V COLOR_RESET
                    " Con no (VND)    " COLOR_CYAN LINE_V COLOR_RESET
                    " Tong (VND)      " COLOR_CYAN LINE_V COLOR_RESET "\n");

  printf(COLOR_CYAN "  " LINE_T_RIGHT);
  for (int i = 0; i < 16; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_LEFT "\n" COLOR_RESET);

  double grandCollected = 0.0;
  double grandOutstanding = 0.0;

  for (int team = TEAM_ACADEMIC; team <= TEAM_MEDIA; team++) {
    double total = collected[team] + outstanding[team];
    grandCollected += collected[team];
    grandOutstanding += outstanding[team];

    printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
    printf(" %-14s ", teamName(team));
    printf(COLOR_CYAN LINE_V COLOR_RESET);
    printf(" " COLOR_GREEN "%15.0f" COLOR_RESET " ", collected[team]);
    printf(COLOR_CYAN LINE_V COLOR_RESET);
    printf(" " COLOR_RED "%15.0f" COLOR_RESET " ", outstanding[team]);
    printf(COLOR_CYAN LINE_V COLOR_RESET);
    printf(" " COLOR_PURPLE "%15.0f" COLOR_RESET " ", total);
    printf(COLOR_CYAN LINE_V COLOR_RESET "\n");
  }

  /* Bottom separator before grand total */
  printf(COLOR_CYAN "  " LINE_T_RIGHT);
  for (int i = 0; i < 16; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_DOWN);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_LEFT "\n" COLOR_RESET);

  /* Grand total row */
  double grandTotal = grandCollected + grandOutstanding;
  printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
  printf(" " COLOR_BOLD "%-14s" COLOR_RESET " ", "Tong Cong");
  printf(COLOR_CYAN LINE_V COLOR_RESET);
  printf(" " COLOR_BOLD COLOR_GREEN "%15.0f" COLOR_RESET " ", grandCollected);
  printf(COLOR_CYAN LINE_V COLOR_RESET);
  printf(" " COLOR_BOLD COLOR_RED "%15.0f" COLOR_RESET " ", grandOutstanding);
  printf(COLOR_CYAN LINE_V COLOR_RESET);
  printf(" " COLOR_BOLD COLOR_PURPLE "%15.0f" COLOR_RESET " ", grandTotal);
  printf(COLOR_CYAN LINE_V COLOR_RESET "\n");

  /* Final bottom border */
  printf(COLOR_CYAN "  " LINE_BL);
  for (int i = 0; i < 16; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_T_UP);
  for (int i = 0; i < 17; i++) {
    printf(LINE_H);
  }
  printf(LINE_BR "\n" COLOR_RESET);
}

void reportSortMembersByViolations(const AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  if (db->memberCount == 0) {
    printf(ERR_INFO "Chua co thanh vien nao trong du lieu\n");
    uiPause();
    return;
  }

  while (1) {
    uiClear();
    uiDrawBreadcrumb("[3] Quan ly bao cao -> [2] Sap xep theo so lan vi pham");
    uiDrawMenuRow("  1. Tang dan");
    uiDrawMenuRow("  2. Giam dan");
    uiDrawMenuRow(COLOR_DIM "  0. Quay lai" COLOR_RESET);
    // uiDrawMenuRow(COLOR_DIM " -1. Ve menu" COLOR_RESET);
    uiDrawSeparator();

    int choice =
        readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, -1, 2);

    if (choice == -1 || choice == 0) {
      return;
    }

    const Member *sorted[MAX_MEMBERS] = {NULL};
    int activeCount = 0;
    for (int i = 0; i < db->memberCount; i++) {
      if (!db->members[i].isDeleted &&
          db->members[i].isActive == STATUS_ACTIVE) {
        sorted[activeCount++] = &db->members[i];
      }
    }

    if (activeCount == 0) {
      printf(ERR_INFO "Chua co thanh vien dang hoat dong nao trong du lieu\n");
      printf(COLOR_DIM "  Nhan Enter de tiep tuc..." COLOR_RESET " ");
      char dummy[10];
      readString(dummy, sizeof(dummy));
      continue;
    }

    sortMemberPointersByViolationCount(db, sorted, activeCount, choice == 1);

    int totalPages = (activeCount + ROWS_PER_PAGE - 1) / ROWS_PER_PAGE;
    int currentPage = 0;

    while (currentPage < totalPages) {
      uiClear();
      uiDrawBreadcrumb(
          "[3] Quan ly bao cao -> [2] Sap xep theo so lan vi pham");
      printf(COLOR_BOLD
             "  DANH SACH THANH VIEN THEO SO LAN VI PHAM\n" COLOR_RESET);
      static const TableColumn RANKING_COLS[] = {
          {22, "Ho va ten"}, {12, "MSSV"}, {14, "Ban"}, {14, "So vi pham"}};
      static const int RANKING_COL_COUNT =
          (int)(sizeof(RANKING_COLS) / sizeof(RANKING_COLS[0]));

      uiTableBegin(RANKING_COLS, RANKING_COL_COUNT);

      int start = currentPage * ROWS_PER_PAGE;
      int end = start + ROWS_PER_PAGE;
      if (end > activeCount) {
        end = activeCount;
      }

      for (int i = start; i < end; i++) {
        int violationCount = countMemberViolations(db, sorted[i]->studentId);
        uiTableRowBegin();
        uiTableCell(sorted[i]->fullName, 22, "");
        uiTableCell(sorted[i]->studentId, 12, "");
        uiTableCell(teamName(sorted[i]->team), 14, "");
        if (violationCount > 0) {
          uiTableCellFmt(14, COLOR_RED, "%d", violationCount);
        } else {
          uiTableCellFmt(14, "", "%d", violationCount);
        }
        uiTableRowEnd();
      }

      uiTableEnd(RANKING_COLS, RANKING_COL_COUNT);

      printf("  Trang " COLOR_BOLD "%d/%d" COLOR_RESET " — Tong: " COLOR_BOLD
             "%d" COLOR_RESET " thanh vien\n",
             currentPage + 1, totalPages, activeCount);

      printf(COLOR_DIM "  p: trang truoc | n: trang tiep | 0: thoat" COLOR_RESET
                       " > ");
      char buf[10];
      readString(buf, sizeof(buf));
      char c = buf[0];
      if (c == '0') {
        break;
      }
      if ((c == 'n' || c == 'N') && currentPage < totalPages - 1) {
        currentPage++;
      } else if ((c == 'p' || c == 'P') && currentPage > 0) {
        currentPage--;
      }
    }
  }
}

void reportExportTxt(const AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  double collected[4];
  double outstanding[4];
  char filePath[2048];
  char timestampForFile[32];
  char timestampDisplay[32];
  time_t now = time(NULL);
  /* Note: localtime is not thread-safe but this application is
     single-threaded. */
  struct tm *timeInfo = localtime(&now);

  if (timeInfo == NULL) {
    printf("[LOI] Khong the lay thoi gian he thong de xuat bao cao\n");
    return;
  }

  strftime(timestampForFile, sizeof(timestampForFile), "%Y%m%d_%H%M%S",
           timeInfo);
  strftime(timestampDisplay, sizeof(timestampDisplay), "%d/%m/%Y %H:%M:%S",
           timeInfo);

  char reportsDir[1024];
  char reportFilename[256];
  resolvePath("reports", NULL, reportsDir, sizeof(reportsDir));
  (void)MKDIR(reportsDir);

  snprintf(reportFilename, sizeof(reportFilename), "violation_report_%s.txt",
           timestampForFile);
  resolvePath("reports", reportFilename, filePath, sizeof(filePath));

  FILE *fp = fopen(filePath, "w");
  if (fp == NULL) {
    printf("[LOI] Khong the tao file bao cao %s\n", filePath);
    uiPause();
    return;
  }

  aggregateTeamTotals(db, collected, outstanding);

  fprintf(fp, "BAO CAO VI PHAM CLB F-CODE\n");
  fprintf(fp, "Thoi gian xuat: %s\n", timestampDisplay);
  fprintf(fp, "========================================\n\n");

  fprintf(fp, "TONG HOP THEO BAN\n");
  fprintf(fp, "%-15s | %-15s | %-15s | %-15s\n", "Ban", "Da thu (VND)",
          "Con no (VND)", "Tong (VND)");
  fprintf(fp, "----------------------------------------------------------------"
              "-------\n");
  for (int team = TEAM_ACADEMIC; team <= TEAM_MEDIA; team++) {
    double total = collected[team] + outstanding[team];
    fprintf(fp, "%-15s | %15.0f | %15.0f | %15.0f\n", teamName(team),
            collected[team], outstanding[team], total);
  }

  fprintf(fp, "\nTHANH VIEN CON NO TIEN PHAT\n");
  fprintf(fp, "%-20s | %-10s | %-25s | %-12s | %-15s\n", "Ho va ten", "MSSV",
          "Email", "SDT", "Con no (VND)");
  fprintf(fp, "--------------------------------------------------------------"
              "------------------\n");

  int foundOutstanding = 0;
  for (int i = 0; i < db->memberCount; i++) {
    if (db->members[i].isDeleted) {
      continue;
    }
    double totalOwed = 0.0;

    for (int j = 0; j < db->violationCount; j++) {
      const Violation *v = &db->violations[j];
      if (v->isVoided) {
        continue;
      }
      if (strcmp(v->studentId, db->members[i].studentId) == 0 &&
          v->isPaid == 0 && v->fine > 0) {
        totalOwed += v->fine;
      }
    }

    if (totalOwed > 0.0) {
      fprintf(fp, "%-20.20s | %-10.10s | %-25.25s | %-12.12s | %15.0f\n",
              db->members[i].fullName, db->members[i].studentId,
              db->members[i].email, db->members[i].phone, totalOwed);
      foundOutstanding++;
    }
  }

  if (foundOutstanding == 0) {
    fprintf(fp, "Khong co thanh vien nao con no tien phat.\n");
  }

  fclose(fp);
  printf("[OK] Da xuat bao cao ra file: %s\n", filePath);
}

void reportDashboard(const AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  uiClear();
  uiDrawBreadcrumb("[3] Quan ly bao cao -> [4] Xem Dashboard ky luat");

  /* 1. Calculate general stats */
  int totalViolations = 0;
  int jacketCount = 0;
  int absentCount = 0;
  int activityCount = 0;
  int violenceCount = 0;
  double totalIssued = 0.0;
  double totalPaid = 0.0;

  for (int i = 0; i < db->violationCount; i++) {
    const Violation *v = &db->violations[i];
    if (v->isVoided) {
      continue;
    }
    totalViolations++;
    totalIssued += v->fine;
    if (v->isPaid) {
      totalPaid += v->fine;
    }
    switch (v->reason) {
    case REASON_NO_JACKET:
      jacketCount++;
      break;
    case REASON_ABSENT:
      absentCount++;
      break;
    case REASON_NO_ACTIVITY:
      activityCount++;
      break;
    case REASON_VIOLENCE:
      violenceCount++;
      break;
    default:
      break;
    }
  }

  /* 2. Top 5 violators (active and non-deleted members only) */
  const Member *sorted[MAX_MEMBERS] = {NULL};
  int activeCount = 0;
  for (int i = 0; i < db->memberCount; i++) {
    if (!db->members[i].isDeleted && db->members[i].isActive == STATUS_ACTIVE) {
      sorted[activeCount++] = &db->members[i];
    }
  }

  sortMemberPointersByViolationCount(db, sorted, activeCount,
                                     0); /* descending */

  uiDrawMenuRow(COLOR_BOLD COLOR_YELLOW
                "  [TOP 5 THANH VIEN VI PHAM NHIEU NHAT]" COLOR_RESET);

  int showCount = activeCount < 5 ? activeCount : 5;
  if (showCount == 0) {
    uiDrawMenuRow("  - Khong co du lieu thanh vien hoat dong.");
  } else {
    for (int i = 0; i < showCount; i++) {
      int count = countMemberViolations(db, sorted[i]->studentId);
      char temp[128];
      snprintf(temp, sizeof(temp), "    %d. %s (%s) - %d lan", i + 1,
               sorted[i]->fullName, sorted[i]->studentId, count);
      uiDrawMenuRowFmt("  %-66.66s", temp);
    }
  }

  uiDrawSeparator();

  /* 3. Reason Breakdown */
  uiDrawMenuRow(COLOR_BOLD COLOR_YELLOW
                "  [PHAN TICH LY DO VI PHAM]" COLOR_RESET);

  if (totalViolations == 0) {
    uiDrawMenuRow("  - Khong co vi pham nao ghi nhan.");
  } else {
    double pJacket = ((double)jacketCount / totalViolations) * 100.0;
    double pAbsent = ((double)absentCount / totalViolations) * 100.0;
    double pActivity = ((double)activityCount / totalViolations) * 100.0;
    double pViolence = ((double)violenceCount / totalViolations) * 100.0;

    char temp1[128];
    char temp2[128];
    char temp3[128];
    char temp4[128];
    snprintf(temp1, sizeof(temp1),
             "    + Khong mac ao CLB:      %3d lan (%5.1f%%)", jacketCount,
             pJacket);
    snprintf(temp2, sizeof(temp2),
             "    + Vang hop/Train-C:      %3d lan (%5.1f%%)", absentCount,
             pAbsent);
    snprintf(temp3, sizeof(temp3),
             "    + Khong tham gia HD:     %3d lan (%5.1f%%)", activityCount,
             pActivity);
    snprintf(temp4, sizeof(temp4),
             "    + Bao luc (Out CLB):     %3d lan (%5.1f%%)", violenceCount,
             pViolence);

    uiDrawMenuRowFmt("  %-66.66s", temp1);
    uiDrawMenuRowFmt("  %-66.66s", temp2);
    uiDrawMenuRowFmt("  %-66.66s", temp3);
    uiDrawMenuRowFmt("  %-66.66s", temp4);
  }

  uiDrawSeparator();

  /* 4. Fine Collection Progress */
  uiDrawMenuRow(COLOR_BOLD COLOR_YELLOW
                "  [TIEN DO THU TIEN PHAT]" COLOR_RESET);

  char issuedStr[128];
  snprintf(issuedStr, sizeof(issuedStr),
           "    - Tong tien phat da phat: %.0f VND", totalIssued);

  uiDrawMenuRowFmt("  %-66.66s", issuedStr);

  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf("    - Tong tien phat da thu:  ");
  printf(COLOR_GREEN "%.0f VND" COLOR_RESET, totalPaid);

  char digits[32];
  snprintf(digits, sizeof(digits), "%.0f", totalPaid);
  int printedLen = 30 + (int)strlen(digits) + 4;
  for (int i = printedLen; i < UI_TERM_WIDTH - 2; i++) {
    printf(" ");
  }
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  double progress = 0.0;
  if (totalIssued > 0.0) {
    progress = (totalPaid / totalIssued) * 100.0;
  }

  int filledBlocks = (int)(progress / 5.0);
  if (filledBlocks > 20) {
    filledBlocks = 20;
  }

  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf("    Tien do: [");
  printf(COLOR_GREEN);
  for (int i = 0; i < filledBlocks; i++) {
    printf("\xE2\x96\x88");
  }
  printf(COLOR_GRAY);
  for (int i = filledBlocks; i < 20; i++) {
    printf("\xE2\x96\x91");
  }
  printf(COLOR_RESET);
  printf("] %5.1f%%", progress);

  int printedBarLen = 14 + 20 + 2 + 6;
  if (progress >= 100.0) {
    printedBarLen++;
  }
  for (int i = printedBarLen; i < UI_TERM_WIDTH - 2; i++) {
    printf(" ");
  }
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  uiDrawSeparator();
  printf("\n");
}
