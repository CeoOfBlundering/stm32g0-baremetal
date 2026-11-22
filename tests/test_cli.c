// Host-side unit tests for the CLI line editor (src/cli.c). PC only, not part of the firmware.
//
// Build and run from the repository root:
//   gcc -std=c11 -Wall -Wextra -Werror -Wshadow -Wconversion -Isrc src/cli.c tests/test_cli.c -o test_cli
//   ./test_cli
// Exit code is non-zero if any test fails.

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "cli.h"
static void out_ignore(char c) { (void)c; }

static int  lines_ready;
static char last_line[LE_LINE_MAX + 1U];

static void feed_str(struct line_editor *le, const char *s)
{
    for (; *s != '\0'; s++) {
        if (le_feed(le, *s)) {
            lines_ready++;
            strcpy(last_line, le->line);
        }
    }
}

static int failures;

static int bel_count;
static void out_count_bel(char c) { if (c == '\a') { bel_count++; } }

// Feed `n` copies of `c`, then the string `tail`. Returns number of BELs sent.
static int bels_for(char c, int n, const char *tail, char *line_out)
{
    struct line_editor le;
    le_init(&le, out_count_bel);
    bel_count = 0;
    line_out[0] = '\0';
    for (int i = 0; i < n; i++) { (void)le_feed(&le, c); }
    for (; *tail != '\0'; tail++) {
        if (le_feed(&le, *tail)) { strcpy(line_out, le.line); }
    }
    return bel_count;
}

static void check_bel(const char *name, int n, const char *tail, int want_bels, size_t want_len)
{
    char line[LE_LINE_MAX + 1U];
    int bels = bels_for('a', n, tail, line);
    bool ok = (bels == want_bels) && (strlen(line) == want_len);
    printf("%-28s %s  (bels=%d, line len=%zu)\n", name, ok ? "PASS" : "FAIL", bels, strlen(line));
    if (!ok) { failures++; }
}

static void check(const char *name, const char *input, int want_lines, const char *want_line)
{
    struct line_editor le;
    le_init(&le, out_ignore);
    lines_ready = 0;
    last_line[0] = '\0';
    feed_str(&le, input);
    bool ok = (lines_ready == want_lines) && (strcmp(last_line, want_line) == 0);
    printf("%-28s %s  (lines=%d, line=\"%s\")\n", name, ok ? "PASS" : "FAIL", lines_ready, last_line);
    if (!ok) { failures++; }
}

/* ---------------- tokenizer ---------------- */

// want[] lists the expected tokens; want_argc of them are checked.
static void check_tok(const char *name, const char *input, size_t max_args,
                      size_t want_argc, const char *const want[], bool want_trunc)
{
    char line[LE_LINE_MAX + 1U];
    char *argv[CLI_MAX_ARGS];
    bool truncated = true;   // tokenizer must overwrite this

    for (size_t i = 0U; i < CLI_MAX_ARGS; i++) {
        argv[i] = line;      // non-NULL junk, so the argv[argc] == NULL check means something
    }

    strcpy(line, input);
    size_t argc = cli_tokenize(line, argv, max_args, &truncated);

    bool ok = (argc == want_argc) && (truncated == want_trunc);
    for (size_t i = 0U; ok && (i < want_argc); i++) {
        ok = (strcmp(argv[i], want[i]) == 0);
    }
    if (ok && (argc < max_args)) {
        ok = (argv[argc] == NULL);
    }

    printf("%-28s %s  (argc=%zu, truncated=%d", name, ok ? "PASS" : "FAIL", argc, truncated);
    for (size_t i = 0U; (i < argc) && (i < max_args); i++) {
        printf(", argv[%zu]=\"%s\"", i, argv[i]);
    }
    printf(")\n");
    if (!ok) { failures++; }
}

int main(void)
{
    // Basic input, Enter, Backspace
    check("plain CR",             "help\r",            1, "help");
    check("CRLF",                 "help\r\n",          1, "help");
    check("backspace (DEL)",      "helpx\x7f\r",       1, "help");
    check("LF only, twice",       "a\nb\n",            2, "b");
    check("LF after CR+arrow",    "ab\r\x1b[A\n",       2, "");


    // Escape sequences (arrow keys, Delete, Ctrl+Arrow)
    check("arrow up after text",  "help\x1b[A\r",      1, "help");
    check("all four arrows",      "\x1b[A\x1b[B\x1b[C\x1b[Dls\r", 1, "ls");
    check("Delete key ESC[3~",    "ab\x1b[3~c\r",      1, "abc");
    check("Ctrl+Right ESC[1;5C",  "a\x1b[1;5Cb\r",     1, "ab");
    check("lone ESC then h",      "\x1bhi\r",          1, "hi");
    check("Enter inside CSI",     "ab\x1b[\r",         1, "ab");
    check("'A' after arrow kept", "\x1b[AAdd\r",       1, "Add");

    // Ctrl+C cancels the line
    check("Ctrl+C mid line",      "hel\x03",          1, "");
    check("Ctrl+C then new line", "abc\x03xy\r",      2, "xy");
    check("Ctrl+C on empty line", "\x03",             1, "");
    check("Ctrl+C inside CSI",    "ab\x1b[1\x03",     1, "");

    // BEL when the line is full
    check_bel("80 chars, no BEL",            80, "\r",          0, 80U);
    check_bel("81 chars, 1 BEL",             81, "\r",          1, 80U);
    check_bel("85 chars, 5 BEL",             85, "\r",          5, 80U);
    check_bel("full + DEL + 1 char, no BEL", 80, "\x7f" "b\r", 0, 80U);

    // Tokenizer (FAIL until cli_tokenize is implemented)
    check_tok("tok: one word",          "help",               4U, 1U, (const char *const[]){"help"}, false);
    check_tok("tok: two words",         "uart stats",         4U, 2U, (const char *const[]){"uart", "stats"}, false);
    check_tok("tok: empty line",        "",                   4U, 0U, NULL, false);
    check_tok("tok: only spaces",       "   ",                4U, 0U, NULL, false);
    check_tok("tok: extra spaces",      "  uart   stats  ",   4U, 2U, (const char *const[]){"uart", "stats"}, false);
    check_tok("tok: overflow keeps rest","a b c d",           2U, 2U, (const char *const[]){"a", "b c d"}, true);
    check_tok("tok: exactly max_args",  "a b",                2U, 2U, (const char *const[]){"a", "b"}, false);

    printf("\n%s (%d failing)\n", failures == 0 ? "ALL PASS" : "SOME FAIL", failures);
    return (failures == 0) ? 0 : 1;
}
