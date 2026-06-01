#include "auth.h"
#include "fileio.h"
#include "member.h"
#include "report.h"
#include "types.h"
#include "ui.h"
#include "utils.h"
#include "validate.h"
#include "violation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global database state */
static AppDatabase gDb;

/* Menu content width (fits standard terminal) */
#define MENU_CONTENT_W 68

/* ============================================================
 * MENU FUNCTIONS
 * ============================================================ */

/* Member menu (for regular members and leaders) */
static void memberMenu(void) {
  int choice;
  do {
    uiClear();
    /* Top border */
    printf(COLOR_BLUE BOX_TL);
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf(BOX_TR COLOR_RESET "\n");
    /* Title */
    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf(COLOR_DIM " MENU THANH VIEN ");
    int _breadcrumbLen = (int)strlen(" MENU THANH VIEN ");
    for (int i = _breadcrumbLen; i < MENU_CONTENT_W; i++) {
      printf(" ");
    }
    printf(COLOR_RESET);
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");
    /* Separator under title */
    printf(COLOR_BLUE "\xE2\x95\xA0");
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf("\xE2\x95\xA3" COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  %-66s", "1. Xem profile ca nhan");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  %-66s", "2. Xem lich su vi pham");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  %-66s", "3. Xem tong tien phat con no");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  %-66s", "4. Lich su nop tien phat");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  %-66s", "5. Thong ke ca nhan");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  %-66s", "6. Doi mat khau");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf(COLOR_DIM "  %-66s" COLOR_RESET, "0. Dang xuat");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    choice = readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, 0, 6);

    switch (choice) {
    case 1:
      memberViewProfile(&gDb);
      break;
    case 2:
      violationViewOwn(&gDb);
      break;
    case 3:
      violationViewFines(&gDb);
      break;
    case 4:
      violationViewPaymentHistory(&gDb);
      break;
    case 5:
      memberViewStats(&gDb);
      break;
    case 6:
      authChangePassword(&gDb);
      break;
    case 0:
      authLogout(&gDb);
      break;
    default:
      break;
    }

    if (choice != 0) {
      uiPause();
    }
  } while (choice != 0);
}

