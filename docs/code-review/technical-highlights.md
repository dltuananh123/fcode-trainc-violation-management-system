# Tong hop Ky thuat Lap trinh - Du an Quan ly Vi pham CLB F-Code

Tai lieu nay tong hop cac ky thuat lap trinh dac biet, kho hieu, va logic
dang chu y trong toan bo code project. Thiet ke de ho tro thuyet trinh va
giai trinh cau hoi tu giao vien.

---

## 1. Atomic Save voi Crash Recovery (fileio.c)

### Van de

Khi chuong trinh dang ghi file ma bi tat dot ngngat (crash, mat dien, kill),
file du lieu co the bi huong hoan toan — du lieu cu da bi xoa nhung du lieu
moi chua ghi xong.

### Giai phap: 3-file strategy (.tmp → .bak → .dat)

```
Luu file di qua 3 buoc:

Buoc 1: Ghi du lieu moi vao file .tmp (tam)
Buoc 2: Doi ten file .dat cu thanh .bak (backup)
Buoc 3: Doi ten file .tmp thanh .dat (chinh)
Buoc 4: Xoa file .bak
```

**Tai sao phai lam the nay?**

- Neu crash giua buoc 1-2: File .dat cu van nguyen ven, chua bi sua
- Neu crash giua buoc 2-3: File .bak ton tai, co the phuc hoi
- Neu crash giua buoc 3-4: File .dat da cap nhat thanh cong

```c
// fileio.c:154-179 - replaceStoreFile()
static int replaceStoreFile(...) {
    // Step 1: .dat → .bak
    rename(dataFile, bakFile);
    // Step 2: .tmp → .dat (neu loi → restore tu .bak)
    if (rename(tmpFile, dataFile) != 0) {
        rename(bakFile, dataFile);  // Phuc hoi!
        return -1;
    }
    // Step 3: xoa .bak
    remove(bakFile);
}
```

### Crash Recovery luc khoi dong

Luc chuong trinh bat dau, kiem tra xem co file .tmp ton tai khong:

```c
// fileio.c:69-83 - recoverFromTmp()
static void recoverFromTmp(tmpFile, datFile) {
    if (tmpFile ton tai) {
        if (datFile khong ton tai) {
            // .dat mat nhung .tmp co → phuc hoi tu .tmp
            rename(tmpFile, datFile);
        } else {
            // Ca hai deu co → .dat hop le, xoa .tmp (bo thao tac chua xong)
            remove(tmpFile);
        }
    }
}
```

**Dien giai:** Neu app bi crash giua luc save, lan chay tiep theo se tu dong
phuc hoi hoac don dep.

---

## 2. Transaction Rollback Pattern (violation.c, member.c)

### Van de

Khi mot thao tac can luu nhieu file (vi du: ghi vi pham can luu ca violations.dat
VA members.dat), neu file thu 2 that bai thi file thu 1 da bi luu sai.

### Giai phap: Snapshot + Rollback

```c
// violation.c:208-274 - violationRecord()
Member oldMemberState = *member;       // Chup anh trang thai cu
int oldViolationCount = db->violationCount;

// ... sua du lieu trong memory ...
db->violations[db->violationCount++] = newViolation;
member->violationCount++;
member->totalFine += newViolation.fine;

// Thu luu violations
if (fileioSaveViolations(db) != 0) {
    db->violationCount = oldViolationCount;  // Rollback!
    *member = oldMemberState;                // Rollback!
    return -1;
}

// Thu luu members
if (fileioSaveMembers(db) != 0) {
    db->violationCount = oldViolationCount;  // Rollback ca 2!
    *member = oldMemberState;
    fileioSaveViolations(db);  // Luu lai violations cu (phuc hoi)
    return -1;
}
```

**Tai sao:** Day la bai toan "transaction" co ban — dam bao du lieu luon nhat
quan, khong bi trang thai nua chua nua moi. Cung tuong tu trong `memberAdd()`
o member.c:156-171.

---

