#!/bin/bash

# BitcoinOil Multi-Platform Build Script
# Builds for Windows, Linux, Mac Intel, and Mac Apple Silicon

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

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

# Build configuration
BUILD_DIR="$(pwd)"
RELEASE_DIR="${BUILD_DIR}/release_binaries"
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo "4")

# Platform configurations
declare -A PLATFORMS=(
    ["x86_64-w64-mingw32"]="Windows 64-bit"
    ["i686-w64-mingw32"]="Windows 32-bit"
    ["x86_64-linux-gnu"]="Linux 64-bit"
    ["aarch64-linux-gnu"]="Linux ARM64"
    ["x86_64-apple-darwin"]="Mac Intel"
    ["aarch64-apple-darwin"]="Mac Apple Silicon"
)

print_header "BitcoinOil Multi-Platform Build System"

echo -e "${CYAN}Build Configuration:${NC}"
echo -e "  Source Directory: ${BOLD}${BUILD_DIR}${NC}"
echo -e "  Release Directory: ${BOLD}${RELEASE_DIR}${NC}"
echo -e "  Parallel Jobs: ${BOLD}${JOBS}${NC}"
echo ""

# Check dependencies
print_info "Checking build dependencies..."

# Check for required tools
REQUIRED_TOOLS=("make" "g++" "autoconf" "automake" "libtool" "pkg-config")
MISSING_TOOLS=()

for tool in "${REQUIRED_TOOLS[@]}"; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        MISSING_TOOLS+=("$tool")
    fi
done

if [ ${#MISSING_TOOLS[@]} -ne 0 ]; then
    print_error "Missing required tools: ${MISSING_TOOLS[*]}"
    echo ""
    echo "Install missing tools:"
    echo "  macOS: brew install autoconf automake libtool pkg-config"
    echo "  Ubuntu/Debian: sudo apt-get install build-essential autoconf automake libtool pkg-config"
    exit 1
fi

print_success "All required tools found"

# Create release directory
mkdir -p "$RELEASE_DIR"

# Function to build for a specific platform
build_platform() {
    local host="$1"
    local name="$2"
    
    print_header "Building $name ($host)"
    
    local start_time=$(date +%s)
    
    # Build dependencies
    print_info "Building dependencies for $host..."
    cd depends
    if ! make HOST="$host" -j"$JOBS"; then
        print_error "Failed to build dependencies for $host"
        return 1
    fi
    cd ..
    
    # Configure and build BitcoinOil
    print_info "Configuring BitcoinOil for $host..."
    
    # Clean previous build
    make distclean >/dev/null 2>&1 || true
    
    # Set config site
    export CONFIG_SITE="$PWD/depends/$host/share/config.site"
    
    if ! ./configure --prefix="/" >/dev/null; then
        print_error "Configuration failed for $host"
        return 1
    fi
    
    print_info "Building BitcoinOil for $host..."
    if ! make -j"$JOBS" >/dev/null; then
        print_error "Build failed for $host"
        return 1
    fi
    
    # Create platform-specific directory
    local platform_dir="$RELEASE_DIR/$name"
    mkdir -p "$platform_dir"
    
    # Copy binaries with proper extensions
    if [[ "$host" == *"mingw32"* ]]; then
        # Windows binaries
        cp src/bitcoinoild.exe "$platform_dir/"
        cp src/bitcoinoil-cli.exe "$platform_dir/"
        cp src/bitcoinoil-tx.exe "$platform_dir/"
        cp src/bitcoinoil-wallet.exe "$platform_dir/"
        cp src/bitcoinoil-util.exe "$platform_dir/"
    else
        # Unix binaries
        cp src/bitcoinoild "$platform_dir/"
        cp src/bitcoinoil-cli "$platform_dir/"
        cp src/bitcoinoil-tx "$platform_dir/"
        cp src/bitcoinoil-wallet "$platform_dir/"
        cp src/bitcoinoil-util "$platform_dir/"
    fi
    
    # Copy documentation
    cp README.md "$platform_dir/" 2>/dev/null || true
    cp COPYING "$platform_dir/" 2>/dev/null || true
    
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    local minutes=$((duration / 60))
    local seconds=$((duration % 60))
    
    print_success "$name build completed in ${minutes}m ${seconds}s"
    
    # Show binary info
    if [[ "$host" != *"mingw32"* ]]; then
        print_info "Binary info: $(file "$platform_dir/bitcoinoild" | cut -d: -f2)"
    fi
    
    return 0
}

# Main build loop
total_start=$(date +%s)
successful_builds=0
failed_builds=0

for host in "${!PLATFORMS[@]}"; do
    name="${PLATFORMS[$host]}"
    
    if build_platform "$host" "$name"; then
        ((successful_builds++))
    else
        ((failed_builds++))
        print_warning "Continuing with remaining platforms..."
    fi
    echo ""
done

# Build summary
total_end=$(date +%s)
total_duration=$((total_end - total_start))
total_minutes=$((total_duration / 60))
total_seconds=$((total_duration % 60))

print_header "Build Summary"

echo -e "${GREEN}✓ Successful builds: ${BOLD}$successful_builds${NC}"
echo -e "${RED}✗ Failed builds: ${BOLD}$failed_builds${NC}"
echo -e "${CYAN}Total build time: ${BOLD}${total_minutes}m ${total_seconds}s${NC}"
echo ""

if [ "$successful_builds" -gt 0 ]; then
    echo -e "${CYAN}Release binaries location:${NC}"
    echo -e "  ${BOLD}$RELEASE_DIR${NC}"
    echo ""
    
    print_info "Available builds:"
    for dir in "$RELEASE_DIR"/*; do
        if [ -d "$dir" ]; then
            local platform_name=$(basename "$dir")
            local binary_count=$(ls "$dir"/bitcoinoil* 2>/dev/null | wc -l)
            echo -e "  ${GREEN}$platform_name${NC} ($binary_count binaries)"
        fi
    done
    echo ""
    
    # Create distribution archives
    print_info "Creating distribution archives..."
    cd "$RELEASE_DIR"
    for dir in */; do
        if [ -d "$dir" ]; then
            local platform_name=$(basename "$dir")
            local archive_name="bitcoinoil-${platform_name}.tar.gz"
            tar -czf "$archive_name" "$platform_name"
            print_success "Created $archive_name"
        fi
    done
    cd "$BUILD_DIR"
fi

if [ "$failed_builds" -eq 0 ]; then
    print_success "All platform builds completed successfully! 🎉"
else
    print_warning "Some builds failed. Check the output above for details."
fi

echo ""
print_header "BitcoinOil Build Complete" 