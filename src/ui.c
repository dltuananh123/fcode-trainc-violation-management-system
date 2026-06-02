/**
 * @file ui.c
 * @brief TUI utility implementations using ANSI escape codes.
 *
 * All UI drawing functions for the beautiful terminal interface.
 * Uses only C standard library — no external dependencies.
 */

#include "ui.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Terminal width used for centering */
static int currentBoxWidth = UI_TERM_WIDTH;

void uiSetBoxWidth(int width) { currentBoxWidth = width; }

int utf8Len(const char *s) {
  int len = 0;
  if (s == NULL) {
    return 0;
  }
  while (*s) {
    /* Continuation bytes start with 10xxxxxx (0x80 to 0xBF) */
    if ((*s & 0xC0) != 0x80) {
      len++;
    }
    s++;
  }
  return len;
}

void printUtf8Padded(const char *text, int width, int leftAlign) {
  if (text == NULL) {
    text = "";
  }
  int len = utf8Len(text);
  int pad = width - len;
  if (pad < 0) {
    pad = 0;
  }

  if (leftAlign) {
    if (len > width) {
      int printedCols = 0;
      while (*text && printedCols < width) {
        if ((*text & 0xC0) != 0x80) {
          if (printedCols + 1 > width) {
            break;
          }
          printedCols++;
        }
        putchar(*text++);
      }
    } else {
      printf("%s", text);
      for (int i = 0; i < pad; i++) {
        putchar(' ');
      }
    }
  } else {
    for (int i = 0; i < pad; i++) {
      putchar(' ');
    }
    if (len > width) {
      int printedCols = 0;
      while (*text && printedCols < width) {
        if ((*text & 0xC0) != 0x80) {
          if (printedCols + 1 > width) {
            break;
          }
          printedCols++;
        }
        putchar(*text++);
      }
    } else {
      printf("%s", text);
    }
  }
}

void uiDrawMenuBoxBegin(const char *title) {
  uiClear();
  uiSetBoxWidth(MENU_BOX_W);

  /* Top border */
  printf(COLOR_BLUE BOX_TL);
  for (int i = 0; i < MENU_BOX_W - 2; i++) {
    printf(BOX_H);
  }
  printf(BOX_TR COLOR_RESET "\n");

  /* Title */
  printf(COLOR_BLUE BOX_V COLOR_RESET);
  printf(COLOR_DIM " %s", title);
  int titleLen = (int)strlen(title) + 1; /* include leading space */
  for (int i = titleLen; i < MENU_BOX_W - 2; i++) {
    printf(" ");
  }
  printf(COLOR_BLUE BOX_V COLOR_RESET "\n");

  /* Separator */
  printf(COLOR_BLUE "\xE2\x95\xA0");
  for (int i = 0; i < MENU_BOX_W - 2; i++) {
    printf(BOX_H);
  }
  printf("\xE2\x95\xA3" COLOR_RESET "\n");
}

void uiDrawMenuBoxEnd(void) {
  printf(COLOR_BLUE BOX_BL);
  for (int i = 0; i < MENU_BOX_W - 2; i++) {
    printf(BOX_H);
  }
  printf(BOX_BR COLOR_RESET "\n");
}

/* ============================================================
 * INITIALIZATION
 * ============================================================ */

int uiGetTermWidth(void) {
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  if (h != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(h, &csbi)) {
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
  }
#else
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
    return w.ws_col;
  }
#endif
  return UI_TERM_WIDTH; /* fallback */
}

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

  int w = uiGetTermWidth();
  if (w < UI_TERM_WIDTH) {
    printf(ERR_CANH_BAO "Terminal hien tai co be rong %d cot.\n", w);
    printf("           Khuyen nghi toi thieu %d cot de hien thi tot nhat!\n",
           UI_TERM_WIDTH);
    uiPause();
  }
}

/* ============================================================
 * SCREEN OPERATIONS
 * ============================================================ */

