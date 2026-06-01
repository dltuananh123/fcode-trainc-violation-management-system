/**
 * @file validate.c
 * @brief Comprehensive input validation implementations.
 *
 * All validation rules for the violation management system.
 * Every function prints specific Vietnamese error messages on failure.
 */

#include "validate.h"
#include "member.h"
#include "types.h"
#include "ui.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ============================================================
 * HELPER UTILITIES
 * ============================================================ */

void trimSpaces(char *str) {
  if (str == NULL || str[0] == '\0') {
    return;
  }

  /* Trim leading spaces */
  int start = 0;
  while (str[start] == ' ') {
    start++;
  }

  /* Trim trailing spaces */
  int end = (int)strlen(str) - 1;
  while (end >= 0 && str[end] == ' ') {
    end--;
  }

  if (start > end) {
    str[0] = '\0';
    return;
  }

  /* Shift string */
  int len = end - start + 1;
  if (start > 0) {
    memmove(str, str + start, (size_t)len);
  }
  str[len] = '\0';
}

int isLettersOnly(const char *str) {
  if (str == NULL) {
    return 0;
  }
  int i = 0;
  while (str[i] != '\0') {
    unsigned char c = (unsigned char)str[i];
    if (c == ' ') {
      i++;
      continue;
    }
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
      i++;
      continue;
    }

    /* Valid UTF-8 multi-byte sequences (Vietnamese diacritics) */
    if (c >= 0xC0 && c <= 0xDF) {
      /* 2-byte: check next byte is continuation (0x80-0xBF) */
      if ((unsigned char)str[i + 1] >= 0x80 &&
          (unsigned char)str[i + 1] <= 0xBF) {
        i += 2;
        continue;
      }
      return 0; /* Invalid UTF-8 */
    }
    if (c >= 0xE0 && c <= 0xEF) {
      /* 3-byte: Vietnamese diacritics are in this range */
      if ((unsigned char)str[i + 1] >= 0x80 &&
          (unsigned char)str[i + 1] <= 0xBF &&
          (unsigned char)str[i + 2] >= 0x80 &&
          (unsigned char)str[i + 2] <= 0xBF) {
        i += 3;
        continue;
      }
      return 0;
    }
    /* Reject 4-byte (emoji, CJK, etc.) and invalid bytes */
    return 0;
  }
  return 1;
}

int countWords(const char *str) {
  if (str == NULL || str[0] == '\0') {
    return 0;
  }
  int count = 0;
  int inWord = 0;
  for (int i = 0; str[i] != '\0'; i++) {
    if (str[i] == ' ') {
      inWord = 0;
    } else if (!inWord) {
      count++;
      inWord = 1;
    }
  }
  return count;
}

/* ============================================================
 * MSSV — FPT STUDENT ID
 * ============================================================ */

int validateMSSVFormat(const char *mssv) {
  if (mssv == NULL || mssv[0] == '\0') {
    printf(ERR_LOI "MSSV khong duoc de trong!\n");
    return 0;
  }

  if (strlen(mssv) != 8) {
    printf(ERR_LOI "MSSV phai dung 8 ky tu (dang XY123456)! "
                   "Ban nhap %d ky tu.\n",
           (int)strlen(mssv));
    return 0;
  }

  /* Char 0: Campus */
  const char *campus = "SHDCQshdcq";
  if (strchr(campus, mssv[0]) == NULL) {
    printf(ERR_LOI "Ky tu dau tien phai la Campus: "
                   "S(HCM), H(HN), D(DN), C(CT), Q(QN)!\n");
    return 0;
  }

  /* Char 1: Division */
  const char *division = "EAS eas";
  if (strchr(division, mssv[1]) == NULL) {
    printf(ERR_LOI "Ky tu thu hai phai la khoi nganh: "
                   "E(Engineering), A(Arts), S(Social)!\n");
    return 0;
  }

  /* Chars 2-7: 6 digits */
  for (int i = 2; i < 8; i++) {
    if (mssv[i] < '0' || mssv[i] > '9') {
      printf(ERR_LOI "6 ky tu cuoi cua MSSV phai la so!\n");
      return 0;
    }
  }

  /* No spaces */
  if (strchr(mssv, ' ') != NULL) {
    printf(ERR_LOI "MSSV khong duoc chua khoang trang!\n");
    return 0;
  }

  return 1;
}