## 3. Pointer-based Sorting khong lam thay doi mang goc (report.c)

### Van de

Sap xep danh sach thanh vien theo so lan vi pham, NHUNG khong duoc thay doi
thu tu mang `members[]` goc vi cac module khac dang dua vao thu tu do.

### Giai phap: Mang con tro tro den phan tu goc

```c
// report.c:143-148
const Member *sorted[MAX_MEMBERS];  // Mang con tro (khong phai mang Member)
for (int i = 0; i < db->memberCount; i++) {
    sorted[i] = &db->members[i];    // Tro den phan tu goc
}

sortMemberPointersByViolationCount(db, sorted, db->memberCount, ascending);
```

**Ket qua:**
- Mang `db->members[]` goc **khong bi thay doi thu tu**
- Mang `sorted[]` chua con tro duoc sap xep → in theo thu tu moi
- Swap chi doi cho tro (8 bytes), khong copy toan bo struct Member (~200 bytes)

```c
// report.c:48-52
static void swapMemberPointers(const Member **a, const Member **b) {
    const Member *tmp = *a;
    *a = *b;
    *b = tmp;
}
```

**Tai sao dung con tro:** Hieu qua hon copy struct nguyen. Cung la cach an toan
vi khong anh huong den du lieu goc. Day la ky thuat thuong dung trong C khi
can sap xep ma khong muon thay doi nguon du lieu.

---

## 4. Cascade Delete (member.c)

### Van de

Khi xoa mot thanh vien, can dong thoi xoa:
- Tai khoan dang nhap cua ho (trong accounts[])
- Tat ca vi pham cua ho (trong violations[])
- Ban ghi thanh vien (trong members[])

### Giai phap: Xoa theo thu tu + in-place array shift

```c
// member.c:432-461
// 1. Xoa account (thu hoi quyen truy cap truoc)
int aIndex = 0;
while (aIndex < db->accountCount) {
    if (strcmp(db->accounts[aIndex].studentId, studentId) == 0) {
        // Dich phan tu phia sau len de lap day khoang trong
        for (int j = aIndex; j < db->accountCount - 1; j++) {
            db->accounts[j] = db->accounts[j + 1];
        }
        db->accountCount--;
        // KHONG tang aIndex — can kiem tra lai vi tri nay
    } else {
        aIndex++;
    }
}

// 2. Xoa violations (cung pattern)
// 3. Xoa member
```

**Tai sao dung while thay vi for:** Khi xoa phan tu tai vi tri `i`, phan tu
duoc dich len tu `i+1` cung can kiem tra. Neu dung `for` voi `i++` se bo sot.

**Thu tu luu file:** Accounts → Violations → Members. Luu account truoc de
dam bao ngay ca khi loi xay ra o buoc giua, tai khoan da bi thu hoi (an toan
hon la de tai khoan ton tai ma khong co thanh vien).

---

## 5. Role-based Fine Recalculation (member.c)

### Van de

Khi doi chuc vu mot thanh vien (vi du tu Thanh vien → Truong nhom), muc phat
thay doi: Thanh vien phat 20.000 VND, Truong nhom phat 50.000 VND. Can tinh
lai toan bo cac vi pham **chua dong** cua thanh vien do.

### Giai phap

```c
// member.c:286-303 - recalcFines()
static void recalcFines(AppDatabase *db, Member *m) {
    double newFineRate = (m->role == MEMBER_ROLE_MEMBER) ? 20000.0 : 50000.0;
    m->totalFine = 0.0;

    for (int i = 0; i < db->violationCount; i++) {
        Violation *v = &db->violations[i];
        if (strcmp(v->studentId, m->studentId) != 0 || v->isPaid != 0) {
            continue;  // Bo qua: khong phai cua thanh vien nay, hoac da dong
        }
        if (v->reason != REASON_VIOLENCE) {
            v->fine = newFineRate;  // Cap nhat muc phat moi
        }
        m->totalFine += v->fine;   // Tinh lai tong no
    }
}
```

