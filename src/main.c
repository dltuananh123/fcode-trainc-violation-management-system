#include "auth.h"
#include "fileio.h"
#include "member.h"
#include "report.h"
#include "types.h"
#include "ui.h"
#include "utils.h"
#include "validate.h"
#include "violation.h"
#include <ctype.h>
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
static void setupFirstRun(AppDatabase *db);

/* Member menu (for regular members and leaders) */
static void memberMenu(void) {
  int choice;
  do {
    uiDrawMenuBoxBegin("MENU THANH VIEN");

    uiDrawMenuRowFmt("  " COLOR_YELLOW "[1]" COLOR_RESET
                     " Xem profile ca nhan");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[2]" COLOR_RESET
                     " Xem lich su vi pham");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[3]" COLOR_RESET
                     " Xem tong tien phat con no");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[4]" COLOR_RESET
                     " Xem lich su nop tien phat");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[5]" COLOR_RESET " Thong ke ca nhan");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[6]" COLOR_RESET
                     " Xem nguong Out CLB & Vi pham");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[7]" COLOR_RESET " Doi mat khau");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[0]" COLOR_RESET " Dang xuat");

    uiDrawMenuBoxEnd();

    choice = readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, 0, 7);

    switch (choice) {
    case 1:
      memberViewProfile(&gDb);
      uiPause();
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
      uiPause();
      break;
    case 6:
      memberViewOutThreshold(&gDb);
      break;
    case 7:
      authChangePassword(&gDb, 0);
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
    uiDrawMenuBoxBegin("MENU BAN CHU NHIEM");

    uiDrawMenuRowFmt("  " COLOR_YELLOW "[1]" COLOR_RESET " Quan ly thanh vien");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[2]" COLOR_RESET " Quan ly vi pham");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[3]" COLOR_RESET " Quan ly bao cao");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[4]" COLOR_RESET " Quan ly he thong");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[5]" COLOR_RESET
                     " Xem profile ca nhan");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[0]" COLOR_RESET " Dang xuat");

    uiDrawMenuBoxEnd();

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
    uiDrawMenuBoxBegin("MENU BAN CHU NHIEM -> [1] QUAN LY THANH VIEN");

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

    uiDrawMenuBoxEnd();

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
      break;
    case 5:
      memberKickOrRestore(&gDb);
      uiPause();
      break;
    case 6:
      memberViewKicked(&gDb);
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
    uiDrawMenuBoxBegin("MENU BAN CHU NHIEM -> [2] QUAN LY VI PHAM");

    uiDrawMenuRowFmt("  " COLOR_YELLOW "[1]" COLOR_RESET
                     " Ghi nhan vi pham moi");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[2]" COLOR_RESET
                     " Danh dau da thu tien phat");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[3]" COLOR_RESET
                     " Xem danh sach vi pham");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[4]" COLOR_RESET
                     " Xem lich su vi pham theo thanh vien");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[5]" COLOR_RESET
                     " Tim kiem vi pham theo ngay");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[6]" COLOR_RESET
                     " Kiem tra nguong Out CLB");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[7]" COLOR_RESET " Huy vi pham (Void)");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[8]" COLOR_RESET
                     " Import vi pham tu file CSV");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[0]" COLOR_RESET
                     " Quay lai menu chinh");

    uiDrawMenuBoxEnd();

    choice = readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, 0, 8);

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
      break;
    case 4:
      violationViewByMSSV(&gDb);
      break;
    case 5:
      violationSearchByDate(&gDb);
      break;
    case 6:
      violationCheckAllOutClb(&gDb);
      uiPause();
      break;
    case 7:
      violationVoid(&gDb);
      uiPause();
      break;
    case 8:
      violationImportCsv(&gDb);
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
    uiDrawMenuBoxBegin("MENU BAN CHU NHIEM -> [3] QUAN LY BAO CAO");

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

    uiDrawMenuBoxEnd();

    choice = readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, 0, 4);

    switch (choice) {
    case 1:
      reportTeamStats(&gDb);
      uiPause();
      break;
    case 2:
      reportSortMembersByViolations(&gDb);
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
    uiDrawMenuBoxBegin("MENU BAN CHU NHIEM -> [4] QUAN LY HE THONG");

    uiDrawMenuRowFmt("  " COLOR_YELLOW "[1]" COLOR_RESET " Doi mat khau");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[2]" COLOR_RESET
                     " Xem nhat ky he thong");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[3]" COLOR_RESET " Huong dan su dung");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[4]" COLOR_RESET
                     " Thong tin phien ban");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[5]" COLOR_RESET
                     " Xuat du lieu (Export)");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[6]" COLOR_RESET
                     " Nhap du lieu (Import)");
    uiDrawMenuRowFmt("  " COLOR_YELLOW "[0]" COLOR_RESET
                     " Quay lai menu chinh");

    uiDrawMenuBoxEnd();

    choice = readMenuChoice(COLOR_CYAN "  Nhap lua chon: " COLOR_RESET, 0, 6);

    switch (choice) {
    case 1:
      authChangePassword(&gDb, 0);
      uiPause();
      break;
    case 2:
      viewSystemLogs();
      break;
    case 3:
      uiDrawHelp();
      uiPause();
      break;
    case 4:
      uiClear();
      printf("\n  FCode TrainC Violation Management System\n");
      printf("  Phien ban: v2.3\n");
      uiPause();
      break;
    case 5:
      fileioExportArchive(&gDb);
      break;
    case 6:
      fileioImportArchive(&gDb);
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

  /* Smooth UX: Small delay to let user appreciate the ASCII art */
  uiSleep(1200);

  /* Load data from files */
  if (fileioLoadAll(&gDb) != 0) {
    printf(ERR_LOI "Khong the tai du lieu! "
                   "Vui long kiem tra thu muc data/\n");
    return 1;
  }

  /* First-run check: if accounts are empty, trigger Setup Wizard */
  if (gDb.accountCount == 0) {
    setupFirstRun(&gDb);
  } else {
    printf(ERR_OK "Tai du lieu thanh cong!\n");
    printf(COLOR_GRAY "  - Thanh vien: %d/%d\n", gDb.memberCount, MAX_MEMBERS);
    printf("  - Vi pham: %d\n" COLOR_RESET, gDb.violationCount);

    /* Smooth UX: Brief pause before login screen */
    uiSleep(800);
  }

  /* Main application loop */
  while (1) {
    if (authLogin(&gDb) != 0) {
      break;
    }

    uiClear();

    Account *session = authGetSession();
    if (session != NULL) {
      if (session->role == ACCOUNT_ROLE_DIRECTOR) {
        adminMenu();
      } else {
        memberMenu();
      }
    }
  }

  printf(ERR_OK "Thoat chuong trinh. Hen gap lai!\n");
  uiPause();
  return 0;
}