int validateMSSV(const char *mssv, const AppDatabase *db) {
  if (!validateMSSVFormat(mssv)) {
    return 0;
  }

  /* Check uniqueness */
  if (db != NULL) {
    if (memberFindById(db, mssv) != -1) {
      printf(ERR_LOI "MSSV \"%s\" da ton tai trong he thong!\n", mssv);
      return 0;
    }
  }

  return 1;
}

const char *mssvCampusName(char campus) {
  switch (campus) {
  case 'S':
  case 's':
    return "TP.HCM";
  case 'H':
  case 'h':
    return "Ha Noi";
  case 'D':
  case 'd':
    return "Da Nang";
  case 'C':
  case 'c':
    return "Can Tho";
  case 'Q':
  case 'q':
    return "Quy Nhon";
  default:
    return "Khong ro";
  }
}

const char *mssvDivisionName(char division) {
  switch (division) {
  case 'E':
  case 'e':
    return "Engineering & Technology";
  case 'A':
  case 'a':
    return "Arts & Humanities";
  case 'S':
  case 's':
    return "Social Sciences";
  default:
    return "Khong ro";
  }
}

void mssvAutoUpper(char *mssv) {
  if (mssv == NULL) {
    return;
  }
  for (int i = 0; mssv[i] != '\0'; i++) {
    mssv[i] = (char)toupper((unsigned char)mssv[i]);
  }
}

/* ============================================================
 * FULL NAME
 * ============================================================ */

int validateName(const char *name) {
  if (name == NULL || name[0] == '\0') {
    printf(ERR_LOI "Ho ten khong duoc de trong!\n");
    return 0;
  }

  /* Check for whitespace-only */
  int allSpace = 1;
  for (int i = 0; name[i] != '\0'; i++) {
    if (name[i] != ' ') {
      allSpace = 0;
      break;
    }
  }
  if (allSpace) {
    printf(ERR_LOI "Ho ten khong duoc de trong!\n");
    return 0;
  }

  /* Min 2 words */
  int words = countWords(name);
  if (words < 2) {
    printf(ERR_LOI "Ho ten phai co it nhat 2 tu (ho va ten)!\n");
    return 0;
  }

  /* Max 5 words */
  if (words > 5) {
    printf(ERR_LOI "Ho ten khong duoc qua 5 tu!\n");
    return 0;
  }

  /* Max 50 chars */
  if ((int)strlen(name) > 50) {
    printf(ERR_LOI "Ho ten khong duoc vuot qua 50 ky tu!\n");
    return 0;
  }

  /* Letters and spaces only */
  if (!isLettersOnly(name)) {
    printf(ERR_LOI "Ho ten chi duoc chua chu cai va khoang trang! "
                   "Khong duoc chua so hoac ky tu dac biet.\n");
    return 0;
  }

  /* Banned words check */
  const char *banned[] = {"admin", "system", "null", "test", "none"};
  char lowerName[MAX_NAME_LEN];
  strncpy(lowerName, name, MAX_NAME_LEN - 1);
  lowerName[MAX_NAME_LEN - 1] = '\0';
  for (int i = 0; lowerName[i]; i++) {
    lowerName[i] = (char)tolower((unsigned char)lowerName[i]);
  }
  for (int b = 0; b < 5; b++) {
    if (strcmp(lowerName, banned[b]) == 0) {
      printf(ERR_LOI "Ho ten khong hop le!\n");
      return 0;
    }
  }

  return 1;
}

