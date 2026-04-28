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

/* ============================================================
 * Save functions
 * ============================================================ */
int fileioSaveMembers(AppDatabase *db) {
  FILE *fp = fopen(TMP_MEMBERS, "wb");
  if (fp == NULL) {
    printf("[LOI] Khong the tao file tam de luu members!\n");
    return -1;
  }
  /* Write member count header to temp file */
  fwrite(&(db->memberCount), sizeof(int), 1, fp);

  /* Write member data */
  if (db->memberCount > 0) {
    fwrite(db->members, sizeof(Member), (size_t)db->memberCount, fp);
  }
  fclose(fp);

  /* Replace old file with temp file */
  remove(FILE_MEMBERS);
  if (rename(TMP_MEMBERS, FILE_MEMBERS) != 0) {
    printf("[LOI] Khong the ghi de file members.dat!\n");
    return -1;
  }
  return 0;
}

int fileioSaveViolations(AppDatabase *db) {
  FILE *fp = fopen(TMP_VIOLATIONS, "wb");
  if (fp == NULL) {
    printf("[LOI] Khong the tao file tam de luu violations!\n");
    return -1;
  }

  fwrite(&(db->violationCount), sizeof(int), 1, fp);

  if (db->violationCount > 0) {
    fwrite(db->violations, sizeof(Violation), (size_t)db->violationCount, fp);
  }
  fclose(fp);

  remove(FILE_VIOLATIONS);
  if (rename(TMP_VIOLATIONS, FILE_VIOLATIONS) != 0) {
    printf("[LOI] Khong the ghi de file violations.dat!\n");
    return -1;
  }
  return 0;
}

int fileioSaveAccounts(AppDatabase *db) {
  FILE *fp = fopen(TMP_ACCOUNTS, "wb");
  if (fp == NULL) {
    printf("[LOI] Khong the tao file tam de luu accounts!\n");
    return -1;
  }

  fwrite(&(db->accountCount), sizeof(int), 1, fp);

  if (db->accountCount > 0) {
    fwrite(db->accounts, sizeof(Account), (size_t)db->accountCount, fp);
  }

  fclose(fp);

  remove(FILE_ACCOUNTS);
  if (rename(TMP_ACCOUNTS, FILE_ACCOUNTS) != 0) {
    printf("[LOI] Khong the ghi de file accounts.dat!\n");
    return -1;
  }
  return 0;
}

/* ============================================================
 * Load & init
 * ============================================================ */
int fileioLoadAll(AppDatabase *db) {
  db->memberCount = 0;
  db->violationCount = 0;
  db->accountCount = 0;

  /* Handle crash-residue .tmp files before loading */
  handleTmpFiles();

  /* Load accounts */
  FILE *fpAcc = fopen(FILE_ACCOUNTS, "rb");
  if (fpAcc != NULL) {
    fread(&(db->accountCount), sizeof(int), 1, fpAcc);
    if (db->accountCount > 0) {
      fread(db->accounts, sizeof(Account), (size_t)db->accountCount, fpAcc);
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
    fileioSaveAccounts(db);
  }

  /* Load Member */
  FILE *fpMen = fopen(FILE_MEMBERS, "rb");
  if (fpMen != NULL) {
    fread(&(db->memberCount), sizeof(int), 1, fpMen);
    if (db->memberCount > 0) {
      fread(db->members, sizeof(Member), (size_t)db->memberCount, fpMen);
    }
    fclose(fpMen);
  } else {
    /* First-run: create empty members file */
    fileioSaveMembers(db);
  }

  /* Load violations */
  FILE *fpVio = fopen(FILE_VIOLATIONS, "rb");
  if (fpVio != NULL) {
    fread(&(db->violationCount), sizeof(int), 1, fpVio);
    if (db->violationCount > 0) {
      fread(db->violations, sizeof(Violation), (size_t)db->violationCount,
            fpVio);
    }
    fclose(fpVio);
  } else {
    fileioSaveViolations(db);
  }
  return 0;
}
