@echo off
chcp 65001 >nul

if exist bin rmdir /S /Q bin

mingw32-make
mingw32-make seed

bin\seed_data.exe
bin\violation-management-system.exe
