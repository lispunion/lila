#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct token {
    int type;
    char *string;
};

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
    return tok;
}

struct token *
token_from_mark_to_pos(int type)
{
    struct token *tok;

    if (!(tok = calloc(1, sizeof(*tok)))) {
        diemem();
    }
    tok->type = type;
    if (!(tok->string = calloc(1, pos - mark + 1))) {
        diemem();
    }
    memcpy(tok->string, buf + mark, pos - mark);
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
skip_whitespace(void)
{
    while (read_ascii_char_if(whitespace_char_p))
        ;
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

    skip_whitespace();
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
skip_rest_of_line(void)
{
    while (read_ascii_char_if(horizontal_char_p))
        ;
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
parse(void)
{
    struct token *tok;

    skip_shebang_line();
    while ((tok = read_token())) {
        switch (tok->type) {
        case '(': printf("\nlist starts\n"); break;
        case ')': printf("list ends\n\n"); break;
        case '"': printf("string %s\n", tok->string); break;
        case 's': printf("symbol %s\n", tok->string); break;
        default: printf("???\n"); break;
        }
    }
}

int
main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
    }
    script = argv[1];
    printf("%s\n", script);
    slurp_file();
    parse();
    return 0;
}
