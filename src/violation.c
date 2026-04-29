#include "violation.h"
#include "fileio.h"
#include "member.h"
#include "types.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ============================================================
 * Private helpers
 * ============================================================ */

/**
 * @brief Calculate fine amount based on member role.
 *
 * Member (role 0) = 20,000 VND
 * Leader/Vice (role 1) or BCN (role 2) = 50,000 VND
 */
static double calculateFine(int memberRole) {
  if (memberRole == MEMBER_ROLE_MEMBER) {
    return 20000.0;
  }
  /* Leader/Vice or BCN */
  return 50000.0;
}

/**
 * @brief Display violation reason menu and read user's choice.
 *
 * @param reason Pointer to store the selected reason.
 * @return 0 on valid selection, -1 on invalid input.
 */
static int selectViolationReason(int *reason) {
  printf("\nChon ly do vi pham:\n");
  printf("  0. Khong mac ao CLB\n");
  printf("  1. Vang hop\n");
  printf("  2. Khong tham gia hoat dong\n");
  printf("  3. Bao luc\n");
  printf("Nhap lua chon: ");

  if (read_int(reason) != 1) {
    printf("[LOI] Lua chon khong hop le\n");
    return -1;
  }

  if (*reason < REASON_NO_JACKET || *reason > REASON_VIOLENCE) {
    printf("[LOI] Ly do vi pham khong hop le\n");
    return -1;
  }

  return 0;
}

/**
 * @brief Ask BCN to confirm an Out CLB action.
 *
 * @param memberName The member's full name for display.
 * @return 1 if confirmed, 0 if cancelled.
 */
static int confirmOutClb(const char *memberName) {
  char confirm[4];
  printf("[XAC NHAN] Ban co chac chan muon Out CLB thanh vien %s? (y/n): ",
         memberName);
  read_string(confirm, sizeof(confirm));

  if (confirm[0] == 'y' || confirm[0] == 'Y') {
    return 1;
  }
  printf("[THONG BAO] Da huy thao tac Out CLB\n");
  return 0;
}

/**
 * @brief Handle violence violation — immediate Out CLB.
 *
 * Sets fine=0, penalty=PENALTY_OUT_CLB. Prompts BCN to confirm.
 * If confirmed, sets member.isActive = STATUS_OUT_CLB.
 *
 * @param db      Pointer to AppDatabase
 * @param member  Pointer to the offending Member
 * @param v       Pointer to the Violation being recorded
 */
static void handleViolence(AppDatabase *db, Member *member, Violation *v) {
  (void)db; /* save happens in violationRecord() after return */
  v->fine = 0.0;
  v->penalty = PENALTY_OUT_CLB;

  printf("\n[CANH BAO] Vi pham BAO LUC!\n");
  printf("  Thanh vien: %s (%s)\n", member->fullName, member->studentId);
  printf("  Hinh thuc xu ly: OUT CLB (khong phat tien)\n");

  if (confirmOutClb(member->fullName)) {
    member->isActive = STATUS_OUT_CLB;
    member->consecutiveAbsences = 0;
    printf("[OK] Thanh vien %s da bi Out CLB do bao luc\n", member->fullName);
  }

  /* Reset consecutive absences — violence means member was present */
  member->consecutiveAbsences = 0;
}

/**
 * @brief Handle absent violation — increment consecutiveAbsences,
 *        then check Out CLB threshold.
 *
 * @param db      Pointer to AppDatabase
 * @param member  Pointer to the absent Member
 * @param v       Pointer to the Violation being recorded
 */
static void handleAbsent(AppDatabase *db, Member *member, Violation *v) {
  v->fine = calculateFine(member->role);
  v->penalty = PENALTY_FINE;

  member->consecutiveAbsences++;

  printf("[THONG BAO] So buoi vang lien tiep cua %s: %d\n", member->fullName,
         member->consecutiveAbsences);

  /* Check Out CLB threshold (Story 3.2) */
  violationCheckOutThreshold(db, member);
}

/* ============================================================
 * Story 3.1 — Record Violation
 * ============================================================ */

