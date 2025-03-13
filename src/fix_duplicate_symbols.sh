#!/bin/bash
# Fix duplicate symbols by adding --allow-multiple-definition to the linker flags

# Find the Makefile
MAKEFILE="/Users/bernardo/Desktop/BitcoinOil/bitcoinoil/src/Makefile"

if [ ! -f "$MAKEFILE" ]; then
    echo "Makefile not found at $MAKEFILE"
    exit 1
fi

# Backup the original Makefile
cp "$MAKEFILE" "${MAKEFILE}.bak"

# Add the --allow-multiple-definition flag to the linker flags
sed -i '' 's/LDFLAGS = \(.*\)/LDFLAGS = \1 -Wl,--allow-multiple-definition/' "$MAKEFILE"

echo "Added --allow-multiple-definition to linker flags in $MAKEFILE"
echo "Original Makefile backed up to ${MAKEFILE}.bak"
echo "Now you can run 'make' to build with duplicate symbols allowed" 