**Tai sao chi tinh lai vi pham chua dong:** Vi pham da dong la lich su, khong
duoc sua lai. Day la quy tac nghiep vu — khong thay doi qua khu.

**Tai sao bo qua REASON_VIOLENCE:** Vi pham bao luc khong phat tien (chi Out CLB),
nen khong ap dung muc phat moi.

---

## 6. Tu dong Out CLB khi vang qua nhieu (violation.c)

### Logic

```
Vang 1 buoi:         Binh thuong
Vang 2 buoi lien tiep: Theo doi
Vang 3 buoi lien tiep: CANH BAO (them 1 buoi nua → Out)
Vang 4+ buoi lien tiep: QUa nguong → hoi xac nhan Out CLB
```

### Ky thuat: reset consecutiveAbsences theo ly do

```c
// violation.c:74-84 - handleAbsent()
member->consecutiveAbsences++;  // Tang khi vang

// violation.c:246-247 - case REASON_NO_JACKET/NO_ACTIVITY
member->consecutiveAbsences = 0;  // Reset khi khong phai vang!
```

**Tai sao:** So buoi vang la "lien tiep" (consecutive). Neu thanh vien vang
hom nay nhung ngay mai di binh thuong (mac ao, tham gia HD) thi bo dem ve 0.
Chi dem vang hop mat moi tang.

### Bao luc → Out CLB ngay lap tuc

```c
// violation.c:56-72 - handleViolence()
v->fine = 0.0;          // Khong phat tien
v->penalty = PENALTY_OUT_CLB;

if (confirmOutClb(member->fullName)) {
    member->isActive = STATUS_OUT_CLB;  // Out ngay!
}
member->consecutiveAbsences = 0;  // Reset vi khong phai vang
```

**Tai sao reset absences:** Thanh vien da Out CLB roi, khong can theo doi
vang nua. Gia su thanh vien duoc khoi phuc trang thai sau do.

---

## 7. Session Management bang static variable (auth.c)

### Ky thuat

```c
// auth.c:11-12
static Account currentSession;   // Chi file auth.c moi truy cap duoc
static int sessionActive = 0;
```

**Tai sao dung static file-scope:** Trong C, `static` o file-scope gioi han
bien chi hien trong file do. Khac o module khac KHONG THE truy cap truc tiep
vao `currentSession` — phai thong qua `authGetSession()`.

Day la cach C giai quyet "encapsulation" (dong goi) — tuong tu nhu `private`
trong OOP nhung khong co class.

```c
// auth.c:117-122
Account *authGetSession(void) {
    if (sessionActive) return &currentSession;
    return NULL;  // Chua dang nhap → tra NULL
}
```

**Tai sao copy struct khi login:**
```c
// auth.c:102
currentSession = *acc;  // Copy toan bo struct Account
```

Copy gia tri thay vi giu con tro → session la ban doc lap. Neu database thay
doi (vi du: khoa tai khoan), session van giu trang thai cu cho den khi dang
xuat. Tuy nhien, khi doi mat khau thi can **cap nhat ca session**:
```c
// auth.c:175-176
strncpy(session->password, newPass, MAX_PASS_LEN - 1);  // Cap nhat session
```

---

## 8. Cross-reference Aggregation (report.c)

### Van de

Tinh tien phat theo ban, nhung Violation chi luu `studentId`, khong luu `team`.
Can "tra cuu" ngược tu violation → member → team.

### Giai phap: Nested loop

```c
// report.c:8-34 - aggregateTeamTotals()
for (int i = 0; i < db->violationCount; i++) {         // Moi vi pham
    const Violation *v = &db->violations[i];
    int team = -1;

    for (int j = 0; j < db->memberCount; j++) {         // Tim thanh vien
        if (strcmp(db->members[j].studentId, v->studentId) == 0) {
            team = db->members[j].team;                  // Lay team
            break;                                       // Tim thay → dung
        }
    }

    if (team >= TEAM_ACADEMIC && team <= TEAM_MEDIA) {
        if (v->isPaid) collected[team] += v->fine;
        else           outstanding[team] += v->fine;
    }
}
```