int violationRecord(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  /* Check violation capacity */
  if (db->violationCount >= MAX_VIOLATIONS) {
    printf("[LOI] Da dat gioi han so luong vi pham (%d)\n", MAX_VIOLATIONS);
    return -1;
  }

  printf("\nGHI NHAN VI PHAM\n");

  /* Step 1: Find member by MSSV */
  char studentId[MAX_MSSV_LEN];
  printf("Nhap MSSV thanh vien: ");
  read_string(studentId, MAX_MSSV_LEN);

  int memberIdx = memberFindById(db, studentId);
  if (memberIdx == -1) {
    printf("[LOI] Khong tim thay thanh vien voi MSSV: %s\n", studentId);
    return -1;
  }

  Member *member = &db->members[memberIdx];

  /* Check if member is still active */
  if (member->isActive != STATUS_ACTIVE) {
    printf("[LOI] Thanh vien %s da bi Out CLB, khong the ghi nhan vi pham\n",
           member->fullName);
    return -1;
  }

  /* Display member info */
  printf("  Thanh vien: %s\n", member->fullName);
  printf("  Ban: %s\n", team_name(member->team));
  printf("  Chuc vu: %s\n", member_role_name(member->role));

  /* Step 2: Select violation reason */
  int reason;
  if (selectViolationReason(&reason) != 0) {
    return -1;
  }

  /* Step 3: Create violation record */
  Violation newViolation;
  memset(&newViolation, 0, sizeof(Violation));

  strncpy(newViolation.studentId, studentId, MAX_MSSV_LEN - 1);
  newViolation.studentId[MAX_MSSV_LEN - 1] = '\0';
  newViolation.reason = reason;
  newViolation.violationTime = time(NULL);
  newViolation.isPaid = 0;

  /* Step 4: Handle by reason type */
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
    /* Non-absent, non-violence: calculate fine, reset absence streak */
    newViolation.fine = calculateFine(member->role);
    newViolation.penalty = PENALTY_FINE;
    member->consecutiveAbsences = 0;
    break;
  }

  /* Step 5: Optional note */
  printf("Ghi chu (Enter de bo qua): ");
  read_string(newViolation.note, MAX_NOTE_LEN);

  /* Step 6: Add violation to database */
  db->violations[db->violationCount++] = newViolation;

  /* Step 7: Update member stats */
  member->violationCount++;
  member->totalFine += newViolation.fine;

  /* Step 8: Persist to files */
  if (fileioSaveViolations(db) != 0) {
    printf("[LOI] Khong the luu du lieu vi pham\n");
    /* Rollback in-memory changes */
    db->violationCount--;
    member->violationCount--;
    member->totalFine -= newViolation.fine;
    return -1;
  }

  if (fileioSaveMembers(db) != 0) {
    printf("[LOI] Khong the luu du lieu thanh vien\n");
    /* Rollback member stats */
    member->violationCount--;
    member->totalFine -= newViolation.fine;
    return -1;
  }

  /* Step 9: Display confirmation */
  char timeBuf[20];
  format_time(newViolation.violationTime, timeBuf, sizeof(timeBuf));

  printf("\n[OK] Ghi nhan vi pham thanh cong\n");
  printf("  Thanh vien: %s (%s)\n", member->fullName, member->studentId);
  printf("  Ly do: %s\n", reason_name(newViolation.reason));
  printf("  Thoi gian: %s\n", timeBuf);

  if (newViolation.penalty == PENALTY_OUT_CLB) {
    printf("  Xu ly: OUT CLB\n");
  } else {
    printf("  Tien phat: %.0f VND\n", newViolation.fine);
  }

  return 0;
}

/* ============================================================
 * Story 3.2 — Out CLB Warning & Enforcement
 * ============================================================ */

int violationCheckOutThreshold(AppDatabase *db, Member *member) {
  if (db == NULL || member == NULL) {
    return -1;
  }

  if (member->consecutiveAbsences == 3) {
    /* Warning at exactly 3 consecutive absences */
    printf("\n[CANH BAO] Thanh vien %s da vang 3 buoi lien tiep!\n",
           member->fullName);
    printf("  Neu vang them 1 buoi nua se bi Out CLB.\n");
    return 0;
  }

  if (member->consecutiveAbsences >= 4) {
    /* Out CLB threshold reached — more than 3 absences */
    printf("\n[CANH BAO] Thanh vien %s da vang qua 3 buoi lien tiep -> Out "
           "CLB\n",
           member->fullName);
    printf("  So buoi vang lien tiep: %d\n", member->consecutiveAbsences);

    if (confirmOutClb(member->fullName)) {
      member->isActive = STATUS_OUT_CLB;
      printf("[OK] Thanh vien %s da bi Out CLB\n", member->fullName);
      return 1;
    }
  }

  return 0;
}

void violationCheckAllOutClb(AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  printf("\nKIEM TRA NGUONG OUT CLB\n");
  printf("+------------+----------------------+-----------+------------+\n");
  printf("| MSSV       | Ho va ten            | Vang LT   | Trang thai |\n");
  printf("+------------+----------------------+-----------+------------+\n");

  int found = 0;

  for (int i = 0; i < db->memberCount; i++) {
    Member *m = &db->members[i];

    /* Show members at warning threshold (>= 2) or already Out CLB */
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

      printf("| %-10.10s | %-20.20s | %-9d | %-10.10s |\n",
             m->studentId, m->fullName,
             m->consecutiveAbsences, status);
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
  printf("  QUA NGUONG: Vang qua 3 buoi, cho BCN xu ly\n");
  printf("  Out CLB   : Da bi Out CLB\n\n");
}
