//
// Created by Pars on 19.11.2026.
//

#include "cli.h"

#define CH_BS         0x08   // Backspace
#define CH_DEL        0x7F   // Delete (sent by most terminals for Backspace)
#define CH_PRINT_MIN  0x20   // ' ', first printable ASCII
#define CH_PRINT_MAX  0x7E   // '~', last printable ASCII

void le_init(le_t *le, le_out_fn out)
{
    // Initialize the fields
    le->len = 0U;
    le->last_was_cr = false;
    le->out = out;
    le->line[0] = '\0';
}

bool le_feed(le_t *le, char c)
{
    bool ready = false;
    if ((c == '\r') || (c == '\n')) {
        if ((c == '\n') && le->last_was_cr) {
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
    else if(((c >= CH_PRINT_MIN) && (c <= CH_PRINT_MAX)) && (le->len < LE_LINE_MAX)) {
        le->line[le->len] = c;
        le->len++;
        le->out(c);
    }
    else if (((c == CH_BS) || (c == CH_DEL)) && (le->len > 0U)){
        le->len--;
        le->out('\b');
        le->out(' ');
        le->out('\b');
    }
    else {
        // ignore other control chars
    }

    le->last_was_cr = (c == '\r');
    return ready;
}