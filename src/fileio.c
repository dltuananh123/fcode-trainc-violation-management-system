#include "fileio.h"
#include "auth.h"
#include "member.h"
#include "types.h"
#include "ui.h"
#include "utils.h"
#include "validate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_MAGIC "FCE1"
#define MAGIC_LEN 4

#ifdef _WIN32
#include <windows.h>
#endif

static unsigned char gXorKey[32];
#define XOR_KEY_LEN 32
static int gXorKeyDerived = 0;

static void deriveXorKey(unsigned char *key, size_t keyLen) {
  char seed[1024] = {0};
  char exeDir[512] = {0};
  getExeDir(exeDir, sizeof(exeDir));

  /* Combine: exe path + computer name */
  snprintf(seed, sizeof(seed), "%s", exeDir);

#ifdef _WIN32
  char compName[256] = {0};
  DWORD size = sizeof(compName);
  if (GetComputerNameA(compName, &size)) {
    strncat(seed, compName, sizeof(seed) - strlen(seed) - 1);
  }
#endif

  /* FNV-1a hash the seed to fill key buffer */
  unsigned long long h = 0xCBF29CE484222325ULL;
  for (int i = 0; seed[i] != '\0'; i++) {
    h ^= (unsigned char)seed[i];
    h *= 0x00000100000001B3ULL;
  }
  for (size_t i = 0; i < keyLen; i++) {
    key[i] = (unsigned char)((h >> ((i % 8) * 8)) & 0xFF);
    if (i % 8 == 7) {
      h *= 0x00000100000001B3ULL;
    }
  }
}

void xorBuffer(unsigned char *data, size_t size) {
  if (!gXorKeyDerived) {
    deriveXorKey(gXorKey, XOR_KEY_LEN);
    gXorKeyDerived = 1;
  }
  for (size_t i = 0; i < size; i++) {
    data[i] ^= gXorKey[i % XOR_KEY_LEN];
  }
}

#define MAX_PATH_BASE 512
#define MAX_PATH_FULL 1024

static char pathMembers[MAX_PATH_FULL];
static char pathViolations[MAX_PATH_FULL];
static char pathAccounts[MAX_PATH_FULL];

static char pathTmpMembers[MAX_PATH_FULL];
static char pathTmpViolations[MAX_PATH_FULL];
static char pathTmpAccounts[MAX_PATH_FULL];

static char pathBakMembers[MAX_PATH_FULL];
static char pathBakViolations[MAX_PATH_FULL];
static char pathBakAccounts[MAX_PATH_FULL];

static int pathsInitialized = 0;

static void initPaths(void) {
  if (pathsInitialized) {
    return;
  }

  char exeDir[MAX_PATH_BASE];
  getExeDir(exeDir, sizeof(exeDir));

  char dataDir[MAX_PATH_BASE + 5];
#ifdef _WIN32
  snprintf(dataDir, sizeof(dataDir), "%s\\data", exeDir);
  const char *sep = "\\";
#else
  snprintf(dataDir, sizeof(dataDir), "%s/data", exeDir);
  const char *sep = "/";
#endif

  /* Ensure data directory exists next to the executable */
  if (MKDIR(dataDir) != 0) {
    /* If directory exists, MKDIR returns non-zero on some systems,
       but we don't need to treat it as a hard error here. */
  }

  snprintf(pathMembers, MAX_PATH_FULL, "%s%smembers.dat", dataDir, sep);
  snprintf(pathViolations, MAX_PATH_FULL, "%s%sviolations.dat", dataDir, sep);
  snprintf(pathAccounts, MAX_PATH_FULL, "%s%saccounts.dat", dataDir, sep);

  snprintf(pathTmpMembers, MAX_PATH_FULL, "%s%smembers.dat.tmp", dataDir, sep);
  snprintf(pathTmpViolations, MAX_PATH_FULL, "%s%sviolations.dat.tmp", dataDir,
           sep);
  snprintf(pathTmpAccounts, MAX_PATH_FULL, "%s%saccounts.dat.tmp", dataDir,
           sep);

  snprintf(pathBakMembers, MAX_PATH_FULL, "%s%smembers.dat.bak", dataDir, sep);
  snprintf(pathBakViolations, MAX_PATH_FULL, "%s%sviolations.dat.bak", dataDir,
           sep);
  snprintf(pathBakAccounts, MAX_PATH_FULL, "%s%saccounts.dat.bak", dataDir,
           sep);

  pathsInitialized = 1;
}

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
  initPaths();
  recoverFromTmp(pathTmpMembers, pathMembers);
  recoverFromTmp(pathTmpViolations, pathViolations);
  recoverFromTmp(pathTmpAccounts, pathAccounts);
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