void nameAutoFix(char *name) {
  if (name == NULL || name[0] == '\0') {
    return;
  }

  /* Trim spaces */
  trimSpaces(name);

  /* Remove consecutive spaces */
  int j = 0;
  int prevSpace = 0;
  for (int i = 0; name[i] != '\0'; i++) {
    if (name[i] == ' ') {
      if (prevSpace) {
        continue;
      }
      prevSpace = 1;
    } else {
      prevSpace = 0;
    }
    name[j++] = name[i];
  }
  name[j] = '\0';

  /* Capitalize first letter of each word */
  int newWord = 1;
  for (int i = 0; name[i] != '\0'; i++) {
    unsigned char c = (unsigned char)name[i];
    if (c == ' ') {
      newWord = 1;
    } else if (c < 0x80) {
      /* ASCII only — don't mess with UTF-8 diacritics */
      if (newWord && c >= 'a' && c <= 'z') {
        name[i] = (char)(c - 32);
      } else if (!newWord && c >= 'A' && c <= 'Z') {
        name[i] = (char)(c + 32);
      }
      newWord = 0;
    } else {
      newWord = 0;
    }
  }
}

/* ============================================================
 * EMAIL
 * ============================================================ */

static int isValidEmailChar(char c, int isLocal) {
  if (c >= 'a' && c <= 'z') {
    return 1;
  }
  if (c >= 'A' && c <= 'Z') {
    return 1;
  }
  if (c >= '0' && c <= '9') {
    return 1;
  }
  if (isLocal) {
    return c == '.' || c == '_' || c == '%' || c == '+' || c == '-';
  }
  return c == '.' || c == '-';
}

