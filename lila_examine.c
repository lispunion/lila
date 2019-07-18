#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lila.h"

static const struct implementation *
examine_implementation(const char *name)
{
    const struct implementation **impls;
    const struct implementation *impl;

    if (verbosity >= 2) {
        fprintf(stderr, "%s: file lists implementation %s\n", progname, name);
    }
    for (impls = implementations; (impl = *impls); impls++) {
        if (!strcmp(impl->name, name)) {
            return impl;
        }
    }
    return 0;
}

static void
examine_implementation_list(uint32_t list)
{
    const struct implementation *impl;

    for (; list; list = value_d(list)) {
        if (value_type(value_a(list)) == TYPE_SYMBOL) {
            impl = examine_implementation(bytes + value_a(value_a(list)));
            if (impl && !implementation) {
                implementation = impl;
                if (verbosity >= 2) {
                    fprintf(
                        stderr, "%s: choosing that implementation\n", progname);
                }
            }
        }
    }
}

static void
examine_declare_file_sublist(uint32_t list)
{
    if (value_the_symbol_p(value_a(list), "implementations")) {
        examine_implementation_list(value_d(list));
    }
}

static void
examine_declare_file(uint32_t list)
{
    for (; list; list = value_d(list)) {
        if (value_type(value_a(list)) == TYPE_PAIR) {
            examine_declare_file_sublist(value_a(list));
        }
    }
}

void
examine_all(void)
{
    uint32_t index;

    for (index = 0; index < nvalue; index++) {
        if (values[index] & TOPLEVEL) {
            if (value_type(index) == TYPE_PAIR) {
                if (value_the_symbol_p(value_a(index), "declare-file")) {
                    examine_declare_file(value_d(index));
                }
            }
        }
    }
}