static int encryptAndWrite(FILE *fp, const void *buffer, size_t itemSize,
                           int count, const char *label, unsigned int *outCrc) {
  if (outCrc != NULL) {
    *outCrc = 0;
  }
  if (count < 0 || count > MAX_VIOLATIONS) {
    printf("[LOI] So luong phan tu de ma hoa khong hop le: %d\n", count);
    return -1;
  }
  if (count == 0) {
    if (outCrc != NULL) {
      *outCrc = calculateCrc32(NULL, 0);
    }
    return 0;
  }
  size_t totalSize = itemSize * (size_t)count;
  unsigned char *encrypted = malloc(totalSize);
  if (encrypted == NULL) {
    printf("[LOI] Khong the cap phat bo nho de ma hoa %s!\n", label);
    return -1;
  }
  memcpy(encrypted, buffer, totalSize);
  xorBuffer(encrypted, totalSize);

  if (outCrc != NULL) {
    *outCrc = calculateCrc32(encrypted, totalSize);
  }

  if (fwrite(encrypted, itemSize, (size_t)count, fp) != (size_t)count) {
    free(encrypted);
    printf("[LOI] Khong the ghi day du du lieu ma hoa %s vao file tam!\n",
           label);
    return -1;
  }
  free(encrypted);
  return 0;
}

static int readAndDecrypt(FILE *fp, void *buffer, size_t itemSize, int count,
                          const char *label) {
  if (count == 0) {
    unsigned int expectedCrc = 0;
    if (fread(&expectedCrc, sizeof(unsigned int), 1, fp) != 1) {
      printf("[LOI] Khong the doc checksum CRC32 cho %s!\n", label);
      return -1;
    }
    unsigned int computed = calculateCrc32(NULL, 0);
    if (computed != expectedCrc) {
      printf("[LOI] Checksum CRC32 khong khop cho %s!\n", label);
      return -1;
    }
    return 0;
  }
  size_t totalSize = itemSize * (size_t)count;
  if (fread(buffer, itemSize, (size_t)count, fp) != (size_t)count) {
    printf("[LOI] Khong the doc day du du lieu %s tu file!\n", label);
    return -1;
  }

  /* Read the 4-byte CRC32 at the end of the file */
  unsigned int expectedCrc = 0;
  if (fread(&expectedCrc, sizeof(unsigned int), 1, fp) != 1) {
    printf("[LOI] Khong the doc checksum CRC32 cho %s!\n", label);
    return -1;
  }

  /* Calculate CRC32 of the encrypted data (still encrypted in buffer) */
  unsigned int computed = calculateCrc32(buffer, totalSize);
  if (computed != expectedCrc) {
    printf("[LOI] Checksum CRC32 khong khop cho %s! (File co the bi loi)\n",
           label);
    return -1;
  }

  xorBuffer(buffer, totalSize);
  return 0;
}

/* --- Generic save (Fix #2) --- */

