#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "utils.h"
#include "types.h"
#include "ui.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

/* ============================================================
 * INPUT HANDLING HELPERS
 * ============================================================ */

void readString(char *buffer, size_t size) {
  if (buffer == NULL || size == 0) {
    return;
  }

  if (fgets(buffer, (int)size, stdin) != NULL) {
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
      buffer[len - 1] = '\0';
    } else {
      /* Buffer is full and no newline was read. Flush stdin. */
      int c;
      while ((c = getchar()) != '\n' && c != EOF) {
        /* discard characters */
      }
    }
  } else {
    buffer[0] = '\0';
  }
}

int readInt(int *value) {
  int result = scanf("%d", value);
  int c;
  while ((c = getchar()) != '\n' && c != EOF) {
    /* discard characters */
  }
  return result == 1 ? 1 : 0;
}

void readPassword(char *buffer, size_t size) {
  if (buffer == NULL || size == 0) {
    return;
  }

  size_t i = 0;
#ifdef _WIN32
  int ch;
  while ((ch = _getch()) != '\r' && ch != EOF) {
    if (ch == '\b' || ch == 127) {
      /* Backspace */
      if (i > 0) {
        i--;
        printf("\b \b");
      }
    } else if (ch >= 32 && ch < 127 && i < size - 1) {
      buffer[i++] = (char)ch;
      printf("*");
    }
  }
#else
  /* Linux: use termios for no-echo */
  struct termios oldTerm, newTerm;
  tcgetattr(STDIN_FILENO, &oldTerm);
  newTerm = oldTerm;
  newTerm.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSANOW, &newTerm);

  int ch;
  while ((ch = getchar()) != '\n' && ch != EOF) {
    if (ch == 127 || ch == '\b') {
      if (i > 0) {
        i--;
        printf("\b \b");
      }
    } else if (ch >= 32 && ch < 127 && i < size - 1) {
      buffer[i++] = (char)ch;
      printf("*");
    }
  }
  tcsetattr(STDIN_FILENO, TCSANOW, &oldTerm);
#endif
  buffer[i] = '\0';
  printf("\n");
}

int readMenuChoice(const char *prompt, int min, int max) {
  int choice;
  while (1) {
    printf("%s", prompt);
    if (readInt(&choice)) {
      if (choice >= min && choice <= max) {
        return choice;
      }
      printf(ERR_LOI "Lua chon khong hop le! "
                     "Vui long chon tu %d-%d!\n",
             min, max);
    } else {
      printf(ERR_LOI "Vui long nhap so!\n");
    }
  }
}

/* ============================================================
 * VALIDATION HELPERS
 * ============================================================ */

int isIdValid(const char *id) {
  if (id == NULL || strlen(id) == 0) {
    return 0;
  }

  size_t len = strlen(id);

  /* MSSV should be at least 4 characters (e.g. "ADMIN") and at most
   * MAX_MSSV_LEN-1 */
  if (len < 4 || len >= MAX_MSSV_LEN) {
    return 0;
  }

  /* Must start with a letter */
  if (!isalpha((unsigned char)id[0])) {
    return 0;
  }

  /* Check if alphanumeric */
  for (size_t i = 0; i < len; i++) {
    if (!isalnum((unsigned char)id[i])) {
      return 0;
    }
  }
  return 1;
}

/* ============================================================
 * TIME & DATE HELPERS
 * ============================================================ */

void formatTime(time_t t, char *buffer, size_t bufSize) {
  if (buffer == NULL || bufSize == 0) {
    return;
  }

  /* Note: localtime is not thread-safe but this application is
     single-threaded. */
  struct tm *timeinfo = localtime(&t);
  if (timeinfo != NULL) {
    strftime(buffer, bufSize, "%d/%m/%Y %H:%M", timeinfo);
  } else {
    buffer[0] = '\0';
  }
}

int parseDate(const char *buffer, time_t *t, int isEndOfDay) {
  if (buffer == NULL || t == NULL) {
    return 0;
  }

  int d;
  int m;
  int y;
  if (sscanf(buffer, "%d/%d/%d", &d, &m, &y) != 3) {
    return 0;
  }

  struct tm timeinfo;
  memset(&timeinfo, 0, sizeof(struct tm));

  timeinfo.tm_mday = d;
  timeinfo.tm_mon = m - 1; /* months are 0-11 */
  timeinfo.tm_year = y - 1900;
  timeinfo.tm_isdst = -1; /* let system determine DST */

  if (isEndOfDay) {
    timeinfo.tm_hour = 23;
    timeinfo.tm_min = 59;
    timeinfo.tm_sec = 59;
  } else {
    timeinfo.tm_hour = 0;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;
  }

  *t = mktime(&timeinfo);

  if (*t == (time_t)-1) {
    return 0;
  }

  return 1;
}

