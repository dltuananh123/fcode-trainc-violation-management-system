#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileio.h"

#define FILE_MEMBERS "data/members.dat"
#define FILE_VIOLATIONS "data/violations.dat"
#define FILE_ACCOUNTS "data/accounts.dat"

#define TMP_MEMBERS "data/members.dat.tmp"
#define TMP_VIOLATIONS "data/violations.dat.tmp"
#define TMP_ACCOUNTS "data/accounts.dat.tmp"

/* Crash residual cleanup */
static void cleanup_tmp_files() {
  /* TODO: Delete .tmp files from crash */
  remove(TMP_MEMBERS);
  remove(TMP_VIOLATIONS);
  remove(TMP_ACCOUNTS);
}

/* Save functions */
int fileio_save_members(AppDatabase *db) {
  /* TODO: Save members to file using atomic write pattern
   * 1. Write to .tmp file
   * 2. Close file
   * 3. Remove old file
   * 4. Rename .tmp to .dat
   */
  return -1;
}

int fileio_save_violations(AppDatabase *db) {
  /* TODO: Save violations to file using atomic write pattern */
  return -1;
}

int fileio_save_accounts(AppDatabase *db) {
  /* TODO: Save accounts to file using atomic write pattern */
  return -1;
}

/* Load & init */
int fileio_load_all(AppDatabase *db) {
  /* TODO: Load all data from .dat files
   * 1. Clean up .tmp files
   * 2. Load accounts.dat (create ADMIN/ADMIN if empty)
   * 3. Load members.dat (create file if missing)
   * 4. Load violations.dat (create file if missing)
   */
  db->memberCount = 0;
  db->violationCount = 0;
  db->accountCount = 0;
  return -1;
}
