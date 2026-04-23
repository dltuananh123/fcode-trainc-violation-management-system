#ifndef FILEIO_H
#define FILEIO_H

#include <types.h>

int fileio_load_all(AppDatabase *db);

int fileio_save_members(AppDatabase *db);
int fileio_save_violations(AppDatabase *db);
int fileio_save_accounts(AppDatabase *db);

#endif // FILEIO_H