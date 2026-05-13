/**
 * Standalone seed-data generator.
 * Writes members.dat, violations.dat, accounts.dat with realistic demo data.
 * Compile: gcc -std=c17 -m64 -Iinclude src/seed_data.c -o bin/seed_data.exe
 * Run:     bin/seed_data.exe
 */
#include "types.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

/* ---- clear: write empty .dat files ---- */

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
    m->totalFine = totalFine;
    m->consecutiveAbsences = consecAbs;
    m->violationCount = violCount;
}

static void makeAccount(Account *a, const char *id, const char *pass, int role) {
    memset(a, 0, sizeof(Account));
    strncpy(a->studentId, id, MAX_MSSV_LEN - 1);
    strncpy(a->password, pass, MAX_PASS_LEN - 1);
    a->role = role;
    a->isLocked = 0;
    a->failCount = 0;
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

static int writeFile(const char *path, const void *data, size_t itemSize, int count) {
    FILE *fp = fopen(path, "wb");
    if (!fp) { printf("[LOI] Khong the ghi %s\n", path); return -1; }
    fwrite(&count, sizeof(int), 1, fp);
    if (count > 0) fwrite(data, itemSize, (size_t)count, fp);
    fclose(fp);
    return 0;
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
        if (writeFile(path, NULL, 0, 0) != 0) return 1;
        printf("[OK] Cleared %s\n", path);
        snprintf(path, sizeof(path), "bin%sdata%s%s.dat", sep, sep, files[i]);
        writeFile(path, NULL, 0, 0);
    }

    printf("\nData cleared. Next run will create default ADMIN account.\n");
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "clear") == 0) {
        return clearData();
    }

    static const char *files[] = {"members", "violations", "accounts"};

    Member   members[15];
    Account  accounts[15];
    Violation violations[25];
    int mc = 0, ac = 0, vc = 0;

    /* === ADMIN account === */
    makeAccount(&accounts[ac++], "ADMIN", "ADMIN", ACCOUNT_ROLE_BCN);

    /* === Members (4 teams, ~3 per team) === */
    /* Hoc thuat (0) */
    makeMember(&members[mc], "SV0001", "Nguyen Van An",     TEAM_ACADEMIC, MEMBER_ROLE_LEADER, 20000, 0, 1); mc++;
    makeMember(&members[mc], "SV0002", "Tran Thi Bich",     TEAM_ACADEMIC, MEMBER_ROLE_MEMBER, 60000, 2, 3); mc++;
    makeMember(&members[mc], "SV0003", "Le Hoang Cuong",    TEAM_ACADEMIC, MEMBER_ROLE_MEMBER, 0,     0, 0); mc++;

    /* Ke hoach (1) */
    makeMember(&members[mc], "SV0004", "Pham Minh Duc",     TEAM_PLANNING, MEMBER_ROLE_LEADER, 40000, 0, 2); mc++;
    makeMember(&members[mc], "SV0005", "Vo Thi Mai",        TEAM_PLANNING, MEMBER_ROLE_MEMBER, 20000, 1, 1); mc++;
    makeMember(&members[mc], "SV0006", "Bui Quoc Phong",    TEAM_PLANNING, MEMBER_ROLE_MEMBER, 20000, 0, 1); mc++;

    /* Nhan su (2) */
    makeMember(&members[mc], "SV0007", "Do Thanh Giang",    TEAM_HR, MEMBER_ROLE_LEADER, 0,     0, 0); mc++;
    makeMember(&members[mc], "SV0008", "Ngo Thi Hanh",      TEAM_HR, MEMBER_ROLE_MEMBER, 40000, 3, 2); mc++;
    makeMember(&members[mc], "SV0009", "Ly Minh Kien",      TEAM_HR, MEMBER_ROLE_MEMBER, 20000, 0, 1); mc++;

    /* Truyen thong (3) */
    makeMember(&members[mc], "SV0010", "Ha Thanh Long",     TEAM_MEDIA, MEMBER_ROLE_LEADER, 20000, 0, 1); mc++;
    makeMember(&members[mc], "SV0011", "Dang Thi Ngoc",     TEAM_MEDIA, MEMBER_ROLE_MEMBER, 80000, 4, 4); mc++;
    makeMember(&members[mc], "SV0012", "Cao Van Phu",       TEAM_MEDIA, MEMBER_ROLE_MEMBER, 0,     0, 0); mc++;

    /* BCN members */
    makeMember(&members[mc], "BCN001", "Tran Quoc Bao",     TEAM_ACADEMIC, MEMBER_ROLE_BCN, 0, 0, 0); mc++;
    makeMember(&members[mc], "BCN002", "Pham Thi Cuc",      TEAM_PLANNING, MEMBER_ROLE_BCN, 0, 0, 0); mc++;

    /* === Accounts for all members === */
    for (int i = 0; i < mc; i++) {
        makeAccount(&accounts[ac], members[i].studentId, "123456",
                    (members[i].role == MEMBER_ROLE_BCN) ? ACCOUNT_ROLE_BCN
                                                         : ACCOUNT_ROLE_MEMBER);
        ac++;
    }

    /* === Violations === */
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

    /* === Write files to data/ === */
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
    if (writeFile(path, members, sizeof(Member), mc) != 0) return 1;
    printf("[OK] %d members -> %s\n", mc, path);

    snprintf(path, sizeof(path), "data%sviolations.dat", sep);
    if (writeFile(path, violations, sizeof(Violation), vc) != 0) return 1;
    printf("[OK] %d violations -> %s\n", vc, path);

    snprintf(path, sizeof(path), "data%saccounts.dat", sep);
    if (writeFile(path, accounts, sizeof(Account), ac) != 0) return 1;
    printf("[OK] %d accounts -> %s\n", ac, path);

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
    printf("[OK] Copied to bin/data/\n");

    printf("\nDemo data ready. All passwords: 123456 | Admin: ADMIN/ADMIN\n");
    return 0;
}