/* ============================================================
 * DISPLAY NAME MAPPERS
 * ============================================================ */

const char *teamName(int teamId) {
  switch (teamId) {
  case TEAM_ACADEMIC:
    return "Hoc thuat";
  case TEAM_PLANNING:
    return "Ke hoach";
  case TEAM_HR:
    return "Nhan su";
  case TEAM_MEDIA:
    return "Truyen thong";
  default:
    return "Khong xac dinh";
  }
}

const char *memberRoleName(int roleId) {
  switch (roleId) {
  case MEMBER_ROLE_MEMBER:
    return "Thanh vien";
  case MEMBER_ROLE_LEADER:
    return "Truong nhom/Pho nhom";
  case MEMBER_ROLE_DIRECTOR:
    return "Ban chu nhiem";
  default:
    return "Khong xac dinh";
  }
}

const char *accountRoleName(int roleId) {
  switch (roleId) {
  case ACCOUNT_ROLE_MEMBER:
    return "Thanh vien";
  case ACCOUNT_ROLE_DIRECTOR:
    return "Ban chu nhiem";
  default:
    return "Khong xac dinh";
  }
}

const char *reasonName(int reasonId) {
  switch (reasonId) {
  case REASON_NO_JACKET:
    return "Khong mac ao CLB";
  case REASON_ABSENT:
    return "Vang mat";
  case REASON_NO_ACTIVITY:
    return "Khong tham gia HD";
  case REASON_VIOLENCE:
    return "Bao luc";
  default:
    return "Khong xac dinh";
  }
}

/* ============================================================
 * PATH & DIRECTORY HELPERS
 * ============================================================ */

void getExeDir(char *buffer, size_t size) {
  if (buffer == NULL || size == 0) {
    return;
  }
#ifdef _WIN32
  GetModuleFileNameA(NULL, buffer, (DWORD)size);
  char *lastSlash = strrchr(buffer, '\\');
  if (lastSlash != NULL) {
    *lastSlash = '\0';
  }
#else
  /* POSIX implementation using /proc/self/exe */
  ssize_t len = readlink("/proc/self/exe", buffer, size - 1);
  if (len != -1) {
    buffer[len] = '\0';
    char *lastSlash = strrchr(buffer, '/');
    if (lastSlash != NULL) {
      *lastSlash = '\0';
    }
  } else {
    /* Fallback to current directory */
    strncpy(buffer, ".", size);
    buffer[size - 1] = '\0';
  }
#endif
}

void resolvePath(const char *subDir, const char *inputPath, char *outPath,
                 size_t outSize) {
  if (outPath == NULL || outSize == 0) {
    return;
  }

  int isAbsolute = 0;
  if (inputPath != NULL && strlen(inputPath) > 0) {
#ifdef _WIN32
    if ((strlen(inputPath) > 1 && inputPath[1] == ':') ||
        inputPath[0] == '\\' || inputPath[0] == '/') {
      isAbsolute = 1;
    }
#else
    if (inputPath[0] == '/') {
      isAbsolute = 1;
    }
#endif
  }

  if (isAbsolute) {
    strncpy(outPath, inputPath, outSize - 1);
    outPath[outSize - 1] = '\0';
  } else {
    char exeDir[512];
    char sep[2] = "/";
#ifdef _WIN32
    sep[0] = '\\';
#endif
    getExeDir(exeDir, sizeof(exeDir));

    if (subDir != NULL && strlen(subDir) > 0) {
      snprintf(outPath, outSize, "%s%s%s%s%s", exeDir, sep, subDir, sep,
               (inputPath ? inputPath : ""));
    } else {
      snprintf(outPath, outSize, "%s%s%s", exeDir, sep,
               (inputPath ? inputPath : ""));
    }
  }
}

typedef struct {
  unsigned char data[64];
  unsigned int datalen;
  unsigned long long bitlen;
  unsigned int state[8];
} Sha256Ctx;

