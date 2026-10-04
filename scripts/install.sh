#!/bin/sh
# Install into $PREFIX (default /usr/local).   Usage: PREFIX=$HOME/.local scripts/install.sh
set -e
cd "$(dirname "$0")/.."
make all
make install PREFIX="${PREFIX:-/usr/local}"
echo "installed to ${PREFIX:-/usr/local}/bin"
