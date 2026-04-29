#include "fileio.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_MEMBERS    "data/members.dat"
#define FILE_VIOLATIONS "data/violations.dat"
#define FILE_ACCOUNTS   "data/accounts.dat"

#define TMP_MEMBERS     "data/members.dat.tmp"
#define TMP_VIOLATIONS  "data/violations.dat.tmp"
#define TMP_ACCOUNTS    "data/accounts.dat.tmp"

#define BAK_MEMBERS     "data/members.dat.bak"
#define BAK_VIOLATIONS  "data/violations.dat.bak"
#define BAK_ACCOUNTS    "data/accounts.dat.bak"

/* --- Crash recovery --- */

static void recoverFromTmp(const char *tmpFile, const char *datFile) {
  FILE *fTmp = fopen(tmpFile, "rb");
  if (fTmp != NULL) {
    fclose(fTmp);
    FILE *fDat = fopen(datFile, "rb");
    if (fDat == NULL) {
      /* .dat missing but .tmp exists — recover */
      rename(tmpFile, datFile);
    } else {
      /* Both exist — .dat is valid, discard .tmp */
      fclose(fDat);
      remove(tmpFile);
    }
  }
}

static void handleTmpFiles(void) {
  recoverFromTmp(TMP_MEMBERS, FILE_MEMBERS);
  recoverFromTmp(TMP_VIOLATIONS, FILE_VIOLATIONS);
  recoverFromTmp(TMP_ACCOUNTS, FILE_ACCOUNTS);
}

/* --- Checked read/write helpers --- */

static int readCountChecked(FILE *fp, int *count, int maxCount,
                            const char *label) {
  if (fread(count, sizeof(int), 1, fp) != 1) {
    printf("[LOI] Khong the doc so luong %s tu file!\n", label);
    return -1;
  }
  if (*count < 0 || *count > maxCount) {
    printf("[LOI] File %s bi loi: so luong %d nam ngoai gioi han [0, %d].\n",
           label, *count, maxCount);
    return -1;
  }
  return 0;
}

static int readItemsChecked(FILE *fp, void *buffer, size_t itemSize, int count,
                            const char *label) {
  if (count == 0) {
    return 0;
  }
  if (fread(buffer, itemSize, (size_t)count, fp) != (size_t)count) {
    printf("[LOI] Khong the doc day du du lieu %s tu file!\n", label);
    return -1;
  }
  return 0;
}

static int writeCountChecked(FILE *fp, int count, const char *label) {
  if (fwrite(&count, sizeof(int), 1, fp) != 1) {
    printf("[LOI] Khong the ghi so luong %s vao file tam!\n", label);
    return -1;
  }
  return 0;
}

static int writeItemsChecked(FILE *fp, const void *buffer, size_t itemSize,
                             int count, const char *label) {
  if (count == 0) {
    return 0;
  }
  if (fwrite(buffer, itemSize, (size_t)count, fp) != (size_t)count) {
    printf("[LOI] Khong the ghi day du du lieu %s vao file tam!\n", label);
    return -1;
  }
  return 0;
}

/* Fix #6: explicit fflush before fclose for safety on all platforms */
static int closeFileChecked(FILE *fp, const char *label) {
  if (fflush(fp) != 0) {
    printf("[LOI] Khong the flush du lieu %s!\n", label);
    fclose(fp);
    return -1;
  }
  if (fclose(fp) != 0) {
    printf("[LOI] Khong the dong file tam %s!\n", label);
    return -1;
  }
  return 0;
}

/* Fix #1: backup-based replace — if rename fails, restore from backup */
static int replaceStoreFile(const char *tmpFile, const char *dataFile,
                            const char *bakFile, const char *label) {
  /* Step 1: keep old dataFile as .bak */
  remove(bakFile);
  FILE *fDat = fopen(dataFile, "rb");
  if (fDat != NULL) {
    fclose(fDat);
    if (rename(dataFile, bakFile) != 0) {
      printf("[LOI] Khong the tao backup cho %s!\n", label);
      return -1;
    }
  }

  /* Step 2: promote tmp to dataFile */
  if (rename(tmpFile, dataFile) != 0) {
    printf("[LOI] Khong the ghi de file %s! Dang phuc huc tu backup...\n",
           label);
    rename(bakFile, dataFile);
    return -1;
  }

  /* Step 3: clean up backup */
  remove(bakFile);
  return 0;
}

/* --- Generic save (Fix #2) --- */

