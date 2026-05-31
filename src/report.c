#include "report.h"
#include "types.h"
#include "utils.h"
#include "ui.h"
#include <stdio.h>
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
    int team = -1;

    for (int j = 0; j < db->memberCount; j++) {
      if (strcmp(db->members[j].studentId, v->studentId) == 0) {
        team = db->members[j].team;
        break;
      }
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
  int count = 0;

  for (int i = 0; i < db->violationCount; i++) {
    if (strcmp(db->violations[i].studentId, studentId) == 0) {
      count++;
    }
  }

  return count;
}

static void swapMemberPointers(const Member **a, const Member **b) {
  const Member *tmp = *a;
  *a = *b;
  *b = tmp;
}

static void sortMemberPointersByViolationCount(const AppDatabase *db,
                                               const Member *sorted[],
                                               int memberCount, int ascending) {
  for (int i = 0; i < memberCount - 1; i++) {
    int selected = i;
    int selectedCount = countMemberViolations(db, sorted[selected]->studentId);

    for (int j = i + 1; j < memberCount; j++) {
      int currentCount = countMemberViolations(db, sorted[j]->studentId);
      int shouldSelect = 0;

      if (ascending) {
        shouldSelect = currentCount < selectedCount;
      } else {
        shouldSelect = currentCount > selectedCount;
      }

      if (shouldSelect) {
        selected = j;
        selectedCount = currentCount;
      }
    }

    if (selected != i) {
      swapMemberPointers(&sorted[i], &sorted[selected]);
    }
  }
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

  printf("\n");
  printf(COLOR_BOLD "  THONG KE TIEN PHAT THEO BAN\n" COLOR_RESET);
  printf(COLOR_CYAN "  " LINE_TL);
  for (int i = 0; i < 16; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 17; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 17; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 17; i++) printf(LINE_H);
  printf(LINE_TR "\n" COLOR_RESET);

  printf(COLOR_CYAN "  " LINE_V COLOR_RESET
         " Ban            " COLOR_CYAN LINE_V COLOR_RESET
         " Da thu (VND)    " COLOR_CYAN LINE_V COLOR_RESET
         " Con no (VND)    " COLOR_CYAN LINE_V COLOR_RESET
         " Tong (VND)      " COLOR_CYAN LINE_V COLOR_RESET "\n");

  printf(COLOR_CYAN "  " LINE_TL);
  for (int i = 0; i < 16; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 17; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 17; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 17; i++) printf(LINE_H);
  printf(LINE_TR "\n" COLOR_RESET);

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
  printf(COLOR_CYAN "  " LINE_TL);
  for (int i = 0; i < 16; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 17; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 17; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 17; i++) printf(LINE_H);
  printf(LINE_TR "\n" COLOR_RESET);

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
  for (int i = 0; i < 16; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 17; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 17; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 17; i++) printf(LINE_H);
  printf(LINE_BR "\n" COLOR_RESET);
}