**Tai sao khong luu team truc tiep trong Violation:** Vi team cua thanh vien
co the thay doi (chuyen ban). Neu luu team trong violation thi khi thanh vien
chuyen ban, cac vi pham cu se sai. Thiet ke hien tai luon tra cuu tu du lieu
moi nhat.

**Ham nay duoc reuse:** Ca `reportTeamStats()` va `reportExportTxt()` deu goi
`aggregateTeamTotals()` — khong bi lap code (DRY).

---

## 9. Platform Abstraction: Windows + POSIX (utils.c, fileio.c)

### Ky thuat: Conditional Compilation

```c
// utils.c:286-311 - getExeDir()
#ifdef _WIN32
    GetModuleFileNameA(NULL, buffer, (DWORD)size);  // Windows API
    char *lastSlash = strrchr(buffer, '\\');
#else
    ssize_t len = readlink("/proc/self/exe", buffer, size - 1);  // Linux
    char *lastSlash = strrchr(buffer, '/');
#endif
```

```c
// fileio.c:34-36
#ifdef _WIN32
    snprintf(dataDir, ..., "%s\\data", exeDir);    // Backslash
#else
    snprintf(dataDir, ..., "%s/data", exeDir);     // Forward slash
#endif
```

```c
// utils.h:127-133
#ifdef _WIN32
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)               // Windows
#else
    #include <sys/stat.h>
    #define MKDIR(path) mkdir(path, 0755)           // POSIX
#endif
```

**Tai sao:** Windows dung backslash `\`, Linux dung slash `/`. Windows dung
`_mkdir()`, Linux dung `mkdir()`. Windows dung `GetModuleFileNameA()`, Linux
dung `readlink("/proc/self/exe")`. Code nay chay duoc ca hai he dieu hanh.

---

## 10. Makefile voi 30+ Compiler Warnings (Makefile)

### Ky thuat

```makefile
CFLAGS := -std=c17 -m64 -Wall -Wextra -Wpedantic -Wshadow \
          -Wconversion -Wformat=2 -Wformat-security -Wfloat-equal \
          -Wundef -Wsign-conversion -Wcast-align -Wcast-qual \
          -Wmissing-prototypes -Wmissing-declarations \
          -Wunreachable-code -Wnull-dereference \
          -Wimplicit-fallthrough -Wswitch-enum -Wpointer-arith \
          -Iinclude
