#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "utils.h"
#include "types.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
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

/* ============================================================
 * VALIDATION HELPERS
 * ============================================================ */

int isEmailValid(const char *email) {
  if (email == NULL || email[0] == '\0') {
    return 0;
  }

  size_t len = strlen(email);

  /* Must not start or end with '@' or '.' */
  if (email[0] == '@' || email[0] == '.' || email[len - 1] == '@' ||
      email[len - 1] == '.') {
    return 0;
  }

  /* Exactly one '@' */
  const char *atSign = strchr(email, '@');
  if (atSign == NULL) {
    return 0;
  }
  if (strchr(atSign + 1, '@') != NULL) {
    return 0;
  }

  /* Local part (before @) must have at least one character */
  if (atSign == email) {
    return 0;
  }

  /* Domain part (after @) must have at least one '.' with non-empty parts */
  const char *domain = atSign + 1;
  if (*domain == '\0' || *domain == '.') {
    return 0;
  }

  const char *dot = strchr(domain, '.');
  if (dot == NULL) {
    return 0;
  }

  /* Character before dot cannot be '.' (no "..") and after dot must exist */
  if (*(dot - 1) == '.' || *(dot + 1) == '\0') {
    return 0;
  }

  /* Only allow letters, digits, '.', '_', '%', '+', '-', '@' */
  for (size_t i = 0; i < len; i++) {
    char c = email[i];
    if (!isalnum((unsigned char)c) && c != '.' && c != '_' && c != '%' &&
        c != '+' && c != '-' && c != '@') {
      return 0;
    }
  }

  return 1;
}

int isIdValid(const char *id) {
  if (id == NULL || strlen(id) == 0) {
    return 0;
  }

  size_t len = strlen(id);

  /* MSSV should be at least 4 characters (e.g. "ADMIN") and at most MAX_MSSV_LEN-1 */
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

int isPhoneValid(const char *phone) {
  if (phone == NULL || phone[0] == '\0') {
    return 0;
  }

  size_t len = strlen(phone);

  /* Phone must be 7-15 digits, may start with '+' */
  size_t start = 0;
  if (phone[0] == '+') {
    start = 1;
    if (len < 8) {
      return 0;
    }
  }

  /* Must have at least 7 digits after optional '+' */
  if (len - start < 7 || len - start > 15) {
    return 0;
  }

  /* All remaining characters must be digits */
  for (size_t i = start; i < len; i++) {
    if (!isdigit((unsigned char)phone[i])) {
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
  case MEMBER_ROLE_BCN:
    return "Ban chu nhiem";
  default:
    return "Khong xac dinh";
  }
}

const char *accountRoleName(int roleId) {
  switch (roleId) {
  case ACCOUNT_ROLE_MEMBER:
    return "Thanh vien";
  case ACCOUNT_ROLE_BCN:
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
