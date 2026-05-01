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
  printf("---------------------------------------------------------------------"
         "-\n");
}
