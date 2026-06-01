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
#include <time.h>

/* Global database state */
static AppDatabase gDb;

/* Menu content width (fits standard terminal) */
#define MENU_CONTENT_W 68

/* ============================================================
 * MENU FUNCTIONS
 * ============================================================ */

/* Forward declarations for menus */
static void memberMenu(void);
static void adminMenu(void);
static void memberManagementMenu(void);
static void violationManagementMenu(void);
static void reportsAndStatsMenu(void);
static void systemSettingsMenu(void);

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
    int breadcrumbLen = (int)strlen(" MENU THANH VIEN ");
    for (int i = breadcrumbLen; i < MENU_CONTENT_W; i++) {
      printf(" ");
    }
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");
    /* Separator under title */
    printf(COLOR_BLUE "\xE2\x95\xA0");
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf("\xE2\x95\xA3" COLOR_RESET "\n");

    uiDrawMenuRowFmt("  " COLOR_YELLOW "[1]" COLOR_RESET
                     " Xem profile ca nhan");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[2]" COLOR_RESET
                     " Xem lich su vi pham");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[3]" COLOR_RESET
                     " Xem tong tien phat con no");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[4]" COLOR_RESET
                     " Lich su nop tien phat");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[5]" COLOR_RESET " Thong ke ca nhan");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[6]" COLOR_RESET " Doi mat khau");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[0]" COLOR_RESET " Dang xuat");

    printf(COLOR_BLUE BOX_BL);
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf(BOX_BR COLOR_RESET "\n");

    choice = readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, 0, 6);

    switch (choice) {
    case 1:
      memberViewProfile(&gDb);
      uiPause();
      break;
    case 2:
      violationViewOwn(&gDb);
      uiPause();
      break;
    case 3:
      violationViewFines(&gDb);
      uiPause();
      break;
    case 4:
      violationViewPaymentHistory(&gDb);
      uiPause();
      break;
    case 5:
      memberViewStats(&gDb);
      uiPause();
      break;
    case 6:
      authChangePassword(&gDb);
      uiPause();
      break;
    case 0:
      authLogout(&gDb);
      break;
    default:
      break;
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
    int breadcrumbLen = (int)strlen(" MENU BAN CHU NHIEM ");
    for (int i = breadcrumbLen; i < MENU_CONTENT_W; i++) {
      printf(" ");
    }
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");
    /* Separator under title */
    printf(COLOR_BLUE "\xE2\x95\xA0");
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf("\xE2\x95\xA3" COLOR_RESET "\n");

    uiDrawMenuRowFmt("  " COLOR_YELLOW "[1]" COLOR_RESET " Quan ly thanh vien");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[2]" COLOR_RESET " Quan ly vi pham");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[3]" COLOR_RESET
                     " Bao cao va thong ke");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[4]" COLOR_RESET " Quan ly he thong");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[5]" COLOR_RESET
                     " Xem profile ca nhan");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[0]" COLOR_RESET " Dang xuat");

    printf(COLOR_BLUE BOX_BL);
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf(BOX_BR COLOR_RESET "\n");

    choice = readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, 0, 5);

    switch (choice) {
    case 1:
      memberManagementMenu();
      break;
    case 2:
      violationManagementMenu();
      break;
    case 3:
      reportsAndStatsMenu();
      break;
    case 4:
      systemSettingsMenu();
      break;
    case 5:
      memberViewProfile(&gDb);
      uiPause();
      break;
    case 0:
      authLogout(&gDb);
      break;
    default:
      break;
    }
  } while (choice != 0);
}

