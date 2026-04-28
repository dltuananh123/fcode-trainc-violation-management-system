#include "auth.h"
#include "fileio.h"
#include "member.h"
#include "types.h"
#include "utils.h"
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
    printf("\nMENU THANH VIEN\n");
    printf("1. Xem profile ca nhan\n");
    printf("2. Xem lich su vi pham\n");
    printf("3. Xem tong tien phat con no\n");
    printf("4. Xem danh sach thanh vien\n");
    printf("0. Dang xuat\n");
    printf("Nhap lua chon: ");

    if (scanf("%d", &choice) != 1) {
      printf("[LOI] Lua chon khong hop le\n");
      while (getchar() != '\n')
        ;
      continue;
    }
    while (getchar() != '\n')
      ;

    switch (choice) {
    case 1:
      memberViewProfile(&gDb);
      break;
    case 2:
      printf("[CANH BAO] Chua cai dat\n");
      break;
    case 3:
      printf("[CANH BAO] Chua cai dat\n");
      break;
    case 4:
      memberListAll(&gDb);
      break;
    case 0:
      authLogout(&gDb);
      break;
    default:
      printf("[LOI] Lua chon khong hop le\n");
    }
  } while (choice != 0);
}

/* BCN menu (for admins) */
static void adminMenu(void) {
  int choice;
  do {
    printf("\nMENU BAN CHU NHIEM\n");
    printf("1. Them thanh vien moi\n");
    printf("2. Sua thong tin thanh vien\n");
    printf("3. Xoa thanh vien\n");
    printf("4. Ghi nhan vi pham\n");
    printf("5. Danh dau da thu tien\n");
    printf("6. Xem danh sach vi pham\n");
    printf("7. Thong ke tien phat theo ban\n");
    printf("8. Kiem tra nguong Out CLB\n");
    printf("9. Sap xep theo so lan vi pham\n");
    printf("10. Xuat bao cao\n");
    printf("11. Tim kiem theo ngay\n");
    printf("12. Xem profile ca nhan\n");
    printf("13. Xem danh sach thanh vien\n");
    printf("0. Dang xuat\n");
    printf("Nhap lua chon: ");

    if (scanf("%d", &choice) != 1) {
      printf("[LOI] Lua chon khong hop le\n");
      while (getchar() != '\n')
        ;
      continue;
    }
    while (getchar() != '\n')
      ;

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
      printf("[CANH BAO] Chua cai dat\n");
      break;
    case 6:
      printf("[CANH BAO] Chua cai dat\n");
      break;
    case 7:
      printf("[CANH BAO] Chua cai dat\n");
      break;
    case 8:
      violationCheckAllOutClb(&gDb);
      break;
    case 9:
      printf("[CANH BAO] Chua cai dat\n");
      break;
    case 10:
      printf("[CANH BAO] Chua cai dat\n");
      break;
    case 11:
      printf("[CANH BAO] Chua cai dat\n");
      break;
    case 12:
      memberViewProfile(&gDb);
      break;
    case 13:
      memberListAll(&gDb);
      break;
    case 0:
      authLogout(&gDb);
      break;
    default:
      printf("[LOI] Lua chon khong hop le\n");
    }
  } while (choice != 0);
}

/* ============================================================
 * MAIN ENTRY POINT
 * ============================================================ */

int main(void) {
  printf("  F-CODE VIOLATION MANAGEMENT SYSTEM\n");
  /* Load data from files */
  if (fileioLoadAll(&gDb) != 0) {
    printf("[LOI] Khong the tai du lieu. Vui long kiem tra thu muc data/\n");
    return 1;
  }

  printf("[OK] Tai du lieu thanh cong\n");
  printf("  - Thanh vien: %d/%d\n", gDb.memberCount, MAX_MEMBERS);
  printf("  - Vi pham: %d\n", gDb.violationCount);
  printf("  - Tai khoan: %d\n", gDb.accountCount);

  /* Main application loop */
  while (1) {
    /* Login first */
    if (authLogin(&gDb) != 0) {
      /* Account locked or exit requested */
      break;
    }

    /* Route to appropriate menu based on role */
    Account *session = authGetSession();
    if (session != NULL) {
      if (session->role == ACCOUNT_ROLE_BCN) {
        adminMenu();
      } else {
        memberMenu();
      }
    }
  }

  printf("[OK] Thoat chuong trinh\n");
  return 0;
}
