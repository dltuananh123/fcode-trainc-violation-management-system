@echo off
chcp 65001 >nul

if exist bin rmdir /S /Q bin

mingw32-make

gcc -std=c17 -m64 -Wall -Iinclude tools/seed_data.c -o bin/seed_data.exe

bin\seed_data.exe
bin\violation-management-system.exe
