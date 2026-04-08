#!/bin/bash
# Auto format code (Linux/macOS version)

echo "=== Auto format code (clang-format) ==="

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    echo "ERROR: clang-format not found, please install it first"
    echo "Ubuntu/Debian: sudo apt-get install clang-format"
    echo "macOS: brew install clang-format"
    exit 1
fi

# Define directories to format
DIRS="libs modules app tests"

# Format all files
echo "Formatting source files..."

for dir in $DIRS; do
    if [ -d "$dir" ]; then
        while IFS= read -r -d $'\0' file; do
            clang-format -i "$file"
            echo "Formatted: $file"
        done < <(find "$dir" -name "*.c" -o -name "*.h" -print0)
    fi
done

echo ""
echo "Formatting completed"