void reportSortMembersByViolations(const AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  if (db->memberCount == 0) {
    printf(ERR_INFO "Chua co thanh vien nao trong du lieu\n");
    return;
  }

  int choice;
  printf("\n");
  uiDrawSeparator();
  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD COLOR_CYAN "  SAP XEP THEO SO LAN VI PHAM");
  printf("                     ");
  printf(COLOR_RESET COLOR_BLUE BOX_V COLOR_RESET "\n");
  uiDrawSeparator();
  printf(COLOR_BLUE BOX_V COLOR_RESET "  1. Tang dan                                                " COLOR_BLUE BOX_V COLOR_RESET "\n");
  printf(COLOR_BLUE BOX_V COLOR_RESET "  2. Giam dan                                                " COLOR_BLUE BOX_V COLOR_RESET "\n");
  printf(COLOR_BLUE BOX_V COLOR_RESET "  0. Quay lai                                                " COLOR_BLUE BOX_V COLOR_RESET "\n");
  uiDrawSeparator();

  choice = readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, 0, 2);

  if (choice == 0) {
    return;
  }

  const Member *sorted[MAX_MEMBERS] = {NULL};
  for (int i = 0; i < db->memberCount; i++) {
    sorted[i] = &db->members[i];
  }

  sortMemberPointersByViolationCount(db, sorted, db->memberCount, choice == 1);

  printf("\n");
  printf(COLOR_BOLD "  DANH SACH THANH VIEN THEO SO LAN VI PHAM\n" COLOR_RESET);
  printf(COLOR_CYAN "  " LINE_TL);
  for (int i = 0; i < 22; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 12; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 14; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 14; i++) printf(LINE_H);
  printf(LINE_TR "\n" COLOR_RESET);

  printf(COLOR_CYAN "  " LINE_V COLOR_RESET
         " Ho va ten            " COLOR_CYAN LINE_V COLOR_RESET
         " MSSV       " COLOR_CYAN LINE_V COLOR_RESET
         " Ban          " COLOR_CYAN LINE_V COLOR_RESET
         " So vi pham   " COLOR_CYAN LINE_V COLOR_RESET "\n");

  printf(COLOR_CYAN "  " LINE_TL);
  for (int i = 0; i < 22; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 12; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 14; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 14; i++) printf(LINE_H);
  printf(LINE_TR "\n" COLOR_RESET);

  for (int i = 0; i < db->memberCount; i++) {
    int violationCount = countMemberViolations(db, sorted[i]->studentId);
    printf(COLOR_CYAN "  " LINE_V COLOR_RESET);
    printf(" %-20.20s ", sorted[i]->fullName);
    printf(COLOR_CYAN LINE_V COLOR_RESET);
    printf(" %-10.10s ", sorted[i]->studentId);
    printf(COLOR_CYAN LINE_V COLOR_RESET);
    printf(" %-12.12s ", teamName(sorted[i]->team));
    printf(COLOR_CYAN LINE_V COLOR_RESET);
    if (violationCount > 0) {
      printf(" " COLOR_RED "%-12d" COLOR_RESET " ", violationCount);
    } else {
      printf(" %-12d ", violationCount);
    }
    printf(COLOR_CYAN LINE_V COLOR_RESET "\n");
  }

  printf(COLOR_CYAN "  " LINE_BL);
  for (int i = 0; i < 22; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 12; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 14; i++) printf(LINE_H);
  printf(LINE_cross);
  for (int i = 0; i < 14; i++) printf(LINE_H);
  printf(LINE_BR "\n" COLOR_RESET);

  printf("  Tong: " COLOR_BOLD "%d" COLOR_RESET " thanh vien\n\n", db->memberCount);
}

