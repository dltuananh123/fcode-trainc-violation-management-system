/**
 * Standalone seed-data generator (v2.1 upgraded).
 * Writes members.dat, violations.dat, accounts.dat with realistic demo data,
 * using the secure magic signature (FCE1) and salted password hashing.
 * Compile: gcc -std=c17 -m64 -Iinclude tools/seed_data.c -o bin/seed_data.exe
 * Run:     bin/seed_data.exe
 */
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

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

static void hashPassword(const char *password, const char *salt,
                         char *outHashHex) {
  char temp[256];
  unsigned char hash[32];
  char hexHash[65];
  Sha256Ctx ctx;

  snprintf(temp, sizeof(temp), "%s%s", password, salt);

  sha256Init(&ctx);
  sha256Update(&ctx, (const unsigned char *)temp, strlen(temp));
  sha256Final(&ctx, hash);

  for (int i = 0; i < 32; i++) {
    snprintf(&hexHash[i * 2], 3, "%02x", hash[i]);
  }
  hexHash[64] = '\0';

  for (int iter = 1; iter < 1000; iter++) {
    snprintf(temp, sizeof(temp), "%s%s", hexHash, salt);

    sha256Init(&ctx);
    sha256Update(&ctx, (const unsigned char *)temp, strlen(temp));
    sha256Final(&ctx, hash);

    for (int i = 0; i < 32; i++) {
      snprintf(&hexHash[i * 2], 3, "%02x", hash[i]);
    }
  }

  strncpy(outHashHex, hexHash, 65);
  outHashHex[64] = '\0';
}

static void generateSalt(char *salt, size_t size) {
  const char charset[] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  if (size == 0) {
    return;
  }
  for (size_t n = 0; n < size - 1; n++) {
    int key = rand() % (int)(sizeof(charset) - 1);
    salt[n] = charset[key];
  }
  salt[size - 1] = '\0';
}

static void getExeDir(char *buffer, size_t size) {
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
  ssize_t len = readlink("/proc/self/exe", buffer, size - 1);
  if (len != -1) {
    buffer[len] = '\0';
    char *lastSlash = strrchr(buffer, '/');
    if (lastSlash != NULL) {
      *lastSlash = '\0';
    }
  } else {
    strncpy(buffer, ".", size);
    buffer[size - 1] = '\0';
  }
#endif
}

static unsigned char gXorKey[32];
#define XOR_KEY_LEN 32
static int gXorKeyDerived = 0;

