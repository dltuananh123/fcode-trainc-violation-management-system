#include "violation.h"
#include "auth.h"
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

static double calculateFine(int memberRole) {
  if (memberRole == MEMBER_ROLE_MEMBER) {
    return 20000.0;
  }
  return 50000.0;
}

static int selectViolationReason(int *reason) {
  printf("\nChon ly do vi pham:\n");
  printf("  0. Khong mac ao CLB\n");
  printf("  1. Vang hop\n");
  printf("  2. Khong tham gia hoat dong\n");
  printf("  3. Bao luc\n");
  printf("Nhap lua chon: ");

  if (readInt(reason) != 1) {
    printf("[LOI] Lua chon khong hop le\n");
    return -1;
  }

  if (*reason < REASON_NO_JACKET || *reason > REASON_VIOLENCE) {
    printf("[LOI] Ly do vi pham khong hop le\n");
    return -1;
  }

  return 0;
}

static int confirmOutClb(const char *memberName) {
  char confirm[4];
  printf("[XAC NHAN] Ban co chac chan muon Out CLB thanh vien %s? (y/n): ",
         memberName);
  readString(confirm, sizeof(confirm));

  if (confirm[0] == 'y' || confirm[0] == 'Y') {
    return 1;
  }
  printf("[THONG BAO] Da huy thao tac Out CLB\n");
  return 0;
}

static void handleViolence(AppDatabase *db, Member *member, Violation *v) {
  (void)db;
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

  member->consecutiveAbsences = 0;
}

