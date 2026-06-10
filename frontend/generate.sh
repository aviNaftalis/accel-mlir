#!/usr/bin/env bash
# Regenerate the ANTLR Python parser from Acl.g4 into frontend/gen/.
# Requires Java. The ANTLR complete jar is downloaded on demand if missing.
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
VERSION=4.13.2
JAR="${ANTLR_JAR:-$HERE/tools/antlr-${VERSION}-complete.jar}"

if [ ! -f "$JAR" ]; then
  echo "downloading ANTLR ${VERSION} ..."
  mkdir -p "$(dirname "$JAR")"
  curl -fsSL -o "$JAR" "https://www.antlr.org/download/antlr-${VERSION}-complete.jar"
fi

java -jar "$JAR" -Dlanguage=Python3 -visitor -no-listener \
  -o "$HERE/gen" "$HERE/Acl.g4"
echo "generated parser in $HERE/gen"
