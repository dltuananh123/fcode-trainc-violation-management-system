#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

/* =========================================
 * INPUT HANDLING HELPERS
 * ========================================= */

void read_string(char *buffer, size_t size) {
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

int read_int(int *value) {
  int result = scanf("%d", value);
  int c;
  while ((c = getchar()) != '\n' && c != EOF) {
    /* discard characters */
  }
  return result == 1 ? 1 : 0;
}

/* =========================================
 * VALIDATION HELPERS
 * ========================================= */

int is_email_valid(const char *email) {
  if (email == NULL) {
    return 0;
  }

  /* Very simple validation as per AC 1: contains '@' and '.' */
  const char *at_sign = strchr(email, '@');
  const char *dot_sign = strchr(email, '.');

  if (at_sign != NULL && dot_sign != NULL) {
    return 1;
  }
  return 0;
}

int is_id_valid(const char *id) {
  if (id == NULL || strlen(id) == 0) {
    return 0;
  }

  /* Check length <= MAX_MSSV_LEN - 1 */
  if (strlen(id) >= MAX_MSSV_LEN) {
    return 0;
  }

  /* Check if alphanumeric */
  for (size_t i = 0; i < strlen(id); i++) {
    if (!isalnum((unsigned char)id[i])) {
      return 0;
    }
  }
  return 1;
}

/* =========================================
 * TIME & DATE HELPERS
 * ========================================= */

void format_time(time_t t, char *buffer, size_t buf_size) {
  if (buffer == NULL || buf_size == 0) {
    return;
  }

  struct tm *timeinfo = localtime(&t);
  if (timeinfo != NULL) {
    strftime(buffer, buf_size, "%d/%m/%Y %H:%M", timeinfo);
  } else {
    buffer[0] = '\0';
  }
}

int parse_date(const char *buffer, time_t *t, int is_end_of_day) {
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

  if (is_end_of_day) {
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

/* =========================================
 * DISPLAY NAME MAPPERS
 * ========================================= */

const char *team_name(int team_id) {
  switch (team_id) {
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

const char *member_role_name(int role_id) {
  switch (role_id) {
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

const char *account_role_name(int role_id) {
  switch (role_id) {
  case ACCOUNT_ROLE_MEMBER:
    return "Thanh vien";
  case ACCOUNT_ROLE_BCN:
    return "Ban chu nhiem";
  default:
    return "Khong xac dinh";
  }
}

const char *reason_name(int reason_id) {
  switch (reason_id) {
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
