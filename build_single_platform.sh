#!/bin/bash

# BitcoinOil Single Platform Build Script
# Tests the build system with one platform first

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

print_header() {
    echo ""
    echo -e "${PURPLE}${BOLD}========================================${NC}"
    echo -e "${PURPLE}${BOLD}    $1${NC}"
    echo -e "${PURPLE}${BOLD}========================================${NC}"
    echo ""
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

# Build configuration
BUILD_DIR="$(pwd)"
RELEASE_DIR="${BUILD_DIR}/release_binaries"
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo "4")

# Platform to build (can be changed)
TARGET_HOST="${1:-x86_64-linux-gnu}"
TARGET_NAME="${2:-Linux_64-bit}"

print_header "BitcoinOil Single Platform Build Test"

echo -e "${CYAN}Build Configuration:${NC}"
echo -e "  Target Platform: ${BOLD}${TARGET_HOST}${NC}"
echo -e "  Platform Name: ${BOLD}${TARGET_NAME}${NC}"
echo -e "  Source Directory: ${BOLD}${BUILD_DIR}${NC}"
echo -e "  Release Directory: ${BOLD}${RELEASE_DIR}${NC}"
echo -e "  Parallel Jobs: ${BOLD}${JOBS}${NC}"
echo ""

# Check dependencies
print_info "Checking build dependencies..."

# Check for required tools
REQUIRED_TOOLS="make autoconf automake libtool pkg-config"
MISSING_TOOLS=""

for tool in $REQUIRED_TOOLS; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        MISSING_TOOLS="$MISSING_TOOLS $tool"
    fi
done

if [ -n "$MISSING_TOOLS" ]; then
    print_error "Missing required tools:$MISSING_TOOLS"
    echo ""
    echo "Install missing tools:"
    echo "  macOS: brew install autoconf automake libtool pkg-config"
    echo "  Ubuntu/Debian: sudo apt-get install build-essential autoconf automake libtool pkg-config"
    exit 1
fi

print_success "All required tools found"

# Create release directory
mkdir -p "$RELEASE_DIR"

start_time=$(date +%s)

print_header "Building Dependencies for $TARGET_NAME"

# Build dependencies
print_info "Building dependencies for $TARGET_HOST..."
cd depends
if ! make HOST="$TARGET_HOST" -j"$JOBS"; then
    print_error "Failed to build dependencies for $TARGET_HOST"
    exit 1
fi
cd ..

print_success "Dependencies built successfully"

print_header "Building BitcoinOil for $TARGET_NAME"

# Configure and build BitcoinOil
print_info "Configuring BitcoinOil for $TARGET_HOST..."

# Clean previous build
make distclean >/dev/null 2>&1 || true

# Set config site
export CONFIG_SITE="$PWD/depends/$TARGET_HOST/share/config.site"

if ! ./configure --prefix="/"; then
    print_error "Configuration failed for $TARGET_HOST"
    exit 1
fi

print_info "Building BitcoinOil for $TARGET_HOST..."
if ! make -j"$JOBS"; then
    print_error "Build failed for $TARGET_HOST"
    exit 1
fi

print_success "BitcoinOil built successfully"

print_header "Packaging Binaries"

# Create platform-specific directory
platform_dir="$RELEASE_DIR/$TARGET_NAME"
mkdir -p "$platform_dir"

# Copy binaries with proper extensions
if [[ "$TARGET_HOST" == *"mingw32"* ]]; then
    # Windows binaries
    print_info "Copying Windows binaries..."
    cp src/bitcoinoild.exe "$platform_dir/" 2>/dev/null || cp src/bitcoinoild "$platform_dir/bitcoinoild.exe"
    cp src/bitcoinoil-cli.exe "$platform_dir/" 2>/dev/null || cp src/bitcoinoil-cli "$platform_dir/bitcoinoil-cli.exe"
    cp src/bitcoinoil-tx.exe "$platform_dir/" 2>/dev/null || cp src/bitcoinoil-tx "$platform_dir/bitcoinoil-tx.exe"
    cp src/bitcoinoil-wallet.exe "$platform_dir/" 2>/dev/null || cp src/bitcoinoil-wallet "$platform_dir/bitcoinoil-wallet.exe"
    cp src/bitcoinoil-util.exe "$platform_dir/" 2>/dev/null || cp src/bitcoinoil-util "$platform_dir/bitcoinoil-util.exe"
else
    # Unix binaries
    print_info "Copying Unix binaries..."
    cp src/bitcoinoild "$platform_dir/"
    cp src/bitcoinoil-cli "$platform_dir/"
    cp src/bitcoinoil-tx "$platform_dir/"
    cp src/bitcoinoil-wallet "$platform_dir/"
    cp src/bitcoinoil-util "$platform_dir/"
fi

# Copy documentation
cp README.md "$platform_dir/" 2>/dev/null || echo "README.md not found"
cp COPYING "$platform_dir/" 2>/dev/null || echo "COPYING not found"

end_time=$(date +%s)
duration=$((end_time - start_time))
minutes=$((duration / 60))
seconds=$((duration % 60))

print_success "Build completed in ${minutes}m ${seconds}s"

# Show binary info and sizes
print_header "Build Results"

echo -e "${CYAN}Platform Directory:${NC} ${BOLD}$platform_dir${NC}"
echo ""

print_info "Binary Information:"
for binary in "$platform_dir"/bitcoinoil*; do
    if [ -f "$binary" ]; then
        binary_name=$(basename "$binary")
        binary_size=$(ls -lh "$binary" | awk '{print $5}')
        if [[ "$TARGET_HOST" != *"mingw32"* ]]; then
            binary_arch=$(file "$binary" | cut -d: -f2 | awk '{print $1 " " $2}')
            echo -e "  ${GREEN}$binary_name${NC}: $binary_size ($binary_arch)"
        else
            echo -e "  ${GREEN}$binary_name${NC}: $binary_size"
        fi
    fi
done

echo ""

# Create archive
print_info "Creating distribution archive..."
cd "$RELEASE_DIR"
archive_name="bitcoinoil-${TARGET_NAME}.tar.gz"
tar -czf "$archive_name" "$TARGET_NAME"
archive_size=$(ls -lh "$archive_name" | awk '{print $5}')
print_success "Created $archive_name ($archive_size)"
cd "$BUILD_DIR"

print_header "Build Test Complete"

echo -e "${GREEN}✓ Single platform build successful!${NC}"
echo -e "${CYAN}Ready to build all platforms with: ${BOLD}./build_all_platforms.sh${NC}"
echo ""
echo -e "${CYAN}Available platforms:${NC}"
echo -e "  ${YELLOW}x86_64-w64-mingw32${NC}     - Windows 64-bit"
echo -e "  ${YELLOW}i686-w64-mingw32${NC}       - Windows 32-bit"
echo -e "  ${YELLOW}x86_64-linux-gnu${NC}       - Linux 64-bit"
echo -e "  ${YELLOW}aarch64-linux-gnu${NC}      - Linux ARM64"
echo -e "  ${YELLOW}x86_64-apple-darwin${NC}    - Mac Intel"
echo -e "  ${YELLOW}aarch64-apple-darwin${NC}   - Mac Apple Silicon"
echo "" 