//
// Created by Pars on 19.11.2026.
//

#ifndef STM32_BAREMETAL_CLI_H
#define STM32_BAREMETAL_CLI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define LE_LINE_MAX 80U   // max characters per line; extra input is ignored

typedef void (*le_out_fn)(char c);   // echo callback: "send one character"

enum le_state {
    LE_ST_NORMAL,   // plain input
    LE_ST_ESC,      // got ESC, waiting for '['
    LE_ST_CSI       // inside "ESC [": swallow parameters until the final byte
};

struct line_editor {
    char          line[LE_LINE_MAX + 1U]; // +1 for the '\0' terminator
    size_t        len;                    // chars currently in line
    bool          skip_next_lf;           // true right after '\r': ignore a following '\n' (CRLF)
    enum le_state state;                  // escape-sequence parser state
    le_out_fn     out;                    // where echo goes
};

void le_init(struct line_editor *le, le_out_fn out);
bool le_feed(struct line_editor *le, char c);   // true when a full line is ready in le->line
#define CLI_MAX_ARGS 8U   // max tokens per command line, command name included

// Splits line in place: separators become '\0', argv[i] points to each token.
// Returns argc. If there are more than max_args tokens, the last argv entry holds the
// rest of the line as typed and *truncated is set. argv[argc] = NULL when argc < max_args.
size_t cli_tokenize(char *line, char *argv[], size_t max_args, bool *truncated);
/* ---------------- commands ---------------- */

typedef void (*cli_cmd_fn)(size_t argc, char *argv[]);

struct cli_command {
    const char *name;   // argv[0] that selects this command, e.g. "uart"
    cli_cmd_fn  fn;     // called with the full argc/argv (argv[0] is the name)
    const char *help;   // one line for the help command
};

void cli_init(le_out_fn out);     // where cli_print output goes
void cli_print(const char *s);
void cli_print_u32(uint32_t value);   // decimal digits only, no newline
void cli_print_hex8(uint8_t value);   // "0x" + two uppercase hex digits, no newline

// Finds argv[0] in table[0..n-1] and calls its fn. Prints an error for an unknown
// command or a truncated line; prints nothing for an empty line (argc == 0).
void cli_dispatch(const struct cli_command *table, size_t n,
                  size_t argc, char *argv[], bool truncated);

#endif //STM32_BAREMETAL_CLI_H
