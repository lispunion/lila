#include <stdint.h>
#include <stdlib.h>

#include "lila.h"

static const struct implementation ccl = {
    .name = "ccl",
    .command = "ccl64",
    .imageflag = "--image-name",
    .scriptflag = "--load",
    .flags = FLAG_DOUBLE_DASH,
};

static const struct implementation chez = {
    .name = "chez",
    .command = "chez",
    .scriptflag = "--program",
    .flags = FLAG_DOUBLE_DASH,
};

static const struct implementation chibi = {
    .name = "chibi",
    .command = "chibi-scheme",
    .imageflag = "-i",
    .flags = FLAG_DOUBLE_DASH,
};

static const struct implementation chicken = {
    .name = "chicken",
    .command = "csi",
};

static const struct implementation clisp = {
    .name = "clisp",
    .command = "clisp",
};

static const struct implementation cljsc = {
    .name = "cljsc",
    .command = "cljsc",
};

static const struct implementation clojure = {
    .name = "clojure",
    .command = "clojure",
    .flags = 0,
};

static const struct implementation emacs = {
    .name = "emacs",
    .command = "emacs",
    .scriptflag = "--script",
};

static const struct implementation gauche = {
    .name = "gauche",
    .command = "gosh",
    .flags = FLAG_DOUBLE_DASH,
};

static const struct implementation guile = {
    .name = "guile",
    .command = "guile",
    .scriptflag = "--no-auto-compile",
};

static const struct implementation lumo = {
    .name = "lumo",
    .command = "lumo",
    .flags = FLAG_DOUBLE_DASH,
};

static const struct implementation newlisp = {
    .name = "newlisp",
    .command = "newlisp",
    .flags = FLAG_DOUBLE_DASH,
};

static const struct implementation planck = {
    .name = "planck",
    .command = "planck",
    .flags = FLAG_DOUBLE_DASH,
};

static const struct implementation racket = {
    .name = "racket",
    .command = "racket",
    .flags = FLAG_DOUBLE_DASH,
};

static const struct implementation sbcl = {
    .name = "sbcl",
    .command = "sbcl",
    .imageflag = "--core",
    .scriptflag = "--script",
    .flags = 0,
};

const struct implementation *implementations[] = {
    &ccl,
    &chez,
    &chibi,
    &chicken,
    &clisp,
    &cljsc,
    &clojure,
    &emacs,
    &gauche,
    &guile,
    &lumo,
    &newlisp,
    &planck,
    &racket,
    &sbcl,
};

const struct implementation *implementation;
const char progname[] = "lila";
char *script;

// S-expressions are read using two pages of static memory.
// The first page is up to 4096 bytes from the start of the source file.
// The second page stores up to 1024 32-bit values parsed from source.
// A parsed value is 4-bit type tag (lowest bits), then 14-bit start, then
// 14-bit limit (highest bits)

char bytes[4096];
uint32_t values[MAXVALUES];
size_t nvalue = 1; // First value is null.
size_t nbyte;
size_t bytepos;
unsigned int verbosity = 3; // 0=quiet, 1=normal, 2=-v, 3=-vv