/* Category Menu: Member Management */
static void memberManagementMenu(void) {
  int choice;
  do {
    uiClear();
    printf(COLOR_BLUE BOX_TL);
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf(BOX_TR COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf(COLOR_DIM " QUAN LY THANH VIEN ");
    int titleLen = (int)strlen(" QUAN LY THANH VIEN ");
    for (int i = titleLen; i < MENU_CONTENT_W; i++) {
      printf(" ");
    }
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    printf(COLOR_BLUE "\xE2\x95\xA0");
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf("\xE2\x95\xA3" COLOR_RESET "\n");

    uiDrawMenuRowFmt("  " COLOR_YELLOW "[1]" COLOR_RESET
                     " Them thanh vien moi");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[2]" COLOR_RESET
                     " Sua thong tin thanh vien");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[3]" COLOR_RESET
                     " Tim kiem va xem chi tiet thanh vien");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[4]" COLOR_RESET
                     " Xem danh sach thanh vien dang hoat dong");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[5]" COLOR_RESET
                     " Kick hoac khoi phuc thanh vien");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[6]" COLOR_RESET
                     " Xem danh sach thanh vien da kick");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[7]" COLOR_RESET
                     " Reset mat khau thanh vien");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[0]" COLOR_RESET
                     " Quay lai menu chinh");

    printf(COLOR_BLUE BOX_BL);
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf(BOX_BR COLOR_RESET "\n");

    choice = readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, 0, 7);

    switch (choice) {
    case 1:
      memberAdd(&gDb);
      uiPause();
      break;
    case 2:
      memberEdit(&gDb);
      uiPause();
      break;
    case 3:
      memberSearchDetails(&gDb);
      uiPause();
      break;
    case 4:
      memberListAll(&gDb);
      uiPause();
      break;
    case 5:
      memberKickOrRestore(&gDb);
      uiPause();
      break;
    case 6:
      memberViewKicked(&gDb);
      uiPause();
      break;
    case 7: {
      printf("Nhap MSSV can reset mat khau (0 de quay lai): ");
      char targetId[MAX_MSSV_LEN];
      readString(targetId, MAX_MSSV_LEN);
      trimSpaces(targetId);
      if (strcmp(targetId, "0") == 0) {
        printf(ERR_INFO "Da huy thao tac.\n");
      } else if (authResetPassword(&gDb, targetId) == RC_OK) {
        Account *session = authGetSession();
        if (session != NULL) {
          logSystemAction(session->studentId, "Reset mat khau", targetId);
        }
      }
      uiPause();
      break;
    }
    default:
      break;
    }
  } while (choice != 0);
}

/* Category Menu: Violation Management */
static void violationManagementMenu(void) {
  int choice;
  do {
    uiClear();
    printf(COLOR_BLUE BOX_TL);
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf(BOX_TR COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf(COLOR_DIM " QUAN LY VI PHAM ");
    int titleLen = (int)strlen(" QUAN LY VI PHAM ");
    for (int i = titleLen; i < MENU_CONTENT_W; i++) {
      printf(" ");
    }
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    printf(COLOR_BLUE "\xE2\x95\xA0");
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf("\xE2\x95\xA3" COLOR_RESET "\n");

    uiDrawMenuRowFmt("  " COLOR_YELLOW "[1]" COLOR_RESET
                     " Ghi nhan vi pham moi");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[2]" COLOR_RESET
                     " Danh dau da thu tien phat");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[3]" COLOR_RESET
                     " Xem danh sach vi pham (co loc)");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[4]" COLOR_RESET
                     " Xem lich su vi pham theo thanh vien");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[5]" COLOR_RESET
                     " Tim kiem vi pham theo khoang ngay");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[6]" COLOR_RESET
                     " Kiem tra nguong Out CLB");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[7]" COLOR_RESET " Huy vi pham (Void)");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[0]" COLOR_RESET
                     " Quay lai menu chinh");

    printf(COLOR_BLUE BOX_BL);
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf(BOX_BR COLOR_RESET "\n");

    choice = readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, 0, 7);

    switch (choice) {
    case 1:
      violationRecord(&gDb);
      uiPause();
      break;
    case 2:
      violationMarkPaid(&gDb);
      uiPause();
      break;
    case 3:
      violationViewAllFiltered(&gDb);
      uiPause();
      break;
    case 4:
      violationViewByMSSV(&gDb);
      uiPause();
      break;
    case 5:
      violationSearchByDate(&gDb);
      uiPause();
      break;
    case 6:
      violationCheckAllOutClb(&gDb);
      uiPause();
      break;
    case 7:
      violationVoid(&gDb);
      uiPause();
      break;
    default:
      break;
    }
  } while (choice != 0);
}

