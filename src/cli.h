//
// Created by Pars on 19.11.2026.
//

#ifndef STM32_BAREMETAL_CLI_H
#define STM32_BAREMETAL_CLI_H

#include <stdbool.h>
#include <stddef.h>

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

#endif //STM32_BAREMETAL_CLI_H
