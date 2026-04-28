#ifndef FILEIO_H
#define FILEIO_H

#include <types.h>

int fileioLoadAll(AppDatabase *db);

int fileioSaveMembers(AppDatabase *db);
int fileioSaveViolations(AppDatabase *db);
int fileioSaveAccounts(AppDatabase *db);

#endif /* FILEIO_H */