static void handleAbsent(AppDatabase *db, Member *member, Violation *v) {
  v->fine = calculateFine(member->role);
  v->penalty = PENALTY_FINE;

  member->consecutiveAbsences++;

  printf("[THONG BAO] So buoi vang lien tiep cua %s: %d\n", member->fullName,
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

static const char *paymentStatusName(const Violation *v) {
  if (v->penalty == PENALTY_OUT_CLB) {
    return "OUT CLB";
  }
  return v->isPaid ? "Da thu" : "Chua thu";
}

static void printViolationTableHeader(void) {
  printf("\nDANH SACH VI PHAM\n");
  printf("+------------+----------------------+--------------+----------------------+------------------+------------+------------+\n");
  printf("| MSSV       | Ho va ten            | Ban          | Ly do                | Thoi gian        | Tien phat  | Trang thai |\n");
  printf("+------------+----------------------+--------------+----------------------+------------------+------------+------------+\n");
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

  printf("| %-10.10s | %-20.20s | %-12.12s | %-20.20s | %-16s | %-10.0f | %-10.10s |\n",
         v->studentId, memberName, team, reasonName(v->reason), timeBuf,
         v->fine, paymentStatusName(v));
}

static int violationMatchesTeam(const AppDatabase *db, const Violation *v,
                                int expectedTeam) {
  const Member *member = findMemberForViolation(db, v);
  return member != NULL && member->team == expectedTeam;
}

static int violationMatchesPayment(const Violation *v, int paymentFilter) {
  if (paymentFilter == 0) {
    return v->isPaid != 0;
  }
  if (paymentFilter == 1) {
    return v->isPaid == 0;
  }
  return 0;
}

static void displayViolationsByFilter(const AppDatabase *db, int filterType,
                                      int filterValue) {
  int found = 0;

  printViolationTableHeader();
  for (int i = 0; i < db->violationCount; i++) {
    const Violation *v = &db->violations[i];
    int match = 0;

    switch (filterType) {
    case 0:
      match = 1;
      break;
    case 1:
      match = violationMatchesTeam(db, v, filterValue);
      break;
    case 2:
      match = (v->reason == filterValue);
      break;
    case 3:
      match = violationMatchesPayment(v, filterValue);
      break;
    default:
      break;
    }

    if (match) {
      printViolationRow(findMemberForViolation(db, v), v);
      found++;
    }
  }

  printf("+------------+----------------------+--------------+----------------------+------------------+------------+------------+\n");
  if (found == 0) {
    printf("Khong co vi pham nao\n");
  } else {
    printf("Tong: %d vi pham\n", found);
  }
}

/* ============================================================
 * Story 3.1 - Record Violation
 * ============================================================ */

int violationRecord(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  if (db->violationCount >= MAX_VIOLATIONS) {
    printf("[LOI] Da dat gioi han so luong vi pham (%d)\n", MAX_VIOLATIONS);
    return -1;
  }

  printf("\nGHI NHAN VI PHAM\n");

  char input[MAX_NAME_LEN];
  printf("Nhap MSSV hoac ten thanh vien: ");
  readString(input, sizeof(input));

  /* First try exact MSSV match */
  int memberIdx = memberFindById(db, input);

  if (memberIdx == -1) {
    /* No MSSV match — search by name */
    int indices[MAX_MEMBERS];
    int count = memberSearchByName(db, input, indices, MAX_MEMBERS);

    if (count == 0) {
      printf("[LOI] Khong tim thay thanh vien voi MSSV/ten: %s\n", input);
      return -1;
    }

    if (count == 1) {
      memberIdx = indices[0];
    } else {
      printf("\nTim thay %d thanh vien:\n", count);
      for (int i = 0; i < count; i++) {
        Member *m = &db->members[indices[i]];
        printf("  %d. %s - %s (%s)\n", i + 1, m->studentId, m->fullName,
               teamName(m->team));
      }
      printf("Chon STT (1-%d, 0 de huy): ", count);
      int choice;
      if (readInt(&choice) != 1 || choice < 1 || choice > count) {
        printf("[THONG BAO] Da huy ghi nhan vi pham\n");
        return -1;
      }
      memberIdx = indices[choice - 1];
    }
  }

  Member *member = &db->members[memberIdx];
  Member oldMemberState = *member;
  int oldViolationCount = db->violationCount;

  if (member->isActive != STATUS_ACTIVE) {
    printf("[LOI] Thanh vien %s da bi Out CLB, khong the ghi nhan vi pham\n",
           member->fullName);
    return -1;
  }

  printf("  Thanh vien: %s\n", member->fullName);
  printf("  Ban: %s\n", teamName(member->team));
  printf("  Chuc vu: %s\n", memberRoleName(member->role));

  int reason;
  if (selectViolationReason(&reason) != 0) {
    return -1;
  }

  Violation newViolation;
  memset(&newViolation, 0, sizeof(Violation));

  strncpy(newViolation.studentId, studentId, MAX_MSSV_LEN - 1);
  newViolation.studentId[MAX_MSSV_LEN - 1] = '\0';
  newViolation.reason = reason;
  newViolation.violationTime = time(NULL);
  newViolation.isPaid = 0;

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

  printf("Ghi chu (Enter de bo qua): ");
  readString(newViolation.note, MAX_NOTE_LEN);

  db->violations[db->violationCount++] = newViolation;
  member->violationCount++;
  member->totalFine += newViolation.fine;

  if (fileioSaveViolations(db) != 0) {
    printf("[LOI] Khong the luu du lieu vi pham\n");
    db->violationCount = oldViolationCount;
    *member = oldMemberState;
    return -1;
  }

  if (fileioSaveMembers(db) != 0) {
    db->violationCount = oldViolationCount;
    *member = oldMemberState;
    if (fileioSaveViolations(db) != 0) {
      printf("[LOI] Khong the phuc hoi du lieu vi pham sau loi luu thanh vien.\n");
      return -1;
    }
    printf("[LOI] Khong the luu du lieu thanh vien\n");
    return -1;
  }

  char timeBuf[20];
  formatTime(newViolation.violationTime, timeBuf, sizeof(timeBuf));

  printf("\n[OK] Ghi nhan vi pham thanh cong\n");
  printf("  Thanh vien: %s (%s)\n", member->fullName, member->studentId);
  printf("  Ly do: %s\n", reasonName(newViolation.reason));
  printf("  Thoi gian: %s\n", timeBuf);

  if (newViolation.penalty == PENALTY_OUT_CLB) {
    printf("  Xu ly: OUT CLB\n");
  } else {
    printf("  Tien phat: %.0f VND\n", newViolation.fine);
  }

  return 0;
}

/* ============================================================
 * Story 3.2 - Out CLB Warning & Enforcement
 * ============================================================ */

int violationCheckOutThreshold(AppDatabase *db, Member *member) {
  if (db == NULL || member == NULL) {
    return -1;
  }

  if (member->consecutiveAbsences == 3) {
    printf("\n[CANH BAO] Thanh vien %s da vang 3 buoi lien tiep!\n",
           member->fullName);
    printf("  Neu vang them 1 buoi nua se bi Out CLB.\n");
    return 0;
  }

  if (member->consecutiveAbsences >= 4) {
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
  printf("  QUA NGUONG: Vang qua 3 buoi, cho BCN xu ly\n");
  printf("  Out CLB   : Da bi Out CLB\n\n");
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
    printf("[LOI] Chua dang nhap!\n");
    return;
  }

  printf("\n--- DANH SACH VI PHAM CUA BAN ---\n");
  printf("+------------------+---------------------------+------------+---------------+\n");
  printf("| Thoi gian        | Ly do                     | Tien phat  | Trang thai    |\n");
  printf("+------------------+---------------------------+------------+---------------+\n");

  int found = 0;
  for (int i = 0; i < db->violationCount; i++) {
    Violation *v = &db->violations[i];
    if (strcmp(v->studentId, session->studentId) == 0) {
      char timeBuf[20];
      formatTime(v->violationTime, timeBuf, sizeof(timeBuf));

      const char *status = v->isPaid ? "Da thu" : "CHUA THU";

      printf("| %-16s | %-25s | %-10.0f | %-13s |\n", timeBuf,
             reasonName(v->reason), v->fine, status);
      found++;
    }
  }

  printf("+------------------+---------------------------+------------+---------------+\n");
  if (found == 0) {
    printf("[THONG BAO] Ban khong co vi pham nao.\n");
  } else {
    printf("Tong cong: %d vi pham.\n", found);
  }
}

void violationViewFines(AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  Account *session = authGetSession();
  if (session == NULL) {
    printf("[LOI] Chua dang nhap!\n");
    return;
  }

  printf("\n--- CAC KHOAN PHAT CHUA DONG ---\n");

  double total = 0.0;
  int found = 0;
  for (int i = 0; i < db->violationCount; i++) {
    Violation *v = &db->violations[i];
    if (strcmp(v->studentId, session->studentId) == 0 && v->isPaid == 0 &&
        v->fine > 0) {
      char timeBuf[20];
      formatTime(v->violationTime, timeBuf, sizeof(timeBuf));
      printf("%d. %s - %s: %.0f VND\n", ++found, timeBuf,
             reasonName(v->reason), v->fine);
      total += v->fine;
    }
  }

  if (found == 0) {
    printf("[THONG BAO] Tuyet voi! Ban khong no khoan phat nao.\n");
  } else {
    printf("------------------------------------\n");
    printf("TONG SO TIEN CAN DONG: %.0f VND\n", total);
  }
}

int violationMarkPaid(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  Account *session = authGetSession();
  if (session == NULL) {
    printf("[LOI] Ban phai dang nhap de thuc hien\n");
    return -1;
  }
  if (session->role != ACCOUNT_ROLE_BCN) {
    printf("[LOI] Chi BCN moi co quyen thu tien phat\n");
    return -1;
  }

  printf("\n--- THU TIEN PHAT ---\n");
  char targetId[MAX_MSSV_LEN];
  printf("Nhap MSSV thanh vien: ");
  readString(targetId, MAX_MSSV_LEN);

  int memberIdx = memberFindById(db, targetId);
  if (memberIdx == -1) {
    printf("[LOI] Khong tim thay thanh vien voi MSSV: %s\n", targetId);
    return -1;
  }

  Member *m = &db->members[memberIdx];
  int unpaidIndices[MAX_VIOLATIONS];
  int unpaidCount = 0;

  printf("\nDanh sach vi pham chua dong phat cua %s:\n", m->fullName);
  for (int i = 0; i < db->violationCount; i++) {
    Violation *v = &db->violations[i];
    if (strcmp(v->studentId, m->studentId) == 0 && v->isPaid == 0 &&
        v->fine > 0) {
      unpaidIndices[unpaidCount] = i;
      char timeBuf[20];
      formatTime(v->violationTime, timeBuf, sizeof(timeBuf));
      printf("  %d. %s - %s (%.0f VND)\n", unpaidCount + 1, timeBuf,
             reasonName(v->reason), v->fine);
      unpaidCount++;
    }
  }

  if (unpaidCount == 0) {
    printf("\n[THONG BAO] Thanh vien nay khong co khoan phat nao can thu.\n");
    return 0;
  }

  printf("\nChon STT de danh dau Da Thu (1-%d, 0 de Huy): ", unpaidCount);
  int choice;
  if (readInt(&choice) != 1 || choice < 0 || choice > unpaidCount) {
    printf("[LOI] Lua chon khong hop le.\n");
    return -1;
  }
  if (choice == 0) {
    printf("[THONG BAO] Da huy thao tac.\n");
    return 0;
  }

  int realIdx = unpaidIndices[choice - 1];
  int oldPaidState = db->violations[realIdx].isPaid;
  double oldTotalFine = m->totalFine;
  db->violations[realIdx].isPaid = 1;

  double newTotal = 0.0;
  for (int i = 0; i < db->violationCount; i++) {
    Violation *v = &db->violations[i];
    if (strcmp(v->studentId, m->studentId) == 0 && v->isPaid == 0) {
      newTotal += v->fine;
    }
  }
  m->totalFine = newTotal;

  if (fileioSaveViolations(db) != 0) {
    db->violations[realIdx].isPaid = oldPaidState;
    m->totalFine = oldTotalFine;
    printf("[LOI] Khong the luu du lieu sau khi thu tien.\n");
    return -1;
  }

  if (fileioSaveMembers(db) != 0) {
    db->violations[realIdx].isPaid = oldPaidState;
    m->totalFine = oldTotalFine;
    if (fileioSaveViolations(db) != 0) {
      printf("[LOI] Khong the phuc hoi du lieu vi pham sau loi luu thanh vien.\n");
      return -1;
    }
    printf("[LOI] Khong the luu du lieu sau khi thu tien.\n");
    return -1;
  }

  printf("[OK] Da thu tien thanh cong! Tong no con lai: %.0f VND\n",
         m->totalFine);
  return 0;
}

/* ============================================================
 * Story 3.4 - View All Violations with Filters
 * ============================================================ */

void violationViewAllFiltered(AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  Account *session = authGetSession();
  if (session == NULL) {
    printf("[LOI] Ban phai dang nhap de thuc hien\n");
    return;
  }
  if (session->role != ACCOUNT_ROLE_BCN) {
    printf("[LOI] Chi BCN moi co quyen xem toan bo vi pham\n");
    return;
  }

  if (db->violationCount == 0) {
    printf("Khong co vi pham nao\n");
    return;
  }

  int choice;
  printf("\nXEM DANH SACH VI PHAM\n");
  printf("1. Xem tat ca\n");
  printf("2. Loc theo ban\n");
  printf("3. Loc theo ly do\n");
  printf("4. Loc theo trang thai thu tien\n");
  printf("0. Quay lai\n");
  printf("Nhap lua chon: ");

  if (readInt(&choice) != 1) {
    printf("[LOI] Lua chon khong hop le\n");
    return;
  }

  switch (choice) {
  case 0:
    return;
  case 1:
    displayViolationsByFilter(db, 0, 0);
    break;
  case 2: {
    int team;
    printf("Chon ban (0-Hoc thuat, 1-Ke hoach, 2-Nhan su, 3-Truyen thong): ");
    if (readInt(&team) != 1 || team < TEAM_ACADEMIC || team > TEAM_MEDIA) {
      printf("[LOI] Ban khong hop le\n");
      return;
    }
    displayViolationsByFilter(db, 1, team);
    break;
  }
  case 3: {
    int reason;
    printf("Chon ly do (0-Khong mac ao CLB, 1-Vang hop, 2-Khong tham gia hoat dong, 3-Bao luc): ");
    if (readInt(&reason) != 1 || reason < REASON_NO_JACKET ||
        reason > REASON_VIOLENCE) {
      printf("[LOI] Ly do vi pham khong hop le\n");
      return;
    }
    displayViolationsByFilter(db, 2, reason);
    break;
  }
  case 4: {
    int payment;
    printf("Chon trang thai (0-Da thu, 1-Chua thu): ");
    if (readInt(&payment) != 1 || payment < 0 || payment > 1) {
      printf("[LOI] Trang thai thu tien khong hop le\n");
      return;
    }
    displayViolationsByFilter(db, 3, payment);
    break;
  }
  default:
    printf("[LOI] Lua chon khong hop le\n");
    break;
  }
}

/* ============================================================
 * Story 4.4 - Search Violations by Date Range
 * ============================================================ */

void violationSearchByDate(AppDatabase *db) {
  if (db == NULL) {
    return;
  }

  if (db->violationCount == 0) {
    printf("[THONG BAO] Khong co vi pham nao trong du lieu\n");
    return;
  }

  char startBuf[16];
  char endBuf[16];
  time_t start;
  time_t end;

  printf("\nTIM KIEM VI PHAM THEO KHOANG NGAY\n");

  printf("Nhap ngay bat dau (dd/mm/yyyy): ");
  readString(startBuf, sizeof(startBuf));
  if (parseDate(startBuf, &start, 0) != 1) {
    printf("[LOI] Dinh dang ngay khong hop le (dd/mm/yyyy)\n");
    return;
  }

  printf("Nhap ngay ket thuc (dd/mm/yyyy): ");
  readString(endBuf, sizeof(endBuf));
  if (parseDate(endBuf, &end, 1) != 1) {
    printf("[LOI] Dinh dang ngay khong hop le (dd/mm/yyyy)\n");
    return;
  }

  if (start > end) {
    printf("[LOI] Ngay bat dau phai truoc hoac bang ngay ket thuc\n");
    return;
  }

  printViolationTableHeader();

  int found = 0;
  for (int i = 0; i < db->violationCount; i++) {
    const Violation *v = &db->violations[i];
    if (v->violationTime >= start && v->violationTime <= end) {
      printViolationRow(findMemberForViolation(db, v), v);
      found++;
    }
  }

  printf("+------------+----------------------+--------------+----------------------+------------------+------------+------------+\n");
  if (found == 0) {
    printf("Khong co vi pham nao trong khoang ngay nay\n");
  } else {
    printf("Tong: %d vi pham\n", found);
  }
}