static int saveStore(const char *tmpFile, const char *dataFile,
                     const char *bakFile, const void *buffer, size_t itemSize,
                     int count, const char *label) {
  FILE *fp = fopen(tmpFile, "wb");
  if (fp == NULL) {
    printf("[LOI] Khong the tao file tam de luu %s!\n", label);
    return -1;
  }

  /* Step 1: write magic header */
  if (fwrite(FILE_MAGIC, 1, MAGIC_LEN, fp) != MAGIC_LEN) {
    printf("[LOI] Khong the ghi magic signature %s vao file tam!\n", label);
    fclose(fp);
    remove(tmpFile);
    return -1;
  }

  if (writeCountChecked(fp, count, label) != 0) {
    fclose(fp);
    remove(tmpFile);
    return -1;
  }

  unsigned int crc = 0;
  if (encryptAndWrite(fp, buffer, itemSize, count, label, &crc) != 0) {
    fclose(fp);
    remove(tmpFile);
    return -1;
  }

  /* Write 4-byte CRC32 at the end of the file */
  if (fwrite(&crc, sizeof(unsigned int), 1, fp) != 1) {
    printf("[LOI] Khong the ghi checksum CRC32 cho %s vao file tam!\n", label);
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
  initPaths();
  return saveStore(pathTmpMembers, pathMembers, pathBakMembers, db->members,
                   sizeof(Member), db->memberCount, "members");
}

int fileioSaveViolations(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }
  initPaths();
  return saveStore(pathTmpViolations, pathViolations, pathBakViolations,
                   db->violations, sizeof(Violation), db->violationCount,
                   "violations");
}

int fileioSaveAccounts(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }
  initPaths();
  return saveStore(pathTmpAccounts, pathAccounts, pathBakAccounts, db->accounts,
                   sizeof(Account), db->accountCount, "accounts");
}

/* --- Load helpers (Fix #3) --- */

static int loadAccountsFromStream(FILE *fp, AppDatabase *db) {
  char magic[MAGIC_LEN];
  int isEncrypted = 0;
  if (fread(magic, 1, MAGIC_LEN, fp) == MAGIC_LEN &&
      memcmp(magic, FILE_MAGIC, MAGIC_LEN) == 0) {
    isEncrypted = 1;
  } else {
    fseek(fp, 0, SEEK_SET);
  }

  if (readCountChecked(fp, &(db->accountCount), MAX_MEMBERS, "accounts") != 0) {
    return -1;
  }

  if (isEncrypted) {
    if (readAndDecrypt(fp, db->accounts, sizeof(Account), db->accountCount,
                       "accounts") != 0) {
      return -1;
    }
  } else {
    if (readItemsChecked(fp, db->accounts, sizeof(Account), db->accountCount,
                         "accounts") != 0) {
      return -1;
    }
  }
  return 0;
}

static int loadAccounts(AppDatabase *db) {
  initPaths();
  int loaded = 0;

  FILE *fp = fopen(pathAccounts, "rb");
  if (fp != NULL) {
    if (loadAccountsFromStream(fp, db) == 0) {
      loaded = 1;
    }
    fclose(fp);
  }

  if (!loaded) {
    fp = fopen(pathBakAccounts, "rb");
    if (fp != NULL) {
      printf(COLOR_YELLOW "[CANH BAO] File accounts.dat bi loi hoac thieu. "
                          "Dang thu doc tu backup...\n" COLOR_RESET);
      if (loadAccountsFromStream(fp, db) == 0) {
        loaded = 1;
        fclose(fp);
        fileioSaveAccounts(db); /* Restore main file */
      } else {
        fclose(fp);
      }
    }
  }
  if (!loaded) {
    db->accountCount = 0;
    if (fileioSaveAccounts(db) != 0) {
      return -1;
    }
  }
  return 0;
}

static int loadMembersFromStream(FILE *fp, AppDatabase *db) {
  char magic[MAGIC_LEN];
  int isEncrypted = 0;
  if (fread(magic, 1, MAGIC_LEN, fp) == MAGIC_LEN &&
      memcmp(magic, FILE_MAGIC, MAGIC_LEN) == 0) {
    isEncrypted = 1;
  } else {
    fseek(fp, 0, SEEK_SET);
  }

  if (readCountChecked(fp, &(db->memberCount), MAX_MEMBERS, "members") != 0) {
    return -1;
  }

  if (isEncrypted) {
    if (readAndDecrypt(fp, db->members, sizeof(Member), db->memberCount,
                       "members") != 0) {
      return -1;
    }
  } else {
    if (readItemsChecked(fp, db->members, sizeof(Member), db->memberCount,
                         "members") != 0) {
      return -1;
    }
  }
  return 0;
}

