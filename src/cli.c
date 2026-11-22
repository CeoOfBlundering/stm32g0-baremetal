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
    bool ready = false;     // true when a finished line is in le->line
    bool consumed = false;  // true when the escape-sequence parser swallowed c

    if (le->state == LE_ST_ESC) {
        if (c == CSI_START) {
            le->state = LE_ST_CSI;
            consumed = true;
        }
        else {
            le->state = LE_ST_NORMAL;   // not a CSI: handle c as normal input
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
            le->state = LE_ST_NORMAL;   // malformed sequence: handle c as normal input
        }
    }
    else {
        // LE_ST_NORMAL: nothing to do before the chain below
    }

    if (!consumed) {
        if (c == CH_ESC) {
            le->state = LE_ST_ESC;
        }
        else if (c == CH_ETX) {
            le->out('^');
            le->out('C');
            le->out('\r');
            le->out('\n');
            le->line[0] = '\0';
            le->len = 0U;
            ready = true;
        }
        else if ((c == '\r') || (c == '\n')) {
            if ((c == '\n') && le->skip_next_lf) {
                // '\n' of a "\r\n" pair: line already ended on '\r'
            }
            else {
                le->out('\r');
                le->out('\n');
                le->line[le->len] = '\0';
                le->len = 0U;
                ready = true;
            }
        }
        else if ((c >= CH_PRINT_MIN) && (c <= CH_PRINT_MAX)) {
            if (le->len < LE_LINE_MAX) {
                le->line[le->len] = c;
                le->len++;
                le->out(c);
            }
            else {
                le->out(CH_BEL);   // line is full
            }
        }
        else if (((c == CH_BS) || (c == CH_DEL)) && (le->len > 0U)) {
            le->len--;
            le->out('\b');
            le->out(' ');
            le->out('\b');
        }
        else {
            // ignore other control chars
        }
    }

    le->skip_next_lf = (c == '\r');
    return ready;
}
