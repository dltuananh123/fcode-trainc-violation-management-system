#ifndef UI_H
#define UI_H

/**
 * @file ui.h
 * @brief TUI utilities using ANSI escape codes for colorful terminal output.
 *
 * Provides color macros, box drawing helpers, and screen layout functions
 * for building a beautiful CLI interface without external libraries.
 */

/* ============================================================
 * ANSI COLOR CODES
 * ============================================================ */

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[38;5;203m"
#define COLOR_GREEN "\033[38;5;114m"
#define COLOR_YELLOW "\033[38;5;180m"
#define COLOR_CYAN "\033[38;5;117m"
#define COLOR_BLUE "\033[38;5;111m"
#define COLOR_PURPLE "\033[38;5;183m"
#define COLOR_ORANGE "\033[38;5;215m"
#define COLOR_GRAY "\033[38;5;145m"
#define COLOR_BOLD "\033[1m"
#define COLOR_DIM "\033[2m"

/* Background colors */
#define BG_DARK "\033[48;5;235m"
#define BG_DARKER "\033[48;5;236m"

/* ============================================================
 * COLORED ERROR/SUCCESS/WARNING PREFIXES
 * ============================================================ */

#define ERR_LOI COLOR_RED "[LOI] " COLOR_RESET
#define ERR_CANH_BAO COLOR_YELLOW "[CANH BAO] " COLOR_RESET
#define ERR_OK COLOR_GREEN "[OK] " COLOR_RESET
#define ERR_INFO COLOR_CYAN "[INFO] " COLOR_RESET

/* ============================================================
 * LAYOUT CONSTANTS
 * ============================================================ */

#define UI_TERM_WIDTH 100
#define ROWS_PER_PAGE 15

#define MENU_CONTENT_W 68
#define DETAIL_CARD_W 70
#define LOGIN_BOX_W 42
#define TABLE_DEFAULT_W 100

/* ============================================================
 * BOX DRAWING CHARACTERS (UTF-8 encoded)
 * ============================================================ */

#define BOX_TL "\xE2\x95\x94" /* ╔ */
#define BOX_TR "\xE2\x95\x97" /* ╗ */
#define BOX_BL "\xE2\x95\x9A" /* ╚ */
#define BOX_BR "\xE2\x95\x9D" /* ╝ */
#define BOX_H "\xE2\x95\x90"  /* ═ */
#define BOX_V "\xE2\x95\x91"  /* ║ */

#define LINE_TL "\xE2\x94\x8C"      /* ┌ */
#define LINE_TR "\xE2\x94\x90"      /* ┐ */
#define LINE_BL "\xE2\x94\x94"      /* └ */
#define LINE_BR "\xE2\x94\x98"      /* ┘ */
#define LINE_H "\xE2\x94\x80"       /* ─ */
#define LINE_V "\xE2\x94\x82"       /* │ */
#define LINE_T_RIGHT "\xE2\x94\x9C" /* ├ */
#define LINE_T_LEFT "\xE2\x94\xA4"  /* ┤ */
#define LINE_T_DOWN "\xE2\x94\xAC"  /* ┬ */
#define LINE_T_UP "\xE2\x94\xB4"    /* ┴ */
#define LINE_CROSS "\xE2\x94\xBC"   /* ┼ */
#define LINE_TL2 "\xE2\x95\x94"     /* ╔ (double) */
#define LINE_V2 "\xE2\x95\x91"      /* ║ (double) */

/* ============================================================
 * UI FUNCTION DECLARATIONS
 * ============================================================ */

/**
 * @brief Initialize UI subsystem (enable ANSI on Windows).
 */
void uiInit(void);

/**
 * @brief Clear the terminal screen.
 */
void uiClear(void);

/**
 * @brief Draw a double-line box at the specified position.
 * @param width  Width of the box (in characters).
 * @param height Height of the box (in rows).
 */
void uiDrawBox(int width, int height);

/**
 * @brief Draw a centered title inside a double-line box.
 * @param title The title text to display.
 */
void uiDrawTitle(const char *title);

/**
 * @brief Draw the top status bar with time, user, and app name.
 * @param user The current logged-in user MSSV.
 * @param role The current user role string (e.g., "BCN", "Member").
 */
void uiDrawStatusBar(const char *user, const char *role);

/**
 * @brief Draw a breadcrumb navigation path.
 * @param path The navigation path (e.g., "Trang chu > Quan ly vi pham").
 */
void uiDrawBreadcrumb(const char *path);

/**
 * @brief Draw a horizontal separator line across the terminal.
 */
void uiDrawSeparator(void);

/**
 * @brief Draw a single menu row inside the box (left-aligned, padded to full
 * width).
 * @param text The menu item text (must NOT include leading/trailing spaces
 * padding).
 */
void uiDrawMenuRow(const char *text);

/**
 * @brief Formatted version of uiDrawMenuRow.
 * @param fmt printf-style format string.
 */
void uiDrawMenuRowFmt(const char *fmt, ...);

/**
 * @brief Wait for Enter key press ("Press any key to continue").
 */
void uiPause(void);

/**
 * @brief Draw a single-line box (for sub-panels).
 * @param width  Width of the box.
 * @param height Height of the box.
 */
void uiDrawPanel(int width, int height);

/**
 * @brief Print colored text.
 * @param text  The text to print.
 * @param color The ANSI color code (e.g., COLOR_RED).
 */
void uiPrintColored(const char *text, const char *color);

/**
 * @brief Print a centered line of text with left/right padding.
 * @param text  The text to center.
 * @param width The total width to center within.
 */
void uiPrintCentered(const char *text, int width);

/**
 * @brief Get the current time as a formatted string.
 * @param buffer  Output buffer for the formatted time string.
 * @param bufSize Size of the output buffer.
 */
void uiGetCurrentTime(char *buffer, int bufSize);

/**
 * @brief Draw the FCODE Firebird ASCII art logo on startup screen.
 * Displays a colorful firebird phoenix with "FCODE" text and "Code the dream"
 * subtitle.
 */
void uiDrawLogo(void);

#endif /* UI_H */
