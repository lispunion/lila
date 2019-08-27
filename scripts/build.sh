#!/bin/sh
set -eu
os="$(uname | tr A-Z- a-z_)"
default_cc=gcc
default_cflags="-Og -g -Wall -Werror -Wextra -pedantic -std=gnu99"
default_lflags="-static"
case "$(uname)" in
Darwin) default_cc=clang ;;
DragonFly) ;;
FreeBSD) default_cc=clang ;;
Haiku) default_cflags="-g -Wall -Werror" ;;
Linux) default_lflags="" ;;
NetBSD) default_lflags="" ;;
OpenBSD) default_cc=clang ;;
SunOS) ;;
*)
    echo "Unknown operating system: $os" >&2
    exit 1
    ;;
esac
CC="${CC:-$default_cc}"
CFLAGS="${CFLAGS:-$default_cflags}"
LFLAGS="${LFLAGS:-$default_lflags}"
builddir="build-$os-$(uname -m | tr A-Z- a-z_)-$(basename "$CC")"
cd "$(dirname "$0")"/..
mkdir -p "$builddir"
cd "$builddir"
echo "Entering directory '$PWD'"
set -x
$CC $CFLAGS -o lila \
    ../lila.c ../lila_read.c ../lila_print.c ../lila_examine.c ../lila_unix.c