static int saveStore(const char *tmpFile, const char *dataFile,
                     const char *bakFile, const void *buffer,
                     size_t itemSize, int count, const char *label) {
  FILE *fp = fopen(tmpFile, "wb");
  if (fp == NULL) {
    printf("[LOI] Khong the tao file tam de luu %s!\n", label);
    return -1;
  }

  if (writeCountChecked(fp, count, label) != 0) {
    fclose(fp);
    remove(tmpFile);
    return -1;
  }

  if (writeItemsChecked(fp, buffer, itemSize, count, label) != 0) {
    fclose(fp);
    remove(tmpFile);
    return -1;
  }

  if (closeFileChecked(fp, label) != 0) {
    remove(tmpFile);
    return -1;
  }

  return replaceStoreFile(tmpFile, dataFile, bakFile, label);
}

/* --- Public save API (thin wrappers) --- */

int fileioSaveMembers(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }
  return saveStore(TMP_MEMBERS, FILE_MEMBERS, BAK_MEMBERS,
                   db->members, sizeof(Member), db->memberCount, "members");
}

int fileioSaveViolations(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }
  return saveStore(TMP_VIOLATIONS, FILE_VIOLATIONS, BAK_VIOLATIONS,
                   db->violations, sizeof(Violation), db->violationCount,
                   "violations");
}

int fileioSaveAccounts(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }
  return saveStore(TMP_ACCOUNTS, FILE_ACCOUNTS, BAK_ACCOUNTS,
                   db->accounts, sizeof(Account), db->accountCount,
                   "accounts");
}

/* --- Load helpers (Fix #3) --- */

static int loadAccounts(AppDatabase *db) {
  FILE *fp = fopen(FILE_ACCOUNTS, "rb");
  if (fp != NULL) {
    if (readCountChecked(fp, &(db->accountCount), MAX_MEMBERS,
                         "accounts") != 0) {
      fclose(fp);
      return -1;
    }
    if (readItemsChecked(fp, db->accounts, sizeof(Account),
                         db->accountCount, "accounts") != 0) {
      fclose(fp);
      return -1;
    }
    fclose(fp);
  }

  /* First-run: create default admin account */
  if (db->accountCount == 0) {
    printf("[CANH BAO] Khong tim thay tai khoan nao. Dang tao tai khoan ADMIN "
           "mac dinh...\n");
    strcpy(db->accounts[0].studentId, "ADMIN");
    strcpy(db->accounts[0].password, "ADMIN");
    db->accounts[0].role = ACCOUNT_ROLE_BCN;
    db->accounts[0].isLocked = 0;
    db->accounts[0].failCount = 0;
    db->accountCount = 1;
    if (fileioSaveAccounts(db) != 0) {
      return -1;
    }
  }
  return 0;
}

static int loadMembers(AppDatabase *db) {
  FILE *fp = fopen(FILE_MEMBERS, "rb");
  if (fp != NULL) {
    if (readCountChecked(fp, &(db->memberCount), MAX_MEMBERS,
                         "members") != 0) {
      fclose(fp);
      return -1;
    }
    if (readItemsChecked(fp, db->members, sizeof(Member), db->memberCount,
                         "members") != 0) {
      fclose(fp);
      return -1;
    }
    fclose(fp);
  } else {
    /* First-run: create empty members file */
    if (fileioSaveMembers(db) != 0) {
      return -1;
    }
  }
  return 0;
}

static int loadViolations(AppDatabase *db) {
  FILE *fp = fopen(FILE_VIOLATIONS, "rb");
  if (fp != NULL) {
    if (readCountChecked(fp, &(db->violationCount), MAX_VIOLATIONS,
                         "violations") != 0) {
      fclose(fp);
      return -1;
    }
    if (readItemsChecked(fp, db->violations, sizeof(Violation),
                         db->violationCount, "violations") != 0) {
      fclose(fp);
      return -1;
    }
    fclose(fp);
  } else {
    /* First-run: create empty violations file */
    if (fileioSaveViolations(db) != 0) {
      return -1;
    }
  }
  return 0;
}

/* --- Public load API --- */

int fileioLoadAll(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  db->memberCount = 0;
  db->violationCount = 0;
  db->accountCount = 0;

  handleTmpFiles();

  if (loadAccounts(db) != 0) {
    return -1;
  }
  if (loadMembers(db) != 0) {
    return -1;
  }
  if (loadViolations(db) != 0) {
    return -1;
  }
  return 0;
}
