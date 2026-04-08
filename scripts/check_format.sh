#!/bin/bash
# Check code format (Linux/macOS version)

echo "=== Check code format (clang-format) ==="

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    echo "ERROR: clang-format not found, please install it first"
    echo "Ubuntu/Debian: sudo apt-get install clang-format"
    echo "macOS: brew install clang-format"
    exit 1
fi

# Define directories to check
DIRS="libs modules app tests"

# Run clang-format check
echo "Checking source files..."
NEED_FORMAT=0

for dir in $DIRS; do
    if [ -d "$dir" ]; then
        while IFS= read -r -d $'\0' file; do
            clang-format --dry-run --Werror "$file" > /dev/null 2>&1
            if [ $? -ne 0 ]; then
                echo "Needs formatting: $file"
                ((NEED_FORMAT++))
            fi
        done < <(find "$dir" -name "*.c" -o -name "*.h" -print0)
    fi
done

if [ "$NEED_FORMAT" -eq 0 ]; then
    echo ""
    echo "All files are properly formatted"
else
    echo ""
    echo "$NEED_FORMAT files need formatting"
    echo ""
    echo "Run the following command to auto-fix:"
    echo "  scripts/format.sh"
    exit 1
fi
