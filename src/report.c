#include "report.h"
#include "types.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ============================================================
 * REPORT EXPORT FUNCTIONS
 * ============================================================ */

void reportTeamStats(const AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  double collected[4] = {0.0, 0.0, 0.0, 0.0};
  double outstanding[4] = {0.0, 0.0, 0.0, 0.0};

  for (int i = 0; i < db->violationCount; i++) {
    const Violation *v = &db->violations[i];

    /* Find the team of the member who committed the violation */
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

/* ============================================================
 * 4.3 REPORT VIOLATION TXT
 * ============================================================ */

int reportExportTxt(const AppDatabase *db) {
  if (db == NULL) return -1;

  /* 1. Tạo tên file theo định dạng report_YYYYMMDD_HHMMSS.txt */
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char filename[64];
  strftime(filename, sizeof(filename), "report_%Y%m%d_%H%M%S.txt", t);

  FILE *fp = fopen(filename, "w");
  if (fp == NULL) {
    printf("[LOI] Khong the tao file: %s\n", filename);
    return -1;
  }

  char timeStr[26];
  strftime(timeStr, sizeof(timeStr), "%d/%m/%Y %H:%M:%S", t);

  /* 2. Ghi Header báo cáo */
  fprintf(fp, "=================================================\n");
  fprintf(fp, "          BAO CAO VI PHAM CLB F-CODE\n");
  fprintf(fp, "          Thoi gian xuat: %s\n", timeStr);
  fprintf(fp, "=================================================\n\n");

  /* 3. Tính toán và ghi Thống kê theo Ban */
  double collected[4] = {0.0, 0.0, 0.0, 0.0};
  double outstanding[4] = {0.0, 0.0, 0.0, 0.0};

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
      if (v->isPaid) collected[team] += v->fine;
      else outstanding[team] += v->fine;
    }
  }

  fprintf(fp, "1. THONG KE TIEN PHAT THEO BAN\n");
  fprintf(fp, "----------------------------------------------------------\n");
  fprintf(fp, "%-15s | %-12s | %-12s | %-12s\n", "Ban", "Da thu", "Con no", "Tong");
  fprintf(fp, "----------------------------------------------------------\n");
  for (int i = TEAM_ACADEMIC; i <= TEAM_MEDIA; i++) {
    fprintf(fp, "%-15s | %12.0f | %12.0f | %12.0f\n", 
            teamName(i), collected[i], outstanding[i], collected[i] + outstanding[i]);
  }
  fprintf(fp, "----------------------------------------------------------\n\n");

  /* 4. Ghi Danh sách thành viên còn nợ phí */
  fprintf(fp, "2. DANH SACH THANH VIEN CON NO TIEN PHAT\n");
  fprintf(fp, "----------------------------------------------------------\n");
  fprintf(fp, "%-10s | %-25s | %-15s\n", "MSSV", "Ho va ten", "So tien no");
  fprintf(fp, "----------------------------------------------------------\n");

  int debtors = 0;
  for (int i = 0; i < db->memberCount; i++) {
    if (db->members[i].totalFine > 0) {
      fprintf(fp, "%-10s | %-25s | %15.0f\n", 
              db->members[i].studentId, db->members[i].fullName, db->members[i].totalFine);
      debtors++;
    }
  }
  
  if (debtors == 0) fprintf(fp, "(Khong co thanh vien nao no tien phat)\n");
  fprintf(fp, "----------------------------------------------------------\n");
  fprintf(fp, "Tong so thanh vien con no: %d\n", debtors);
  fprintf(fp, "=================================================\n");

  fclose(fp);
  printf("[OK] Da xuat bao cao thanh cong: %s\n", filename);
  return 0;
}