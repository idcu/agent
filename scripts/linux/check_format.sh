#!/bin/bash

echo "Checking code format..."

HAS_ERRORS=0

find . -name "*.c" -o -name "*.h" | while read -r file; do
    if ! clang-format --dry-run --Werror "$file" > /dev/null 2>&1; then
        echo "File needs formatting: $file"
        HAS_ERRORS=1
    fi
done

if [ "$HAS_ERRORS" -eq 0 ]; then
    echo "All files are properly formatted!"
else
    echo "Some files need formatting!"
    exit 1
fi