static int loadMembers(AppDatabase *db) {
  initPaths();
  int loaded = 0;

  FILE *fp = fopen(pathMembers, "rb");
  if (fp != NULL) {
    if (loadMembersFromStream(fp, db) == 0) {
      loaded = 1;
    }
    fclose(fp);
  }

  if (!loaded) {
    fp = fopen(pathBakMembers, "rb");
    if (fp != NULL) {
      printf(COLOR_YELLOW "[CANH BAO] File members.dat bi loi hoac thieu. Dang "
                          "thu doc tu backup...\n" COLOR_RESET);
      if (loadMembersFromStream(fp, db) == 0) {
        loaded = 1;
        fclose(fp);
        fileioSaveMembers(db); /* Restore main file */
      } else {
        fclose(fp);
      }
    }
  }

  if (!loaded) {
    /* First-run: create empty members file */
    if (fileioSaveMembers(db) != 0) {
      return -1;
    }
  }
  return 0;
}

static int loadViolationsFromStream(FILE *fp, AppDatabase *db) {
  char magic[MAGIC_LEN];
  int isEncrypted = 0;
  if (fread(magic, 1, MAGIC_LEN, fp) == MAGIC_LEN &&
      memcmp(magic, FILE_MAGIC, MAGIC_LEN) == 0) {
    isEncrypted = 1;
  } else {
    fseek(fp, 0, SEEK_SET);
  }

  if (readCountChecked(fp, &(db->violationCount), MAX_VIOLATIONS,
                       "violations") != 0) {
    return -1;
  }

  if (isEncrypted) {
    if (readAndDecrypt(fp, db->violations, sizeof(Violation),
                       db->violationCount, "violations") != 0) {
      return -1;
    }
  } else {
    if (readItemsChecked(fp, db->violations, sizeof(Violation),
                         db->violationCount, "violations") != 0) {
      return -1;
    }
  }
  return 0;
}

static int loadViolations(AppDatabase *db) {
  initPaths();
  int loaded = 0;

  FILE *fp = fopen(pathViolations, "rb");
  if (fp != NULL) {
    if (loadViolationsFromStream(fp, db) == 0) {
      loaded = 1;
    }
    fclose(fp);
  }

  if (!loaded) {
    fp = fopen(pathBakViolations, "rb");
    if (fp != NULL) {
      printf(COLOR_YELLOW "[CANH BAO] File violations.dat bi loi hoac thieu. "
                          "Dang thu doc tu backup...\n" COLOR_RESET);
      if (loadViolationsFromStream(fp, db) == 0) {
        loaded = 1;
        fclose(fp);
        fileioSaveViolations(db); /* Restore main file */
      } else {
        fclose(fp);
      }
    }
  }

  if (!loaded) {
    /* First-run: create empty violations file */
    if (fileioSaveViolations(db) != 0) {
      return -1;
    }
  }

  db->nextViolationId = 1;
  for (int i = 0; i < db->violationCount; i++) {
    if (db->violations[i].id >= db->nextViolationId) {
      db->nextViolationId = db->violations[i].id + 1;
    }
  }

  return 0;
}

static int verifyDataIntegrity(const AppDatabase *db) {
  int warnings = 0;
  for (int i = 0; i < db->violationCount; i++) {
    const Violation *v = &db->violations[i];
    if (v->isVoided) {
      continue;
    }
    int memberIdx = memberFindById(db, v->studentId);
    if (memberIdx == -1) {
      printf(COLOR_YELLOW "[CANH BAO] Vi pham ID #%d tham chieu toi MSSV '%s' "
                          "nhung khong tim thay thanh vien!\n" COLOR_RESET,
             v->id, v->studentId);
      warnings++;
    }
  }
  return warnings;
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
  memberRebuildIndex(db);
  memberPurgeExpired(db, AUTO_PURGE_RETENTION_DAYS);
  verifyDataIntegrity(db);
  return 0;
}

