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
#else
#include <sys/stat.h>
#endif

/* Password security keys & prime numbers */
#define FNV_PRIME 0x00000100000001B3ULL
#define FNV_OFFSET_BASIS 0xCBF29CE484222325ULL

static void hashPassword(const char *password, const char *salt, char *outHashHex) {
    char temp[256];
    snprintf(temp, sizeof(temp), "%s%s", password, salt);

    unsigned long long hash = FNV_OFFSET_BASIS;
    for (int iter = 0; iter < 1000; iter++) {
        for (int i = 0; temp[i] != '\0'; i++) {
            hash ^= (unsigned char)temp[i];
            hash *= FNV_PRIME;
        }
        snprintf(temp, sizeof(temp), "%016llx%s", hash, salt);
    }

    unsigned long long hash2 = FNV_OFFSET_BASIS;
    for (int i = 0; temp[i] != '\0'; i++) {
        hash2 ^= (unsigned char)temp[i];
        hash2 *= FNV_PRIME;
    }

    snprintf(outHashHex, 32, "%015llx%016llx", hash & 0xFFFFFFFFFFFFFFFULL, hash2);
}

static void generateSalt(char *salt, size_t size) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    if (size == 0) {
        return;
    }
    for (size_t n = 0; n < size - 1; n++) {
        int key = rand() % (int)(sizeof(charset) - 1);
        salt[n] = charset[key];
    }
    salt[size - 1] = '\0';
}

/* Encrypted File writing */
static int writeFileEncrypted(const char *path, const void *data, size_t itemSize, int count) {
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        printf("[LOI] Khong the ghi %s\n", path);
        return -1;
    }
    /* Write magic header "FCE1" */
    fwrite("FCE1", 1, 4, fp);
    /* Write count */
    fwrite(&count, sizeof(int), 1, fp);
    /* Encrypt data copy in heap to prevent altering RAM array, then write */
    if (count > 0) {
        size_t totalSize = itemSize * (size_t)count;
        unsigned char *encrypted = (unsigned char *)malloc(totalSize);
        if (!encrypted) {
            printf("[LOI] Het bo nho heap khi ghi file!\n");
            fclose(fp);
            return -1;
        }
        memcpy(encrypted, data, totalSize);
        
        static const unsigned char XOR_KEY[] = "FCodeTrainC2026_SecureKey!";
        size_t keyLen = strlen((const char *)XOR_KEY);
        for (size_t i = 0; i < totalSize; i++) {
            encrypted[i] ^= XOR_KEY[i % keyLen];
        }
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

static void makeMember(Member *m, const char *id, const char *name,
                       int team, int role, double totalFine,
                       int consecAbs, int violCount) {
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
    t.tm_mon  = m - 1;
    t.tm_mday = d;
    t.tm_hour = 8;
    return mktime(&t);
}

static void makeViolation(Violation *v, const char *id, time_t t,
                           int reason, double fine, int isPaid, int penalty,
                           const char *note) {
    memset(v, 0, sizeof(Violation));
    strncpy(v->studentId, id, MAX_MSSV_LEN - 1);
    v->violationTime = t;
    v->reason  = reason;
    v->fine    = fine;
    v->isPaid  = isPaid;
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
        if (writeEmptyFile(path) != 0) return 1;
        printf("[OK] Cleared %s (Wrote empty encrypted file)\n", path);
        snprintf(path, sizeof(path), "bin%sdata%s%s.dat", sep, sep, files[i]);
        writeEmptyFile(path);
    }

    printf("\nData cleared. Next run of system will initialize a default administrator account.\n");
    return 0;
}

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));

    if (argc > 1 && strcmp(argv[1], "clear") == 0) {
        return clearData();
    }

    static const char *files[] = {"members", "violations", "accounts"};

    Member   members[20];
    Account  accounts[20];
    Violation violations[25];
    int mc = 0, ac = 0, vc = 0;

    /* === Seed beautiful members profiles === */
    /* Hoc thuat (0) */
    makeMember(&members[mc], "SV0001", "Nguyen Van An",     TEAM_ACADEMIC, MEMBER_ROLE_LEADER, 0,     0, 1); mc++;
    makeMember(&members[mc], "SV0002", "Tran Thi Bich",     TEAM_ACADEMIC, MEMBER_ROLE_MEMBER, 20000, 2, 3); mc++;
    makeMember(&members[mc], "SV0003", "Le Hoang Cuong",    TEAM_ACADEMIC, MEMBER_ROLE_MEMBER, 0,     0, 0); mc++;

    /* Ke hoach (1) */
    makeMember(&members[mc], "SV0004", "Pham Minh Duc",     TEAM_PLANNING, MEMBER_ROLE_LEADER, 50000, 0, 2); mc++;
    makeMember(&members[mc], "SV0005", "Vo Thi Mai",        TEAM_PLANNING, MEMBER_ROLE_MEMBER, 20000, 1, 1); mc++;
    makeMember(&members[mc], "SV0006", "Bui Quoc Phong",    TEAM_PLANNING, MEMBER_ROLE_MEMBER, 0,     0, 1); mc++;

    /* Nhan su (2) */
    makeMember(&members[mc], "SV0007", "Do Thanh Giang",    TEAM_HR, MEMBER_ROLE_LEADER, 0,     0, 0); mc++;
    makeMember(&members[mc], "SV0008", "Ngo Thi Hanh",      TEAM_HR, MEMBER_ROLE_MEMBER, 20000, 3, 2); mc++;
    makeMember(&members[mc], "SV0009", "Ly Minh Kien",      TEAM_HR, MEMBER_ROLE_MEMBER, 0,     0, 1); mc++;

    /* Truyen thong (3) */
    makeMember(&members[mc], "SV0010", "Ha Thanh Long",     TEAM_MEDIA, MEMBER_ROLE_LEADER, 0,     0, 1); mc++;
    makeMember(&members[mc], "SV0011", "Dang Thi Ngoc",     TEAM_MEDIA, MEMBER_ROLE_MEMBER, 40000, 4, 4); mc++;
    makeMember(&members[mc], "SV0012", "Cao Van Phu",       TEAM_MEDIA, MEMBER_ROLE_MEMBER, 0,     0, 0); mc++;

    /* BCN members (including custom new defaults) */
    makeMember(&members[mc], "admin", "Administrator", TEAM_ACADEMIC, MEMBER_ROLE_BCN, 0, 0, 0); mc++;
    makeMember(&members[mc], "SE203055", "Le Phuc BCN", TEAM_PLANNING, MEMBER_ROLE_BCN, 0, 0, 0); mc++;
    makeMember(&members[mc], "BCN001", "Tran Quoc Bao",     TEAM_ACADEMIC, MEMBER_ROLE_BCN, 0, 0, 0); mc++;
    makeMember(&members[mc], "BCN002", "Pham Thi Cuc",      TEAM_PLANNING, MEMBER_ROLE_BCN, 0, 0, 0); mc++;

    /* === Seed Accounts securely with Salted stretched hashes === */
    /* 1. SE203055: Phuc@2006 */
    strcpy(accounts[ac].studentId, "SE203055");
    generateSalt(accounts[ac].salt, sizeof(accounts[ac].salt));
    hashPassword("Phuc@2006", accounts[ac].salt, accounts[ac].password);
    accounts[ac].role = ACCOUNT_ROLE_BCN;
    accounts[ac].isLocked = 0;
    accounts[ac].failCount = 0;
    accounts[ac].isDefaultPassword = 0;
    ac++;

    /* 2. admin: admin */
    strcpy(accounts[ac].studentId, "admin");
    generateSalt(accounts[ac].salt, sizeof(accounts[ac].salt));
    hashPassword("admin", accounts[ac].salt, accounts[ac].password);
    accounts[ac].role = ACCOUNT_ROLE_BCN;
    accounts[ac].isLocked = 0;
    accounts[ac].failCount = 0;
    accounts[ac].isDefaultPassword = 0;
    ac++;

    /* 3. accounts for all remaining members (password 123456) */
    for (int i = 0; i < mc; i++) {
        if (strcmp(members[i].studentId, "admin") == 0 ||
            strcmp(members[i].studentId, "SE203055") == 0) {
            continue;
        }
        strcpy(accounts[ac].studentId, members[i].studentId);
        generateSalt(accounts[ac].salt, sizeof(accounts[ac].salt));
        hashPassword("123456", accounts[ac].salt, accounts[ac].password);
        accounts[ac].role = (members[i].role == MEMBER_ROLE_BCN) ? ACCOUNT_ROLE_BCN : ACCOUNT_ROLE_MEMBER;
        accounts[ac].isLocked = 0;
        accounts[ac].failCount = 0;
        accounts[ac].isDefaultPassword = (members[i].role == MEMBER_ROLE_BCN) ? 0 : 1;
        ac++;
    }

    /* === Seed Violations === */
    /* SV0001 - 1 violation, paid */
    makeViolation(&violations[vc++], "SV0001", dateSec(2026,3,10),
                  REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "Lan dau");

    /* SV0002 - 3 violations, 2 paid 1 unpaid */
    makeViolation(&violations[vc++], "SV0002", dateSec(2026,2,5),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SV0002", dateSec(2026,3,12),
                  REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SV0002", dateSec(2026,4,20),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "Vang lan 3");

    /* SV0004 - 2 violations, 1 paid 1 unpaid */
    makeViolation(&violations[vc++], "SV0004", dateSec(2026,1,15),
                  REASON_NO_ACTIVITY, 50000, 1, PENALTY_FINE, "Leader");
    makeViolation(&violations[vc++], "SV0004", dateSec(2026,4,1),
                  REASON_ABSENT, 50000, 0, PENALTY_FINE, "");

    /* SV0005 - 1 unpaid */
    makeViolation(&violations[vc++], "SV0005", dateSec(2026,3,22),
                  REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");

    /* SV0006 - 1 paid */
    makeViolation(&violations[vc++], "SV0006", dateSec(2026,2,28),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");

    /* SV0008 - 2 violations */
    makeViolation(&violations[vc++], "SV0008", dateSec(2026,3,5),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SV0008", dateSec(2026,4,10),
                  REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");

    /* SV0009 - 1 paid */
    makeViolation(&violations[vc++], "SV0009", dateSec(2026,1,20),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");

    /* SV0010 - 1 paid */
    makeViolation(&violations[vc++], "SV0010", dateSec(2026,2,14),
                  REASON_NO_ACTIVITY, 50000, 1, PENALTY_FINE, "Leader");

    /* SV0011 - 4 violations, approaching Out CLB */
    makeViolation(&violations[vc++], "SV0011", dateSec(2026,1,8),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SV0011", dateSec(2026,2,12),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SV0011", dateSec(2026,3,18),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SV0011", dateSec(2026,4,25),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "Vang 4 lan LT");

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
    if (writeFileEncrypted(path, members, sizeof(Member), mc) != 0) return 1;
    printf("[OK] %d members -> %s (ENCRYPTED)\n", mc, path);

    snprintf(path, sizeof(path), "data%sviolations.dat", sep);
    if (writeFileEncrypted(path, violations, sizeof(Violation), vc) != 0) return 1;
    printf("[OK] %d violations -> %s (ENCRYPTED)\n", vc, path);

    snprintf(path, sizeof(path), "data%saccounts.dat", sep);
    if (writeFileEncrypted(path, accounts, sizeof(Account), ac) != 0) return 1;
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
                while ((ch = fgetc(fin)) != EOF) fputc(ch, fout);
                fclose(fout);
            }
            fclose(fin);
        }
    }
    printf("[OK] Encrypted seed files copied to bin/data/\n");

    printf("\nSecure demo data seeded successfully!\n");
    printf("- Admin default account:   SE203055 | Phuc@2006 (BCN role)\n");
    printf("- Legacy fallback account:  admin    | admin     (BCN role)\n");
    printf("- Regular members passwords:  123456            (Change on first login forced)\n");
    return 0;
}
