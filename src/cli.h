//
// Created by Pars on 19.11.2026.
//

#ifndef STM32_BAREMETAL_CLI_H
#define STM32_BAREMETAL_CLI_H

#include <stdbool.h>
#include <stddef.h>

#define LE_LINE_MAX 80U   // max characters per line; extra input is ignored

typedef void (*le_out_fn)(char c);   // echo callback: "send one character"

typedef struct {
    char      line[LE_LINE_MAX + 1U]; // +1 for the '\0' terminator
    bool last_was_cr;
    size_t    len;                    // chars currently in line
    le_out_fn out;                    // where echo goes
} le_t;

void le_init(le_t *le, le_out_fn out);
bool le_feed(le_t *le, char c);       // true when a full line is ready in le->line

#endif //STM32_BAREMETAL_CLI_H
