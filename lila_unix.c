#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lila.h"

static void
usage(void)
{
    fprintf(stderr, "usage\n");
    exit(64);
}

void
die(const char *msg)
{
    fprintf(stderr, "%s: error: %s\n", progname, msg);
    exit(50);
}

static void
diesys(const char *msg)
{
    fprintf(stderr, "%s: error: %s: %s\n", progname, msg, strerror(errno));
    exit(50);
}

static void
slurp_fd(int fd)
{
    ssize_t nr;

    for (;;) {
        nr = read(fd, bytes, sizeof(bytes));
        if (nr != (ssize_t)-1) {
            break;
        }
        if (errno != EINTR) {
            diesys("cannot read from file");
        }
    }
    nbyte = (size_t)nr;
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

static void
run(void)
{
    const char *argv[5];
    const char **ins;

    if (!implementation) {
        die("no suitable implementation found");
    }
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
    if (vflags >= 1) {
        for (ins = argv; *ins; ins++) {
            fprintf(stderr, "%s: arg: %s\n", progname, *ins);
        }
    }
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
    if (vflags >= 1) {
        fprintf(stderr, "%s: script: %s\n", progname, script);
    }
    slurp_file();
    read_all();
    if (vflags >= 1) {
        print_all();
    }
    examine_all();
    run();
    return 0;
}
