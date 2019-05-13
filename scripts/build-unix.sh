#!/bin/sh
set -eu
cd "$(dirname "$0")"/..
b="build-$(uname | tr A-Z- a-z_)-$(uname -m | tr A-Z- a-z_)"
mkdir -p "$b"
cd "$b"
echo "Entering directory '$PWD'"
set -x
${CC:-clang} ${CFLAGS:--Wall -Wextra -pedantic -std=gnu99 -Og -g} \
	-o lila ../lila_unix.c