/* File Archive structure:
   - Header magic: "FARC"
   - Version: 1
   - Pin hash (FNV1a of PIN)
   - Account count
   - Account encrypted data
   - Member count
   - Member encrypted data
   - Violation count
   - Violation encrypted data
   - Footer checksum (CRC32 of all contents up to this point)
*/

#define ARCHIVE_MAGIC "FARC"

static unsigned int hashPin(const char *pin) {
  unsigned int h = 0x811C9DC5U;
  for (int i = 0; pin[i] != '\0'; i++) {
    h ^= (unsigned char)pin[i];
    h *= 0x01000193U;
  }
  return h;
}

int fileioExportArchive(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  uiClear();
  uiDrawBreadcrumb(
      "MENU BAN CHU NHIEM -> [4] QUAN LY HE THONG -> Xuat du lieu (Export)");

  Account *session = authGetSession();
  char filename[256];

  /* Get timestamp */
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char timeStr[32];
  if (t != NULL) {
    strftime(timeStr, sizeof(timeStr), "%Y%m%d_%H%M%S", t);
  } else {
    snprintf(timeStr, sizeof(timeStr), "unknown");
  }

  /* Get computer name hash */
  char compName[128] = "unknown_pc";
#ifdef _WIN32
  DWORD cSize = sizeof(compName);
  if (!GetComputerNameA(compName, &cSize)) {
    strncpy(compName, "win_pc", sizeof(compName) - 1);
  }
#endif
  /* Clean compName from space and special characters */
  for (int i = 0; compName[i] != '\0'; i++) {
    if (compName[i] == ' ' || compName[i] == '/' || compName[i] == '\\' ||
        compName[i] == ':') {
      compName[i] = '_';
    }
  }

  /* Generate filename */
  snprintf(filename, sizeof(filename), "backup_%s_%s_%s.bin", timeStr,
           (session ? session->studentId : "unknown"), compName);

  printf(ERR_INFO "He thong tu dong dat ten file backup: " COLOR_YELLOW
                  "%s" COLOR_RESET "\n\n",
         filename);

  char pin[16];
  while (1) {
    printf(COLOR_CYAN "  Nhap ma PIN bao mat (4 ky tu so): " COLOR_RESET);
    readPassword(pin, sizeof(pin));
    trimSpaces(pin);
    if (strlen(pin) == 4 && isdigit((unsigned char)pin[0]) &&
        isdigit((unsigned char)pin[1]) && isdigit((unsigned char)pin[2]) &&
        isdigit((unsigned char)pin[3])) {
      break;
    }
    printf(ERR_LOI "Ma PIN phai co dung 4 ky tu so!\n");
  }

  /* Target output path (Save inside 'data/' folder) */
  char dataDir[512];
  char outPath[1024];
  char sep[2] = "/";
#ifdef _WIN32
  sep[0] = '\\';
#endif

  getExeDir(dataDir, sizeof(dataDir));
  strncat(dataDir, sep, sizeof(dataDir) - strlen(dataDir) - 1);
  strncat(dataDir, "data", sizeof(dataDir) - strlen(dataDir) - 1);
  snprintf(outPath, sizeof(outPath), "%s%s%s", dataDir, sep, filename);

  FILE *fp = fopen(outPath, "wb");
  if (fp == NULL) {
    printf(ERR_LOI "Khong the khoi tao hoac mo file de ghi: %s\n", filename);
    printf(ERR_INFO "Hay dam bao thu muc du lieu ton tai va co quyen ghi: %s\n",
           dataDir);
    uiPause();
    return -1;
  }

  /* Calculate CRC stream */
  unsigned int pinHashValue = hashPin(pin);

  /* Write Header Magic */
  fwrite(ARCHIVE_MAGIC, 1, 4, fp);

  /* Write Pin Hash */
  fwrite(&pinHashValue, sizeof(unsigned int), 1, fp);

  /* Write counts */
  fwrite(&db->accountCount, sizeof(int), 1, fp);
  fwrite(&db->memberCount, sizeof(int), 1, fp);
  fwrite(&db->violationCount, sizeof(int), 1, fp);
  fwrite(&db->nextViolationId, sizeof(int), 1, fp);

  /* Encrypt data with PIN byte key */
  unsigned char pinByte = (unsigned char)(pinHashValue & 0xFF);
  if (pinByte == 0)
    pinByte = 0xAA;

  /* Encrypt and write Accounts */
  if (db->accountCount > 0) {
    size_t sz = sizeof(Account) * (size_t)db->accountCount;
    unsigned char *buf = malloc(sz);
    if (buf) {
      memcpy(buf, db->accounts, sz);
      for (size_t i = 0; i < sz; i++)
        buf[i] ^= pinByte;
      fwrite(buf, 1, sz, fp);
      free(buf);
    }
  }

  /* Encrypt and write Members */
  if (db->memberCount > 0) {
    size_t sz = sizeof(Member) * (size_t)db->memberCount;
    unsigned char *buf = malloc(sz);
    if (buf) {
      memcpy(buf, db->members, sz);
      for (size_t i = 0; i < sz; i++)
        buf[i] ^= pinByte;
      fwrite(buf, 1, sz, fp);
      free(buf);
    }
  }

  /* Encrypt and write Violations */
  if (db->violationCount > 0) {
    size_t sz = sizeof(Violation) * (size_t)db->violationCount;
    unsigned char *buf = malloc(sz);
    if (buf) {
      memcpy(buf, db->violations, sz);
      for (size_t i = 0; i < sz; i++)
        buf[i] ^= pinByte;
      fwrite(buf, 1, sz, fp);
      free(buf);
    }
  }

  /* Encrypt and write system_audit.log */
  char exeDir[512];
  char auditPath[1024];
  getExeDir(exeDir, sizeof(exeDir));
  snprintf(auditPath, sizeof(auditPath), "%s/data/system_audit.log", exeDir);
  FILE *fLog = fopen(auditPath, "rb");
  int logSize = 0;
  unsigned char *logBuf = NULL;
  if (fLog != NULL) {
    fseek(fLog, 0, SEEK_END);
    logSize = (int)ftell(fLog);
    fseek(fLog, 0, SEEK_SET);
    if (logSize > 0) {
      logBuf = malloc((size_t)logSize);
      if (logBuf) {
        fread(logBuf, 1, (size_t)logSize, fLog);
        for (int i = 0; i < logSize; i++) {
          logBuf[i] ^= pinByte;
        }
      }
    }
    fclose(fLog);
  }

  fwrite(&logSize, sizeof(int), 1, fp);
  if (logSize > 0 && logBuf != NULL) {
    fwrite(logBuf, 1, (size_t)logSize, fp);
    free(logBuf);
  }

  fclose(fp);

  if (session != NULL) {
    logSystemAction(session->studentId, "Export du lieu", filename);
  }

  printf("\n" ERR_OK "Xuat file du lieu thanh cong vao thu muc 'data': %s\n",
         filename);
  printf(ERR_INFO "Duong dan day du: %s\n", outPath);
  printf(ERR_INFO "Vui long nho ma PIN da nhap de import duoc o may khac.\n");
  uiPause();
  return 0;
}

