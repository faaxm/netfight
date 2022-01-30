#!/bin/sh
find . | grep -v -E "/(build|dep-libs)/.*" | grep -E "\\.(c|cpp|h)$" | xargs -L1 clang-format -i