#define ROTRIGHT(word, bits) (((word) >> (bits)) | ((word) << (32 - (bits))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x, 2) ^ ROTRIGHT(x, 13) ^ ROTRIGHT(x, 22))
#define EP1(x) (ROTRIGHT(x, 6) ^ ROTRIGHT(x, 11) ^ ROTRIGHT(x, 25))
#define SIG0(x) (ROTRIGHT(x, 7) ^ ROTRIGHT(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x, 17) ^ ROTRIGHT(x, 19) ^ ((x) >> 10))

static const unsigned int K_SHA256[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

static void sha256Transform(Sha256Ctx *ctx, const unsigned char data[]) {
  unsigned int a;
  unsigned int b;
  unsigned int c;
  unsigned int d;
  unsigned int e;
  unsigned int f;
  unsigned int g;
  unsigned int h;
  unsigned int i;
  unsigned int j;
  unsigned int t1;
  unsigned int t2;
  unsigned int m[64];

  for (i = 0, j = 0; i < 16; ++i, j += 4) {
    m[i] = ((unsigned int)data[j] << 24) | ((unsigned int)data[j + 1] << 16) |
           ((unsigned int)data[j + 2] << 8) | ((unsigned int)data[j + 3]);
  }
  for (; i < 64; ++i) {
    m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
  }

  a = ctx->state[0];
  b = ctx->state[1];
  c = ctx->state[2];
  d = ctx->state[3];
  e = ctx->state[4];
  f = ctx->state[5];
  g = ctx->state[6];
  h = ctx->state[7];

  for (i = 0; i < 64; ++i) {
    t1 = h + EP1(e) + CH(e, f, g) + K_SHA256[i] + m[i];
    t2 = EP0(a) + MAJ(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + t1;
    d = c;
    c = b;
    b = a;
    a = t1 + t2;
  }

  ctx->state[0] += a;
  ctx->state[1] += b;
  ctx->state[2] += c;
  ctx->state[3] += d;
  ctx->state[4] += e;
  ctx->state[5] += f;
  ctx->state[6] += g;
  ctx->state[7] += h;
}

static void sha256Init(Sha256Ctx *ctx) {
  ctx->datalen = 0;
  ctx->bitlen = 0;
  ctx->state[0] = 0x6a09e667;
  ctx->state[1] = 0xbb67ae85;
  ctx->state[2] = 0x3c6ef372;
  ctx->state[3] = 0xa54ff53a;
  ctx->state[4] = 0x510e527f;
  ctx->state[5] = 0x9b05688c;
  ctx->state[6] = 0x1f83d9ab;
  ctx->state[7] = 0x5be0cd19;
}

static void sha256Update(Sha256Ctx *ctx, const unsigned char data[],
                         size_t len) {
  for (size_t i = 0; i < len; ++i) {
    ctx->data[ctx->datalen] = data[i];
    ctx->datalen++;
    if (ctx->datalen == 64) {
      sha256Transform(ctx, ctx->data);
      ctx->bitlen += 512;
      ctx->datalen = 0;
    }
  }
}

static void sha256Final(Sha256Ctx *ctx, unsigned char hash[]) {
  unsigned int i = ctx->datalen;

  /* Pad with a single 1 bit, followed by 0 bits */
  if (ctx->datalen < 56) {
    ctx->data[i++] = 0x80;
    while (i < 56) {
      ctx->data[i++] = 0x00;
    }
  } else {
    ctx->data[i++] = 0x80;
    while (i < 64) {
      ctx->data[i++] = 0x00;
    }
    sha256Transform(ctx, ctx->data);
    memset(ctx->data, 0, 56);
  }

  /* Append the length of the message in bits as a big-endian 64-bit integer */
  ctx->bitlen += (unsigned long long)ctx->datalen * 8ULL;
  ctx->data[63] = (unsigned char)(ctx->bitlen & 0xFF);
  ctx->data[62] = (unsigned char)((ctx->bitlen >> 8) & 0xFF);
  ctx->data[61] = (unsigned char)((ctx->bitlen >> 16) & 0xFF);
  ctx->data[60] = (unsigned char)((ctx->bitlen >> 24) & 0xFF);
  ctx->data[59] = (unsigned char)((ctx->bitlen >> 32) & 0xFF);
  ctx->data[58] = (unsigned char)((ctx->bitlen >> 40) & 0xFF);
  ctx->data[57] = (unsigned char)((ctx->bitlen >> 48) & 0xFF);
  ctx->data[56] = (unsigned char)((ctx->bitlen >> 56) & 0xFF);
  sha256Transform(ctx, ctx->data);

  /* Convert the state to big-endian bytes */
  for (i = 0; i < 4; ++i) {
    hash[i] = (unsigned char)((ctx->state[0] >> (24 - i * 8)) & 0x000000ff);
    hash[i + 4] = (unsigned char)((ctx->state[1] >> (24 - i * 8)) & 0x000000ff);
    hash[i + 8] = (unsigned char)((ctx->state[2] >> (24 - i * 8)) & 0x000000ff);
    hash[i + 12] =
        (unsigned char)((ctx->state[3] >> (24 - i * 8)) & 0x000000ff);
    hash[i + 16] =
        (unsigned char)((ctx->state[4] >> (24 - i * 8)) & 0x000000ff);
    hash[i + 20] =
        (unsigned char)((ctx->state[5] >> (24 - i * 8)) & 0x000000ff);
    hash[i + 24] =
        (unsigned char)((ctx->state[6] >> (24 - i * 8)) & 0x000000ff);
    hash[i + 28] =
        (unsigned char)((ctx->state[7] >> (24 - i * 8)) & 0x000000ff);
  }
}

void hashPassword(const char *password, const char *salt, char *outHashHex) {
  char temp[256];
  unsigned char hash[32];
  char hexHash[65];
  Sha256Ctx ctx;

  /* First iteration: hash (password + salt) */
  snprintf(temp, sizeof(temp), "%s%s", password, salt);

  sha256Init(&ctx);
  sha256Update(&ctx, (const unsigned char *)temp, strlen(temp));
  sha256Final(&ctx, hash);

  for (size_t i = 0; i < 32; i++) {
    snprintf(&hexHash[i * 2], 3, "%02x", hash[i]);
  }
  hexHash[64] = '\0';

  /* Remaining 999 iterations: hash (prev_hex_hash + salt) */
  for (int iter = 1; iter < 1000; iter++) {
    snprintf(temp, sizeof(temp), "%s%s", hexHash, salt);

    sha256Init(&ctx);
    sha256Update(&ctx, (const unsigned char *)temp, strlen(temp));
    sha256Final(&ctx, hash);

    for (size_t i = 0; i < 32; i++) {
      snprintf(&hexHash[i * 2], 3, "%02x", hash[i]);
    }
  }

  /* Output is exactly 64 hex characters, write it to outHashHex */
  strncpy(outHashHex, hexHash, 65);
  outHashHex[64] = '\0';
}

void secureZero(void *ptr, size_t len) {
  if (ptr == NULL || len == 0) {
    return;
  }
  volatile unsigned char *p = (volatile unsigned char *)ptr;
  while (len--) {
    *p++ = 0;
  }
}

void generateSalt(char *salt, size_t size) {
  const char CHARSET[] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  if (size == 0) {
    return;
  }
  for (size_t n = 0; n < size - 1; n++) {
    int key = rand() % (int)(sizeof(CHARSET) - 1);
    salt[n] = CHARSET[key];
  }
  salt[size - 1] = '\0';
}

void logSystemAction(const char *actor, const char *action,
                     const char *target) {
  char exeDir[512];
  char auditPath[1024];

  getExeDir(exeDir, sizeof(exeDir));

#ifdef _WIN32
  snprintf(auditPath, sizeof(auditPath), "%s\\data\\system_audit.log", exeDir);
#else
  snprintf(auditPath, sizeof(auditPath), "%s/data/system_audit.log", exeDir);
#endif

  time_t now = time(NULL);
  /* Note: localtime is not thread-safe but this application is
     single-threaded. */
  struct tm *t = localtime(&now);
  char timeBuf[32];
  if (t != NULL) {
    strftime(timeBuf, sizeof(timeBuf), "%d/%m/%Y %H:%M:%S", t);
  } else {
    snprintf(timeBuf, sizeof(timeBuf), "Unknown Time");
  }

  /* 1. Append to audit log (Encrypted with XOR) */
  FILE *fa = fopen(auditPath, "ab");
  if (fa != NULL) {
    char logLine[1024];
    snprintf(logLine, sizeof(logLine),
             "[%s] [%-10s] ACTION: %-25s | TARGET: %s\n", timeBuf, actor,
             action, target);

    /* Encrypt line with a simple fixed key or machine key */
    size_t lineLen = strlen(logLine);
    for (size_t i = 0; i < lineLen - 1;
         i++) { /* Keep newline unencrypted to preserve line-by-line structure
                 */
      logLine[i] = (char)(logLine[i] ^ 0x5A);
    }
    fwrite(logLine, 1, lineLen, fa);
    fclose(fa);
  }
}

void viewSystemLogs(void) {
  char exeDir[512];
  char auditPath[1024];
  getExeDir(exeDir, sizeof(exeDir));

#ifdef _WIN32
  snprintf(auditPath, sizeof(auditPath), "%s\\data\\system_audit.log", exeDir);
#else
  snprintf(auditPath, sizeof(auditPath), "%s/data/system_audit.log", exeDir);
#endif

  FILE *f = fopen(auditPath, "rb");
  if (f == NULL) {
    /* Attempt to create the system audit log file if it does not exist yet */
    f = fopen(auditPath, "wb");
    if (f != NULL) {
      fclose(f);
      f = fopen(auditPath, "rb");
    }
  }

  if (f == NULL) {
    printf(ERR_LOI "Khong the khoi tao hoac mo file nhat ky he thong!\n");
    printf(ERR_INFO "Thuong truc tai: %s\n", auditPath);
    uiPause();
    return;
  }

  uiClear();
  uiDrawBreadcrumb("[4] Quan ly he thong -> [2] Xem nhat ky he thong");

  char line[1024];
  int lineCount = 0;
  int pageSize = 20;

  while (fgets(line, sizeof(line), f) != NULL) {
    size_t len = strlen(line);
    if (len > 0) {
      /* Decrypt line */
      for (size_t i = 0; i < len; i++) {
        if (line[i] != '\n' && line[i] != '\r') {
          line[i] = (char)(line[i] ^ 0x5A);
        }
      }
    }
    if (len > 0 && line[len - 1] == '\n') {
      line[len - 1] = '\0';
    }
    if (len > 1 && line[len - 2] == '\r') {
      line[len - 2] = '\0';
    }

    lineCount++;

    printf("  " COLOR_GRAY "%4d. " COLOR_RESET, lineCount);
    printf(COLOR_CYAN LINE_V COLOR_RESET " ");

    char timePart[64];
    char actorPart[64];
    char actionPart[64];
    char targetPart[256];
    timePart[0] = '\0';
    actorPart[0] = '\0';
    actionPart[0] = '\0';
    targetPart[0] = '\0';

    int parsed =
        sscanf(line, "[%63[^]]] [%63[^]]] ACTION: %63[^|]| TARGET: %255[^\n]",
               timePart, actorPart, actionPart, targetPart);

    if (parsed == 4) {
      printf(COLOR_DIM "[%s]" COLOR_RESET " ", timePart);
      printf(COLOR_GREEN "[%s]" COLOR_RESET " ", actorPart);
      printf(COLOR_YELLOW "%s" COLOR_RESET, actionPart);
      printf(COLOR_RESET " | " COLOR_RESET);
      printf(COLOR_CYAN "%s" COLOR_RESET, targetPart);
    } else {
      printf("%s", line);
    }

    printf("\n" COLOR_RESET);

    if (lineCount % pageSize == 0) {
      printf("\n" COLOR_DIM "  -- Nhan Enter de xem tiep (hoac 'q' + Enter de "
             "thoat) --" COLOR_RESET);
      int ch = getchar();
      if (ch == 'q' || ch == 'Q' || ch == EOF) {
        break;
      }
      if (ch != '\n') {
        int nextCh;
        while ((nextCh = getchar()) != '\n' && nextCh != EOF) {
          ;
        }
      }
      uiDrawBreadcrumb(
          "[4] Quan ly he thong -> [2] Xem nhat ky he thong (tiep)");
    }
  }

  fclose(f);

  if (lineCount == 0) {
    printf("  " COLOR_DIM "(Trong)" COLOR_RESET "\n");
  } else {
    printf("\n" COLOR_GREEN "  Tong cong: %d dong nhat ky." COLOR_RESET "\n",
           lineCount);
  }
  uiPause();
}

unsigned int calculateCrc32(const unsigned char *data, size_t length) {
  unsigned int crc = 0xFFFFFFFFU;
  if (data == NULL) {
    return 0;
  }
  for (size_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 1) {
        crc = (crc >> 1) ^ 0xEDB88320U;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc ^ 0xFFFFFFFFU;
}

/* ============================================================
 * UI/UX HELPERS - Smooth Transitions
 * ============================================================ */

void uiSleep(int milliseconds) {
#ifdef _WIN32
  Sleep((DWORD)milliseconds);
#else
  struct timespec ts;
  ts.tv_sec = milliseconds / 1000;
  ts.tv_nsec = (milliseconds % 1000) * 1000000;
  nanosleep(&ts, NULL);
#endif
}