int validateEmail(const char *email) {
  if (email == NULL || email[0] == '\0') {
    printf(ERR_LOI "Email khong duoc de trong!\n");
    return 0;
  }

  int len = (int)strlen(email);
  if (len > 100) {
    printf(ERR_LOI "Email khong duoc vuot qua 100 ky tu!\n");
    return 0;
  }

  /* No spaces */
  if (strchr(email, ' ') != NULL) {
    printf(ERR_LOI "Email khong duoc chua khoang trang!\n");
    return 0;
  }

  /* Find @ */
  const char *at = strchr(email, '@');
  if (at == NULL) {
    printf(ERR_LOI "Email phai chua dau \"@\"!\n");
    return 0;
  }

  /* Only one @ */
  if (strchr(at + 1, '@') != NULL) {
    printf(ERR_LOI "Email chi duoc chua mot dau \"@\"!\n");
    return 0;
  }

  /* Text before @ */
  if (at == email) {
    printf(ERR_LOI "Email phai co ten truoc dau \"@\"!\n");
    return 0;
  }

  /* Local part (before @) length validation */
  int localLen = (int)(at - email);
  if (localLen > 64) {
    printf(ERR_LOI "Phan ten email truoc dau \"@\" khong duoc qua 64 ky tu!\n");
    return 0;
  }

  /* Validate local part (before @) */
  for (const char *p = email; p < at; p++) {
    if (!isValidEmailChar(*p, 1)) {
      printf(ERR_LOI "Email chua ky tu khong hop le trong phan ten!\n");
      return 0;
    }
  }

  /* No consecutive dots in local part */
  for (const char *p = email; p < at - 1; p++) {
    if (*p == '.' && *(p + 1) == '.') {
      printf(ERR_LOI "Email khong duoc chua dau cham lien tiep!\n");
      return 0;
    }
  }

  /* Local part cannot start or end with dot */
  if (email[0] == '.' || at[-1] == '.') {
    printf(ERR_LOI "Email khong duoc bat dau hoac ket thuc bang dau cham!\n");
    return 0;
  }

  /* Domain after @ */
  const char *domain = at + 1;
  if (domain[0] == '\0') {
    printf(ERR_LOI "Email phai co ten mien sau dau \"@\"!\n");
    return 0;
  }

  /* Domain length validation */
  int domainLen = (int)strlen(domain);
  if (domainLen > 253) {
    printf(ERR_LOI "Ten mien email sau dau \"@\" khong duoc qua 253 ky tu!\n");
    return 0;
  }

  /* Validate domain characters */
  for (const char *p = domain; *p != '\0'; p++) {
    if (!isValidEmailChar(*p, 0)) {
      printf(ERR_LOI "Email chua ky tu khong hop le trong ten mien!\n");
      return 0;
    }
  }

  /* Domain must contain . */
  const char *lastDot = strrchr(domain, '.');
  if (lastDot == NULL) {
    printf(ERR_LOI "Email phai co ten mien hop le (vd: gmail.com)!\n");
    return 0;
  }

  /* No consecutive dots */
  if (strstr(domain, "..") != NULL) {
    printf(ERR_LOI "Email khong duoc chua dau cham lien tiep!\n");
    return 0;
  }

  /* Split domain by '.' and check label lengths and hyphens */
  const char *startLabel = domain;
  while (1) {
    const char *endLabel = strchr(startLabel, '.');
    int labelLen;
    if (endLabel == NULL) {
      labelLen = (int)strlen(startLabel);
    } else {
      labelLen = (int)(endLabel - startLabel);
    }

    if (labelLen == 0) {
      printf(ERR_LOI "Ten mien co label rong!\n");
      return 0;
    }
    if (labelLen > 63) {
      printf(ERR_LOI "Moi phan cua ten mien khong duoc qua 63 ky tu!\n");
      return 0;
    }
    if (startLabel[0] == '-' || startLabel[labelLen - 1] == '-') {
      printf(ERR_LOI "Phan ten mien khong duoc bat dau hoac ket thuc bang dau "
                     "gach ngang!\n");
      return 0;
    }

    if (endLabel == NULL) {
      break;
    }
    startLabel = endLabel + 1;
  }

  /* TLD must be at least 2 letters */
  const char *tld = lastDot + 1;
  int tldLen = (int)strlen(tld);
  if (tldLen < 2) {
    printf(
        ERR_LOI
        "Domain phai co it nhat 2 ky tu sau dau cham cuoi (vd: .com, .vn)!\n");
    return 0;
  }

  /* TLD must be only letters */
  for (const char *p = tld; *p != '\0'; p++) {
    if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'))) {
      printf(ERR_LOI "Duoi domain chi duoc chua chu cai (vd: .com, .vn)!\n");
      return 0;
    }
  }

  /* Soft warn for uncommon TLDs */
  static const char *commonTlds[] = {"com", "vn",  "net", "org",  "edu",
                                     "io",  "dev", "gov", "info", "co",
                                     "me",  "app", NULL};
  int knownTld = 0;
  for (int i = 0; commonTlds[i] != NULL; i++) {
    int match = 1;
    for (int j = 0; tld[j] != '\0' || commonTlds[i][j] != '\0'; j++) {
      if (tolower((unsigned char)tld[j]) !=
          tolower((unsigned char)commonTlds[i][j])) {
        match = 0;
        break;
      }
    }
    if (match) {
      knownTld = 1;
      break;
    }
  }
  if (!knownTld) {
    printf(ERR_CANH_BAO "Ten mien quoc gia hoac to chuc \".%s\" la it pho "
                        "bien. Vui long kiem tra lai.\n",
           tld);
  }

  return 1;
}

int validateEmailUnique(const char *email, const AppDatabase *db,
                        const char *excludeId) {
  if (!validateEmail(email)) {
    return 0;
  }

  if (db != NULL) {
    for (int i = 0; i < db->memberCount; i++) {
      if (db->members[i].isDeleted) {
        continue;
      }
      if (excludeId != NULL &&
          strcmp(db->members[i].studentId, excludeId) == 0) {
        continue;
      }
      if (strcmp(db->members[i].email, email) == 0) {
        printf(ERR_LOI "Email \"%s\" da duoc su dung boi thanh vien khac!\n",
               email);
        return 0;
      }
    }
  }

  return 1;
}

/* ============================================================
 * PHONE — Vietnamese only
 * ============================================================ */

