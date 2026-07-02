#!/usr/bin/env bash
#
# Local equivalent of .github/workflows/codechecks.yml
#
# Usage: ./codechecks.sh [tidy|format|md|all]  (default: all)

set -uo pipefail
cd "$(dirname "$0")"

CLANG_TIDY=${CLANG_TIDY:-clang-tidy-17}
CLANG_FORMAT=${CLANG_FORMAT:-clang-format-17}
PHP_CONFIG=${PHP_CONFIG:-php-config}
JOBS=${JOBS:-$(nproc)}

TARGET=${1:-all}

mapfile -t FILES < <(
  { [ -f main.cpp ] && echo main.cpp
    [ -f php-gtk.cpp ] && echo php-gtk.cpp
    find src -name '*.cpp'
  } | sort
)

# Same check set as the cpp-linter-action step in the workflow.
TIDY_CHECKS='-*,bugprone-*,performance-*,readability-*,modernize-*,cppcoreguidelines-*'

PHPFLAGS=$($PHP_CONFIG --includes)
GTKFLAGS=$(pkg-config --cflags gtk+-3.0 gladeui-2.0 gtksourceview-3.0 webkit2gtk-4.1)
BASE_FLAGS="-std=c++11 -Wall -Wdeprecated-declarations -Woverloaded-virtual -DWITH_WEBKIT"

run_tidy() {
  echo "== clang-tidy ($TIDY_CHECKS) -- applying fixes =="
  printf '%s\n' "${FILES[@]}" | xargs -P "$JOBS" -I{} sh -c '
    '"$CLANG_TIDY"' "{}" --checks="'"$TIDY_CHECKS"'" --quiet --fix -- '"$PHPFLAGS $GTKFLAGS $BASE_FLAGS"' 2>&1 \
      | grep -E "warning:|error:" && echo "-- in {}"
  '
}

run_format() {
  echo "== clang-format -- applying in place =="
  local status=0
  for f in "${FILES[@]}"; do
    "$CLANG_FORMAT" -style=file -i "$f" || status=1
  done
  return $status
}

run_md() {
  echo "== Markdown metadata check =="
  local status=0
  if [ -f README.md ]; then echo "README.md found"; else echo "README.md not found"; status=1; fi
  if [ -f .github/copilot-instructions.md ]; then echo ".github/copilot-instructions.md found"; else echo ".github/copilot-instructions.md not found"; status=1; fi
  return $status
}

case "$TARGET" in
  tidy) run_tidy ;;
  format) run_format ;;
  md) run_md ;;
  all)
    run_tidy
    run_format
    run_md
    ;;
  *)
    echo "Unknown target: $TARGET (use tidy|format|md|all)" >&2
    exit 2
    ;;
esac
