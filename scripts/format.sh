#!/bin/bash
find -name "*.cxx" -o -name "*.hxx" -not -path "./vendored/*" | xargs clang-format -i -style=file
(cd nds && cargo fmt)