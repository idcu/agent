#!/bin/bash

echo "Formatting C/C++ files..."

find . -name "*.c" -o -name "*.h" | while read -r file; do
    echo "Formatting: $file"
    clang-format -i "$file"
done

echo "Formatting complete!"
