//
// Created by Pars on 19.11.2026.
//

#include "cli.h"

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

static void echo_str(const struct line_editor *le, const char *s)
{
    while (*s != '\0') {
        le->out(*s);
        s++;
    }
}

// Escape-sequence parser. Returns true if c belongs to a sequence and was swallowed.
// A byte that breaks a sequence resets the state and is left for normal handling.
static bool esc_consume(struct line_editor *le, char c)
{
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
static bool handle_enter(struct line_editor *le, char c)
{
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

static void handle_cancel(struct line_editor *le)
{
    echo_str(le, "^C\r\n");
    le->line[0] = '\0';
    le->len = 0U;
}

static void handle_backspace(struct line_editor *le)
{
    if (le->len > 0U) {
        le->len--;
        echo_str(le, "\b \b");
    }
}

static void insert_char(struct line_editor *le, char c)
{
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

void le_init(struct line_editor *le, le_out_fn out)
{
    le->len = 0U;
    le->skip_next_lf = false;
    le->state = LE_ST_NORMAL;
    le->out = out;
    le->line[0] = '\0';
}

bool le_feed(struct line_editor *le, char c)
{
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