/* BCN menu (for admins) */
static void adminMenu(void) {
  int choice;
  do {
    uiClear();
    /* Top border */
    printf(COLOR_BLUE BOX_TL);
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf(BOX_TR COLOR_RESET "\n");
    /* Title */
    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf(COLOR_DIM " MENU BAN CHU NHIEM ");
    int _breadcrumbLen = (int)strlen(" MENU BAN CHU NHIEM ");
    for (int i = _breadcrumbLen; i < MENU_CONTENT_W; i++) {
      printf(" ");
    }
    printf(COLOR_RESET);
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");
    /* Separator under title */
    printf(COLOR_BLUE "\xE2\x95\xA0");
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf("\xE2\x95\xA3" COLOR_RESET "\n");

    /* Menu rows helper - build content then print with borders */
    char row[128];
#define PRINT_MENU_ROW(n1, t1, n2, t2)                                         \
  do {                                                                         \
    sprintf(row, "  %2s. %-27s  %2s. %-29s", #n1, t1, #n2, t2);                \
    printf(COLOR_BLUE BOX_V COLOR_RESET "%s" COLOR_BLUE BOX_V COLOR_RESET      \
                                        "\n",                                  \
           row);                                                               \
  } while (0)

    printf(COLOR_GREEN);
    PRINT_MENU_ROW(1, "Them thanh vien moi", 9, "Sap xep theo so lan VP");
    printf(COLOR_GREEN);
    PRINT_MENU_ROW(2, "Sua thong tin TV", 10, "Xuat bao cao");
    printf(COLOR_GREEN);
    PRINT_MENU_ROW(3, "Tim kiem & xem CT TV", 11, "Tim kiem theo ngay");
    printf(COLOR_GREEN);
    PRINT_MENU_ROW(4, "Ghi nhan vi pham", 12, "Xem profile ca nhan");
    printf(COLOR_GREEN);
    PRINT_MENU_ROW(5, "Danh dau da thu tien", 13, "Xem DS TV dang hoat dong");
    printf(COLOR_GREEN);
    PRINT_MENU_ROW(6, "Xem DS vi pham", 14, "Doi mat khau");
    printf(COLOR_GREEN);
    PRINT_MENU_ROW(7, "Thong ke tien phat theo ban", 15, "Reset mat khau TV");
    printf(COLOR_GREEN);
    PRINT_MENU_ROW(8, "Kiem tra nguong Out CLB", 16, "Xem Dashboard ky luat");
    printf(COLOR_GREEN);
    PRINT_MENU_ROW(17, "Xem lich su VP theo MSSV", 18, "Kick/Khoi phuc TV");
    printf(COLOR_GREEN);
    PRINT_MENU_ROW(19, "Xem DS thanh vien da kick", 20, "Xem nhat ky he thong");

    /* Row 11 - Thoat */
    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf(COLOR_DIM "  %-66s" COLOR_RESET, "0. Dang xuat");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    choice = readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, 0, 20);

    switch (choice) {
    case 1:
      memberAdd(&gDb);
      break;
    case 2:
      memberEdit(&gDb);
      break;
    case 3:
      memberSearchDetails(&gDb);
      break;
    case 4:
      violationRecord(&gDb);
      break;
    case 5:
      violationMarkPaid(&gDb);
      break;
    case 6:
      violationViewAllFiltered(&gDb);
      break;
    case 7:
      reportTeamStats(&gDb);
      break;
    case 8:
      violationCheckAllOutClb(&gDb);
      break;
    case 9:
      reportSortMembersByViolations(&gDb);
      break;
    case 10:
      reportExportTxt(&gDb);
      break;
    case 11:
      violationSearchByDate(&gDb);
      break;
    case 12:
      memberViewProfile(&gDb);
      break;
    case 13:
      memberListAll(&gDb);
      break;
    case 14:
      authChangePassword(&gDb);
      break;
    case 15: {
      printf("Nhap MSSV can reset mat khau (0 de quay lai): ");
      char targetId[MAX_MSSV_LEN];
      readString(targetId, MAX_MSSV_LEN);
      trimSpaces(targetId);
      if (strcmp(targetId, "0") == 0) {
        printf(ERR_INFO "Da huy thao tac.\n");
      } else if (authResetPassword(&gDb, targetId) == 0) {
        Account *session = authGetSession();
        if (session != NULL) {
          logSystemAction(session->studentId, "Reset mat khau", targetId);
        }
      }
      break;
    }
    case 16:
      reportDashboard(&gDb);
      break;
    case 17:
      violationViewByMSSV(&gDb);
      break;
    case 18:
      memberKickOrRestore(&gDb);
      break;
    case 19:
      memberViewKicked(&gDb);
      break;
    case 20:
      viewSystemLogs();
      break;
    case 0:
      authLogout(&gDb);
      break;
    default:
      break;
    }

    if (choice != 0) {
      uiPause();
    }
  } while (choice != 0);
}

/* ============================================================
 * MAIN ENTRY POINT
 * ============================================================ */

int main(void) {
  /* Initialize UI (enable ANSI colors on Windows, set UTF-8) */
  uiInit();
  srand((unsigned int)time(NULL));

  /* Display FCODE Firebird logo on startup */
  uiDrawLogo();

  /* Load data from files */
  if (fileioLoadAll(&gDb) != 0) {
    printf(ERR_LOI "Khong the tai du lieu! "
                   "Vui long kiem tra thu muc data/\n");
    return 1;
  }

  printf(ERR_OK "Tai du lieu thanh cong!\n");
  printf(COLOR_GRAY "  - Thanh vien: %d/%d\n", gDb.memberCount, MAX_MEMBERS);
  printf("  - Vi pham: %d\n" COLOR_RESET, gDb.violationCount);

  /* Main application loop */
  while (1) {
    if (authLogin(&gDb) != 0) {
      break;
    }

    uiClear();

    Account *session = authGetSession();
    if (session != NULL) {
      if (session->role == ACCOUNT_ROLE_BCN) {
        adminMenu();
      } else {
        memberMenu();
      }
    }
  }

  printf(ERR_OK "Thoat chuong trinh. Hen gap lai!\n");
  return 0;
}
