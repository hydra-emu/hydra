#!/bin/bash
# check if .git folder exists
if [ ! -d ".git" ]; then
    echo "Please run from the root of the repository"
    exit 1
fi

mkdir -p .git/hooks
mkdir -p .git/hooks/pre-commit
mv .git/hooks/pre-commit .git/hooks/pre-commit.bak
cp scripts/hooks/pre-commit.linux .git/hooks/pre-commit
chmod +x .git/hooks/pre-commit
