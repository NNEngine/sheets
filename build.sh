# build.sh - compiles libcsv + sheets/core.c into csv/core.so
#
# Requires: gcc, pkg-config, Lua dev headers (e.g. liblua5.4-dev)
# Usage:    ./build.sh [lua-pkgconfig-name]   (default: lua5.4)

set -e
LUAPKG="${1:-lua5.4}"

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

CFLAGS=$(pkg-config --cflags "$LUAPKG")
echo "Building sheets/core.so against $LUAPKG ($CFLAGS)"

gcc -O2 -fPIC -Wall -shared $CFLAGS \
    -o sheets/core.so \
    lib/libcsv/libcsv.c sheets/core.c

echo "Done -> sheets/core.so"
echo ""
echo "Now require(\"sheets\") will work as long as your script can see this"
echo "directory on package.path/package.cpath (run from the project root,"
echo "or add this directory to LUA_PATH / LUA_CPATH)."
