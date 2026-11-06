#!/bin/sh

DIR="$(cd "$(dirname "$0")" && pwd)"
export LD_LIBRARY_PATH="$DIR/plugins:$LD_LIBRARY_PATH"
export VAMP_PATH="$DIR/plugins"
exec "$DIR/usr/bin//RiffTabs" "$@"