static void setupFirstRun(AppDatabase *db) {
  uiClear();
  uiSetBoxWidth(MENU_BOX_W);

  /* Draw Box Header */
  printf(COLOR_BLUE BOX_TL);
  for (int i = 0; i < MENU_BOX_W - 2; i++) {
    printf(BOX_H);
  }
  printf(BOX_TR COLOR_RESET "\n");

  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD COLOR_YELLOW
         " THIET LAP HE THONG LAN DAU TIEN " COLOR_RESET);
  /* " THIET LAP HE THONG LAN DAU TIEN " is 33 characters */
  for (int i = 33; i < MENU_BOX_W - 2; i++) {
    printf(" ");
  }
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  printf(COLOR_BLUE "\xE2\x95\xA0");
  for (int i = 0; i < MENU_BOX_W - 2; i++) {
    printf(BOX_H);
  }
  printf("\xE2\x95\xA3" COLOR_RESET "\n");

  uiDrawMenuRow("  He thong chua co du lieu tai khoan.");
  uiDrawMenuRow("  Vui long khoi tao tai khoan Ban Chu Nhiem de bat dau.");
  uiDrawMenuRow("");
  uiDrawMenuRow(COLOR_DIM "  Nhap 0 de thoat." COLOR_RESET);

  printf(COLOR_BLUE BOX_BL);
  for (int i = 0; i < MENU_BOX_W - 2; i++) {
    printf(BOX_H);
  }
  printf(BOX_BR COLOR_RESET "\n");
  printf("\n");

  Member newAdmin;
  memset(&newAdmin, 0, sizeof(Member));

  /* MSSV loop */
  while (1) {
    printf(COLOR_CYAN "  [1/6] Nhap MSSV (0 de thoat): " COLOR_RESET);
    readString(newAdmin.studentId, MAX_MSSV_LEN);
    trimSpaces(newAdmin.studentId);
    mssvAutoUpper(newAdmin.studentId);
    if (strcmp(newAdmin.studentId, "0") == 0) {
      printf(ERR_INFO "Da thoat chuong trinh.\n");
      exit(0);
    }
    if (validateMSSVFormat(newAdmin.studentId)) {
      break;
    }
  }

  /* Full Name loop */
  while (1) {
    printf(COLOR_CYAN "  [2/6] Nhap Ho va Ten (0 de thoat): " COLOR_RESET);
    readString(newAdmin.fullName, MAX_NAME_LEN);
    trimSpaces(newAdmin.fullName);
    if (strcmp(newAdmin.fullName, "0") == 0) {
      printf(ERR_INFO "Da thoat chuong trinh.\n");
      exit(0);
    }
    nameAutoFix(newAdmin.fullName);
    if (validateName(newAdmin.fullName)) {
      break;
    }
  }

  /* Email loop */
  while (1) {
    printf(COLOR_CYAN "  [3/6] Nhap Email (0 de thoat): " COLOR_RESET);
    readString(newAdmin.email, MAX_EMAIL_LEN);
    trimSpaces(newAdmin.email);
    if (strcmp(newAdmin.email, "0") == 0) {
      printf(ERR_INFO "Da thoat chuong trinh.\n");
      exit(0);
    }
    /* Convert to lowercase */
    for (int i = 0; newAdmin.email[i]; i++) {
      newAdmin.email[i] = (char)tolower((unsigned char)newAdmin.email[i]);
    }
    if (validateEmail(newAdmin.email)) {
      break;
    }
  }

  /* Phone loop */
  while (1) {
    printf(COLOR_CYAN "  [4/6] Nhap So dien thoai (0 de thoat): " COLOR_RESET);
    readString(newAdmin.phone, MAX_PHONE_LEN);
    trimSpaces(newAdmin.phone);
    if (strcmp(newAdmin.phone, "0") == 0) {
      printf(ERR_INFO "Da thoat chuong trinh.\n");
      exit(0);
    }
    phoneNormalize(newAdmin.phone);
    if (validatePhone(newAdmin.phone)) {
      printf(ERR_INFO "Nha mang: %s\n", phoneCarrier(newAdmin.phone));
      break;
    }
  }

  /* Team loop */
  while (1) {
    printf(COLOR_CYAN "  [5/6] Chon ban (0-Hoc thuat, 1-Ke hoach, 2-Nhan su, "
                      "3-Truyen thong, q-Thoat): " COLOR_RESET);
    char buf[32];
    readString(buf, sizeof(buf));
    trimSpaces(buf);
    if (strcmp(buf, "q") == 0 || strcmp(buf, "exit") == 0) {
      printf(ERR_INFO "Da thoat chuong trinh.\n");
      exit(0);
    }
    if (strcmp(buf, "0") == 0 && strlen(buf) == 1) {
      newAdmin.team = TEAM_ACADEMIC;
      break;
    }
    int val = -1;
    if (sscanf(buf, "%d", &val) == 1) {
      if (val == 0) {
        printf(ERR_INFO "Da thoat chuong trinh.\n");
        exit(0);
      }
      if (val >= TEAM_ACADEMIC && val <= TEAM_MEDIA) {
        newAdmin.team = val;
        break;
      }
    }
    printf(ERR_LOI "Vui long chon tu 0 den 3 hoac q de thoat!\n");
  }

  newAdmin.role = MEMBER_ROLE_DIRECTOR;
  newAdmin.isActive = STATUS_ACTIVE;
  newAdmin.isDeleted = 0;

  /* Password loop */
  char password[MAX_PASS_LEN];
  char confirm[MAX_PASS_LEN];
  while (1) {
    printf("\n  Tieu chuan mat khau:\n");
    printf("  - Chieu dai tu 8 den 30 ky tu.\n");
    printf("  - Chua it nhat 1 chu hoa, 1 chu thuong, 1 chu so, 1 ky tu dac "
           "biet.\n");
    printf("  - Khong chua khoang trang.\n\n");

    printf(COLOR_CYAN "  [6/6] Nhap mat khau moi (0 de thoat): " COLOR_RESET);
    readPassword(password, sizeof(password));
    if (strcmp(password, "0") == 0) {
      printf(ERR_INFO "Da thoat chuong trinh.\n");
      exit(0);
    }

    if (!validatePassword(password)) {
      continue;
    }

    printf(COLOR_CYAN "  Xac nhan mat khau (0 de thoat): " COLOR_RESET);
    readPassword(confirm, sizeof(confirm));
    if (strcmp(confirm, "0") == 0) {
      printf(ERR_INFO "Da thoat chuong trinh.\n");
      exit(0);
    }

    if (strcmp(password, confirm) != 0) {
      printf(ERR_LOI "Mat khau xac nhan khong khop! Vui long nhap lai.\n");
      continue;
    }
    break;
  }

  /* Add to database */
  db->members[0] = newAdmin;
  db->memberCount = 1;

  Account newAcc;
  memset(&newAcc, 0, sizeof(Account));
  strncpy(newAcc.studentId, newAdmin.studentId, sizeof(newAcc.studentId) - 1);
  newAcc.studentId[sizeof(newAcc.studentId) - 1] = '\0';
  newAcc.role = ACCOUNT_ROLE_DIRECTOR;
  newAcc.isLocked = 0;
  newAcc.failCount = 0;
  newAcc.isDefaultPassword = 0; /* Created securely */
  generateSalt(newAcc.salt, sizeof(newAcc.salt));
  hashPassword(password, newAcc.salt, newAcc.password);

  db->accounts[0] = newAcc;
  db->accountCount = 1;

  if (fileioSaveMembers(db) != 0 || fileioSaveAccounts(db) != 0) {
    printf(ERR_LOI "Khong the ghi du lieu thiet lap xuong dia!\n");
    exit(1);
  }

  printf("\n");
  printf(ERR_OK "Khoi tao tai khoan Ban Chu Nhiem thanh cong!\n");
  printf(COLOR_BOLD "  MSSV:     " COLOR_RESET "%s\n", newAdmin.studentId);
  printf(COLOR_BOLD "  Ho ten:   " COLOR_RESET "%s\n", newAdmin.fullName);
  printf(COLOR_BOLD "  Email:    " COLOR_RESET "%s\n", newAdmin.email);
  printf(COLOR_BOLD "  Sdt:      " COLOR_RESET "%s\n", newAdmin.phone);
  printf(COLOR_BOLD "  Ban:      " COLOR_RESET "%s\n", teamName(newAdmin.team));
  printf(COLOR_BOLD "  Chuc vu:  " COLOR_RESET "Ban chu nhiem\n");
  printf("\n");
  secureZero(password, sizeof(password));
  secureZero(confirm, sizeof(confirm));

  memberRebuildIndex(db);
  uiPause();
}
