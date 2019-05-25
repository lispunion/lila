#include <stdint.h>
#include <stdio.h>

#include "lila.h"

static void
print_value(uint32_t index);

void
print_value_string(uint32_t index)
{
    uint32_t a, n;

    a = value_a(index);
    n = value_d(index);
    for (; n; n--) {
        printf("%c", bytes[a++]);
    }
}

static void
print_list_items(uint32_t index)
{
    int first = 1;

    while (index) {
        if (first) {
            first = 0;
        } else {
            printf(" ");
        }
        print_value(value_a(index));
        index = value_d(index);
    }
}

static void
print_value(uint32_t index)
{
    switch (value_type(index)) {
    case TYPE_STRING:
        printf("\"");
        print_value_string(index);
        printf("\"");
        break;
    case TYPE_SYMBOL: print_value_string(index); break;
    case TYPE_PAIR:
        printf("(");
        print_list_items(index);
        printf(")");
        break;
    case TYPE_PAIR_SQUARE: break;
    case TYPE_PAIR_CURLY: break;
    }
}

void
print_all(void)
{
    uint32_t index;

    for (index = 0; index < nvalue; index++) {
        if (values[index] & TOPLEVEL) {
            print_value(index);
            printf("\n");
        }
    }
}