int fileioImportArchive(AppDatabase *db) {
  if (db == NULL) {
    return -1;
  }

  uiClear();
  uiDrawBreadcrumb(
      "MENU BAN CHU NHIEM -> [4] QUAN LY HE THONG -> Nhap du lieu (Import)");

  char filename[256];
  while (1) {
    printf(COLOR_CYAN "  Nhap ten file backup can nhap tu thu muc 'data' (mac "
                      "dinh: backup.bin, 0 de thoat): " COLOR_RESET);
    readString(filename, sizeof(filename));
    trimSpaces(filename);

    if (strcmp(filename, "0") == 0) {
      return -1;
    }
    if (strlen(filename) == 0) {
      strncpy(filename, "backup.bin", sizeof(filename) - 1);
      filename[sizeof(filename) - 1] = '\0';
      printf(
          ERR_INFO
          "Ban khong nhap ten file. He thong tim file mac dinh: " COLOR_YELLOW
          "data/backup.bin" COLOR_RESET "\n");
    }

    /* Validate path traversal or forbidden filesystem characters */
    int is_valid = 1;
    for (int i = 0; filename[i] != '\0'; i++) {
      char c = filename[i];
      if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' ||
          c == '"' || c == '<' || c == '>' || c == '|') {
        is_valid = 0;
        break;
      }
    }
    if (!is_valid) {
      printf(ERR_LOI "Ten file chua ky tu dac biet khong hop le (/, \\, :, *, "
                     "?, \", <, >, |)!\n");
      continue;
    }
    break;
  }

  /* Target path (Read from 'data/' folder) */
  char dataDir[512];
  char inPath[1024];
  char sep[2] = "/";
