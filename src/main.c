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

/* ============================================================
 * MENU FUNCTIONS
 * ============================================================ */

/* Member menu (for regular members and leaders) */
static void memberMenu(void) {
  int choice;
  do {
    printf("\n");
    uiDrawSeparator();
    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf(COLOR_BOLD COLOR_CYAN "  MENU THANH VIEN");
    printf("                              ");
    printf(COLOR_RESET);
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");
    uiDrawSeparator();

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  1. Xem profile ca nhan");
    printf("                            ");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  2. Xem lich su vi pham");
    printf("                          ");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  3. Xem tong tien phat con no");
    printf("                       ");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  4. Xem danh sach thanh vien");
    printf("                        ");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  5. Doi mat khau");
    printf("                                 ");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf(COLOR_DIM "  0. Dang xuat");
    printf("                                   ");
    printf(COLOR_RESET);
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    uiDrawSeparator();

    choice = readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, 0, 5);

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
      memberListAll(&gDb);
      break;
    case 5:
      authChangePassword(&gDb);
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
    printf("\n");
    uiDrawSeparator();
    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf(COLOR_BOLD COLOR_CYAN "  MENU BAN CHU NHIEM");
    for (int i = 20; i < 68; i++) {
      printf(" ");
    }
    printf(COLOR_RESET);
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");
    uiDrawSeparator();

    /* Row 1 */
    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  " COLOR_GREEN "1" COLOR_RESET ". Them thanh vien moi");
    printf("          ");
    printf(COLOR_GREEN "9" COLOR_RESET ". Sap xep theo so lan VP");
    printf("         ");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    /* Row 2 */
    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  " COLOR_GREEN "2" COLOR_RESET ". Sua thong tin TV");
    printf("             ");
    printf(COLOR_GREEN "10" COLOR_RESET ". Xuat bao cao");
    printf("                  ");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    /* Row 3 */
    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  " COLOR_GREEN "3" COLOR_RESET ". Xoa thanh vien");
    printf("               ");
    printf(COLOR_GREEN "11" COLOR_RESET ". Tim kiem theo ngay");
    printf("            ");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    /* Row 4 */
    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  " COLOR_GREEN "4" COLOR_RESET ". Ghi nhan vi pham");
    printf("             ");
    printf(COLOR_GREEN "12" COLOR_RESET ". Xem profile ca nhan");
    printf("           ");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    /* Row 5 */
    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  " COLOR_GREEN "5" COLOR_RESET ". Danh dau da thu tien");
    printf("         ");
    printf(COLOR_GREEN "13" COLOR_RESET ". Xem DS thanh vien");
    printf("             ");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    /* Row 6 */
    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  " COLOR_GREEN "6" COLOR_RESET ". Xem DS vi pham");
    printf("               ");
    printf(COLOR_GREEN "14" COLOR_RESET ". Doi mat khau");
    printf("                  ");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    /* Row 7 */
    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  " COLOR_GREEN "7" COLOR_RESET ". Thong ke tien phat theo ban");
    printf("  ");
    printf(COLOR_GREEN "15" COLOR_RESET ". Reset mat khau TV");
    printf("             ");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    /* Row 8 */
    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  " COLOR_GREEN "8" COLOR_RESET ". Kiem tra nguong Out CLB");
    printf("      ");
    printf(COLOR_GREEN "16" COLOR_RESET ". Xem Dashboard ky luat");
    printf("         ");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    /* Row 9 */
    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf("  " COLOR_GREEN "17" COLOR_RESET ". Quan ly kho luu tru");
    printf("         ");
    printf(COLOR_GREEN "18" COLOR_RESET ". Dong bang/Mo khoa TK");
    printf("          ");
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    /* Row 10 */
    printf(COLOR_BLUE BOX_V COLOR_RESET);
    printf(COLOR_DIM "  0. Dang xuat");
    for (int i = 14; i < 68; i++) {
      printf(" ");
    }
    printf(COLOR_RESET);
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

    uiDrawSeparator();

    choice = readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, 0, 18);

    switch (choice) {
    case 1:
      memberAdd(&gDb);
      break;
    case 2:
      memberEdit(&gDb);
      break;
    case 3:
      memberDelete(&gDb);
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
      printf("Nhap MSSV can reset mat khau: ");
      char targetId[MAX_MSSV_LEN];
      readString(targetId, MAX_MSSV_LEN);
      trimSpaces(targetId);
      authResetPassword(&gDb, targetId);
      break;
    }
    case 16:
      reportDashboard(&gDb);
      break;
    case 17:
      memberViewArchive(&gDb);
      break;
    case 18:
      memberFreezeAccount(&gDb);
      break;
    case 0:
      authLogout(&gDb);
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

  printf(COLOR_BOLD COLOR_BLUE "\n  F-CODE VIOLATION MANAGEMENT SYSTEM v2.0\n" COLOR_RESET);

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