void phoneNormalize(char *phone) {
  if (phone == NULL) {
    return;
  }

  char normalized[MAX_PHONE_LEN];
  int j = 0;

  /* Strip everything except digits and leading + */
  for (int i = 0; phone[i] != '\0' && j < MAX_PHONE_LEN - 1; i++) {
    if (phone[i] >= '0' && phone[i] <= '9') {
      normalized[j++] = phone[i];
    }
    /* skip spaces, dashes, dots, parens */
  }
  normalized[j] = '\0';

  /* Convert +84/84 prefix to 0 */
  if (j >= 3 && normalized[0] == '8' && normalized[1] == '4') {
    /* Remove "84" prefix, add "0" */
    char temp[MAX_PHONE_LEN];
    temp[0] = '0';
    strncpy(temp + 1, normalized + 2, MAX_PHONE_LEN - 2);
    temp[MAX_PHONE_LEN - 1] = '\0';
    strncpy(normalized, temp, MAX_PHONE_LEN - 1);
    normalized[MAX_PHONE_LEN - 1] = '\0';
  }

  strncpy(phone, normalized, MAX_PHONE_LEN - 1);
  phone[MAX_PHONE_LEN - 1] = '\0';
}

int validatePhone(const char *phone) {
  if (phone == NULL || phone[0] == '\0') {
    printf(ERR_LOI "So dien thoai khong duoc de trong!\n");
    return 0;
  }

  /* Make a copy and normalize */
  char normalized[MAX_PHONE_LEN];
  strncpy(normalized, phone, MAX_PHONE_LEN - 1);
  normalized[MAX_PHONE_LEN - 1] = '\0';
  phoneNormalize(normalized);

  /* Must start with 0 */
  if (normalized[0] != '0') {
    printf(ERR_LOI "So dien thoai Viet Nam phai bat dau bang so \"0\"!\n");
    return 0;
  }

  /* Must be exactly 10 digits */
  int len = (int)strlen(normalized);
  if (len != 10) {
    printf(ERR_LOI "So dien thoai phai dung 10 chu so! "
                   "Ban nhap %d chu so.\n",
           len);
    return 0;
  }

  /* All must be digits */
  for (int i = 0; i < len; i++) {
    if (normalized[i] < '0' || normalized[i] > '9') {
      printf(ERR_LOI "So dien thoai chi duoc chua chu so!\n");
      return 0;
    }
  }

  /* Second digit must be 3, 5, 7, 8, or 9 */
  if (normalized[1] != '3' && normalized[1] != '5' && normalized[1] != '7' &&
      normalized[1] != '8' && normalized[1] != '9') {
    printf(ERR_LOI "Dau so \"%c%c\" khong phai dau so dien thoai "
                   "Viet Nam hop le!\n",
           normalized[0], normalized[1]);
    return 0;
  }

  return 1;
}

const char *phoneCarrier(const char *phone) {
  if (phone == NULL || strlen(phone) < 3) {
    return "Khong ro";
  }

  /* Normalize first */
  char normalized[MAX_PHONE_LEN];
  strncpy(normalized, phone, MAX_PHONE_LEN - 1);
  normalized[MAX_PHONE_LEN - 1] = '\0';
  phoneNormalize(normalized);

  /* Check 3-digit prefix */
  int prefix = 0;
  prefix = (normalized[1] - '0') * 10 + (normalized[2] - '0');

  /* Viettel: 86, 96, 97, 98, 32-39 */
  if ((prefix >= 32 && prefix <= 39) || prefix == 86 || prefix == 96 ||
      prefix == 97 || prefix == 98) {
    return "Viettel";
  }

  /* Vinaphone: 91, 94, 88, 83, 84, 85, 81, 82 */
  if (prefix == 81 || prefix == 82 || prefix == 83 || prefix == 84 ||
      prefix == 85 || prefix == 88 || prefix == 91 || prefix == 94) {
    return "Vinaphone";
  }

  /* Mobifone: 89, 90, 93, 70, 79, 77, 76, 78 */
  if (prefix == 70 || prefix == 76 || prefix == 77 || prefix == 78 ||
      prefix == 79 || prefix == 89 || prefix == 90 || prefix == 93) {
    return "Mobifone";
  }

  /* iTel */
  if (prefix == 87) {
    return "iTel";
  }

  /* Wintel */
  if (prefix == 55) {
    return "Wintel";
  }

  return "Khong ro";
}

