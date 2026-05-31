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
#include <stdarg.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* Terminal width used for centering */
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

static int uiVisibleLen(const char *s) {
  int len = 0;
  while (*s) {
    if (*s == '\033') {
      s++;
      if (*s == '[') {
        s++;
        while (*s && *s != 'm') s++;
        if (*s) s++;
      }
    } else {
      len++;
      s++;
    }
  }
  return len;
}

void uiDrawMenuRow(const char *text) {
  printf(COLOR_BLUE BOX_V COLOR_RESET);
  int visible = uiVisibleLen(text);
  printf("%s", text);
  for (int i = visible; i < UI_TERM_WIDTH - 2; i++) {
    printf(" ");
  }
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");
}

void uiDrawMenuRowFmt(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char buf[256];
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  uiDrawMenuRow(buf);
}

void uiPause(void) {
  printf(COLOR_CYAN "\n  Nhan Enter de tiep tuc..." COLOR_RESET);
  fflush(stdout);
  while (getchar() != '\n') {
    /* wait for Enter */
  }
}

/* ============================================================
 * LOGO DISPLAY
 * ============================================================ */

void uiDrawLogo(void) {
  printf("\n");
  printf("\n");

  /* Firebird Phoenix ASCII Art */
  /* g4 = darkest green */
  printf("                                    " COLOR_GREEN "--" COLOR_RESET "\n");
  printf("                              " COLOR_GREEN "======" COLOR_GRAY "--" COLOR_RESET "\n");
  /* g3 = dark green */
  printf("                         " COLOR_GREEN "===========-" COLOR_RESET "\n");
  printf("                    " COLOR_GREEN "=================" COLOR_RESET "\n");
  /* g2 = green */
  printf("                " COLOR_GREEN "++++================" COLOR_RESET "\n");
  printf("               " COLOR_GREEN "+++++===========" COLOR_RESET "\n");
  /* g1 = light green, p1 = pale green */
  printf("               " COLOR_GREEN "+++++++====" COLOR_RESET "     " COLOR_GREEN "****" COLOR_RESET "\n");
  printf("               " COLOR_GREEN "++++++" COLOR_RESET "     " COLOR_GREEN "********" COLOR_RESET "\n");
  printf("              " COLOR_GREEN "++" COLOR_RESET "     " COLOR_GREEN "***********" COLOR_RESET COLOR_GREEN "+" COLOR_RESET "\n");
  /* p1 = pale green, yl = yellow light */
  printf("                " COLOR_GREEN "**********" COLOR_RESET COLOR_YELLOW "+++++++" COLOR_RESET "\n");
  printf("               " COLOR_GREEN "*****" COLOR_RESET COLOR_YELLOW "+++++++++++" COLOR_RESET "\n");
  /* yl = yellow light */
  printf("               " COLOR_YELLOW "*++++++++++" COLOR_RESET "\n");
  /* yl = yellow light, yd = orange */
  printf("              " COLOR_YELLOW "++++++" COLOR_RESET COLOR_ORANGE "====" COLOR_RESET "\n");
  printf("             " COLOR_ORANGE "+==-----------" COLOR_RESET "\n");
  /* d = dark (use GRAY for visibility) */
  printf("               " COLOR_GRAY "---------------" COLOR_RESET "\n");
  printf("                  " COLOR_GRAY "-------------" COLOR_RESET "\n");
  printf("                     " COLOR_GRAY "-----" COLOR_RESET "\n");
  printf("\n");
  printf("\n");

  /* FCODE text */
  printf("              " COLOR_BOLD COLOR_GREEN "███████╗       ██████╗ ██████╗ ██████╗ ███████╗" COLOR_RESET "\n");
  printf("              " COLOR_BOLD COLOR_GREEN "██╔════╝      ██╔════╝██╔═══██╗██╔══██╗██╔════╝" COLOR_RESET "\n");
  printf("              " COLOR_BOLD COLOR_GREEN "█████╗  ───── ██║     ██║   ██║██║  ██║█████╗  " COLOR_RESET "\n");
  printf("              " COLOR_BOLD COLOR_GREEN "██╔══╝        ██║     ██║   ██║██║  ██║██╔══╝  " COLOR_RESET "\n");
  printf("              " COLOR_BOLD COLOR_GREEN "██╗           ╚██████╗╚██████╔╝██████╔╝███████╗" COLOR_RESET "\n");
  printf("              " COLOR_BOLD COLOR_GREEN "╚═╝            ╚═════╝ ╚═════╝ ╚═════╝ ╚══════╝" COLOR_RESET "\n");
  printf("\n");
  printf("                 " COLOR_DIM COLOR_GREEN " C o d e   t h e   d r e a m" COLOR_RESET "\n");
  printf("\n");
  printf("\n");
}