```

**Tai sao nhieu flag the nay:**
- `-Wshadow`: Bao loi khi bien cuc bo trung ten bien ben ngoai → de bi bug
- `-Wconversion`: Bao khi ep kieu mat du lieu (int → char, double → float)
- `-Wformat=2`: Kiem tra printf/scanf chong che, bao loi sai format specifier
- `-Wswitch-enum`: Bao khi switch thieu case (vi du: them ly do vi pham moi
  nhung quen them case trong switch)
- `-Wimplicit-fallthrough`: Bao khi switch case bi "roi qua" case tiep theo
  ma khong co break hoac comment
- `-Wnull-dereference`: Bao khi truy cap con tro NULL

**Dong gop:** Project nay build SACH, khong co warning nao. Dieu nay dam bao
chat luong code tu giai doan compile.

---

## 11. Account Lockout sau 3 lan sai mat khau (auth.c)

### Ky thuat: Persistent state

```c
// auth.c:74-93
if (strcmp(acc->password, password) != 0) {
    acc->failCount++;                                    // Tang so lan sai
    if (acc->failCount >= 3) {
        acc->isLocked = 1;                               // Khoa tai khoan
        fileioSaveAccounts(db);                          // Luu ngay vao file!
        return -1;
    }
    fileioSaveAccounts(db);                              // Luu so lan sai
    printf("[THONG BAO] Con lai %d lan thu\n", 3 - acc->failCount);
}
```

**Tai sao phai save vao file ngay:** Neu khong save, khi app crash hoac tat,
bo dem failCount se mat. Thanh vien co the thu vo han. Luu vao file dam bao
trang thai "khoa" duoc giu lau dai.

**Reset khi dang nhap thanh cong:**
```c
// auth.c:97
acc->failCount = 0;  // Reset ve 0 khi dang nhap dung
```

**Chi BCN moi duoc reset mat khau:**
```c
// auth.c:188-225 - authResetPassword()
// Reset mat khau ve MSSV, dong thoi mo khoa tai khoan
strncpy(db->accounts[idx].password, targetStudentId, MAX_PASS_LEN - 1);
db->accounts[idx].failCount = 0;
db->accounts[idx].isLocked = 0;
```

---

## 12. Auto-create Default Admin (fileio.c)

### Ky thuat: First-run detection

```c
// fileio.c:261-274
if (db->accountCount == 0) {
    // Chua co tai khoan nao → lan chay dau tien
    strcpy(db->accounts[0].studentId, "ADMIN");
    strcpy(db->accounts[0].password, "ADMIN");
    db->accounts[0].role = ACCOUNT_ROLE_BCN;
    db->accountCount = 1;
    fileioSaveAccounts(db);  // Luu ngay
}
```

**Tai sao:** De demo va trien khai — chuong trinh tu tao tai khoan admin
mac dinh, khong can tao thu cong. Cung dam bao luon co it nhat 1 tai khoan
de dang nhap.

---

## 13. Pagination trong Member List (member.c)

### Ky thuat: In 20 dong roi dung cho nhan Enter

```c
// member.c:541-555
if ((i + 1) % 20 == 0 && (i + 1) < db->memberCount) {
    printf("\n[Nhan Enter de xem trang tiep theo hoac nhap 'q' roi Enter de thoat]: ");
    char buf[10];
    readString(buf, sizeof(buf));
    if (buf[0] == 'q' || buf[0] == 'Q') {
        break;  // Thoat giua chang
    }
    // In lai header cua bang
}
```

**Tai sao:** Neu co 1000 thanh vien, in het ra terminal se rat dai va kho doc.
Pagination cho phep nguoi dung doc tung trang, giong nhu lenh `more` trong
Linux.

---

## 14. Date Parsing voi Day-bound Normalization (utils.c)

### Ky thuat: start-of-day va end-of-day

```c
// utils.c:185-222 - parseDate()
if (isEndOfDay) {
    timeinfo.tm_hour = 23;
    timeinfo.tm_min = 59;
    timeinfo.tm_sec = 59;    // Cuoi ngay
} else {
    timeinfo.tm_hour = 0;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;     // Dau ngay
}
```

**Tai sao:** Khi search vi pham tu "01/03/2026" den "31/03/2026":
- Start = 01/03/2026 00:00:00 (dau ngay)
- End = 31/03/2026 23:59:59 (cuoi ngay)

Neu khong lam the nay, vi pham xay ra luc 14:00 ngay 31/03 se bi bo sot vi
`time_t` cua "31/03/2026" mac dinh la 00:00 (dau ngay), nho hon 14:00.

---

## 15. Selected Sort Implementation (report.c)

### Tai sao dung Selection Sort thay vi qsort()

```c
// report.c:54-83 - sortMemberPointersByViolationCount()
// Selection Sort tu implement
for (int i = 0; i < memberCount - 1; i++) {
    int selected = i;
    for (int j = i + 1; j < memberCount; j++) {
        // Tim phan tu nho nhat/lon nhat
    }
    if (selected != i) swapMemberPointers(&sorted[i], &sorted[selected]);
}
```

**Ly do:**
1. `qsort()` cua thu vien C khong co tham so context — khong the truyen
   `db` vao de goi `countMemberViolations()`. Khong the dung con tro ham
   thuong vi ham so sanh can tham chieu den database.
2. Voi so luong thanh vien toi da 1000, Selection Sort O(n^2) van chap nhan
   duoc (toi da ~500.000 phep so sanh, chay trong duoi 1 giay).
3. De hieu, de trinh bay cho giao vien — khong can giai thich function pointer
   phuc tap cua `qsort()`.

---

## 16. Singleton Pattern cho Database Toan cuc (main.c)

```c
// main.c:13
static AppDatabase gDb;  // Mot ban duy nhat cho toan bo chuong trinh
```

**Tai sao khong dung malloc:**
- AppDatabase co kich thuoc co dinh (1000 members × ~200 bytes + 10000 violations
  × ~130 bytes ≈ ~1.5 MB)
- Stack khong du cho (Windows stack mac dinh ~1 MB)
- Nhung static/global duoc cap phat trong segment .bss/.data, khong can malloc
- Khong co rui ro memory leak, khong can free()
- Don gian, de trinh bay

---

## 17. Ngan xoa tai khoan cua chinh minh (member.c)

```c
// member.c:404-410
Account *session = authGetSession();
if (session != NULL && strcmp(session->studentId, studentId) == 0) {
    printf("[LOI] Khong the xoa tai khoan cua chinh ban.\n");
    return -1;
}
```

**Tai sao:** Neu BCN dang nhap va xoa chinh minh, session hien tai se tro
den tai khoan da bi xoa → crash hoac hanh vi khong xac dinh. Day la business
rule thuong thay trong moi he thong quan tri.

---

## 18. Auto-create Account khi Them Member (member.c)

```c
// member.c:140-154
// Tu dong tao tai khoan voi mat khau mac dinh = MSSV
Account newAccount;
strncpy(newAccount.studentId, newMember.studentId, MAX_MSSV_LEN - 1);
strncpy(newAccount.password, newMember.studentId, MAX_PASS_LEN - 1);  // MK = MSSV
newAccount.role = (newMember.role == MEMBER_ROLE_BCN) ? ACCOUNT_ROLE_BCN
                                                       : ACCOUNT_ROLE_MEMBER;
