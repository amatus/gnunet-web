#!/bin/sh -e

cd gnunet-build
export PATH="$PATH":"$(pwd)/scripts"
build
cp sysroot/var/lib/gnunet/hostlist ../resources/public/
cp sysroot/var/lib/gnunet/js/* ../resources/public/js/

# vim: set expandtab ts=2 sw=2:
