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

    Member   members[100];
    Account  accounts[100];
    Violation violations[100];
    int mc = 0, ac = 0, vc = 0;

    /* === Seed real Challenge 3 members === */
    /* Team mapping: Nhom 1-4 -> Hoc thuat, 5-7 -> Ke hoach, 8-10 -> Nhan su, 11-14 -> Truyen thong */
    static const struct {
        const char *id, *name;
        int team, role;
    } seedMembers[] = {
        /* === NHOM 1-4: Hoc thuat (TEAM_ACADEMIC) === */
        {"SE201018", "Lam Hoang An",          TEAM_ACADEMIC, MEMBER_ROLE_LEADER},
        {"SE200972", "Mai Xuan Hieu",         TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
        {"SE212092", "Nguyen Tien Khai",      TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
        {"SE211866", "Pham Gia Bao",          TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
        {"SE203677", "Ngo Ngoc Gia Han",      TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
        {"SE210946", "Nguyen Vu Hao",         TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
        {"SE210474", "Do Thanh Binh",         TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
        {"SE200516", "Tran Vu Hai Duy",       TEAM_ACADEMIC, MEMBER_ROLE_LEADER},
        {"SE210518", "Le Gia Phuc Chanh",     TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
        {"SE210364", "Chu Minh Cuong",        TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
        {"SE210380", "Nguyen Phi Lam",        TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
        {"SE211601", "Nguyen Hoang Minh Nhat", TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
        {"SE211596", "Pham Do Minh Dang",     TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
        {"SE211591", "Le Kha Huy",            TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
        {"SE201183", "Le Minh Dang",          TEAM_ACADEMIC, MEMBER_ROLE_LEADER},
        {"SE211377", "Le Kha Hoang",          TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
        {"SE201619", "Tran Trung Tin",        TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
        {"SE204789", "Le Khac Minh Quang",    TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},
        {"SE210130", "Nguyen Minh Quan",      TEAM_ACADEMIC, MEMBER_ROLE_LEADER},
        {"SE200932", "Vo Hieu Thang",         TEAM_ACADEMIC, MEMBER_ROLE_MEMBER},

        /* === NHOM 5-7: Ke hoach (TEAM_PLANNING) === */
        {"SE210773", "Nguyen Kim Tien Dat",   TEAM_PLANNING, MEMBER_ROLE_MEMBER},
        {"SE190210", "Nguyen Vo Tien Dat",    TEAM_PLANNING, MEMBER_ROLE_MEMBER},
        {"SE212026", "Bui Phuoc Trong",       TEAM_PLANNING, MEMBER_ROLE_LEADER},
        {"SE210117", "Nguyen Hung Hien",      TEAM_PLANNING, MEMBER_ROLE_MEMBER},
        {"SE211927", "Hoang Minh Trong",      TEAM_PLANNING, MEMBER_ROLE_MEMBER},
        {"SE211615", "Nguyen Thanh Triet",    TEAM_PLANNING, MEMBER_ROLE_MEMBER},
        {"SE210043", "Nguyen Van Phu",        TEAM_PLANNING, MEMBER_ROLE_MEMBER},
        {"SE203055", "Nguyen Ngoc Phuc",      TEAM_PLANNING, MEMBER_ROLE_BCN},
        {"SE210768", "Ngo Xuan Huyen",        TEAM_PLANNING, MEMBER_ROLE_MEMBER},
        {"SE211766", "Vo Pham Hoang Nam",     TEAM_PLANNING, MEMBER_ROLE_MEMBER},
        {"SE211156", "Do Thiet Thach",        TEAM_PLANNING, MEMBER_ROLE_MEMBER},
        {"SE211093", "Vu Minh Tuan",          TEAM_PLANNING, MEMBER_ROLE_MEMBER},
        {"SE201404", "Ta Anh Duc",            TEAM_PLANNING, MEMBER_ROLE_MEMBER},
        {"SE204111", "Dam Le Tuan Anh",       TEAM_PLANNING, MEMBER_ROLE_LEADER},
        {"SE211888", "Nguyen Quoc Viet",      TEAM_PLANNING, MEMBER_ROLE_MEMBER},

        /* === NHOM 8-10: Nhan su (TEAM_HR) === */
        {"SE210556", "Vo Le Kien Huy",        TEAM_HR, MEMBER_ROLE_MEMBER},
        {"SE200441", "Tran Le Anh Quan",      TEAM_HR, MEMBER_ROLE_LEADER},
        {"SE210041", "Tran Cao Thanh",        TEAM_HR, MEMBER_ROLE_MEMBER},
        {"SE203237", "Bui Pham Chi Nhan",     TEAM_HR, MEMBER_ROLE_MEMBER},
        {"SE211953", "Vu Mai Anh Dat",        TEAM_HR, MEMBER_ROLE_MEMBER},
        {"SE211968", "Nguyen Van Binh",       TEAM_HR, MEMBER_ROLE_LEADER},
        {"SE203555", "Nguyen Van Gia Binh",   TEAM_HR, MEMBER_ROLE_MEMBER},
        {"SE211059", "Nguyen Tan Loi",        TEAM_HR, MEMBER_ROLE_MEMBER},
        {"SE211449", "Tran Khanh Tuong",      TEAM_HR, MEMBER_ROLE_MEMBER},
        {"SE201682", "Nguyen Van Quoc Bao",   TEAM_HR, MEMBER_ROLE_MEMBER},
        {"SE210741", "Nguyen Ngoc Minh Tu",   TEAM_HR, MEMBER_ROLE_MEMBER},
        {"SE211898", "Nguyen Khoi Nguyen",    TEAM_HR, MEMBER_ROLE_MEMBER},
        {"SE201843", "Le Thi Minh Tam",       TEAM_HR, MEMBER_ROLE_LEADER},
        {"SE211960", "Tran Ngoc Doan Anh",    TEAM_HR, MEMBER_ROLE_MEMBER},
        {"SE211201", "Phan Tuong Quan",       TEAM_HR, MEMBER_ROLE_MEMBER},

        /* === NHOM 11-14: Truyen thong (TEAM_MEDIA) === */
        {"SE211528", "Nguyen Thai Huy",       TEAM_MEDIA, MEMBER_ROLE_MEMBER},
        {"SE211470", "Tran Ngoc Xuan Phuc",   TEAM_MEDIA, MEMBER_ROLE_MEMBER},
        {"SE203367", "Trinh Thi Minh Tam",    TEAM_MEDIA, MEMBER_ROLE_MEMBER},
        {"SE210179", "Nguyen To Uyen",        TEAM_MEDIA, MEMBER_ROLE_MEMBER},
        {"SE201566", "Ho Le Thien An",        TEAM_MEDIA, MEMBER_ROLE_LEADER},
        {"SE211107", "Dau Duc Thanh",         TEAM_MEDIA, MEMBER_ROLE_LEADER},
        {"SE211135", "Luu Hoang Anh Kiet",    TEAM_MEDIA, MEMBER_ROLE_MEMBER},
        {"SE210756", "Nguyen Van Minh",       TEAM_MEDIA, MEMBER_ROLE_MEMBER},
        {"SE204913", "Huynh Gia Bao",         TEAM_MEDIA, MEMBER_ROLE_MEMBER},
        {"SE211611", "Nguyen Cao Tien Dat",   TEAM_MEDIA, MEMBER_ROLE_MEMBER},
        {"SE211043", "Nguyen Quoc Anh",       TEAM_MEDIA, MEMBER_ROLE_LEADER},
        {"SE210810", "Duong Thien Phu",       TEAM_MEDIA, MEMBER_ROLE_MEMBER},
        {"SE211932", "Nguyen Van Duy Nhan",   TEAM_MEDIA, MEMBER_ROLE_MEMBER},
        {"SE211914", "Vo Le Khoi Nguyen",     TEAM_MEDIA, MEMBER_ROLE_MEMBER},
        {"SE210496", "Nguyen Duc Nhat Khang", TEAM_MEDIA, MEMBER_ROLE_MEMBER},
        {"SE200481", "Vang Khanh Khuyen",     TEAM_MEDIA, MEMBER_ROLE_LEADER},
        {"SE210918", "Tran Thi Thuy Vy",      TEAM_MEDIA, MEMBER_ROLE_MEMBER},
        {"SE210452", "Phan Kim Phuong",       TEAM_MEDIA, MEMBER_ROLE_MEMBER},
        {"SE210777", "Nguyen Ngoc My Vy",     TEAM_MEDIA, MEMBER_ROLE_MEMBER},
        {"SE212030", "Du Dong Ngoc Hao",      TEAM_MEDIA, MEMBER_ROLE_MEMBER},

        /* BCN members */
        {"BCN001",   "Tran Quoc Bao",         TEAM_ACADEMIC, MEMBER_ROLE_BCN},
        {"BCN002",   "Pham Thi Cuc",          TEAM_PLANNING, MEMBER_ROLE_BCN},
    };

    for (int i = 0; i < (int)(sizeof(seedMembers)/sizeof(seedMembers[0])); i++) {
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

    /* Lock accounts for kicked members */
    accounts[5].isLocked = 1;   /* SE210946 */
    accounts[23].isLocked = 1;  /* SE210117 */
    accounts[52].isLocked = 1;  /* SE203367 */
    accounts[7].isLocked = 1;   /* SE200516 */

    /* === Seed Accounts securely with Salted stretched hashes === */
    for (int i = 0; i < mc; i++) {
        strcpy(accounts[ac].studentId, members[i].studentId);
        generateSalt(accounts[ac].salt, sizeof(accounts[ac].salt));
        if (strcmp(members[i].studentId, "SE203055") == 0) {
            hashPassword("Phuc@2006", accounts[ac].salt, accounts[ac].password);
            accounts[ac].isDefaultPassword = 0;
        } else {
            hashPassword(members[i].studentId, accounts[ac].salt, accounts[ac].password);
            accounts[ac].isDefaultPassword = (members[i].role == MEMBER_ROLE_BCN) ? 0 : 1;
        }
        accounts[ac].role = (members[i].role == MEMBER_ROLE_BCN) ? ACCOUNT_ROLE_BCN : ACCOUNT_ROLE_MEMBER;
        accounts[ac].isLocked = 0;
        accounts[ac].failCount = 0;
        ac++;
    }

    /* === Seed Violations === */
    makeViolation(&violations[vc++], "SE201018", dateSec(2026,3,10),
                  REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210946", dateSec(2026,2,5),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210946", dateSec(2026,4,20),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "Vang lan 3");
    makeViolation(&violations[vc++], "SE200516", dateSec(2026,1,15),
                  REASON_NO_ACTIVITY, 50000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE200516", dateSec(2026,4,1),
                  REASON_ABSENT, 50000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211591", dateSec(2026,3,22),
                  REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE201619", dateSec(2026,2,28),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE212026", dateSec(2026,3,5),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE212026", dateSec(2026,4,10),
                  REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210117", dateSec(2026,1,20),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE201566", dateSec(2026,2,14),
                  REASON_NO_ACTIVITY, 50000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE203367", dateSec(2026,1,8),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE203367", dateSec(2026,2,12),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE203367", dateSec(2026,3,18),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE203367", dateSec(2026,4,25),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "Vang 4 lan LT");
    makeViolation(&violations[vc++], "SE211043", dateSec(2026,3,15),
                  REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "Lan dau");
    makeViolation(&violations[vc++], "SE200481", dateSec(2026,4,5),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");

    /* === Additional violations — Academic === */
    makeViolation(&violations[vc++], "SE200972", dateSec(2026,4,12),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "Vang khong phep");
    makeViolation(&violations[vc++], "SE200972", dateSec(2026,5,8),
                  REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE212092", dateSec(2026,2,18),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "Co phep");
    makeViolation(&violations[vc++], "SE212092", dateSec(2026,3,22),
                  REASON_NO_ACTIVITY, 50000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211866", dateSec(2026,1,25),
                  REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE203677", dateSec(2026,3,15),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210474", dateSec(2026,4,8),
                  REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210474", dateSec(2026,5,20),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "Vang lan 2");
    makeViolation(&violations[vc++], "SE210518", dateSec(2026,2,12),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210364", dateSec(2026,3,28),
                  REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210380", dateSec(2026,4,15),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211601", dateSec(2026,1,10),
                  REASON_NO_ACTIVITY, 50000, 0, PENALTY_FINE, "Khong tham gia su kien");
    makeViolation(&violations[vc++], "SE211596", dateSec(2026,5,5),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE201183", dateSec(2026,2,20),
                  REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211377", dateSec(2026,3,5),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE204789", dateSec(2026,4,22),
                  REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210130", dateSec(2026,5,12),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "Vang khong phep");
    makeViolation(&violations[vc++], "SE200932", dateSec(2026,1,28),
                  REASON_NO_ACTIVITY, 50000, 1, PENALTY_FINE, "");

    /* === Additional violations — Planning === */
    makeViolation(&violations[vc++], "SE210773", dateSec(2026,3,8),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210773", dateSec(2026,5,18),
                  REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE190210", dateSec(2026,4,2),
                  REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211927", dateSec(2026,1,14),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211615", dateSec(2026,5,25),
                  REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210043", dateSec(2026,2,8),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210768", dateSec(2026,3,30),
                  REASON_NO_ACTIVITY, 50000, 0, PENALTY_FINE, "Khong tham gia HD");
    makeViolation(&violations[vc++], "SE211766", dateSec(2026,4,18),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211156", dateSec(2026,5,2),
                  REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211093", dateSec(2026,1,22),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE201404", dateSec(2026,2,25),
                  REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE204111", dateSec(2026,3,12),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211888", dateSec(2026,4,28),
                  REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");

    /* === Additional violations — HR === */
    makeViolation(&violations[vc++], "SE210556", dateSec(2026,2,14),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE200441", dateSec(2026,3,20),
                  REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210041", dateSec(2026,4,10),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE203237", dateSec(2026,5,15),
                  REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211953", dateSec(2026,1,18),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211968", dateSec(2026,3,25),
                  REASON_NO_ACTIVITY, 50000, 0, PENALTY_FINE, "Khong tham gia su kien");
    makeViolation(&violations[vc++], "SE203555", dateSec(2026,4,5),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211059", dateSec(2026,5,22),
                  REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211449", dateSec(2026,2,28),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "Vang khong phep");
    makeViolation(&violations[vc++], "SE201682", dateSec(2026,3,18),
                  REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210741", dateSec(2026,4,12),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");

    /* === Additional violations — Media === */
    makeViolation(&violations[vc++], "SE211528", dateSec(2026,2,22),
                  REASON_ABSENT, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211470", dateSec(2026,4,8),
                  REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210179", dateSec(2026,5,18),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211107", dateSec(2026,1,20),
                  REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211135", dateSec(2026,3,10),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210756", dateSec(2026,4,25),
                  REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE204913", dateSec(2026,5,28),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211611", dateSec(2026,2,5),
                  REASON_NO_ACTIVITY, 50000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210810", dateSec(2026,3,18),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211932", dateSec(2026,4,30),
                  REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE211914", dateSec(2026,5,12),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210496", dateSec(2026,1,30),
                  REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210918", dateSec(2026,3,25),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210452", dateSec(2026,4,15),
                  REASON_NO_JACKET, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE210777", dateSec(2026,5,22),
                  REASON_ABSENT, 20000, 0, PENALTY_FINE, "");
    makeViolation(&violations[vc++], "SE212030", dateSec(2026,2,10),
                  REASON_NO_JACKET, 20000, 1, PENALTY_FINE, "Lan dau");

    /* === Violations with penalty OUT_CLB (kick reasons) === */
    makeViolation(&violations[vc++], "SE210946", dateSec(2026,5,1),
                  REASON_ABSENT, 0, 1, PENALTY_OUT_CLB, "Vang qua 3 buoi lien tiep");
    makeViolation(&violations[vc++], "SE210117", dateSec(2026,4,15),
                  REASON_ABSENT, 0, 1, PENALTY_OUT_CLB, "Vi pham qua nhieu lan");
    makeViolation(&violations[vc++], "SE203367", dateSec(2026,5,10),
                  REASON_ABSENT, 0, 1, PENALTY_OUT_CLB, "Vang 4 buoi lien tiep");
    makeViolation(&violations[vc++], "SE200516", dateSec(2026,5,15),
                  REASON_VIOLENCE, 0, 1, PENALTY_OUT_CLB, "Danh nhau trong CLB");

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

    printf("\n[OK] Du lieu Seed da duoc tao thanh cong.\n");
    return 0;
}
