/**
 * @file ui.c
 * @brief TUI utility implementations using ANSI escape codes.
 *
 * All UI drawing functions for the beautiful terminal interface.
 * Uses only C standard library — no external dependencies.
 */

#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* Terminal width used for centering */
#define UI_TERM_WIDTH 70

/* ============================================================
 * INITIALIZATION
 * ============================================================ */

void uiInit(void) {
#ifdef _WIN32
  /* Enable ANSI escape codes on Windows 10+ */
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut != INVALID_HANDLE_VALUE) {
    DWORD mode = 0;
    if (GetConsoleMode(hOut, &mode)) {
      mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
      SetConsoleMode(hOut, mode);
    }
  }
  /* Enable UTF-8 output on Windows */
  SetConsoleOutputCP(CP_UTF8);
#endif
}

/* ============================================================
 * SCREEN OPERATIONS
 * ============================================================ */

void uiClear(void) {
#ifdef _WIN32
  system("cls");
#else
  printf("\033[2J\033[H\033[3J");
#endif
}

/* ============================================================
 * DRAWING FUNCTIONS
 * ============================================================ */

void uiDrawBox(int width, int height) {
  /* Top border */
  printf(COLOR_BLUE BOX_TL);
  for (int i = 0; i < width - 2; i++) {
    printf(BOX_H);
  }
  printf(BOX_TR COLOR_RESET "\n");

  /* Side borders */
  for (int row = 0; row < height - 2; row++) {
    printf(COLOR_BLUE BOX_V COLOR_RESET);
    for (int i = 0; i < width - 2; i++) {
      printf(" ");
    }
    printf(COLOR_BLUE BOX_V COLOR_RESET "\n");
  }

  /* Bottom border */
  printf(COLOR_BLUE BOX_BL);
  for (int i = 0; i < width - 2; i++) {
    printf(BOX_H);
  }
  printf(BOX_BR COLOR_RESET "\n");
}

void uiDrawTitle(const char *title) {
  int titleLen = 0;
  /* Count visible characters (skip UTF-8 multi-byte) */
  for (int i = 0; title[i] != '\0'; i++) {
    if ((unsigned char)title[i] < 0x80 ||
        (unsigned char)title[i] >= 0xC0) {
      titleLen++;
    }
  }

  int padding = (UI_TERM_WIDTH - titleLen - 2) / 2;
  if (padding < 0) {
    padding = 0;
  }

  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD COLOR_CYAN);
  for (int i = 0; i < padding; i++) {
    printf(" ");
  }
  printf("%s", title);
  for (int i = 0;
       i < UI_TERM_WIDTH - padding - titleLen - 2; i++) {
    printf(" ");
  }
  printf(COLOR_RESET);
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");
}

void uiDrawStatusBar(const char *user, const char *role) {
  char timeStr[30];
  uiGetCurrentTime(timeStr, sizeof(timeStr));

  /* Top line of status bar */
  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_BOLD);
  printf(" %s | %s (%s) | fcode VMS v2.0 ",
         timeStr, user, role);

  /* Fill remaining width */
  int used = (int)strlen(timeStr) + 3 + (int)strlen(user) + 1 +
             (int)strlen(role) + 1 + 18;
  for (int i = used; i < UI_TERM_WIDTH - 2; i++) {
    printf(" ");
  }
  printf(COLOR_RESET);
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  /* Separator under status bar */
  printf(COLOR_BLUE "\xE2\x95\xA0");
  for (int i = 0; i < UI_TERM_WIDTH - 2; i++) {
    printf(BOX_H);
  }
  printf("\xE2\x95\xA3" COLOR_RESET "\n");
}

void uiDrawBreadcrumb(const char *path) {
  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_DIM " %s ", path);
  int used = (int)strlen(path) + 2;
  for (int i = used; i < UI_TERM_WIDTH - 2; i++) {
    printf(" ");
  }
  printf(COLOR_RESET);
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  /* Separator under breadcrumb */
  printf(COLOR_BLUE "\xE2\x95\xA0");
  for (int i = 0; i < UI_TERM_WIDTH - 2; i++) {
    printf(BOX_H);
  }
  printf("\xE2\x95\xA3" COLOR_RESET "\n");
}

void uiDrawSeparator(void) {
  printf(COLOR_BLUE BOX_V COLOR_RESET);
  for (int i = 0; i < UI_TERM_WIDTH - 2; i++) {
    printf(BOX_H);
  }
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");
}

void uiDrawPanel(int width, int height) {
  /* Top border */
  printf(COLOR_CYAN LINE_TL);
  for (int i = 0; i < width - 2; i++) {
    printf(LINE_H);
  }
  printf(LINE_TR COLOR_RESET "\n");

  /* Side borders */
  for (int row = 0; row < height - 2; row++) {
    printf(COLOR_CYAN LINE_V COLOR_RESET);
    for (int i = 0; i < width - 2; i++) {
      printf(" ");
    }
    printf(COLOR_CYAN LINE_V COLOR_RESET "\n");
  }

  /* Bottom border */
  printf(COLOR_CYAN LINE_BL);
  for (int i = 0; i < width - 2; i++) {
    printf(LINE_H);
  }
  printf(LINE_BR COLOR_RESET "\n");
}

/* ============================================================
 * TEXT UTILITIES
 * ============================================================ */

void uiPrintColored(const char *text, const char *color) {
  printf("%s%s%s", color, text, COLOR_RESET);
}

void uiPrintCentered(const char *text, int width) {
  int textLen = (int)strlen(text);
  int padding = (width - textLen) / 2;
  if (padding < 0) {
    padding = 0;
  }
  for (int i = 0; i < padding; i++) {
    printf(" ");
  }
  printf("%s", text);
  for (int i = padding + textLen; i < width; i++) {
    printf(" ");
  }
}

void uiGetCurrentTime(char *buffer, int bufSize) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  snprintf(buffer, (size_t)bufSize, "%02d/%02d/%04d %02d:%02d",
           t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
           t->tm_hour, t->tm_min);
}