```

**Tai sao:** Moi thanh vien them vao tu dong co tai khoan dang nhap. Khong can
BCN phai tao tai khoan rieng. Mat khau mac dinh la MSSV de de nho, thanh vien
nen doi ngay sau khi dang nhap dau tien.

---

## Tom tat cac ky thuat theo loai

| Loai | Ky thuat | Vi tri |
|------|----------|--------|
| An toan du lieu | Atomic save (3-file) | fileio.c |
| An toan du lieu | Crash recovery tu .tmp | fileio.c |
| An toan du lieu | Transaction rollback | violation.c, member.c |
| An toan du lieu | Backup + restore khi loi | fileio.c |
| Bao mat | Account lockout 3 lan | auth.c |
| Bao mat | Session private static | auth.c |
| Bao mat | Ngan xoa chinh minh | member.c |
| Bao mat | Role-based quyen (BCN/TV) | auth.c, violation.c |
| Thiet ke OOP | Encapsulation qua static | auth.c |
| Thiet ke OOP | Singleton global DB | main.c |
| Thiet ke OOP | DRY - reuse aggregateTeamTotals | report.c |
| Thuat toan | Pointer-based sort | report.c |
| Thuat toan | Cross-reference aggregation | report.c |
| Thuat toan | In-place array shift delete | member.c |
| Thuat toan | Selection sort tu viet | report.c |
| Nghiep vu | Cascade delete (TV+VP+TK) | member.c |
| Nghiep vu | Role-based fine recalc | member.c |
| Nghiep vu | Out CLB tu dong (vang 4+ LT) | violation.c |
| Nghiep vu | Day-bound date normalization | utils.c |
| Nghiep vu | Auto-create account | member.c |
| Nghiep vu | Reset consecutive on non-absent | violation.c |
| Cross-platform | Windows/POSIX path | utils.c, fileio.c |
| Cross-platform | Conditional compilation | utils.h |
| Build | 30+ compiler warnings | Makefile |
| UX | Pagination 20 dong | member.c |
| UX | Enter de giu nguyen khi edit | member.c |