static void deriveXorKey(unsigned char *key, size_t keyLen) {
  char seed[1024] = {0};
  char exeDir[512] = {0};
  getExeDir(exeDir, sizeof(exeDir));

  snprintf(seed, sizeof(seed), "%s", exeDir);

#ifdef _WIN32
  char compName[256] = {0};
  DWORD size = sizeof(compName);
  if (GetComputerNameA(compName, &size)) {
    strncat(seed, compName, sizeof(seed) - strlen(seed) - 1);
  }
#endif

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

static void xorBuffer(unsigned char *data, size_t size) {
  if (!gXorKeyDerived) {
    deriveXorKey(gXorKey, XOR_KEY_LEN);
    gXorKeyDerived = 1;
  }
  for (size_t i = 0; i < size; i++) {
    data[i] ^= gXorKey[i % XOR_KEY_LEN];
  }
}

static int writeFileEncrypted(const char *path, const void *data,
                              size_t itemSize, int count) {
  FILE *fp = fopen(path, "wb");
  if (!fp) {
    printf("[LOI] Khong the ghi %s\n", path);
    return -1;
  }
  fwrite("FCE1", 1, 4, fp);
  fwrite(&count, sizeof(int), 1, fp);
  if (count > 0) {
    size_t totalSize = itemSize * (size_t)count;
    unsigned char *encrypted = (unsigned char *)malloc(totalSize);
    if (!encrypted) {
      printf("[LOI] Het bo nho heap khi ghi file!\n");
      fclose(fp);
      return -1;
    }
    memcpy(encrypted, data, totalSize);
    xorBuffer(encrypted, totalSize);
    fwrite(encrypted, itemSize, (size_t)count, fp);
    free(encrypted);
  }
  fclose(fp);
  return 0;
}

static int writeEmptyFile(const char *path) {
  FILE *fp = fopen(path, "wb");
  if (!fp) {
    printf("[LOI] Khong the ghi %s\n", path);
    return -1;
  }
  int zero = 0;
  fwrite("FCE1", 1, 4, fp);
  fwrite(&zero, sizeof(int), 1, fp);
  fclose(fp);
  return 0;
}

/* ---- helpers ---- */

static void makeMember(Member *m, const char *id, const char *name, int team,
                       int role, double totalFine, int consecAbs,
                       int violCount) {
  memset(m, 0, sizeof(Member));
  strncpy(m->studentId, id, MAX_MSSV_LEN - 1);
  strncpy(m->fullName, name, MAX_NAME_LEN - 1);
  snprintf(m->email, MAX_EMAIL_LEN, "%s@fcode.vn", id);
  strncpy(m->phone, "0901234567", MAX_PHONE_LEN - 1);
  m->team = team;
  m->role = role;
  m->isActive = STATUS_ACTIVE;
  m->isDeleted = 0;
  m->deletedAt = 0;
  m->totalFine = totalFine;
  m->consecutiveAbsences = consecAbs;
  m->violationCount = violCount;
}

static time_t dateSec(int y, int m, int d) {
  struct tm t;
  memset(&t, 0, sizeof(t));
  t.tm_year = y - 1900;
  t.tm_mon = m - 1;
  t.tm_mday = d;
  t.tm_hour = 8;
  return mktime(&t);
}

static void makeViolation(Violation *v, const char *id, time_t t, int reason,
                          double fine, int isPaid, int penalty,
                          const char *note) {
  static int currentId = 1;
  memset(v, 0, sizeof(Violation));
  v->id = currentId++;
  strncpy(v->studentId, id, MAX_MSSV_LEN - 1);
  v->violationTime = t;
  v->reason = reason;
  v->fine = fine;
  v->isPaid = isPaid;
  v->paidAt = isPaid ? t : 0;
  v->penalty = penalty;
  strncpy(v->note, note, MAX_NOTE_LEN - 1);
}

static void ensureDir(const char *dir) {
#ifdef _WIN32
  _mkdir(dir);
#else
  mkdir(dir, 0755);
#endif
}

static int clearData(void) {
  const char *files[] = {"members", "violations", "accounts"};
  char path[256];
#ifdef _WIN32
  const char *sep = "\\";
#else
  const char *sep = "/";
#endif

  for (int i = 0; i < 3; i++) {
    snprintf(path, sizeof(path), "data%s%s.dat", sep, files[i]);
    if (writeEmptyFile(path) != 0)
      return 1;
    printf("[OK] Cleared %s (Wrote empty encrypted file)\n", path);
    snprintf(path, sizeof(path), "bin%sdata%s%s.dat", sep, sep, files[i]);
    writeEmptyFile(path);
  }

  printf("\nData cleared. Next run of system will initialize a default "
         "administrator account.\n");
  return 0;
}

int main(int argc, char *argv[]) {
  srand((unsigned int)time(NULL));

  if (argc > 1 && strcmp(argv[1], "clear") == 0) {
    return clearData();
  }

  static const char *files[] = {"members", "violations", "accounts"};

  Member members[100];
  Account accounts[100];
  Violation violations[100];
  int mc = 0, ac = 0, vc = 0;

  /* === Seed real Challenge 3 members === */
  /* Team mapping: Nhom 1-4 -> Hoc thuat, 5-7 -> Ke hoach, 8-10 -> Nhan su,
   * 11-14 -> Truyen thong */
  static const struct {
    const char *id, *name;
    int team, role;
  } seedMembers[] = {
      /* === NHOM 1-4: Hoc thuat (TEAM_ACADEMIC) === */
      {"SE201018", "Lam Hoang An", TEAM_ACADEMIC, MEMBER_ROLE_LEADER},
      {"SE200972", "Mai Xuan Hieu", TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
      {"SE212092", "Nguyen Tien Khai", TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
      {"SE211866", "Pham Gia Bao", TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
      {"SE203677", "Ngo Ngoc Gia Han", TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
      {"SE210946", "Nguyen Vu Hao", TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
      {"SE210474", "Do Thanh Binh", TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
      {"SE200516", "Tran Vu Hai Duy", TEAM_ACADEMIC, MEMBER_ROLE_LEADER},
      {"SE210518", "Le Gia Phuc Chanh", TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
      {"SE210364", "Chu Minh Cuong", TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
      {"SE210380", "Nguyen Phi Lam", TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
      {"SE211601", "Nguyen Hoang Minh Nhat", TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
      {"SE211596", "Pham Do Minh Dang", TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
      {"SE211591", "Le Kha Huy", TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
      {"SE201183", "Le Minh Dang", TEAM_ACADEMIC, MEMBER_ROLE_LEADER},
      {"SE211377", "Le Kha Hoang", TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
      {"SE201619", "Tran Trung Tin", TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
      {"SE204789", "Le Khac Minh Quang", TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
      {"SE210130", "Nguyen Minh Quan", TEAM_ACADEMIC, MEMBER_ROLE_LEADER},
      {"SE200932", "Vo Hieu Thang", TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},

      /* === NHOM 5-7: Ke hoach (TEAM_PLANNING) === */
      {"SE210773", "Nguyen Kim Tien Dat", TEAM_PLANNING, MEMBER_ROLE_MEMBER},
      {"SE190210", "Nguyen Vo Tien Dat", TEAM_PLANNING, MEMBER_ROLE_MEMBER},
      {"SE212026", "Bui Phuoc Trong", TEAM_PLANNING, MEMBER_ROLE_LEADER},
      {"SE210117", "Nguyen Hung Hien", TEAM_PLANNING, MEMBER_ROLE_MEMBER},
      {"SE211927", "Hoang Minh Trong", TEAM_PLANNING, MEMBER_ROLE_MEMBER},
      {"SE211615", "Nguyen Thanh Triet", TEAM_PLANNING, MEMBER_ROLE_MEMBER},
      {"SE210043", "Nguyen Van Phu", TEAM_PLANNING, MEMBER_ROLE_MEMBER},
      {"SE203055", "Nguyen Ngoc Phuc", TEAM_PLANNING, MEMBER_ROLE_BCN},
      {"SE210768", "Ngo Xuan Huyen", TEAM_PLANNING, MEMBER_ROLE_MEMBER},
      {"SE211766", "Vo Pham Hoang Nam", TEAM_PLANNING, MEMBER_ROLE_MEMBER},
      {"SE211156", "Do Thiet Thach", TEAM_PLANNING, MEMBER_ROLE_MEMBER},
      {"SE211093", "Vu Minh Tuan", TEAM_PLANNING, MEMBER_ROLE_MEMBER},
      {"SE201404", "Ta Anh Duc", TEAM_PLANNING, MEMBER_ROLE_MEMBER},
      {"SE204111", "Dam Le Tuan Anh", TEAM_PLANNING, MEMBER_ROLE_LEADER},
      {"SE211888", "Nguyen Quoc Viet", TEAM_PLANNING, MEMBER_ROLE_MEMBER},

      /* === NHOM 8-10: Nhan su (TEAM_HR) === */
      {"SE210556", "Vo Le Kien Huy", TEAM_HR, MEMBER_ROLE_MEMBER},
      {"SE200441", "Tran Le Anh Quan", TEAM_HR, MEMBER_ROLE_LEADER},
      {"SE210041", "Tran Cao Thanh", TEAM_HR, MEMBER_ROLE_MEMBER},
      {"SE203237", "Bui Pham Chi Nhan", TEAM_HR, MEMBER_ROLE_MEMBER},
      {"SE211953", "Vu Mai Anh Dat", TEAM_HR, MEMBER_ROLE_MEMBER},
      {"SE211968", "Nguyen Van Binh", TEAM_HR, MEMBER_ROLE_LEADER},
      {"SE203555", "Nguyen Van Gia Binh", TEAM_HR, MEMBER_ROLE_MEMBER},
      {"SE211059", "Nguyen Tan Loi", TEAM_HR, MEMBER_ROLE_MEMBER},
      {"SE211449", "Tran Khanh Tuong", TEAM_HR, MEMBER_ROLE_MEMBER},
      {"SE201682", "Nguyen Van Quoc Bao", TEAM_HR, MEMBER_ROLE_MEMBER},
      {"SE210741", "Nguyen Ngoc Minh Tu", TEAM_HR, MEMBER_ROLE_MEMBER},
      {"SE211898", "Nguyen Khoi Nguyen", TEAM_HR, MEMBER_ROLE_MEMBER},
      {"SE201843", "Le Thi Minh Tam", TEAM_HR, MEMBER_ROLE_LEADER},
      {"SE211960", "Tran Ngoc Doan Anh", TEAM_HR, MEMBER_ROLE_MEMBER},
      {"SE211201", "Phan Tuong Quan", TEAM_HR, MEMBER_ROLE_MEMBER},

      /* === NHOM 11-14: Truyen thong (TEAM_MEDIA) === */
      {"SE211528", "Nguyen Thai Huy", TEAM_MEDIA, MEMBER_ROLE_MEMBER},
      {"SE211470", "Tran Ngoc Xuan Phuc", TEAM_MEDIA, MEMBER_ROLE_MEMBER},
      {"SE203367", "Trinh Thi Minh Tam", TEAM_MEDIA, MEMBER_ROLE_MEMBER},
      {"SE210179", "Nguyen To Uyen", TEAM_MEDIA, MEMBER_ROLE_MEMBER},
      {"SE201566", "Ho Le Thien An", TEAM_MEDIA, MEMBER_ROLE_LEADER},
      {"SE211107", "Dau Duc Thanh", TEAM_MEDIA, MEMBER_ROLE_LEADER},
      {"SE211135", "Luu Hoang Anh Kiet", TEAM_MEDIA, MEMBER_ROLE_MEMBER},
      {"SE210756", "Nguyen Van Minh", TEAM_MEDIA, MEMBER_ROLE_MEMBER},
      {"SE204913", "Huynh Gia Bao", TEAM_MEDIA, MEMBER_ROLE_MEMBER},
      {"SE211611", "Nguyen Cao Tien Dat", TEAM_MEDIA, MEMBER_ROLE_MEMBER},
      {"SE211043", "Nguyen Quoc Anh", TEAM_MEDIA, MEMBER_ROLE_LEADER},
      {"SE210810", "Duong Thien Phu", TEAM_MEDIA, MEMBER_ROLE_MEMBER},
      {"SE211932", "Nguyen Van Duy Nhan", TEAM_MEDIA, MEMBER_ROLE_MEMBER},
      {"SE211914", "Vo Le Khoi Nguyen", TEAM_MEDIA, MEMBER_ROLE_MEMBER},
      {"SE210496", "Nguyen Duc Nhat Khang", TEAM_MEDIA, MEMBER_ROLE_MEMBER},
      {"SE200481", "Vang Khanh Khuyen", TEAM_MEDIA, MEMBER_ROLE_LEADER},
      {"SE210918", "Tran Thi Thuy Vy", TEAM_MEDIA, MEMBER_ROLE_MEMBER},
      {"SE210452", "Phan Kim Phuong", TEAM_MEDIA, MEMBER_ROLE_MEMBER},
      {"SE210777", "Nguyen Ngoc My Vy", TEAM_MEDIA, MEMBER_ROLE_MEMBER},
      {"SE212030", "Du Dong Ngoc Hao", TEAM_MEDIA, MEMBER_ROLE_MEMBER},

      /* BCN members */
      {"BCN001", "Tran Quoc Bao", TEAM_ACADEMIC, MEMBER_ROLE_BCN},
      {"BCN002", "Pham Thi Cuc", TEAM_PLANNING, MEMBER_ROLE_BCN},
  };

  for (int i = 0; i < (int)(sizeof(seedMembers) / sizeof(seedMembers[0]));
       i++) {
    makeMember(&members[mc], seedMembers[i].id, seedMembers[i].name,
               seedMembers[i].team, seedMembers[i].role, 0, 0, 0);
    mc++;
  }

  /* === Kick some chronic violators (isActive=0, isDeleted=0) === */
  /* SE210946 - Nguyen Vu Hao (academic, index 5) - Vang qua 3 buoi */
  members[5].isActive = STATUS_OUT_CLB;
  members[5].isDeleted = 0;
  members[5].consecutiveAbsences = 0;
  members[5].violationCount = 3;
  /* SE210117 - Nguyen Hung Hien (planning, index 23) - VP nhieu lan */
  members[23].isActive = STATUS_OUT_CLB;
  members[23].isDeleted = 0;
  members[23].consecutiveAbsences = 0;
  members[23].violationCount = 2;
  /* SE203367 - Trinh Thi Minh Tam (media, index 52) - Vang 4 buoi LT */
  members[52].isActive = STATUS_OUT_CLB;
  members[52].isDeleted = 0;
  members[52].consecutiveAbsences = 0;
  members[52].violationCount = 5;
  /* SE200516 - Tran Vu Hai Duy (academic, index 7) - Bao luc */
  members[7].isActive = STATUS_OUT_CLB;
  members[7].isDeleted = 0;
  members[7].consecutiveAbsences = 0;

  /* === Seed Accounts securely with Salted stretched hashes === */
  for (int i = 0; i < mc; i++) {
    strcpy(accounts[ac].studentId, members[i].studentId);
    generateSalt(accounts[ac].salt, sizeof(accounts[ac].salt));
    if (strcmp(members[i].studentId, "SE203055") == 0) {
      hashPassword("Phuc@2006", accounts[ac].salt, accounts[ac].password);
      accounts[ac].isDefaultPassword = 0;
    } else {
      hashPassword(members[i].studentId, accounts[ac].salt,
                   accounts[ac].password);
      accounts[ac].isDefaultPassword =
          (members[i].role == MEMBER_ROLE_BCN) ? 0 : 1;
    }
    accounts[ac].role = (members[i].role == MEMBER_ROLE_BCN)
                            ? ACCOUNT_ROLE_BCN
                            : ACCOUNT_ROLE_MEMBER;
    accounts[ac].isLocked = (members[i].isActive == STATUS_OUT_CLB) ? 1 : 0;
    accounts[ac].failCount = 0;
    ac++;
  }

  /* === Seed Violations === */
  makeViolation(&violations[vc++], "SE201018", dateSec(2026, 3, 10),
                REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210946", dateSec(2026, 2, 5),
                REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210946", dateSec(2026, 4, 20),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "Vang lan 3");
  makeViolation(&violations[vc++], "SE200516", dateSec(2026, 1, 15),
                REASON_NO_ACTIVITY, 50000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE200516", dateSec(2026, 4, 1),
                REASON_ABSENT, 50000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211591", dateSec(2026, 3, 22),
                REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE201619", dateSec(2026, 2, 28),
                REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE212026", dateSec(2026, 3, 5),
                REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE212026", dateSec(2026, 4, 10),
                REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210117", dateSec(2026, 1, 20),
                REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE201566", dateSec(2026, 2, 14),
                REASON_NO_ACTIVITY, 50000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE203367", dateSec(2026, 1, 8),
                REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE203367", dateSec(2026, 2, 12),
                REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE203367", dateSec(2026, 3, 18),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE203367", dateSec(2026, 4, 25),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "Vang 4 lan LT");
  makeViolation(&violations[vc++], "SE211043", dateSec(2026, 3, 15),
                REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "Lan dau");
  makeViolation(&violations[vc++], "SE200481", dateSec(2026, 4, 5),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");

  /* === Additional violations — Academic === */
  makeViolation(&violations[vc++], "SE200972", dateSec(2026, 4, 12),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "Vang khong phep");
  makeViolation(&violations[vc++], "SE200972", dateSec(2026, 5, 8),
                REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE212092", dateSec(2026, 2, 18),
                REASON_ABSENT, 20000, 1, PENALTY_FINE, "Co phep");
  makeViolation(&violations[vc++], "SE212092", dateSec(2026, 3, 22),
                REASON_NO_ACTIVITY, 50000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211866", dateSec(2026, 1, 25),
                REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE203677", dateSec(2026, 3, 15),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210474", dateSec(2026, 4, 8),
                REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210474", dateSec(2026, 5, 20),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "Vang lan 2");
  makeViolation(&violations[vc++], "SE210518", dateSec(2026, 2, 12),
                REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210364", dateSec(2026, 3, 28),
                REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210380", dateSec(2026, 4, 15),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211601", dateSec(2026, 1, 10),
                REASON_NO_ACTIVITY, 50000, 0, PENALTY_FINE,
                "Khong tham gia su kien");
  makeViolation(&violations[vc++], "SE211596", dateSec(2026, 5, 5),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE201183", dateSec(2026, 2, 20),
                REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211377", dateSec(2026, 3, 5),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE204789", dateSec(2026, 4, 22),
                REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210130", dateSec(2026, 5, 12),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "Vang khong phep");
  makeViolation(&violations[vc++], "SE200932", dateSec(2026, 1, 28),
                REASON_NO_ACTIVITY, 50000, 1, PENALTY_FINE, "");

  /* === Additional violations — Planning === */
  makeViolation(&violations[vc++], "SE210773", dateSec(2026, 3, 8),
                REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210773", dateSec(2026, 5, 18),
                REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE190210", dateSec(2026, 4, 2),
                REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211927", dateSec(2026, 1, 14),
                REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211615", dateSec(2026, 5, 25),
                REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210043", dateSec(2026, 2, 8),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210768", dateSec(2026, 3, 30),
                REASON_NO_ACTIVITY, 50000, 0, PENALTY_FINE,
                "Khong tham gia HD");
  makeViolation(&violations[vc++], "SE211766", dateSec(2026, 4, 18),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211156", dateSec(2026, 5, 2),
                REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211093", dateSec(2026, 1, 22),
                REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE201404", dateSec(2026, 2, 25),
                REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE204111", dateSec(2026, 3, 12),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211888", dateSec(2026, 4, 28),
                REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");

  /* === Additional violations — HR === */
  makeViolation(&violations[vc++], "SE210556", dateSec(2026, 2, 14),
                REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE200441", dateSec(2026, 3, 20),
                REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210041", dateSec(2026, 4, 10),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE203237", dateSec(2026, 5, 15),
                REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211953", dateSec(2026, 1, 18),
                REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211968", dateSec(2026, 3, 25),
                REASON_NO_ACTIVITY, 50000, 0, PENALTY_FINE,
                "Khong tham gia su kien");
  makeViolation(&violations[vc++], "SE203555", dateSec(2026, 4, 5),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211059", dateSec(2026, 5, 22),
                REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211449", dateSec(2026, 2, 28),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "Vang khong phep");
  makeViolation(&violations[vc++], "SE201682", dateSec(2026, 3, 18),
                REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210741", dateSec(2026, 4, 12),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");

  /* === Additional violations — Media === */
  makeViolation(&violations[vc++], "SE211528", dateSec(2026, 2, 22),
                REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211470", dateSec(2026, 4, 8),
                REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210179", dateSec(2026, 5, 18),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211107", dateSec(2026, 1, 20),
                REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211135", dateSec(2026, 3, 10),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210756", dateSec(2026, 4, 25),
                REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE204913", dateSec(2026, 5, 28),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211611", dateSec(2026, 2, 5),
                REASON_NO_ACTIVITY, 50000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210810", dateSec(2026, 3, 18),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211932", dateSec(2026, 4, 30),
                REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE211914", dateSec(2026, 5, 12),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210496", dateSec(2026, 1, 30),
                REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210918", dateSec(2026, 3, 25),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210452", dateSec(2026, 4, 15),
                REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE210777", dateSec(2026, 5, 22),
                REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
  makeViolation(&violations[vc++], "SE212030", dateSec(2026, 2, 10),
                REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "Lan dau");

  /* === Violations with penalty OUT_CLB (kick reasons) === */
  makeViolation(&violations[vc++], "SE210946", dateSec(2026, 5, 1),
                REASON_ABSENT, 0, 1, PENALTY_OUT_CLB,
                "Vang qua 3 buoi lien tiep");
  makeViolation(&violations[vc++], "SE210117", dateSec(2026, 4, 15),
                REASON_ABSENT, 0, 1, PENALTY_OUT_CLB, "Vi pham qua nhieu lan");
  makeViolation(&violations[vc++], "SE203367", dateSec(2026, 5, 10),
                REASON_ABSENT, 0, 1, PENALTY_OUT_CLB, "Vang 4 buoi lien tiep");
  makeViolation(&violations[vc++], "SE200516", dateSec(2026, 5, 15),
                REASON_VIOLENCE, 0, 1, PENALTY_OUT_CLB, "Danh nhau trong CLB");

  /* === Recalculate Member Stats based on seeded Violations === */
  for (int i = 0; i < mc; i++) {
    members[i].violationCount = 0;
    members[i].totalFine = 0.0;
    members[i].consecutiveAbsences = 0;
  }
  for (int i = 0; i < vc; i++) {
    Violation *v = &violations[i];
    for (int j = 0; j < mc; j++) {
      if (strcmp(members[j].studentId, v->studentId) == 0) {
        members[j].violationCount++;
        if (v->isPaid == 0) {
          members[j].totalFine += v->fine;
        }
        if (v->reason == REASON_ABSENT) {
          members[j].consecutiveAbsences++;
        } else {
          members[j].consecutiveAbsences = 0;
        }
        if (v->penalty == PENALTY_OUT_CLB) {
          members[j].isActive = STATUS_OUT_CLB;
          members[j].consecutiveAbsences = 0;
        }
        break;
      }
    }
  }

  /* === Write encrypted files to data/ === */
  char path[256];

#ifdef _WIN32
  const char *sep = "\\";
#else
  const char *sep = "/";
#endif

  ensureDir("data");
  snprintf(path, sizeof(path), "bin%sdata", sep);
  ensureDir(path);

  snprintf(path, sizeof(path), "data%smembers.dat", sep);
  if (writeFileEncrypted(path, members, sizeof(Member), mc) != 0)
    return 1;
  printf("[OK] %d members -> %s (ENCRYPTED)\n", mc, path);

  snprintf(path, sizeof(path), "data%sviolations.dat", sep);
  if (writeFileEncrypted(path, violations, sizeof(Violation), vc) != 0)
    return 1;
  printf("[OK] %d violations -> %s (ENCRYPTED)\n", vc, path);

  snprintf(path, sizeof(path), "data%saccounts.dat", sep);
  if (writeFileEncrypted(path, accounts, sizeof(Account), ac) != 0)
    return 1;
  printf("[OK] %d accounts -> %s (ENCRYPTED)\n", ac, path);

  /* Also copy to bin/data/ so the app can read it */
  for (int i = 0; i < 3; i++) {
    char src[256], dst[256];
    snprintf(src, sizeof(src), "data%s%s.dat", sep, files[i]);
    snprintf(dst, sizeof(dst), "bin%sdata%s%s.dat", sep, sep, files[i]);
    FILE *fin = fopen(src, "rb");
    if (fin) {
      FILE *fout = fopen(dst, "wb");
      if (fout) {
        int ch;
        while ((ch = fgetc(fin)) != EOF)
          fputc(ch, fout);
        fclose(fout);
      }
      fclose(fin);
    }
  }
  printf("[OK] Encrypted seed files copied to bin/data/\n");

  printf("\n[OK] Du lieu Seed da duoc tao thanh cong.\n");
  return 0;
}
