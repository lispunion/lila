#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lila.h"

static uint32_t
read_value(void);

static uint32_t
make_value(uint32_t flags, uint32_t a, uint32_t d)
{
    if (vflags >= 2) {
        fprintf(stderr, "%s: value %3d := %d %3d %3d\n", progname, (int)nvalue,
            (int)flags, (int)a, (int)d);
    }
    if (nvalue >= MAXVALUES) {
        die("too many values");
    }
    values[nvalue] = flags | (a << 4) | (d << 18);
    return nvalue++;
}

static void
set_value_flag(uint32_t index, uint32_t flag)
{
    values[index] |= flag;
}

static void
set_value_d(uint32_t index, uint32_t d)
{
    values[index] = values[index] | (d << 18);
}

uint32_t
value_type(uint32_t index)
{
    return values[index] & 7;
}

uint32_t
value_a(uint32_t index)
{
    return values[index] >> 4 & ((1 << 14) - 1);
}

uint32_t
value_d(uint32_t index)
{
    return values[index] >> 18 & ((1 << 14) - 1);
}

int
value_the_symbol_p(uint32_t index, const char *name)
{
    if (value_type(index) != TYPE_SYMBOL)
        return 0;
    return !strncmp(name, bytes + value_a(index), value_d(index));
}

static int
peek_ascii_char(void)
{
    return (bytepos < nbyte) ? bytes[bytepos] : -1;
}

static int
read_the_ascii_char(int k)
{
    int c;

    c = peek_ascii_char();
    if (!c) {
        return 0;
    }
    if (c != k) {
        return 0;
    }
    if (c != -1) {
        bytepos++;
    }
    return c;
}

static int
read_ascii_char_if(int (*predicate)(int c))
{
    int c;

    c = peek_ascii_char();
    if (!c) {
        return 0;
    }
    if (!predicate(c)) {
        return 0;
    }
    if (c != -1) {
        bytepos++;
    }
    return c;
}

static int
whitespace_char_p(int c)
{
    return ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'));
}

static const char symbol_chars[] = "0123456789"
                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz"
                                   "&#.:/|!" // TODO
                                   "+-_";

static int
symbol_char_p(int c)
{
    return !!strchr(symbol_chars, c);
}

static int
horizontal_char_p(int c)
{
    return (c >= 0x20) && (c <= 0x7e);
}

static void
skip_rest_of_line(void)
{
    while (read_ascii_char_if(horizontal_char_p))
        ;
}

static void
skip_whitespace_and_comments(void)
{
    for (;;) {
        if (read_ascii_char_if(whitespace_char_p)) {
            continue;
        }
        if (read_the_ascii_char(';')) {
            skip_rest_of_line();
            continue;
        }
        break;
    }
}

static uint32_t
read_symbol(void)
{
    uint32_t start, limit;

    start = bytepos;
    while (read_ascii_char_if(symbol_char_p))
        ;
    limit = bytepos;
    if (start == limit) {
        return 0;
    }
    return make_value(TYPE_SYMBOL, start, limit - start);
}

static uint32_t
read_string(void)
{
    uint32_t start, limit;

    start = limit = bytepos;
    while (!read_the_ascii_char('"')) {
        read_the_ascii_char('\\');
        if (!read_ascii_char_if(horizontal_char_p)) {
            die("bad char in string");
        }
        limit = bytepos;
    }
    return make_value(TYPE_STRING, start, limit - start);
}

static uint32_t
read_list(int end_char, uint32_t flags)
{
    uint32_t list, last_pair, pair, elt;

    list = last_pair = 0;
    for (;;) {
        skip_whitespace_and_comments();
        if (read_the_ascii_char(end_char)) {
            break;
        }
        if ((elt = read_value()) == MAXVALUES) {
            die("unterminated list");
        }
        pair = make_value(flags, elt, 0);
        if (!list) {
            list = pair;
        } else {
            set_value_d(last_pair, pair);
        }
        last_pair = pair;
    }
    return list;
}

static uint32_t
read_value(void)
{
    uint32_t value;

    skip_whitespace_and_comments();
    if (read_the_ascii_char(-1)) {
        return MAXVALUES;
    }
    if (read_the_ascii_char('(')) {
        return read_list(')', TYPE_PAIR);
    }
    if (read_the_ascii_char('[')) {
        return read_list(']', TYPE_PAIR_SQUARE);
    }
    if (read_the_ascii_char('{')) {
        return read_list('}', TYPE_PAIR_CURLY);
    }
    if (read_the_ascii_char('"')) {
        return read_string();
    }
    if (!(value = read_symbol())) {
        die("syntax error");
    }
    return value;
}

static void
skip_shebang_line(void)
{
    if (!read_the_ascii_char('#')) {
        return;
    }
    if (!read_the_ascii_char('!')) {
        return;
    }
    skip_rest_of_line();
}

static void
null_terminate_strings(void)
{
    uint32_t index;

    for (index = 0; index < nvalue; index++) {
        if ((value_type(index) == TYPE_SYMBOL) ||
            (value_type(index) == TYPE_STRING)) {
            bytes[value_a(index) + value_d(index)] = 0;
        }
    }
}

void
read_all(void)
{
    uint32_t index;

    skip_shebang_line();
    while ((index = read_value()) != MAXVALUES) {
        set_value_flag(index, TOPLEVEL);
    }
    null_terminate_strings();
}
