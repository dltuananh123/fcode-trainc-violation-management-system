#include "member.h"
#include "utils.h"
#include "fileio.h"
#include <stdio.h>
#include <string.h>

/* ============================================================
 * Story 2.1 — Add Member
 * ============================================================ */

int member_find_by_id(AppDatabase *db, const char *studentId) {
  /* TODO: Find member index by studentId
   * Linear scan through members[] array
   *
   * Return: index if found, -1 if not found or invalid input
   */
  if (db == NULL || studentId == NULL) return -1;

  for (int i = 0; i < db->memberCount; i++) {
    /* TODO: Implement comparison */
  }

  return -1;
}

int member_validate_input(const Member *m, AppDatabase *db) {
  /* TODO: Validate member input
   * 1. Check studentId is not empty
   * 2. Check studentId uniqueness (call member_find_by_id)
   * 3. Check email format (call is_email_valid)
   *
   * Return: 0 if valid, -1 if invalid (print error message)
   */
  if (m == NULL || db == NULL) return -1;

  /* TODO: Implement validation */

  return -1;
}

int member_add(AppDatabase *db) {
  /* TODO: Add new member
   * 1. Check MAX_MEMBERS limit
   * 2. Display "=== THEM THANH VIEN MOI ==="
   * 3. Prompt for:
   *    - fullName, studentId, email, phone
   *    - team (0-3), role (0-2)
   * 4. Validate input (call member_validate_input)
   * 5. Create Member record:
   *    - violationCount = 0
   *    - consecutiveAbsences = 0
   *    - totalFine = 0
   *    - isActive = STATUS_ACTIVE
   * 6. Create Account record:
   *    - password = studentId
   *    - role = ACCOUNT_ROLE_MEMBER
   *    - isLocked = 0, failCount = 0
   * 7. Save to files (fileio_save_members, fileio_save_accounts)
   * 8. Print success message with details
   *
   * Return: 0 on success, -1 on failure
   */
  if (db == NULL) return -1;

  /* TODO: Check capacity */
  if (db->memberCount >= MAX_MEMBERS) {
    printf("[LOI] Da dat gioi han so luong thanh vien (%d)\n", MAX_MEMBERS);
    return -1;
  }

  /* TODO: Implement member add logic */

  return -1;
}

/* ============================================================
 * Story 2.2 — Edit Member
 * ============================================================ */

int member_edit(AppDatabase *db) {
  /* TODO: Edit existing member
   * 1. Prompt for studentId to edit
   * 2. Find member (call member_find_by_id)
   * 3. Display current info
   * 4. Prompt for new values (all EXCEPT studentId)
   * 5. If role changed:
   *    - Recalculate fines for unpaid violations
   *    - Update member.totalFine
   *    - Save violations file
   * 6. Save members file
   *
   * Return: 0 on success, -1 on failure
   */
  if (db == NULL) return -1;

  /* TODO: Implement edit logic */

  return -1;
}

/* ============================================================
 * Story 2.3 — Delete Member
 * ============================================================ */

int member_delete(AppDatabase *db) {
  /* TODO: Delete member with cascade
   * 1. Prompt for studentId to delete
   * 2. Find member (call member_find_by_id)
   * 3. Display member info
   * 4. Ask for confirmation ("Xac nhan xoa? (y/n): ")
   * 5. If confirmed:
   *    - Remove from members[] (shift array)
   *    - Remove all matching violations[]
   *    - Remove matching account from accounts[]
   *    - Update counts
   *    - Save all files
   *
   * Return: 0 on success, -1 on failure or cancelled
   */
  if (db == NULL) return -1;

  /* TODO: Implement delete logic */

  return -1;
}

/* ============================================================
 * Story 2.4 — View Member List
 * ============================================================ */

void member_list_all(AppDatabase *db) {
  /* TODO: Display all members in table format
   * Show only: fullName, studentId, team, role
   * Hide: email, phone, violationCount, totalFine, isActive
   *
   * Format:
   * +------+------------------+------------+-----------+
   * | MSSV | Ho va ten        | Ban        | Chuc vu  |
   * +------+------------------+------------+-----------+
   * | ...  | ...              | ...        | ...       |
   * +------+------------------+------------+-----------+
   */
  if (db == NULL) return;

  /* TODO: Implement display logic */

  printf("[CANH BAO] Chua cai dat chuc nang hien thi danh sach\n");
}