int validatePhoneUnique(const char *phone, const AppDatabase *db,
                        const char *excludeId) {
  if (!validatePhone(phone)) {
    return 0;
  }

  /* Normalize for comparison */
  char normalized[MAX_PHONE_LEN];
  strncpy(normalized, phone, MAX_PHONE_LEN - 1);
  normalized[MAX_PHONE_LEN - 1] = '\0';
  phoneNormalize(normalized);

  if (db != NULL) {
    for (int i = 0; i < db->memberCount; i++) {
      if (db->members[i].isDeleted) {
        continue;
      }
      if (excludeId != NULL &&
          strcmp(db->members[i].studentId, excludeId) == 0) {
        continue;
      }
      char existingNorm[MAX_PHONE_LEN];
      strncpy(existingNorm, db->members[i].phone, MAX_PHONE_LEN - 1);
      existingNorm[MAX_PHONE_LEN - 1] = '\0';
      phoneNormalize(existingNorm);
      if (strcmp(existingNorm, normalized) == 0) {
        printf(ERR_LOI "So dien thoai \"%s\" da duoc su dung!\n", phone);
        return 0;
      }
    }
  }

  return 1;
}

/* ============================================================
 * PASSWORD
 * ============================================================ */

int validatePassword(const char *password) {
  if (password == NULL || password[0] == '\0') {
    printf(ERR_LOI "Mat khau khong duoc de trong!\n");
    return 0;
  }

  int len = (int)strlen(password);

  if (len < 8) {
    printf(ERR_LOI "Mat khau phai co it nhat 8 ky tu!\n");
    return 0;
  }

  if (len > 30) {
    printf(ERR_LOI "Mat khau khong duoc vuot qua 30 ky tu!\n");
    return 0;
  }

  /* Must contain at least 1 uppercase letter */
  int hasUpper = 0;
  for (int i = 0; i < len; i++) {
    if (password[i] >= 'A' && password[i] <= 'Z') {
      hasUpper = 1;
      break;
    }
  }
  if (!hasUpper) {
    printf(ERR_LOI "Mat khau phai chua it nhat 1 chu hoa!\n");
    return 0;
  }

  /* Must contain at least 1 lowercase letter */
  int hasLower = 0;
  for (int i = 0; i < len; i++) {
    if (password[i] >= 'a' && password[i] <= 'z') {
      hasLower = 1;
      break;
    }
  }
  if (!hasLower) {
    printf(ERR_LOI "Mat khau phai chua it nhat 1 chu thuong!\n");
    return 0;
  }

  /* Must contain at least 1 digit */
  int hasDigit = 0;
  for (int i = 0; i < len; i++) {
    if (password[i] >= '0' && password[i] <= '9') {
      hasDigit = 1;
      break;
    }
  }
  if (!hasDigit) {
    printf(ERR_LOI "Mat khau phai chua it nhat 1 chu so!\n");
    return 0;
  }

  /* Must contain at least 1 special character */
  int hasSpecial = 0;
  for (int i = 0; i < len; i++) {
    if ((password[i] >= 33 && password[i] <= 47) ||
        (password[i] >= 58 && password[i] <= 64) ||
        (password[i] >= 91 && password[i] <= 96) ||
        (password[i] >= 123 && password[i] <= 126)) {
      hasSpecial = 1;
      break;
    }
  }
  if (!hasSpecial) {
    printf(ERR_LOI "Mat khau phai chua it nhat 1 ky tu dac biet (@, $, !, %%, "
                   "*, ?, &, ...)!\n");
    return 0;
  }

  /* No spaces */
  if (strchr(password, ' ') != NULL) {
    printf(ERR_LOI "Mat khau khong duoc chua khoang trang!\n");
    return 0;
  }

  return 1;
}

