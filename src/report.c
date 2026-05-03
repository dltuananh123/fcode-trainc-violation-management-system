#include "report.h"
#include "types.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

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
  printf("----------------------------------------------------------------------"
         "\n");
}

/* ============================================================
 * SORT BY VIOLATIONS (Story 4.2)
 * ============================================================ */

void reportSortByViolations(const AppDatabase *db) {
  if (db == NULL || db->memberCount == 0) {
    printf("[CANH BAO] Khong co thanh vien nao trong he thong\n");
    return;
  }

  /* Ask BCN for sort order */
  printf("\nSap xep theo so lan vi pham:\n");
  printf("1. Tang dan (it vi pham nhat truoc)\n");
  printf("2. Giam dan (nhieu vi pham nhat truoc)\n");
  printf("Nhap lua chon: ");

  int orderChoice;
  if (scanf("%d", &orderChoice) != 1) {
    printf("[LOI] Lua chon khong hop le\n");
    while (getchar() != '\n') {
    }
    return;
  }
  while (getchar() != '\n') {
  }

  if (orderChoice != 1 && orderChoice != 2) {
    printf("[LOI] Lua chon khong hop le. Vui long chon 1 hoac 2\n");
    return;
  }

  int ascending = (orderChoice == 1);

  /* Build pointer-array copy — never modify original members[] */
  const Member *sorted[MAX_MEMBERS];
  int count = db->memberCount;

  for (int i = 0; i < count; i++) {
    sorted[i] = &db->members[i];
  }

  /* Selection sort — self-implemented, no qsort */
  for (int i = 0; i < count - 1; i++) {
    int bestIdx = i;
    for (int j = i + 1; j < count; j++) {
      if (ascending) {
        if (sorted[j]->violationCount < sorted[bestIdx]->violationCount) {
          bestIdx = j;
        }
      } else {
        if (sorted[j]->violationCount > sorted[bestIdx]->violationCount) {
          bestIdx = j;
        }
      }
    }
    if (bestIdx != i) {
      const Member *tmp = sorted[i];
      sorted[i] = sorted[bestIdx];
      sorted[bestIdx] = tmp;
    }
  }

  /* Display sorted result */
  printf("\n--- DANH SACH THANH VIEN THEO SO LAN VI PHAM (%s) ---\n",
         ascending ? "Tang dan" : "Giam dan");
  printf("%-5s | %-30s | %-10s | %-15s | %-10s\n", "STT", "Ho ten", "MSSV",
         "Ban", "So vi pham");
  printf("----------------------------------------------------------------------"
         "----------\n");

  for (int i = 0; i < count; i++) {
    printf("%-5d | %-30s | %-10s | %-15s | %-10d\n", i + 1,
           sorted[i]->fullName, sorted[i]->studentId,
           teamName(sorted[i]->team), sorted[i]->violationCount);
  }

  printf("----------------------------------------------------------------------"
         "----------\n");
  printf("Tong so thanh vien: %d\n", count);
}
