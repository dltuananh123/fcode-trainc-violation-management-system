#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "fileio.h"
#include "auth.h"
#include "member.h"
#include "utils.h"

/* Global database state */
static AppDatabase g_db;

/* ============================================================
 * MENU FUNCTIONS
 * ============================================================ */

/* Member menu (for regular members and leaders) */
void member_menu(void) {
  /* TODO: Implement member menu
   * Options:
   * 1. Xem profile ca nhan
   * 2. Xem lich su vi pham
   * 3. Xem tong tien phat con no
   * 4. Xem danh sach thanh vien
   * 0. Dang xuat
   */
  int choice;
  do {
    printf("\n=== MENU THANH VIEN ===\n");
    printf("1. Xem profile ca nhan\n");
    printf("2. Xem lich su vi pham\n");
    printf("3. Xem tong tien phat con no\n");
    printf("4. Xem danh sach thanh vien\n");
    printf("0. Dang xuat\n");
    printf("Nhap lua chon: ");

    if (scanf("%d", &choice) != 1) {
      printf("[LOI] Lua chon khong hop le\n");
      while (getchar() != '\n');
      continue;
    }
    while (getchar() != '\n');

    switch (choice) {
      case 1:
        /* TODO: Call view profile function */
        printf("[CANH BAO] Chua cai dat\n");
        break;
      case 2:
        /* TODO: Call view violations function */
        printf("[CANH BAO] Chua cai dat\n");
        break;
      case 3:
        /* TODO: Call view fines function */
        printf("[CANH BAO] Chua cai dat\n");
        break;
      case 4:
        member_list_all(&g_db);
        break;
      case 0:
        auth_logout(&g_db);
        break;
      default:
        printf("[LOI] Lua chon khong hop le\n");
    }
  } while (choice != 0);
}

/* BCN menu (for admins) */
void admin_menu(void) {
  /* TODO: Implement admin menu
   * Options:
   * 1. Them thanh vien moi
   * 2. Sua thong tin thanh vien
   * 3. Xoa thanh vien
   * 4. Ghi nhan vi pham
   * 5. Danh dau da thu tien
   * 6. Xem danh sach vi pham
   * 7. Thong ke tien phat theo ban
   * 8. Kiem tra nguong Out CLB
   * 9. Sap xep theo so lan vi pham
   * 10. Xuat bao cao
   * 11. Tim kiem theo ngay
   * 0. Dang xuat
   */
  int choice;
  do {
    printf("\n=== MENU BAN CHU NHIEM ===\n");
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
    printf("0. Dang xuat\n");
    printf("Nhap lua chon: ");

    if (scanf("%d", &choice) != 1) {
      printf("[LOI] Lua chon khong hop le\n");
      while (getchar() != '\n');
      continue;
    }
    while (getchar() != '\n');

    switch (choice) {
      case 1:
        member_add(&g_db);
        break;
      case 2:
        member_edit(&g_db);
        break;
      case 3:
        member_delete(&g_db);
        break;
      case 4:
        /* TODO: Call violation_record */
        printf("[CANH BAO] Chua cai dat\n");
        break;
      case 5:
        /* TODO: Call violation_mark_paid */
        printf("[CANH BAO] Chua cai dat\n");
        break;
      case 6:
        /* TODO: Call violation_list_all */
        printf("[CANH BAO] Chua cai dat\n");
        break;
      case 7:
        /* TODO: Call report_team_stats */
        printf("[CANH BAO] Chua cai dat\n");
        break;
      case 8:
        /* TODO: Call violation_check_out_clb */
        printf("[CANH BAO] Chua cai dat\n");
        break;
      case 9:
        /* TODO: Call report_sort_by_violations */
        printf("[CANH BAO] Chua cai dat\n");
        break;
      case 10:
        /* TODO: Call report_export_txt */
        printf("[CANH BAO] Chua cai dat\n");
        break;
      case 11:
        /* TODO: Call violation_search_by_date */
        printf("[CANH BAO] Chua cai dat\n");
        break;
      case 0:
        auth_logout(&g_db);
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
  printf("====================================\n");
  printf("  F-CODE VIOLATION MANAGEMENT SYSTEM\n");
  printf("====================================\n\n");

  /* Load data from files */
  if (fileio_load_all(&g_db) != 0) {
    printf("[LOI] Khong the tai du lieu. Vui long kiem tra thu muc data/\n");
    return 1;
  }

  printf("[OK] Tai du lieu thanh cong\n");
  printf("  - Thanh vien: %d/%d\n", g_db.memberCount, MAX_MEMBERS);
  printf("  - Vi pham: %d\n", g_db.violationCount);
  printf("  - Tai khoan: %d\n", g_db.accountCount);

  /* Main application loop */
  while (1) {
    /* Login first */
    if (auth_login(&g_db) != 0) {
      /* Account locked or exit requested */
      break;
    }

    /* Route to appropriate menu based on role */
    Account *session = auth_get_session();
    if (session != NULL) {
      if (session->role == ACCOUNT_ROLE_BCN) {
        admin_menu();
      } else {
        member_menu();
      }
    }
  }

  printf("[OK] Thoat chuong trinh\n");
  return 0;
}
