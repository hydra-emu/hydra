#!/bin/bash
find -name "*.cxx" -o -name "*.hxx" -not -path "./vendored/*" | xargs clang-format --dry-run --Werror -style=file