/* ============================================================
 * GENERAL INPUT VALIDATION
 * ============================================================ */

int validateNotEmpty(const char *input) {
  if (input == NULL || input[0] == '\0') {
    printf(ERR_LOI "Du lieu khong duoc de trong!\n");
    return 0;
  }
  /* Check whitespace only */
  for (int i = 0; input[i] != '\0'; i++) {
    if (input[i] != ' ') {
      return 1;
    }
  }
  printf(ERR_LOI "Du lieu khong duoc de trong!\n");
  return 0;
}

int validateInRange(int value, int min, int max) {
  if (value < min || value > max) {
    printf(ERR_LOI "Lua chon khong hop le! Vui long chon tu %d-%d!\n", min,
           max);
    return 0;
  }
  return 1;
}

int checkInjectionChars(const char *input) {
  if (input == NULL) {
    return 1;
  }

  /* S1: single quote */
  if (strchr(input, '\'') != NULL) {
    printf(ERR_LOI "Ky tu \"'\" khong duoc phep!\n");
    return 0;
  }

  /* S2: double quote */
  if (strchr(input, '"') != NULL) {
    printf(ERR_LOI "Ky tu '\"' khong duoc phep!\n");
    return 0;
  }

  /* S3: semicolon */
  if (strchr(input, ';') != NULL) {
    printf(ERR_LOI "Ky tu \";\" khong duoc phep!\n");
    return 0;
  }

  /* S4: double dash */
  if (strstr(input, "--") != NULL) {
    printf(ERR_LOI "Chuoi \"--\" khong duoc phep!\n");
    return 0;
  }

  /* S5: block comment */
  if (strstr(input, "/*") != NULL || strstr(input, "*/") != NULL) {
    printf(ERR_LOI "Chuoi \"/*\" va \"*/\" khong duoc phep!\n");
    return 0;
  }

  /* S6: equals */
  if (strchr(input, '=') != NULL) {
    printf(ERR_LOI "Ky tu \"=\" khong duoc phep!\n");
    return 0;
  }

  /* S7: pipe */
  if (strchr(input, '|') != NULL) {
    printf(ERR_LOI "Ky tu \"|\" khong duoc phep!\n");
    return 0;
  }

  /* S8: ampersand */
  if (strchr(input, '&') != NULL) {
    printf(ERR_LOI "Ky tu \"&\" khong duoc phep!\n");
    return 0;
  }

  /* S9: angle brackets */
  if (strchr(input, '<') != NULL || strchr(input, '>') != NULL) {
    printf(ERR_LOI "Ky tu \"<\" va \">\" khong duoc phep!\n");
    return 0;
  }

  return 1;
}

int sanitizeInput(char *input) {
  if (input == NULL) {
    return 1;
  }

  /* Trim spaces */
  trimSpaces(input);

  /* Remove control characters */
  int j = 0;
  for (int i = 0; input[i] != '\0'; i++) {
    unsigned char c = (unsigned char)input[i];
    if (c < 32 && c != '\t') {
      continue; /* Skip control chars except tab */
    }
    input[j++] = input[i];
  }
  input[j] = '\0';

  /* Check for injection characters */
  return checkInjectionChars(input);
}

