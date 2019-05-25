#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FLAG_DOUBLE_DASH 1

struct token {
    int type;
    char *string;
};

struct implementation {
    const char *name;
    const char *command;
    const char *imageflag;
    const char *scriptflag;
    size_t flags;
};

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
    .flags = FLAG_DOUBLE_DASH,
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

static const struct implementation gauche = {
    .name = "gauche",
    .command = "gosh",
    .flags = FLAG_DOUBLE_DASH,
};

static const struct implementation guile = {
    .name = "guile",
    .command = "guile",
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

static const struct implementation sbcl = {
    .name = "sbcl",
    .command = "sbcl",
    .imageflag = "--core",
    .scriptflag = "--script",
    .flags = 0,
};

static const struct implementation *implementations[] = {
    &ccl,
    &chez,
    &chibi,
    &chicken,
    &clisp,
    &cljsc,
    &clojure,
    &gauche,
    &guile,
    &lumo,
    &newlisp,
    &planck,
    &sbcl,
};

static const struct implementation *implementation;
static const char progname[] = "lila";
static char *script;
static char buf[4096];
static size_t mark;
static size_t pos;
static size_t len;

static void
usage(void)
{
    fprintf(stderr, "usage\n");
    exit(64);
}

static void
die(const char *msg)
{
    fprintf(stderr, "%s: error: %s\n", progname, msg);
    exit(50);
}

static void
diemem(void)
{
    die("out of memory");
}

static void
diesys(const char *msg)
{
    fprintf(stderr, "%s: error: %s: %s\n", progname, msg, strerror(errno));
    exit(50);
}

struct token *
token_from_type(int type)
{
    struct token *tok;

    if (!(tok = calloc(1, sizeof(*tok)))) {
        diemem();
    }
    tok->type = type;
    // fprintf(stderr, "token %c\n", type);
    return tok;
}

struct token *
token_from_mark_to_pos(int type)
{
    struct token *tok;

    tok = token_from_type(type);
    if (!(tok->string = calloc(1, pos - mark + 1))) {
        diemem();
    }
    memcpy(tok->string, buf + mark, pos - mark);
    // fprintf(stderr, "string %s\n", tok->string);
    return tok;
}

static void
slurp_fd(int fd)
{
    ssize_t nr;

    for (;;) {
        nr = read(fd, buf, sizeof(buf));
        if (nr != (ssize_t)-1) {
            break;
        }
        if (errno != EINTR) {
            diesys("cannot read from file");
        }
    }
    len = (size_t)nr;
}

static void
slurp_file(void)
{
    int fd;

    fd = open(script, O_RDONLY);
    if (fd == -1) {
        diesys("cannot open file");
    }
    slurp_fd(fd);
    if (close(fd) == -1) {
        diesys("cannot close file");
    }
}

static int
peek_ascii_char(void)
{
    return (pos < len) ? buf[pos] : -1;
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
        pos++;
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
        pos++;
    }
    return c;
}

static int
whitespace_char_p(int c)
{
    return ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'));
}

const char symbol_chars[] = "0123456789"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "abcdefghijklmnopqrstuvwxyz"
                            "&#.|!" // TODO
                            "-_";

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

static struct token *
read_symbol(int c)
{
    mark = pos - 1;
    while ((c = read_ascii_char_if(symbol_char_p))) {
    }
    return token_from_mark_to_pos('s');
}

static struct token *
read_string(void)
{
    struct token *tok;

    mark = pos;
    while (!read_the_ascii_char('"')) {
        read_the_ascii_char('\\');
        if (!read_ascii_char_if(horizontal_char_p)) {
            die("bad char in string");
        }
    }
    pos--;
    tok = token_from_mark_to_pos('"');
    pos++;
    return tok;
}

static struct token *
read_token(void)
{
    int c;

    skip_whitespace_and_comments();
    c = read_the_ascii_char(-1);
    if (c) {
        return 0;
    }
    c = read_the_ascii_char('(');
    if (c) {
        return token_from_type(c);
    }
    c = read_the_ascii_char(')');
    if (c) {
        return token_from_type(c);
    }
    c = read_the_ascii_char('"');
    if (c) {
        return read_string();
    }
    c = read_ascii_char_if(symbol_char_p);
    if (c) {
        return read_symbol(c);
    }
    die("syntax error");
    return 0;
}

static void
skip_shebang_line(void)
{
    if (!read_the_ascii_char('#'))
        return;
    if (!read_the_ascii_char('!'))
        return;
    skip_rest_of_line();
}

static void
skip_rest_of_list(struct token *tok)
{
    for (;;) {
        if (!tok) {
            die("unterminated list");
        }
        if (tok->type == '(') {
            skip_rest_of_list(read_token());
        }
        if (tok->type == ')') {
            break;
        }
        tok = read_token();
    }
}

static int
try_implementation(const char *name)
{
    const struct implementation **impls;
    const struct implementation *impl;

    for (impls = implementations; (impl = *impls); impls++) {
        if (!strcmp(impl->name, name)) {
            implementation = impl;
            return 1;
        }
    }
    return 0;
}

static void
read_implementation(void)
{
    struct token *tok;

    for (;;) {
        tok = read_token();
        if (tok->type == ')') {
            die("no supported implementation is listed");
            break;
        } else if (tok->type == 's') {
            if (try_implementation(tok->string)) {
                skip_rest_of_list(read_token());
                break;
            }
        } else {
            die("weird stuff in implementation list");
        }
    }
}

static void
read_declare_file_list(void)
{
    struct token *tok;

    tok = read_token();
    if (tok->type == 's') {
        if (!strcmp(tok->string, "implementation")) {
            read_implementation();
            return;
        }
    }
    skip_rest_of_list(tok);
}

static void
read_declare_file(void)
{
    struct token *tok;

    for (;;) {
        tok = read_token();
        if (tok->type == '(') {
            read_declare_file_list();
        } else if (tok->type == ')') {
            break;
        } else {
            die("weird stuff right under declare-file");
        }
    }
}

static int
read_toplevel_list(void)
{
    struct token *tok;

    tok = read_token();
    if (tok->type == 's') {
        if (!strcmp(tok->string, "declare-file")) {
            read_declare_file();
            return 1;
        }
    }
    skip_rest_of_list(tok);
    return 0;
}

static void
parse(void)
{
    struct token *tok;

    skip_shebang_line();
    for (;;) {
        tok = read_token();
        if (!tok) {
            break;
        }
        if (tok->type == '(') {
            read_toplevel_list();
        } else if (tok->type == ')') {
            die("Stray closing paren");
        }
    }
}

static void
run(void)
{
    const char *argv[5];
    const char **ins;

    ins = argv;
    *ins++ = implementation->command;
    if (implementation->scriptflag) {
        *ins++ = implementation->scriptflag;
        *ins++ = script;
    }
    if (implementation->flags & FLAG_DOUBLE_DASH) {
        *ins++ = "--";
    }
    if (!implementation->scriptflag) {
        *ins++ = script;
    }
    *ins++ = 0;
    // fprintf(stderr, "running %s\n", implementation->command);
    execvp(argv[0], (char **)argv);
    diesys("cannot exec interpreter");
    exit(126);
}

int
main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
    }
    script = argv[1];
    // printf("%s\n", script);
    slurp_file();
    parse();
    if (!implementation) {
        die("no suitable implementation found");
    }
    run();
    return 0;
}
