#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define MAX_MEMBERS     1000
#define MAX_VIOLATIONS  5000
#define MAX_ACCOUNTS    1000

#define MAX_NAME_LEN    100
#define MAX_EMAIL_LEN   100
#define MAX_PHONE_LEN   15
#define MAX_ID_LEN      10
#define MAX_PASS_LEN    50
#define MAX_NOTE_LEN    200


#define FILE_MEMBERS    "data/members.dat"
#define FILE_VIOLATIONS "data/violations.dat"
#define FILE_ACCOUNTS   "data/accounts.dat"


#define TEAM_ACADEMIC   0
#define TEAM_PLANNING   1
#define TEAM_HR         2
#define TEAM_MEDIA      3
#define TEAM_COUNT      4


#define ROLE_MEMBER     0   
#define ROLE_LEADER     1   
#define ROLE_BCN        2   


#define ACC_ROLE_MEMBER 0
#define ACC_ROLE_BCN    1


#define REASON_NO_SHIRT     0   
#define REASON_ABSENT       1   
#define REASON_NO_ACTIVITY  2   
#define REASON_VIOLENCE     3   
#define REASON_COUNT        4


#define FINE_MEMBER         20000.0
#define FINE_LEADER         50000.0
#define FINE_VIOLENCE       0.0


#define MAX_LOGIN_ATTEMPTS  3   
#define ABSENCE_WARNING     2   
#define ABSENCE_OUT         3   


#define PENALTY_FINE        0   
#define PENALTY_OUT_CLB     1   


#define NOT_PAID            0
#define PAID                1


#define NOT_LOCKED          0
#define LOCKED              1


typedef struct {
    char    fullName[MAX_NAME_LEN];     
    char    email[MAX_EMAIL_LEN];       
    char    phone[MAX_PHONE_LEN];       
    char    studentId[MAX_ID_LEN];      
    int     team;                       
    int     role;                       
    int     violationCount;             
    int     consecutiveAbsences;        
    double  totalFine;                  
} Member;


typedef struct {
    char    studentId[MAX_ID_LEN];      
    int     reason;                     
    time_t  violationTime;              
    double  fine;                       
    int     isPaid;                     
    int     penalty;                    
    char    note[MAX_NOTE_LEN];         
} Violation;


typedef struct {
    char    studentId[MAX_ID_LEN];      
    char    password[MAX_PASS_LEN];     
    int     role;                       
    int     isLocked;                   
    int     failCount;                  
} Account;



extern Member     members[MAX_MEMBERS];
extern Violation  violations[MAX_VIOLATIONS];
extern Account    accounts[MAX_ACCOUNTS];

extern int memberCount;
extern int violationCount;
extern int accountCount;


extern char currentStudentId[MAX_ID_LEN];
extern int  currentRole;   


static const char *TEAM_NAMES[TEAM_COUNT] = {
    "Academic",
    "Planning",
    "HR",
    "Media"
};


static const char *ROLE_NAMES[] = {
    "Member",
    "Leader/Vice",
    "Ban Chu Nhiem"
};


static const char *REASON_NAMES[REASON_COUNT] = {
    "Khong mac ao CLB",
    "Vang hop khong phep",
    "Khong tham gia hoat dong",
    "Bao luc"
};

#endif 