void uiClear(void) {
#ifdef _WIN32
  /* Try ANSI first (already enabled in uiInit) */
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD mode = 0;
  if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &mode) &&
      (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
    printf("\033[2J\033[H\033[3J");
  } else if (hOut != INVALID_HANDLE_VALUE) {
    /* Fallback: Win32 API — no shell call */
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
      DWORD cells = (DWORD)(csbi.dwSize.X * csbi.dwSize.Y);
      COORD home = {0, 0};
      DWORD written;
      FillConsoleOutputCharacterA(hOut, ' ', cells, home, &written);
      FillConsoleOutputAttribute(hOut, csbi.wAttributes, cells, home, &written);
      SetConsoleCursorPosition(hOut, home);
    }
  }
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
    if ((unsigned char)title[i] < 0x80 || (unsigned char)title[i] >= 0xC0) {
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
  for (int i = 0; i < UI_TERM_WIDTH - padding - titleLen - 2; i++) {
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
  printf(" %s | %s (%s) | fcode VMS v2.0 ", timeStr, user, role);

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
  uiSetBoxWidth(UI_TERM_WIDTH);

  /* Top border of the screen box */
  printf(COLOR_BLUE BOX_TL);
  for (int i = 0; i < UI_TERM_WIDTH - 2; i++) {
    printf(BOX_H);
  }
  printf(BOX_TR COLOR_RESET "\n");

  /* Breadcrumb path row */
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
  /* Note: localtime is not thread-safe but this application is
     single-threaded. */
  struct tm *t = localtime(&now);
  if (t != NULL) {
    snprintf(buffer, (size_t)bufSize, "%02d/%02d/%04d %02d:%02d", t->tm_mday,
             t->tm_mon + 1, t->tm_year + 1900, t->tm_hour, t->tm_min);
  } else {
    snprintf(buffer, (size_t)bufSize, "--/--/---- --:--");
  }
}

static int uiVisibleLen(const char *s) {
  int len = 0;
  while (*s) {
    if (*s == '\033') {
      s++;
      if (*s == '[') {
        s++;
        while (*s && *s != 'm') {
          s++;
        }
        if (*s) {
          s++;
        }
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
  for (int i = visible; i < currentBoxWidth - 2; i++) {
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

void uiDrawDualMenuRow(int n1, const char *t1, int n2, const char *t2) {
  char row[128];
  if (n2 > 0) {
    snprintf(row, sizeof(row), "  %2d. %-27s  %2d. %-29s", n1, t1, n2, t2);
  } else {
    snprintf(row, sizeof(row), "  %2d. %-27s  %-33s", n1, t1, t2);
  }
  printf(COLOR_BLUE BOX_V COLOR_RESET "%s" COLOR_BLUE BOX_V COLOR_RESET "\n",
         row);
}

void uiTableBegin(const TableColumn cols[], int colCount) {
  printf(COLOR_CYAN "  " LINE_TL COLOR_RESET);
  for (int i = 0; i < colCount; i++) {
    for (int w = 0; w < cols[i].width; w++) {
      printf(COLOR_CYAN LINE_H COLOR_RESET);
    }
    if (i < colCount - 1) {
      printf(COLOR_CYAN LINE_T_DOWN COLOR_RESET);
    }
  }
  printf(COLOR_CYAN LINE_TR COLOR_RESET "\n");

  /* Header Row */
  uiTableRowBegin();
  for (int i = 0; i < colCount; i++) {
    uiTableCell(cols[i].header, cols[i].width, COLOR_BOLD);
  }
  uiTableRowEnd();

  /* Separator */
  uiTableSeparator(cols, colCount);
}

void uiTableSeparator(const TableColumn cols[], int colCount) {
  printf(COLOR_CYAN "  " LINE_T_RIGHT COLOR_RESET);
  for (int i = 0; i < colCount; i++) {
    for (int w = 0; w < cols[i].width; w++) {
      printf(COLOR_CYAN LINE_H COLOR_RESET);
    }
    if (i < colCount - 1) {
      printf(COLOR_CYAN LINE_T_DOWN COLOR_RESET);
    }
  }
  printf(COLOR_CYAN LINE_T_LEFT COLOR_RESET "\n");
}

void uiTableEnd(const TableColumn cols[], int colCount) {
  printf(COLOR_CYAN "  " LINE_BL COLOR_RESET);
  for (int i = 0; i < colCount; i++) {
    for (int w = 0; w < cols[i].width; w++) {
      printf(COLOR_CYAN LINE_H COLOR_RESET);
    }
    if (i < colCount - 1) {
      printf(COLOR_CYAN LINE_T_UP COLOR_RESET);
    }
  }
  printf(COLOR_CYAN LINE_BR COLOR_RESET "\n");
}

void uiTableRowBegin(void) { printf(COLOR_CYAN "  " COLOR_RESET); }

void uiTableRowEnd(void) { printf(COLOR_CYAN LINE_V COLOR_RESET "\n"); }

void uiTableCell(const char *text, int width, const char *color) {
  printf(COLOR_CYAN LINE_V COLOR_RESET);
  printf(" %s", color);
  printUtf8Padded(text, width - 2, 1);
  printf(" " COLOR_RESET);
}

void uiTableCellFmt(int width, const char *color, const char *fmt, ...) {
  printf(COLOR_CYAN LINE_V COLOR_RESET);
  printf(" %s", color);
  va_list args;
  va_start(args, fmt);
  char buf[256];
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  printUtf8Padded(buf, width - 2, 1);
  printf(" " COLOR_RESET);
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
  printf("                                    " COLOR_GREEN "--" COLOR_RESET
         "\n");
  printf("                              " COLOR_GREEN "======" COLOR_GRAY
         "--" COLOR_RESET "\n");
  /* g3 = dark green */
  printf("                         " COLOR_GREEN "===========-" COLOR_RESET
         "\n");
  printf("                    " COLOR_GREEN "=================" COLOR_RESET
         "\n");
  /* g2 = green */
  printf("                " COLOR_GREEN "++++================" COLOR_RESET
         "\n");
  printf("               " COLOR_GREEN "+++++===========" COLOR_RESET "\n");
  /* g1 = light green, p1 = pale green */
  printf("               " COLOR_GREEN "+++++++====" COLOR_RESET
         "     " COLOR_GREEN "****" COLOR_RESET "\n");
  printf("               " COLOR_GREEN "++++++" COLOR_RESET "     " COLOR_GREEN
         "********" COLOR_RESET "\n");
  printf("              " COLOR_GREEN "++" COLOR_RESET "     " COLOR_GREEN
         "***********" COLOR_RESET COLOR_GREEN "+" COLOR_RESET "\n");
  /* p1 = pale green, yl = yellow light */
  printf("                " COLOR_GREEN "**********" COLOR_RESET COLOR_YELLOW
         "+++++++" COLOR_RESET "\n");
  printf("               " COLOR_GREEN "*****" COLOR_RESET COLOR_YELLOW
         "+++++++++++" COLOR_RESET "\n");
  /* yl = yellow light */
  printf("               " COLOR_YELLOW "*++++++++++" COLOR_RESET "\n");
  /* yl = yellow light, yd = orange */
  printf("              " COLOR_YELLOW "++++++" COLOR_RESET COLOR_ORANGE
         "====" COLOR_RESET "\n");
  printf("             " COLOR_ORANGE "+==-----------" COLOR_RESET "\n");
  /* d = dark (use GRAY for visibility) */
  printf("               " COLOR_GRAY "---------------" COLOR_RESET "\n");
  printf("                  " COLOR_GRAY "-------------" COLOR_RESET "\n");
  printf("                     " COLOR_GRAY "-----" COLOR_RESET "\n");
  printf("\n");
  printf("\n");

  /* FCODE text */
  printf("              " COLOR_BOLD COLOR_GREEN
         "███████╗       ██████╗ ██████╗ ██████╗ ███████╗" COLOR_RESET "\n");
  printf("              " COLOR_BOLD COLOR_GREEN
         "██╔════╝      ██╔════╝██╔═══██╗██╔══██╗██╔════╝" COLOR_RESET "\n");
  printf("              " COLOR_BOLD COLOR_GREEN
         "█████╗  ───── ██║     ██║   ██║██║  ██║█████╗  " COLOR_RESET "\n");
  printf("              " COLOR_BOLD COLOR_GREEN
         "██╔══╝        ██║     ██║   ██║██║  ██║██╔══╝  " COLOR_RESET "\n");
  printf("              " COLOR_BOLD COLOR_GREEN
         "██╗           ╚██████╗╚██████╔╝██████╔╝███████╗" COLOR_RESET "\n");
  printf("              " COLOR_BOLD COLOR_GREEN
         "╚═╝            ╚═════╝ ╚═════╝ ╚═════╝ ╚══════╝" COLOR_RESET "\n");
  printf("\n");
  printf("                 " COLOR_DIM COLOR_GREEN
         " C o d e   t h e   d r e a m" COLOR_RESET "\n");
  printf("\n");
  printf("\n");
}

void uiDrawHelp(void) {
  uiClear();
  printf(COLOR_BLUE BOX_TL);
  for (int i = 0; i < UI_TERM_WIDTH - 2; i++) {
    printf(BOX_H);
  }
  printf(BOX_TR COLOR_RESET "\n");

  uiDrawTitle("HUONG DAN SU DUNG HE THONG");
  uiDrawSeparator();

  uiDrawMenuRow("  GIOI THIEU");
  uiDrawMenuRow("  He thong Quan ly Vi pham CLB F-Code (VMS) ho tro theo doi");
  uiDrawMenuRow("  ky luat, vang hop, va tien phat cua cac thanh vien.");
  uiDrawMenuRow("");
  uiDrawMenuRow("  THAO TAC CO BAN");
  uiDrawMenuRow("  - Nhap phim so tuong ung va nhan Enter de chon menu.");
  uiDrawMenuRow("  - Nhap \"0\" va nhan Enter de QUAY LAI hoac HUY thao tac.");
  uiDrawMenuRow("  - Khi sua thong tin, nhan Enter de GIU NGUYEN gia tri cu.");
  uiDrawMenuRow("");
  uiDrawMenuRow("  QUY QUET PAGINATION (PHAN TRANG)");
  uiDrawMenuRow("  - n: Xem trang tiep theo");
  uiDrawMenuRow("  - p: Xem trang truoc do");
  uiDrawMenuRow("  - q: Thoat che do xem danh sach");
  uiDrawMenuRow("");
  uiDrawMenuRow("  THU TIEN PHAT (BAN CHU NHIEM)");
  uiDrawMenuRow(
      "  - Nhap so thu tu (STT) cach nhau boi dau phay (vd: 1,3,5) de thu.");
  uiDrawMenuRow(
      "  - Nhap \"99\" de thu toan bo cac vi pham cua thanh vien do.");

  printf(COLOR_BLUE BOX_BL);
  for (int i = 0; i < UI_TERM_WIDTH - 2; i++) {
    printf(BOX_H);
  }
  printf(BOX_BR COLOR_RESET "\n");
}