int validateDate(const char *date) {
  if (date == NULL || date[0] == '\0') {
    printf(ERR_LOI "Ngay khong duoc de trong!\n");
    return 0;
  }

  /* Check format: dd/mm/yyyy */
  if (strlen(date) != 10) {
    printf(ERR_LOI "Dinh dang ngay khong hop le! Su dung dd/mm/yyyy.\n");
    return 0;
  }

  if (date[2] != '/' || date[5] != '/') {
    printf(ERR_LOI "Dinh dang ngay khong hop le! Su dung dd/mm/yyyy.\n");
    return 0;
  }

  /* Parse day, month, year */
  int day = 0;
  int month = 0;
  int year = 0;
  for (int i = 0; i < 2; i++) {
    if (!isdigit((unsigned char)date[i]) ||
        !isdigit((unsigned char)date[3 + i]) ||
        !isdigit((unsigned char)date[6 + i])) {
      printf(ERR_LOI "Dinh dang ngay khong hop le! Su dung dd/mm/yyyy.\n");
      return 0;
    }
    day = day * 10 + (date[i] - '0');
    month = month * 10 + (date[3 + i] - '0');
    year = year * 10 + (date[6 + i] - '0');
  }
  for (int i = 8; i < 10; i++) {
    if (!isdigit((unsigned char)date[i])) {
      printf(ERR_LOI "Dinh dang ngay khong hop le! Su dung dd/mm/yyyy.\n");
      return 0;
    }
    year = year * 10 + (date[i] - '0');
  }

  /* Validate ranges */
  if (day < 1 || day > 31) {
    printf(ERR_LOI "Ngay khong hop le! Ngay phai tu 1 den 31.\n");
    return 0;
  }

  if (month < 1 || month > 12) {
    printf(ERR_LOI "Thang khong hop le! Thang phai tu 1 den 12.\n");
    return 0;
  }

  /* Get current time for dynamic year validation */
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int todayDay = 1;
  int todayMonth = 1;
  int todayYear = 2026; /* fallback */
  if (t != NULL) {
    todayDay = t->tm_mday;
    todayMonth = t->tm_mon + 1;
    todayYear = t->tm_year + 1900;
  }

  if (year < 2020 || year > todayYear) {
    printf(ERR_LOI "Nam khong hop le! Nam phai tu 2020 den %d.\n", todayYear);
    return 0;
  }

  /* Days per month */
  int daysPerMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  /* Leap year check */
  int isLeap = ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
  if (isLeap) {
    daysPerMonth[2] = 29;
  }

  if (day > daysPerMonth[month]) {
    if (month == 2 && !isLeap) {
      printf(ERR_LOI "Nam %d khong phai nam nhuan! "
                     "Thang 2 chi co 28 ngay.\n",
             year);
    } else {
      printf(ERR_LOI "Ngay khong hop le! Thang %d toi da %d ngay.\n", month,
             daysPerMonth[month]);
    }
    return 0;
  }

  /* Not future date */
  if (year > todayYear || (year == todayYear && month > todayMonth) ||
      (year == todayYear && month == todayMonth && day > todayDay)) {
    printf(ERR_LOI "Ngay khong duoc lon hon ngay hien tai "
                   "(%02d/%02d/%04d)!\n",
           todayDay, todayMonth, todayYear);
    return 0;
  }

  return 1;
}

int validateDateRange(const char *start, const char *end) {
  if (!validateDate(start)) {
    return 0;
  }
  if (!validateDate(end)) {
    return 0;
  }

  /* Parse both dates and compare */
  int sDay = ((start[0] - '0') * 10) + (start[1] - '0');
  int sMonth = ((start[3] - '0') * 10) + (start[4] - '0');
  int sYear = ((start[6] - '0') * 1000) + ((start[7] - '0') * 100) +
              ((start[8] - '0') * 10) + (start[9] - '0');

  int eDay = ((end[0] - '0') * 10) + (end[1] - '0');
  int eMonth = ((end[3] - '0') * 10) + (end[4] - '0');
  int eYear = ((end[6] - '0') * 1000) + ((end[7] - '0') * 100) +
              ((end[8] - '0') * 10) + (end[9] - '0');

  if (sYear > eYear || (sYear == eYear && sMonth > eMonth) ||
      (sYear == eYear && sMonth == eMonth && sDay > eDay)) {
    printf(ERR_LOI "Ngay bat dau (%s) phai truoc hoac bang "
                   "ngay ket thuc (%s)!\n",
           start, end);
    return 0;
  }

  return 1;
}
