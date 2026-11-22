//
// Created by Pars on 19.11.2026.
//

#include "cli.h"
#include <string.h>

#define CH_ETX          0x03   // Ctrl+C (End of Text)
#define CH_BEL          0x07   // Bell
#define CH_BS           0x08   // Backspace
#define CH_ESC          0x1B   // Escape
#define CH_DEL          0x7F   // Delete (sent by most terminals for Backspace)
#define CH_PRINT_MIN    0x20   // ' ', first printable ASCII
#define CH_PRINT_MAX    0x7E   // '~', last printable ASCII
#define CSI_START       0x5B   // '[': ESC '[' starts a CSI (Control Sequence Introducer)
#define CSI_PARAM_MIN   0x30   // '0'
#define CSI_PARAM_MAX   0x3F   // '?'
#define CSI_FINAL_MIN   0x40   // '@'
#define CSI_FINAL_MAX   0x7E   // '~'
#define U32_MAX_DIGITS  10U    //  "4294967295"

static void echo_str(const struct line_editor *le, const char *s) {
    while (*s != '\0') {
        le->out(*s);
        s++;
    }
}

// Escape-sequence parser. Returns true if c belongs to a sequence and was swallowed.
// A byte that breaks a sequence resets the state and is left for normal handling.
static bool esc_consume(struct line_editor *le, char c) {
    bool consumed = false;

    if (le->state == LE_ST_ESC) {
        if (c == CSI_START) {
            le->state = LE_ST_CSI;
            consumed = true;
        }
        else {
            le->state = LE_ST_NORMAL;
        }
    }
    else if (le->state == LE_ST_CSI) {
        if ((c >= CSI_PARAM_MIN) && (c <= CSI_PARAM_MAX)) {
            consumed = true;
        }
        else if ((c >= CSI_FINAL_MIN) && (c <= CSI_FINAL_MAX)) {
            le->state = LE_ST_NORMAL;
            consumed = true;
        }
        else {
            le->state = LE_ST_NORMAL;
        }
    }
    else {
        // LE_ST_NORMAL: not inside a sequence
    }

    return consumed;
}

// Returns true if a line was finished ('\n' right after '\r' is the rest of a CRLF).
static bool handle_enter(struct line_editor *le, char c) {
    bool ready = false;

    if ((c == '\n') && le->skip_next_lf) {
        // '\n' of a "\r\n" pair: line already ended on '\r'
    }
    else {
        echo_str(le, "\r\n");
        le->line[le->len] = '\0';
        le->len = 0U;
        ready = true;
    }

    return ready;
}

static void handle_cancel(struct line_editor *le) {
    echo_str(le, "^C\r\n");
    le->line[0] = '\0';
    le->len = 0U;
}

static void handle_backspace(struct line_editor *le) {
    if (le->len > 0U) {
        le->len--;
        echo_str(le, "\b \b");
    }
}

static void insert_char(struct line_editor *le, char c) {
    if ((c >= CH_PRINT_MIN) && (c <= CH_PRINT_MAX)) {
        if (le->len < LE_LINE_MAX) {
            le->line[le->len] = c;
            le->len++;
            le->out(c);
        }
        else {
            le->out(CH_BEL);   // line is full
        }
    }
    // other control chars are ignored
}

void le_init(struct line_editor *le, le_out_fn out) {
    le->len = 0U;
    le->skip_next_lf = false;
    le->state = LE_ST_NORMAL;
    le->out = out;
    le->line[0] = '\0';
}

bool le_feed(struct line_editor *le, char c) {
    bool ready = false;   // true when a finished line is in le->line

    if (!esc_consume(le, c)) {
        switch (c) {
        case CH_ESC:
            le->state = LE_ST_ESC;
            break;
        case CH_ETX:
            handle_cancel(le);
            ready = true;   // empty line: main prints a new prompt
            break;
        case '\r':
        case '\n':
            ready = handle_enter(le, c);
            break;
        case CH_BS:
        case CH_DEL:
            handle_backspace(le);
            break;
        default:
            insert_char(le, c);
            break;
        }
    }

    le->skip_next_lf = (c == '\r');
    return ready;
}

size_t cli_tokenize(char *line, char *argv[], size_t max_args, bool *truncated) {
    size_t argc = 0U;
    *truncated = false;
    bool save_next = true;

    for (size_t i = 0U; line[i] != '\0'; i++) {
        if ((line[i] != ' ') && save_next) {
            if (argc < max_args) {
                argv[argc] = &line[i];
                argc++;
                save_next = false;
            }
            else {
                *truncated = true;
            }
        }
        else if (line[i] == ' ') {
            save_next = true;
            if (argc != max_args) {
                line[i] = '\0';
            }
        }
        else {
            // Inside a word but already saved first character, nothing to do.
        }
    }

    if (argc < max_args) {
        argv[argc] = NULL;
    }
    return argc;
}

/* ---------------- commands ---------------- */

static le_out_fn cli_out;   // set by cli_init

void cli_init(le_out_fn out) {
    cli_out = out;
}

void cli_print(const char *s) {
    if (cli_out != NULL) {
        while (*s != '\0') {
            cli_out(*s);
            s++;
        }
    }
}

void cli_print_u32(uint32_t value) {
    char buf[U32_MAX_DIGITS + 1U];
    buf[U32_MAX_DIGITS] = '\0';
    size_t i = U32_MAX_DIGITS;
    size_t digit;
    do {   // runs at least once, so 0 prints "0"
        i--;
        digit = value % 10U;
        buf[i] = (char)('0' + digit);
        value = value / 10U;
    } while (value != 0U);

    cli_print(&buf[i]);
}

// Returns the matching entry, or NULL if name is not in the table.
static const struct cli_command *find_command(const struct cli_command *table, size_t n, const char *name) {
    const struct cli_command *result = NULL;
    for (size_t k = 0U; (k < n) && (result == NULL); k++) {
        if (strcmp(table[k].name, name) == 0) {
            result = &table[k];
        }
    }

    return result;
}

void cli_dispatch(const struct cli_command *table, size_t n,
                  size_t argc, char *argv[], bool truncated) {
    if (argc != 0U) {
        if (truncated) {
            cli_print("too many arguments\r\n");
        }
        else {
            const struct cli_command *cmd = find_command(table, n, argv[0]);
            if (cmd != NULL) {
                cmd->fn(argc, argv);
            }
            else {
                cli_print("unknown command: ");
                cli_print(argv[0]);
                cli_print("\r\n");
            }
        }
    }
}
