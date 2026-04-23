#ifndef FILEIO_H
#define FILEIO_H

#include <types.h>

int fieio_load_all(Appdatabase *db);

int fileio_save_members(Appdatabase *db);
int fileio_save_violations(Appdatabase *db);
int fileio_save_accounts(Appdatabase *db);

#endif // FILEIO_H