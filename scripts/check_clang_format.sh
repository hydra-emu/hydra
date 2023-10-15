#!/bin/bash
find . -type f -iregex '.*\.\(hxx\|cxx\|hpp\|cpp\)$' -not -path "./vendored/*" -not -path "**/build/*" -not -path "**/CMakeFiles/*" | xargs clang-format --dry-run --Werror -style=file