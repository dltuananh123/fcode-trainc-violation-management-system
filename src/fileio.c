#include "fileio.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_MEMBERS "data/members.dat"
#define FILE_VIOLATIONS "data/violations.dat"
#define FILE_ACCOUNTS "data/accounts.dat"

#define TMP_MEMBERS "data/members.dat.tmp"
#define TMP_VIOLATIONS "data/violations.dat.tmp"
#define TMP_ACCOUNTS "data/accounts.dat.tmp"

/* Handle tmp files on startup to recover from crashes */
static void recoverFromTmp(const char *tmpFile, const char *datFile) {
  FILE *fTmp = fopen(tmpFile, "rb");
  if (fTmp != NULL) {
    fclose(fTmp);
    FILE *fDat = fopen(datFile, "rb");
    if (fDat == NULL) {
      /* .dat is missing, but .tmp exists! Recover. */
      rename(tmpFile, datFile);
    } else {
      /* Both exist. .dat is valid. Remove .tmp. */
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

static int closeFileChecked(FILE *fp, const char *label) {
  if (fclose(fp) != 0) {
    printf("[LOI] Khong the dong file tam %s!\n", label);
    return -1;
  }

  return 0;
}

static int replaceStoreFile(const char *tmpFile, const char *dataFile,
                            const char *label) {
  remove(dataFile);
  if (rename(tmpFile, dataFile) != 0) {
    printf("[LOI] Khong the ghi de file %s!\n", label);
    return -1;
  }

  return 0;
}

/* ============================================================
 * Save functions
 * ============================================================ */
int fileioSaveMembers(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  FILE *fp = fopen(TMP_MEMBERS, "wb");
  if (fp == NULL) {
    printf("[LOI] Khong the tao file tam de luu members!\n");
    return -1;
  }

  if (writeCountChecked(fp, db->memberCount, "members") != 0) {
    fclose(fp);
    remove(TMP_MEMBERS);
    return -1;
  }

  if (writeItemsChecked(fp, db->members, sizeof(Member), db->memberCount,
                        "members") != 0) {
    fclose(fp);
    remove(TMP_MEMBERS);
    return -1;
  }

  if (closeFileChecked(fp, "members") != 0) {
    remove(TMP_MEMBERS);
    return -1;
  }

  return replaceStoreFile(TMP_MEMBERS, FILE_MEMBERS, "members.dat");
}

int fileioSaveViolations(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  FILE *fp = fopen(TMP_VIOLATIONS, "wb");
  if (fp == NULL) {
    printf("[LOI] Khong the tao file tam de luu violations!\n");
    return -1;
  }

  if (writeCountChecked(fp, db->violationCount, "violations") != 0) {
    fclose(fp);
    remove(TMP_VIOLATIONS);
    return -1;
  }

  if (writeItemsChecked(fp, db->violations, sizeof(Violation),
                        db->violationCount, "violations") != 0) {
    fclose(fp);
    remove(TMP_VIOLATIONS);
    return -1;
  }

  if (closeFileChecked(fp, "violations") != 0) {
    remove(TMP_VIOLATIONS);
    return -1;
  }

  return replaceStoreFile(TMP_VIOLATIONS, FILE_VIOLATIONS, "violations.dat");
}

int fileioSaveAccounts(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  FILE *fp = fopen(TMP_ACCOUNTS, "wb");
  if (fp == NULL) {
    printf("[LOI] Khong the tao file tam de luu accounts!\n");
    return -1;
  }

  if (writeCountChecked(fp, db->accountCount, "accounts") != 0) {
    fclose(fp);
    remove(TMP_ACCOUNTS);
    return -1;
  }

  if (writeItemsChecked(fp, db->accounts, sizeof(Account), db->accountCount,
                        "accounts") != 0) {
    fclose(fp);
    remove(TMP_ACCOUNTS);
    return -1;
  }

  if (closeFileChecked(fp, "accounts") != 0) {
    remove(TMP_ACCOUNTS);
    return -1;
  }

  return replaceStoreFile(TMP_ACCOUNTS, FILE_ACCOUNTS, "accounts.dat");
}

/* ============================================================
 * Load & init
 * ============================================================ */
int fileioLoadAll(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  db->memberCount = 0;
  db->violationCount = 0;
  db->accountCount = 0;

  /* Handle crash-residue .tmp files before loading */
  handleTmpFiles();

  /* Load accounts */
  FILE *fpAcc = fopen(FILE_ACCOUNTS, "rb");
  if (fpAcc != NULL) {
    if (readCountChecked(fpAcc, &(db->accountCount), MAX_MEMBERS,
                         "accounts") != 0) {
      fclose(fpAcc);
      return -1;
    }
    if (readItemsChecked(fpAcc, db->accounts, sizeof(Account), db->accountCount,
                         "accounts") != 0) {
      fclose(fpAcc);
      return -1;
    }
    fclose(fpAcc);
  }
  /* First-run init: create default admin account if none exists */
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

  /* Load Member */
  FILE *fpMen = fopen(FILE_MEMBERS, "rb");
  if (fpMen != NULL) {
    if (readCountChecked(fpMen, &(db->memberCount), MAX_MEMBERS,
                         "members") != 0) {
      fclose(fpMen);
      return -1;
    }
    if (readItemsChecked(fpMen, db->members, sizeof(Member), db->memberCount,
                         "members") != 0) {
      fclose(fpMen);
      return -1;
    }
    fclose(fpMen);
  } else {
    /* First-run: create empty members file */
    if (fileioSaveMembers(db) != 0) {
      return -1;
    }
  }

  /* Load violations */
  FILE *fpVio = fopen(FILE_VIOLATIONS, "rb");
  if (fpVio != NULL) {
    if (readCountChecked(fpVio, &(db->violationCount), MAX_VIOLATIONS,
                         "violations") != 0) {
      fclose(fpVio);
      return -1;
    }
    if (readItemsChecked(fpVio, db->violations, sizeof(Violation),
                         db->violationCount, "violations") != 0) {
      fclose(fpVio);
      return -1;
    }
    fclose(fpVio);
  } else {
    if (fileioSaveViolations(db) != 0) {
      return -1;
    }
  }
  return 0;
}
