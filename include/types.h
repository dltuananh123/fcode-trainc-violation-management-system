/**
 * @file types.h
 * @brief Central data structures and constants for the FCode TrainC Violation
 * Management System.
 *
 * This file defines all the primary data structures (Account, Member,
 * Violation, AppDatabase) and domain constants used throughout the system. It
 * is designed to be the single source of truth for data shapes to prevent
 * circular dependencies between modules.
 */

#ifndef TYPES_H
#define TYPES_H

#include <time.h>

/* ============================================================
 * CAPACITY LIMITS
 * ============================================================ */

#define MAX_MEMBERS 1000
#define MAX_VIOLATIONS 10000

#define MAX_NAME_LEN 50
#define MAX_MSSV_LEN 10
#define MAX_EMAIL_LEN 50
#define MAX_PHONE_LEN 15
#define MAX_NOTE_LEN 100
#define MAX_PASS_LEN 20

/* ============================================================
 * DOMAIN CONSTANTS
 * ============================================================ */

#define STATUS_ACTIVE 1
#define STATUS_OUT_CLB 0

#define REASON_NO_JACKET 0
#define REASON_ABSENT 1
#define REASON_NO_ACTIVITY 2
#define REASON_VIOLENCE 3

#define TEAM_ACADEMIC 0
#define TEAM_PLANNING 1
#define TEAM_HR 2
#define TEAM_MEDIA 3

#define PENALTY_FINE 0
#define PENALTY_OUT_CLB 1

#define ACCOUNT_ROLE_MEMBER 0
#define ACCOUNT_ROLE_BCN 1

#define MEMBER_ROLE_MEMBER 0
#define MEMBER_ROLE_LEADER 1
#define MEMBER_ROLE_BCN 2

/* ============================================================
 * STRUCT DEFINITIONS
 * ============================================================ */

/**
 * @struct Account
 * @brief Represents user credentials and authentication state.
 */
typedef struct {
  int role;
  int isLocked;
  int failCount;
  char studentId[MAX_MSSV_LEN];
  char password[MAX_PASS_LEN];
} Account;

/**
 * @struct Member
 * @brief Represents a club member with profile and status information.
 */
typedef struct {
  double totalFine;
  int team;
  int role;
  int isActive;
  int consecutiveAbsences;
  int violationCount;
  char studentId[MAX_MSSV_LEN];
  char fullName[MAX_NAME_LEN];
  char email[MAX_EMAIL_LEN];
  char phone[MAX_PHONE_LEN];
} Member;

/**
 * @struct Violation
 * @brief Represents a single recorded violation incident.
 */
typedef struct {
  time_t violationTime;
  double fine;
  int reason;
  int isPaid;
  int penalty;
  char studentId[MAX_MSSV_LEN];
  char note[MAX_NOTE_LEN];
} Violation;

/**
 * @struct AppDatabase
 * @brief Global state container to hold all system data arrays in memory.
 */
typedef struct {
  Member members[MAX_MEMBERS];
  Violation violations[MAX_VIOLATIONS];
  Account accounts[MAX_MEMBERS];
  int memberCount;
  int violationCount;
  int accountCount;
} AppDatabase;

#endif /* TYPES_H */