#ifdef _WIN32
  sep[0] = '\\';
#endif

  getExeDir(dataDir, sizeof(dataDir));
  strncat(dataDir, sep, sizeof(dataDir) - strlen(dataDir) - 1);
  strncat(dataDir, "data", sizeof(dataDir) - strlen(dataDir) - 1);
  snprintf(inPath, sizeof(inPath), "%s%s%s", dataDir, sep, filename);

  FILE *fp = fopen(inPath, "rb");
  if (fp == NULL) {
    printf(ERR_LOI "Khong tim thay file backup: %s\n", filename);
    printf(ERR_INFO "Vui long dam bao file backup nay da duoc copy vao thu muc "
                    "du lieu: %s\n",
           dataDir);
    uiPause();
    return -1;
  }

  char magic[4];
  if (fread(magic, 1, 4, fp) != 4 || memcmp(magic, ARCHIVE_MAGIC, 4) != 0) {
    printf(ERR_LOI "File backup khong hop le hoac bi loi dinh dang!\n");
    fclose(fp);
    uiPause();
    return -1;
  }

  unsigned int filePinHash = 0;
  if (fread(&filePinHash, sizeof(unsigned int), 1, fp) != 1) {
    printf(ERR_LOI "Khong the doc thong tin PIN tu file!\n");
    fclose(fp);
    uiPause();
    return -1;
  }

  char pin[16];
  printf(COLOR_CYAN
         "  Nhap ma PIN bao mat cua file (4 ky tu so): " COLOR_RESET);
  readPassword(pin, sizeof(pin));
  trimSpaces(pin);

  if (hashPin(pin) != filePinHash) {
    printf(ERR_LOI "Ma PIN khong khop! Giai ma file that bai.\n");
    fclose(fp);
    uiPause();
    return -1;
  }

  /* Read metadata */
  int accCount = 0, memCount = 0, vioCount = 0, nextVioId = 0;
  if (fread(&accCount, sizeof(int), 1, fp) != 1 ||
      fread(&memCount, sizeof(int), 1, fp) != 1 ||
      fread(&vioCount, sizeof(int), 1, fp) != 1 ||
      fread(&nextVioId, sizeof(int), 1, fp) != 1) {
    printf(ERR_LOI "Khong the doc thong tin metadata tu file!\n");
    fclose(fp);
    uiPause();
    return -1;
  }

  if (accCount < 0 || accCount > MAX_MEMBERS || memCount < 0 ||
      memCount > MAX_MEMBERS || vioCount < 0 || vioCount > MAX_VIOLATIONS) {
    printf(ERR_LOI "Kich thuoc file backup vuot qua gioi han bo nho!\n");
    fclose(fp);
    uiPause();
    return -1;
  }

  /* Prepare buffers */
  Account *tempAccounts = malloc(sizeof(Account) * (size_t)accCount);
  Member *tempMembers = malloc(sizeof(Member) * (size_t)memCount);
  Violation *tempViolations = malloc(sizeof(Violation) * (size_t)vioCount);

  if ((accCount > 0 && !tempAccounts) || (memCount > 0 && !tempMembers) ||
      (vioCount > 0 && !tempViolations)) {
    printf(ERR_LOI "Loi cap phat bo nho he thong!\n");
    if (tempAccounts)
      free(tempAccounts);
    if (tempMembers)
      free(tempMembers);
    if (tempViolations)
      free(tempViolations);
    fclose(fp);
    uiPause();
    return -1;
  }

  unsigned char pinByte = (unsigned char)(filePinHash & 0xFF);
  if (pinByte == 0)
    pinByte = 0xAA;

  /* Read Accounts */
  if (accCount > 0) {
    size_t sz = sizeof(Account) * (size_t)accCount;
    if (fread(tempAccounts, 1, sz, fp) != sz) {
      printf(ERR_LOI "Loi doc danh sach tai khoan!\n");
      goto cleanup;
    }
    for (size_t i = 0; i < sz; i++)
      ((unsigned char *)tempAccounts)[i] ^= pinByte;
  }

  /* Read Members */
  if (memCount > 0) {
    size_t sz = sizeof(Member) * (size_t)memCount;
    if (fread(tempMembers, 1, sz, fp) != sz) {
      printf(ERR_LOI "Loi doc danh sach thanh vien!\n");
      goto cleanup;
    }
    for (size_t i = 0; i < sz; i++)
      ((unsigned char *)tempMembers)[i] ^= pinByte;
  }

  /* Read Violations */
  if (vioCount > 0) {
    size_t sz = sizeof(Violation) * (size_t)vioCount;
    if (fread(tempViolations, 1, sz, fp) != sz) {
      printf(ERR_LOI "Loi doc danh sach vi pham!\n");
      goto cleanup;
    }
    for (size_t i = 0; i < sz; i++)
      ((unsigned char *)tempViolations)[i] ^= pinByte;
  }

  /* Read system_audit.log */
  int logSize = 0;
  unsigned char *logBuf = NULL;
  if (fread(&logSize, sizeof(int), 1, fp) == 1 && logSize > 0) {
    logBuf = malloc((size_t)logSize);
    if (logBuf) {
      if (fread(logBuf, 1, (size_t)logSize, fp) == (size_t)logSize) {
        for (int i = 0; i < logSize; i++) {
          logBuf[i] ^= pinByte;
        }
      } else {
        free(logBuf);
        logBuf = NULL;
        logSize = 0;
      }
    }
  }

  fclose(fp);

  /* Import data to RAM database */
  db->accountCount = accCount;
  db->memberCount = memCount;
  db->violationCount = vioCount;
  db->nextViolationId = nextVioId;

  if (accCount > 0)
    memcpy(db->accounts, tempAccounts, sizeof(Account) * (size_t)accCount);
  if (memCount > 0)
    memcpy(db->members, tempMembers, sizeof(Member) * (size_t)memCount);
  if (vioCount > 0)
    memcpy(db->violations, tempViolations,
           sizeof(Violation) * (size_t)vioCount);

  free(tempAccounts);
  free(tempMembers);
  free(tempViolations);

  /* Save RAM database back locally */
  if (fileioSaveAccounts(db) != 0 || fileioSaveMembers(db) != 0 ||
      fileioSaveViolations(db) != 0) {
    printf(ERR_LOI "Khong the ghi du lieu ra o dia goc!\n");
    if (logBuf)
      free(logBuf);
    uiPause();
    return -1;
  }

  /* Restore system_audit.log */
  if (logSize > 0 && logBuf != NULL) {
    char exeDir[512];
    char destAuditPath[1024];
    getExeDir(exeDir, sizeof(exeDir));
    snprintf(destAuditPath, sizeof(destAuditPath), "%s/data/system_audit.log",
             exeDir);
    FILE *fdLog = fopen(destAuditPath, "wb");
    if (fdLog != NULL) {
      fwrite(logBuf, 1, (size_t)logSize, fdLog);
      fclose(fdLog);
    }
    free(logBuf);
  }

  memberRebuildIndex(db);

  Account *session = authGetSession();
  if (session != NULL) {
    logSystemAction(session->studentId, "Import du lieu", filename);
  }

  printf("\n" ERR_OK
         "Nhap (Import) va khoi phuc du lieu thanh cong vao may tinh nay!\n");
  uiPause();
  return 0;

cleanup:
  if (tempAccounts)
    free(tempAccounts);
  if (tempMembers)
    free(tempMembers);
  if (tempViolations)
    free(tempViolations);
  fclose(fp);
  uiPause();
  return -1;
}
