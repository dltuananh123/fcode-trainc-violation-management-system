#include "violation.h"
#include "fileio.h"
#include "member.h"
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
static double calculate_fine(int member_role) {
  if (member_role == MEMBER_ROLE_MEMBER) {
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
static int select_violation_reason(int *reason) {
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
 * @param member_name The member's full name for display.
 * @return 1 if confirmed, 0 if cancelled.
 */
static int confirm_out_clb(const char *member_name) {
  char confirm[4];
  printf("[XAC NHAN] Ban co chac chan muon Out CLB thanh vien %s? (y/n): ",
         member_name);
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
static void handle_violence(AppDatabase *db, Member *member, Violation *v) {
  (void)db; /* save happens in violation_record() after return */
  v->fine = 0.0;
  v->penalty = PENALTY_OUT_CLB;

  printf("\n[CANH BAO] Vi pham BAO LUC!\n");
  printf("  Thanh vien: %s (%s)\n", member->fullName, member->studentId);
  printf("  Hinh thuc xu ly: OUT CLB (khong phat tien)\n");

  if (confirm_out_clb(member->fullName)) {
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
static void handle_absent(AppDatabase *db, Member *member, Violation *v) {
  v->fine = calculate_fine(member->role);
  v->penalty = PENALTY_FINE;

  member->consecutiveAbsences++;

  printf("[THONG BAO] So buoi vang lien tiep cua %s: %d\n", member->fullName,
         member->consecutiveAbsences);

  /* Check Out CLB threshold (Story 3.2) */
  violation_check_out_threshold(db, member);
}

/* ============================================================
 * Story 3.1 — Record Violation
 * ============================================================ */

int violation_record(AppDatabase *db) {
  if (db == NULL) return -1;

  /* Check violation capacity */
  if (db->violationCount >= MAX_VIOLATIONS) {
    printf("[LOI] Da dat gioi han so luong vi pham (%d)\n", MAX_VIOLATIONS);
    return -1;
  }

  printf("\nGHI NHAN VI PHAM\n");

  /* Step 1: Find member by MSSV */
  char student_id[MAX_MSSV_LEN];
  printf("Nhap MSSV thanh vien: ");
  read_string(student_id, MAX_MSSV_LEN);

  int member_idx = member_find_by_id(db, student_id);
  if (member_idx == -1) {
    printf("[LOI] Khong tim thay thanh vien voi MSSV: %s\n", student_id);
    return -1;
  }

  Member *member = &db->members[member_idx];

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
  if (select_violation_reason(&reason) != 0) {
    return -1;
  }

  /* Step 3: Create violation record */
  Violation new_violation;
  memset(&new_violation, 0, sizeof(Violation));

  strncpy(new_violation.studentId, student_id, MAX_MSSV_LEN - 1);
  new_violation.reason = reason;
  new_violation.violationTime = time(NULL);
  new_violation.isPaid = 0;

  /* Step 4: Handle by reason type */
  switch (reason) {
    case REASON_VIOLENCE:
      handle_violence(db, member, &new_violation);
      break;

    case REASON_ABSENT:
      handle_absent(db, member, &new_violation);
      break;

    case REASON_NO_JACKET:
    case REASON_NO_ACTIVITY:
    default:
      /* Non-absent, non-violence: calculate fine, reset absence streak */
      new_violation.fine = calculate_fine(member->role);
      new_violation.penalty = PENALTY_FINE;
      member->consecutiveAbsences = 0;
      break;
  }

  /* Step 5: Optional note */
  printf("Ghi chu (Enter de bo qua): ");
  read_string(new_violation.note, MAX_NOTE_LEN);

  /* Step 6: Add violation to database */
  db->violations[db->violationCount++] = new_violation;

  /* Step 7: Update member stats */
  member->violationCount++;
  member->totalFine += new_violation.fine;

  /* Step 8: Persist to files */
  if (fileio_save_violations(db) != 0) {
    printf("[LOI] Khong the luu du lieu vi pham\n");
    /* Rollback in-memory changes */
    db->violationCount--;
    member->violationCount--;
    member->totalFine -= new_violation.fine;
    return -1;
  }

  if (fileio_save_members(db) != 0) {
    printf("[LOI] Khong the luu du lieu thanh vien\n");
    return -1;
  }

  /* Step 9: Display confirmation */
  char time_buf[20];
  format_time(new_violation.violationTime, time_buf, sizeof(time_buf));

  printf("\n[OK] Ghi nhan vi pham thanh cong\n");
  printf("  Thanh vien: %s (%s)\n", member->fullName, member->studentId);
  printf("  Ly do: %s\n", reason_name(new_violation.reason));
  printf("  Thoi gian: %s\n", time_buf);

  if (new_violation.penalty == PENALTY_OUT_CLB) {
    printf("  Xu ly: OUT CLB\n");
  } else {
    printf("  Tien phat: %.0f VND\n", new_violation.fine);
  }

  return 0;
}

/* ============================================================
 * Story 3.2 — Out CLB Warning & Enforcement
 * ============================================================ */

int violation_check_out_threshold(AppDatabase *db, Member *member) {
  if (db == NULL || member == NULL) return -1;

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

    if (confirm_out_clb(member->fullName)) {
      member->isActive = STATUS_OUT_CLB;
      fileio_save_members(db);
      printf("[OK] Thanh vien %s da bi Out CLB\n", member->fullName);
      return 1;
    }
  }

  return 0;
}

void violation_check_all_out_clb(AppDatabase *db) {
  if (db == NULL) return;

  printf("\nKIEM TRA NGUONG OUT CLB\n");
  printf("+------+------------------+---------+-----------+\n");
  printf("| MSSV | Ho va ten        | Vang LT | Trang thai|\n");
  printf("+------+------------------+---------+-----------+\n");

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

      printf("| %-4s | %-16s | %-7d | %-9s |\n", m->studentId, m->fullName,
             m->consecutiveAbsences, status);
      found++;
    }
  }

  printf("+------+------------------+---------+-----------+\n");

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
