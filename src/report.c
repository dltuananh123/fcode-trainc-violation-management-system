#include "report.h"
#include "types.h"
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
  if (db == NULL || studentId == NULL) {
    return 0;
  }
  int count = 0;

  for (int i = 0; i < db->violationCount; i++) {
    if (strcmp(db->violations[i].studentId, studentId) == 0) {
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

  printf("\n--- THONG KE TIEN PHAT THEO BAN ---\n");
  printf("%-15s | %-15s | %-15s | %-15s\n", "Ban", "Da thu (VND)",
         "Con no (VND)", "Tong (VND)");
  printf("---------------------------------------------------------------------"
         "-\n");

  for (int team = TEAM_ACADEMIC; team <= TEAM_MEDIA; team++) {
    double total = collected[team] + outstanding[team];
    printf("%-15s | %15.0f | %15.0f | %15.0f\n", teamName(team),
           collected[team], outstanding[team], total);
  }
  printf("---------------------------------------------------------------------"
         "-\n");
}

void reportSortMembersByViolations(const AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  if (db->memberCount == 0) {
    printf("[THONG BAO] Chua co thanh vien nao trong du lieu\n");
    return;
  }

  int choice;
  printf("\nSAP XEP THEO SO LAN VI PHAM\n");
  printf("1. Tang dan\n");
  printf("2. Giam dan\n");
  printf("0. Quay lai\n");
  printf("Nhap lua chon: ");

  if (readInt(&choice) != 1) {
    printf("[LOI] Lua chon khong hop le\n");
    return;
  }

  if (choice == 0) {
    return;
  }
  if (choice != 1 && choice != 2) {
    printf("[LOI] Lua chon khong hop le\n");
    return;
  }

  MemberViolationCount sorted[MAX_MEMBERS];
  for (int i = 0; i < db->memberCount; i++) {
    sorted[i].member = &db->members[i];
    sorted[i].violationCount =
        countMemberViolations(db, db->members[i].studentId);
  }

  qsort(sorted, (size_t)db->memberCount, sizeof(MemberViolationCount),
        choice == 1 ? compareAscending : compareDescending);

  printf("\nDANH SACH THANH VIEN THEO SO LAN VI PHAM\n");
  printf(
      "+----------------------+------------+--------------+--------------+\n");
  printf(
      "| Ho va ten            | MSSV       | Ban          | So vi pham   |\n");
  printf(
      "+----------------------+------------+--------------+--------------+\n");

  for (int i = 0; i < db->memberCount; i++) {
    if (sorted[i].member == NULL) {
      continue;
    }
    printf("| %-20.20s | %-10.10s | %-12.12s | %-12d |\n",
           sorted[i].member->fullName, sorted[i].member->studentId,
           teamName(sorted[i].member->team), sorted[i].violationCount);
  }

  printf(
      "+----------------------+------------+--------------+--------------+\n");
  printf("Tong: %d thanh vien\n", db->memberCount);
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
