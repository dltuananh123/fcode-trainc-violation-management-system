#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileio.h"

/*
1. Path of file to read/write
*/

#define FILE_MEMBERS "data/members.dat"
#define FILE_VIOLATIONS "data/violations.dat"
#define FILE_ACCOUNTS "data/accounts.dat"

#define TMP_MEMBERS "data/members.dat.tmp"
#define TMP_VIOLATIONS "data/violations.dat.tmp"
#define TMP_ACCOUNTS "data/accounts.dat.tmp"

/*
2. Crash residual cleanup
*/

static void cleanup_tmp_files() {
    /* Delete file forms crash residual */
    remove(TMP_MEMBERS);
    remove(TMP_VIOLATIONS);
    remove(TMP_ACCOUNTS);
}

/*
3.Function save data
*/

int fileio_save_members(AppDatabase *db){
    FILE *fp = fopen(TMP_MEMBERS,"wb");
    if(fp == NULL){
        printf("[LOI] Khong the tao file tam de luu members!\n");
        return -1;
    }
    /* Ghi so luong thanh vien vao file tam */
    fwrite(&(db->memberCount),sizeof(int),1,fp);

    /* Ghi du lieu*/
    if(db->memberCount > 0){
        fwrite(db->members,sizeof(Member),(size_t)db->memberCount,fp);
    }
    fclose(fp);

    /* Xoa file cu, doi ten file tam */

    remove(FILE_MEMBERS);
    if (rename(TMP_MEMBERS,FILE_MEMBERS) != 0) {
        printf("[LOI] Khong the ghi de file members.dat!\n");
        return -1;
    }
    return 0;
}

int fileio_save_violations(AppDatabase *db){
    FILE *fp = fopen(TMP_VIOLATIONS,"wb");
    if (fp == NULL){
        printf("[LOI] Khong the tao file tam de luu violations!\n");
        return -1;
    }

    fwrite(&(db->violationCount),sizeof(int),1,fp);

    if (db->violationCount > 0){
        fwrite(db->violations,sizeof(Violation),(size_t)db->violationCount,fp);

    }
    fclose(fp);

    remove(FILE_VIOLATIONS);
    if (rename(TMP_VIOLATIONS,FILE_VIOLATIONS) != 0){
        printf("[LOI] Khong the ghi de file violations.dat!\n");
        return -1;
    }
    return 0;
}

int fileio_save_accounts(AppDatabase *db){
    FILE *fp = fopen(TMP_ACCOUNTS,"wb");
    if (fp == NULL){
        printf("[LOI] Khong the tao file tam de luu accounts!\n");
        return -1;
    }

    fwrite(&(db->accountCount),sizeof(int),1,fp);

    if (db->accountCount > 0){
        fwrite(db->accounts,sizeof(Account),(size_t)db->accountCount,fp);
    }

    fclose(fp);

    remove(FILE_ACCOUNTS);
    if (rename(TMP_ACCOUNTS,FILE_ACCOUNTS) != 0){
        printf("[LOI] Khong the ghi de file accounts.dat!\n");
        return -1;
    }
    return 0;
}

/*
4.Load data & first-run init
*/

int fileio_load_all(AppDatabase *db){
    db->memberCount = 0;
    db->violationCount = 0;
    db->accountCount = 0;

    /*Delete file crash .tmp before load*/
    cleanup_tmp_files();

    /* Load accounts */
    FILE *fpAcc = fopen(FILE_ACCOUNTS,"rb");
    if (fpAcc != NULL){
        fread(&(db->accountCount),sizeof(int),1,fpAcc);
        if (db->accountCount > 0){
            fread(db->accounts,sizeof(Account),(size_t)db->accountCount,fpAcc);
        }
        fclose(fpAcc);
    }
    /*First-run Init for account: Create account Admin if file is empty*/
    if (db->accountCount == 0){
        printf("[CANH BAO] Khong tim thay tai khoan nao. Dang tao tai khoan ADMIN mac dinh...\n");
        
        strcpy(db->accounts[0].studentId, "ADMIN");
        strcpy(db->accounts[0].password,"ADMIN");
        db->accounts[0].role = ACCOUNT_ROLE_BCN;
        db->accounts[0].isLocked = 0;
        db->accounts[0].failCount = 0;

        db->accountCount = 1;
        fileio_save_accounts(db);

    }

    /* Load Member */

    FILE *fpMen = fopen(FILE_MEMBERS,"rb");
    if (fpMen != NULL){
        fread(&(db->memberCount),sizeof(int),1,fpMen);
        if (db->memberCount > 0){
            fread(db->members,sizeof(Member),(size_t)db->memberCount,fpMen);
        }
        fclose(fpMen);
    }
    else{
        /*First_run create file empty*/
        fileio_save_members(db);
    }

    /*Load violations*/

    FILE *fpVio = fopen(FILE_VIOLATIONS,"rb");
    if (fpVio != NULL){
        fread(&(db->violationCount),sizeof(int),1,fpVio);
        if (db-> violationCount > 0){
            fread(db->violations,sizeof(Violation),(size_t)db->violationCount,fpVio);
        }
        fclose(fpVio);
    }
    else{
        fileio_save_violations(db);
    }
    return 0;
}