void reportExportTxt(const AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  double collected[4];
  double outstanding[4];
  char exeDir[512];
  char filePath[2048];
  char timestampForFile[32];
  char timestampDisplay[32];
  time_t now = time(NULL);
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
  getExeDir(exeDir, sizeof(exeDir));

#ifdef _WIN32
  snprintf(reportsDir, sizeof(reportsDir), "%s\\reports", exeDir);
#else
  snprintf(reportsDir, sizeof(reportsDir), "%s/reports", exeDir);
#endif

  MKDIR(reportsDir);

#ifdef _WIN32
  snprintf(filePath, sizeof(filePath), "%s\\violation_report_%s.txt",
           reportsDir, timestampForFile);
#else
  snprintf(filePath, sizeof(filePath), "%s/violation_report_%s.txt", reportsDir,
           timestampForFile);
#endif

  FILE *fp = fopen(filePath, "w");
  if (fp == NULL) {
    printf("[LOI] Khong the tao file bao cao %s\n", filePath);
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
  fprintf(fp, "%-20s | %-10s | %-15s\n", "Ho va ten", "MSSV", "Con no (VND)");
  fprintf(fp, "---------------------------------------------------\n");

  int foundOutstanding = 0;
  for (int i = 0; i < db->memberCount; i++) {
    double totalOwed = 0.0;

    for (int j = 0; j < db->violationCount; j++) {
      const Violation *v = &db->violations[j];
      if (strcmp(v->studentId, db->members[i].studentId) == 0 &&
          v->isPaid == 0 && v->fine > 0) {
        totalOwed += v->fine;
      }
    }

    if (totalOwed > 0.0) {
      fprintf(fp, "%-20.20s | %-10.10s | %15.0f\n", db->members[i].fullName,
              db->members[i].studentId, totalOwed);
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

  printf("\n");
  uiDrawSeparator();
  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD COLOR_CYAN "  DASHBOARD THONG KE KY LUAT CLB");
  printf("                      ");
  printf(COLOR_RESET COLOR_BLUE BOX_V COLOR_RESET "\n");
  uiDrawSeparator();

  /* 1. Calculate general stats */
  int totalViolations = db->violationCount;
  int jacketCount = 0, absentCount = 0, activityCount = 0, violenceCount = 0;
  double totalIssued = 0.0, totalPaid = 0.0;

  for (int i = 0; i < db->violationCount; i++) {
    const Violation *v = &db->violations[i];
    totalIssued += v->fine;
    if (v->isPaid) {
      totalPaid += v->fine;
    }
    switch (v->reason) {
      case REASON_NO_JACKET: jacketCount++; break;
      case REASON_ABSENT: absentCount++; break;
      case REASON_NO_ACTIVITY: activityCount++; break;
      case REASON_VIOLENCE: violenceCount++; break;
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

  sortMemberPointersByViolationCount(db, sorted, activeCount, 0); /* descending */

  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD "  [TOP 5 THANH VIEN VI PHAM NHIEU NHAT]               " COLOR_RESET);
  printf("              ");
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  int showCount = activeCount < 5 ? activeCount : 5;
  if (showCount == 0) {
    printf(COLOR_BLUE BOX_V COLOR_RESET "  - Khong co du lieu thanh vien hoat dong.            " COLOR_BLUE BOX_V COLOR_RESET "\n");
  } else {
    for (int i = 0; i < showCount; i++) {
      int count = countMemberViolations(db, sorted[i]->studentId);
      char temp[128];
      snprintf(temp, sizeof(temp), "    %d. %s (%s) - %d lan", i + 1, sorted[i]->fullName, sorted[i]->studentId, count);
      printf(COLOR_BLUE BOX_V COLOR_RESET);
      printf("  %-62.62s", temp);
      printf(COLOR_BLUE BOX_V COLOR_RESET "\n");
    }
  }

  uiDrawSeparator();

  /* 3. Reason Breakdown */
  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD "  [PHAN TICH LY DO VI PHAM]                           " COLOR_RESET);
  printf("              ");
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  if (totalViolations == 0) {
    printf(COLOR_BLUE BOX_V COLOR_RESET "  - Khong co vi pham nao ghi nhan.                    " COLOR_BLUE BOX_V COLOR_RESET "\n");
  } else {
    double pJacket = ((double)jacketCount / totalViolations) * 100.0;
    double pAbsent = ((double)absentCount / totalViolations) * 100.0;
    double pActivity = ((double)activityCount / totalViolations) * 100.0;
    double pViolence = ((double)violenceCount / totalViolations) * 100.0;

    char temp1[128], temp2[128], temp3[128], temp4[128];
    snprintf(temp1, sizeof(temp1), "    + Khong mac ao CLB:      %3d lan (%5.1f%%)", jacketCount, pJacket);
    snprintf(temp2, sizeof(temp2), "    + Vang hop/Train-C:      %3d lan (%5.1f%%)", absentCount, pAbsent);
    snprintf(temp3, sizeof(temp3), "    + Khong tham gia HD:     %3d lan (%5.1f%%)", activityCount, pActivity);
    snprintf(temp4, sizeof(temp4), "    + Bao luc (Out CLB):     %3d lan (%5.1f%%)", violenceCount, pViolence);

    printf(COLOR_BLUE BOX_V COLOR_RESET); printf("  %-62.62s", temp1); printf(COLOR_BLUE BOX_V COLOR_RESET "\n");
    printf(COLOR_BLUE BOX_V COLOR_RESET); printf("  %-62.62s", temp2); printf(COLOR_BLUE BOX_V COLOR_RESET "\n");
    printf(COLOR_BLUE BOX_V COLOR_RESET); printf("  %-62.62s", temp3); printf(COLOR_BLUE BOX_V COLOR_RESET "\n");
    printf(COLOR_BLUE BOX_V COLOR_RESET); printf("  %-62.62s", temp4); printf(COLOR_BLUE BOX_V COLOR_RESET "\n");
  }

  uiDrawSeparator();

  /* 4. Fine Collection Progress */
  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD "  [TIEN DO THU TIEN PHAT]                            " COLOR_RESET);
  printf("              ");
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  char issuedStr[128];
  snprintf(issuedStr, sizeof(issuedStr), "    - Tong tien phat da phat: %.0f VND", totalIssued);

  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf("  %-62.62s", issuedStr);
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf("    - Tong tien phat da thu:  ");
  printf(COLOR_GREEN "%.0f VND" COLOR_RESET, totalPaid);
  
  char digits[32];
  snprintf(digits, sizeof(digits), "%.0f", totalPaid);
  int printedLen = 30 + (int)strlen(digits) + 4;
  for (int i = printedLen; i < 66; i++) {
    printf(" ");
  }
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  double progress = 0.0;
  if (totalIssued > 0.0) {
    progress = (totalPaid / totalIssued) * 100.0;
  }

  int filledBlocks = (int)(progress / 5.0);
  if (filledBlocks > 20) filledBlocks = 20;
  
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
  
  int printedBarLen = 14 + 20 + 2 + 5;
  if (progress >= 100.0) printedBarLen++;
  for (int i = printedBarLen; i < 66; i++) {
    printf(" ");
  }
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  uiDrawSeparator();
  printf("\n");
}