/* Category Menu: Reports and Stats */
static void reportsAndStatsMenu(void) {
  int choice;
  do {
    uiClear();
    printf(COLOR_BLUE BOX_TL);
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf(BOX_TR COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf(COLOR_DIM " BAO CAO VA THONG KE ");
    int titleLen = (int)strlen(" BAO CAO VA THONG KE ");
    for (int i = titleLen; i < MENU_CONTENT_W; i++) {
      printf(" ");
    }
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    printf(COLOR_BLUE "\xE2\x95\xA0");
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf("\xE2\x95\xA3" COLOR_RESET "\n");

    uiDrawMenuRowFmt("  " COLOR_YELLOW "[1]" COLOR_RESET
                     " Thong ke tien phat theo ban");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[2]" COLOR_RESET
                     " Sap xep thanh vien theo so lan vi pham");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[3]" COLOR_RESET
                     " Xuat bao cao ra file");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[4]" COLOR_RESET
                     " Xem Dashboard ky luat");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[0]" COLOR_RESET
                     " Quay lai menu chinh");

    printf(COLOR_BLUE BOX_BL);
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf(BOX_BR COLOR_RESET "\n");

    choice = readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, 0, 4);

    switch (choice) {
    case 1:
      reportTeamStats(&gDb);
      uiPause();
      break;
    case 2:
      reportSortMembersByViolations(&gDb);
      uiPause();
      break;
    case 3:
      reportExportTxt(&gDb);
      uiPause();
      break;
    case 4:
      reportDashboard(&gDb);
      uiPause();
      break;
    default:
      break;
    }
  } while (choice != 0);
}

/* Category Menu: System Settings */
static void systemSettingsMenu(void) {
  int choice;
  do {
    uiClear();
    printf(COLOR_BLUE BOX_TL);
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf(BOX_TR COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf(COLOR_DIM " QUAN LY HE THONG ");
    int titleLen = (int)strlen(" QUAN LY HE THONG ");
    for (int i = titleLen; i < MENU_CONTENT_W; i++) {
      printf(" ");
    }
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    printf(COLOR_BLUE "\xE2\x95\xA0");
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf("\xE2\x95\xA3" COLOR_RESET "\n");

    uiDrawMenuRowFmt("  " COLOR_YELLOW "[1]" COLOR_RESET " Doi mat khau");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[2]" COLOR_RESET
                     " Xem nhat ky he thong");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[3]" COLOR_RESET " Huong dan su dung");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[4]" COLOR_RESET
                     " Thong tin phien ban");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[0]" COLOR_RESET
                     " Quay lai menu chinh");

    printf(COLOR_BLUE BOX_BL);
    for (int i = 0; i < MENU_CONTENT_W; i++) {
      printf(BOX_H);
    }
    printf(BOX_BR COLOR_RESET "\n");

    choice = readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, 0, 4);

    switch (choice) {
    case 1:
      authChangePassword(&gDb);
      uiPause();
      break;
    case 2:
      viewSystemLogs();
      uiPause();
      break;
    case 3:
      uiDrawHelp();
      uiPause();
      break;
    case 4:
      uiClear();
      printf("\n  FCode TrainC Violation Management System\n");
      printf("  Phien ban: v2.3\n");
      printf("  Ban quyen thuoc ve CLB F-Code, Campus FPT HCM.\n");
      uiPause();
      break;
    default:
      break;
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
