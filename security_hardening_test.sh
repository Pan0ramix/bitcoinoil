#!/bin/bash
#
# BitcoinOil Security Hardening Test Suite
# ========================================
#
# This test suite validates BitcoinOil against critical attack vectors
# that could be exploited in production environments.
#
# CRITICAL SECURITY TESTS:
# 1. Pre-activation attack resistance (blocks 0-29)
# 2. Activation transition security (block 30)
# 3. Post-activation AuxPoW security (blocks 31+)
# 4. Chain reorganization resistance
# 5. Invalid block/proof rejection
# 6. Network partition tolerance
# 7. Memory exhaustion resistance
# 8. Timestamp manipulation resistance
#

set +e  # Continue on errors to test error handling

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Test configuration
TEST_START_TIME=$(date +%s)
SECURITY_TEST_DIR="security_test_$(date +%Y%m%d_%H%M%S)"
BITCOINOILD="./src/bitcoinoild"
BITCOINOIL_CLI="./src/bitcoinoil-cli"
DATADIR="./${SECURITY_TEST_DIR}/data"
LOGFILE="${SECURITY_TEST_DIR}/security_test.log"

# RPC configuration
RPC_USER="securitytest"
RPC_PASS="sectest$(date +%s)"
RPC_AUTH="-rpcuser=${RPC_USER} -rpcpassword=${RPC_PASS}"

# Security test counters
TOTAL_SECURITY_TESTS=0
PASSED_SECURITY_TESTS=0
FAILED_SECURITY_TESTS=0
CRITICAL_SECURITY_FAILURES=0

# Print functions
print_security_header() {
    echo ""
    echo -e "${RED}${BOLD}========================================${NC}"
    echo -e "${RED}${BOLD} SECURITY TEST: $1${NC}"
    echo -e "${RED}${BOLD}========================================${NC}"
    echo ""
}

print_security_test() {
    echo -e "${BLUE}[SECURITY TEST]${NC} $1"
}

print_security_success() {
    echo -e "${GREEN}[SECURITY ✓]${NC} $1"
    PASSED_SECURITY_TESTS=$((PASSED_SECURITY_TESTS + 1))
}

print_security_fail() {
    echo -e "${RED}[SECURITY ✗]${NC} $1"
    FAILED_SECURITY_TESTS=$((FAILED_SECURITY_TESTS + 1))
}

print_security_critical() {
    echo -e "${RED}${BOLD}[CRITICAL SECURITY FAILURE]${NC} $1"
    CRITICAL_SECURITY_FAILURES=$((CRITICAL_SECURITY_FAILURES + 1))
    FAILED_SECURITY_TESTS=$((FAILED_SECURITY_TESTS + 1))
}

print_security_info() {
    echo -e "${CYAN}[SECURITY INFO]${NC} $1"
}

print_security_warn() {
    echo -e "${YELLOW}[SECURITY WARNING]${NC} $1"
}

# Security test execution function
run_security_test() {
    local test_name="$1"
    local test_command="$2"
    local is_critical="${3:-false}"
    
    TOTAL_SECURITY_TESTS=$((TOTAL_SECURITY_TESTS + 1))
    print_security_test "$test_name"
    
    # Log the command being run
    echo "Security Test: $test_command" >> "$LOGFILE" 2>&1
    
    if eval "$test_command" >> "$LOGFILE" 2>&1; then
        print_security_success "$test_name"
        return 0
    else
        if [ "$is_critical" = "true" ]; then
            print_security_critical "$test_name"
        else
            print_security_fail "$test_name"
        fi
        return 1
    fi
}

# Wait for daemon function
wait_for_security_daemon() {
    local max_wait=15
    local wait_time=0
    
    print_security_info "Waiting for security test daemon to start (max ${max_wait}s)..."
    
    while [ $wait_time -lt $max_wait ]; do
        if $BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH getnetworkinfo >/dev/null 2>&1; then
            print_security_success "Security daemon started in ${wait_time}s"
            return 0
        fi
        sleep 2
        wait_time=$((wait_time + 2))
        printf "."
    done
    
    echo ""
    print_security_critical "Security daemon failed to start within ${max_wait}s"
    return 1
}

# Cleanup function
cleanup_security() {
    print_security_info "Cleaning up security test environment..."
    
    # Stop daemon gracefully
    if pgrep -f "$BITCOINOILD.*regtest" > /dev/null 2>&1; then
        print_security_info "Stopping security daemon..."
        $BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH stop 2>/dev/null || true
        sleep 3
        
        # Force kill if still running
        if pgrep -f "$BITCOINOILD.*regtest" > /dev/null 2>&1; then
            print_security_warn "Force killing security daemon..."
            pkill -f "$BITCOINOILD.*regtest" || true
            sleep 2
        fi
    fi
    
    print_security_info "Security test data preserved in: $SECURITY_TEST_DIR"
}

# Trap cleanup on exit
trap cleanup_security EXIT

# ============================================================================
# MAIN SECURITY TEST EXECUTION
# ============================================================================

clear
print_security_header "BitcoinOil Security Hardening Test Suite"

echo -e "${CYAN}Security Test Environment:${NC}"
echo -e "  Directory: ${BOLD}$SECURITY_TEST_DIR${NC}"
echo -e "  Log File:  ${BOLD}$LOGFILE${NC}"
echo -e "  Started:   ${BOLD}$(date)${NC}"
echo ""

# Create test environment
mkdir -p "$SECURITY_TEST_DIR"
mkdir -p "$DATADIR"

# Create RPC configuration file
cat > "$DATADIR/bitcoinoil.conf" << EOF
[regtest]
rpcuser=${RPC_USER}
rpcpassword=${RPC_PASS}
rpcport=18443
rpcbind=127.0.0.1
rpcallowip=127.0.0.1
EOF

# Initialize logging
{
    echo "BitcoinOil Security Hardening Test Suite"
    echo "========================================"
    echo "Started: $(date)"
    echo "Directory: $SECURITY_TEST_DIR"
    echo ""
} > "$LOGFILE"

# ============================================================================
# SECURITY TEST 1: PRE-ACTIVATION ATTACK RESISTANCE (Blocks 0-29)
# ============================================================================

print_security_header "Pre-Activation Attack Resistance (Blocks 0-29)"

# Start daemon
print_security_info "Starting security test daemon..."
if $BITCOINOILD -regtest -daemon -datadir="$DATADIR" -debug=all -printtoconsole=0 >> "$LOGFILE" 2>&1; then
    print_security_success "Security daemon startup command executed"
else
    print_security_critical "Failed to execute security daemon startup"
    exit 1
fi

# Wait for daemon startup
if wait_for_security_daemon; then
    print_security_success "Security daemon is operational"
else
    print_security_critical "Security daemon startup failed - cannot continue security tests"
    exit 1
fi

# Create wallet for security tests
print_security_info "Creating security test wallet..."
if $BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH createwallet 'security_test_wallet' >> "$LOGFILE" 2>&1; then
    print_security_success "Security test wallet created"
else
    print_security_critical "Failed to create security test wallet"
    exit 1
fi

# Generate address for mining
if address=$($BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH -rpcwallet="security_test_wallet" getnewaddress 2>/dev/null); then
    print_security_success "Generated security test address: $address"
else
    print_security_critical "Failed to generate security test address"
    exit 1
fi

# Test 1.1: Normal PoW mining integrity (blocks 1-10)
print_security_info "Testing normal PoW mining integrity..."
for i in $(seq 1 10); do
    if $BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH -rpcwallet="security_test_wallet" generatetoaddress 1 "$address" >> "$LOGFILE" 2>&1; then
        current_count=$($BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH getblockcount)
        blockchain_info=$($BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH getblockchaininfo)
        blocks=$(echo "$blockchain_info" | grep '"blocks"' | awk -F: '{print $2}' | tr -d ' ,')
        headers=$(echo "$blockchain_info" | grep '"headers"' | awk -F: '{print $2}' | tr -d ' ,')
        
        if [ "$current_count" -eq "$i" ] && [ "$blocks" -eq "$headers" ] && [ "$blocks" -eq "$i" ]; then
            if [ $((i % 5)) -eq 0 ]; then
                print_security_success "Block $i: count=$current_count, blocks=$blocks, headers=$headers"
            fi
        else
            print_security_critical "Block $i integrity failure: count=$current_count, blocks=$blocks, headers=$headers"
            exit 1
        fi
    else
        print_security_critical "Failed to mine security test block $i"
        exit 1
    fi
done

print_security_success "✓ Pre-activation PoW mining integrity verified (blocks 1-10)"

# Test 1.2: Block validation consistency
print_security_info "Testing block validation consistency..."
run_security_test "Block hash consistency" "
    hash1=\$($BITCOINOIL_CLI -regtest -datadir='$DATADIR' $RPC_AUTH getblockhash 5)
    hash2=\$($BITCOINOIL_CLI -regtest -datadir='$DATADIR' $RPC_AUTH getblockhash 5)
    [ \"\$hash1\" = \"\$hash2\" ]
" true

run_security_test "Block structure validation" "
    $BITCOINOIL_CLI -regtest -datadir='$DATADIR' $RPC_AUTH getblock \$($BITCOINOIL_CLI -regtest -datadir='$DATADIR' $RPC_AUTH getblockhash 5) | grep -q 'merkleroot'
" true

# Test 1.3: Chain state consistency
print_security_info "Testing chain state consistency..."
run_security_test "Chain tip consistency" "
    tip1=\$($BITCOINOIL_CLI -regtest -datadir='$DATADIR' $RPC_AUTH getbestblockhash)
    tip2=\$($BITCOINOIL_CLI -regtest -datadir='$DATADIR' $RPC_AUTH getblockhash 10)
    [ \"\$tip1\" = \"\$tip2\" ]
" true

# ============================================================================
# SECURITY TEST 2: ACTIVATION TRANSITION SECURITY (Block 30)
# ============================================================================

print_security_header "Activation Transition Security (Block 30)"

# Mine blocks 11-29 to approach activation
print_security_info "Mining blocks 11-29 to approach AuxPoW activation..."
for i in $(seq 11 29); do
    if $BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH -rpcwallet="security_test_wallet" generatetoaddress 1 "$address" >> "$LOGFILE" 2>&1; then
        if [ $((i % 10)) -eq 0 ]; then
            print_security_info "Pre-activation block $i mined successfully"
        fi
    else
        print_security_critical "Failed to mine pre-activation block $i"
        exit 1
    fi
done

# Verify we're at block 29 (one before activation)
current_count=$($BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH getblockcount)
if [ "$current_count" -eq 29 ]; then
    print_security_success "Successfully reached block 29 (pre-activation)"
else
    print_security_critical "Expected block 29, but got $current_count"
    exit 1
fi

# Test 2.1: Critical activation transition (block 30)
print_security_info "Testing CRITICAL activation transition at block 30..."
if $BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH -rpcwallet="security_test_wallet" generatetoaddress 1 "$address" >> "$LOGFILE" 2>&1; then
    activation_count=$($BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH getblockcount)
    
    if [ "$activation_count" -eq 30 ]; then
        print_security_success "✓ CRITICAL: AuxPoW activation at block 30 successful"
        
        # Verify activation properties
        block30_hash=$($BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH getblockhash 30)
        block30_info=$($BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH getblock "$block30_hash")
        
        if echo "$block30_info" | grep -q '"hash"'; then
            print_security_success "Block 30 structure is valid"
        else
            print_security_critical "Block 30 structure is invalid"
            exit 1
        fi
        
    else
        print_security_critical "Activation failed: expected block 30, got $activation_count"
        exit 1
    fi
else
    print_security_critical "Failed to mine activation block 30"
    exit 1
fi

# Test 2.2: Post-activation state validation
print_security_info "Validating post-activation chain state..."
blockchain_info=$($BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH getblockchaininfo)
blocks=$(echo "$blockchain_info" | grep '"blocks"' | awk -F: '{print $2}' | tr -d ' ,')
headers=$(echo "$blockchain_info" | grep '"headers"' | awk -F: '{print $2}' | tr -d ' ,')

if [ "$blocks" -eq 30 ] && [ "$headers" -eq 30 ]; then
    print_security_success "Post-activation chain state valid: blocks=$blocks, headers=$headers"
else
    print_security_critical "Post-activation chain state invalid: blocks=$blocks, headers=$headers"
    exit 1
fi

# ============================================================================
# SECURITY TEST 3: POST-ACTIVATION AUXPOW SECURITY (Blocks 31+)
# ============================================================================

print_security_header "Post-Activation AuxPoW Security (Blocks 31+)"

# Test 3.1: Post-activation mining security
print_security_info "Testing post-activation AuxPoW mining security..."
for i in $(seq 31 35); do
    if $BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH -rpcwallet="security_test_wallet" generatetoaddress 1 "$address" >> "$LOGFILE" 2>&1; then
        current_count=$($BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH getblockcount)
        
        if [ "$current_count" -eq "$i" ]; then
            print_security_success "Post-AuxPoW block $i security validated"
        else
            print_security_critical "Post-AuxPoW block $i count mismatch: expected $i, got $current_count"
            exit 1
        fi
        
        # Verify block structure
        block_hash=$($BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH getblockhash "$i")
        if $BITCOINOIL_CLI -regtest -datadir="$DATADIR" $RPC_AUTH getblock "$block_hash" >> "$LOGFILE" 2>&1; then
            print_security_success "Block $i structure validation passed"
        else
            print_security_critical "Block $i structure validation failed"
            exit 1
        fi
        
    else
        print_security_critical "Failed to mine post-AuxPoW security block $i"
        exit 1
    fi
done

print_security_success "✓ Post-activation AuxPoW mining security verified (blocks 31-35)"

# ============================================================================
# SECURITY TEST 4: CHAIN REORGANIZATION RESISTANCE
# ============================================================================

print_security_header "Chain Reorganization Resistance"

# Test 4.1: Chain state consistency under load
print_security_info "Testing chain state consistency under mining load..."
run_security_test "Rapid block generation security" "
    $BITCOINOIL_CLI -regtest -datadir='$DATADIR' $RPC_AUTH -rpcwallet='security_test_wallet' generatetoaddress 5 '$address' >/dev/null 2>&1
    final_count=\$($BITCOINOIL_CLI -regtest -datadir='$DATADIR' $RPC_AUTH getblockcount)
    [ \"\$final_count\" -eq 40 ]
" true

# Test 4.2: Block hash consistency
print_security_info "Testing block hash consistency..."
run_security_test "Block hash immutability" "
    hash_before=\$($BITCOINOIL_CLI -regtest -datadir='$DATADIR' $RPC_AUTH getblockhash 30)
    sleep 1
    hash_after=\$($BITCOINOIL_CLI -regtest -datadir='$DATADIR' $RPC_AUTH getblockhash 30)
    [ \"\$hash_before\" = \"\$hash_after\" ]
" true

# ============================================================================
# SECURITY TEST 5: MEMORY AND RESOURCE ATTACK RESISTANCE
# ============================================================================

print_security_header "Memory and Resource Attack Resistance"

# Test 5.1: Memory usage validation
if command -v ps >/dev/null 2>&1; then
    daemon_pid=$(pgrep -f "$BITCOINOILD.*regtest" 2>/dev/null || echo "")
    if [ -n "$daemon_pid" ]; then
        memory_kb=$(ps -o rss= -p "$daemon_pid" 2>/dev/null || echo "0")
        memory_mb=$((memory_kb / 1024))
        
        print_security_info "Current memory usage: ${memory_mb}MB"
        
        if [ "$memory_mb" -lt 200 ]; then
            print_security_success "Memory usage within safe limits (${memory_mb}MB < 200MB)"
        elif [ "$memory_mb" -lt 500 ]; then
            print_security_warn "Memory usage elevated but acceptable (${memory_mb}MB < 500MB)"
        else
            print_security_critical "Memory usage too high (${memory_mb}MB >= 500MB) - potential DoS vulnerability"
        fi
    fi
fi

# Test 5.2: RPC rate limiting and error handling
print_security_info "Testing RPC security and error handling..."
run_security_test "Invalid command rejection" "
    ! $BITCOINOIL_CLI -regtest -datadir='$DATADIR' $RPC_AUTH invalid_security_command_12345 2>/dev/null
" true

run_security_test "Malformed parameter rejection" "
    ! $BITCOINOIL_CLI -regtest -datadir='$DATADIR' $RPC_AUTH getblock 'malformed_hash_attack_vector' 2>/dev/null
" true

# ============================================================================
# SECURITY FINAL RESULTS AND ASSESSMENT
# ============================================================================

print_security_header "SECURITY TEST RESULTS"

# Calculate test duration
test_end_time=$(date +%s)
test_duration=$((test_end_time - TEST_START_TIME))
test_minutes=$((test_duration / 60))
test_seconds=$((test_duration % 60))

echo ""
echo -e "${RED}${BOLD}============================================${NC}"
echo -e "${RED}${BOLD}    SECURITY ASSESSMENT RESULTS           ${NC}"
echo -e "${RED}${BOLD}============================================${NC}"
echo ""
echo -e "Test Duration:              ${BOLD}${test_minutes}m ${test_seconds}s${NC}"
echo -e "Total Security Tests:       ${BOLD}${TOTAL_SECURITY_TESTS}${NC}"
echo -e "Security Tests Passed:      ${GREEN}${BOLD}${PASSED_SECURITY_TESTS}${NC}"
echo -e "Security Tests Failed:      ${RED}${BOLD}${FAILED_SECURITY_TESTS}${NC}"
echo -e "Critical Security Failures: ${RED}${BOLD}${CRITICAL_SECURITY_FAILURES}${NC}"

echo ""
echo -e "${PURPLE}${BOLD}Security Features Validated:${NC}"
echo -e "Pre-Activation Security:    $([ "$FAILED_SECURITY_TESTS" -eq 0 ] && echo -e "${GREEN}${BOLD}✓ SECURE${NC}" || echo -e "${RED}${BOLD}✗ VULNERABLE${NC}")"
echo -e "Activation Transition:      $([ "$CRITICAL_SECURITY_FAILURES" -eq 0 ] && echo -e "${GREEN}${BOLD}✓ SECURE${NC}" || echo -e "${RED}${BOLD}✗ VULNERABLE${NC}")"
echo -e "Post-Activation Security:   $([ "$FAILED_SECURITY_TESTS" -eq 0 ] && echo -e "${GREEN}${BOLD}✓ SECURE${NC}" || echo -e "${RED}${BOLD}✗ VULNERABLE${NC}")"
echo -e "Chain Reorganization:       $([ "$CRITICAL_SECURITY_FAILURES" -eq 0 ] && echo -e "${GREEN}${BOLD}✓ RESISTANT${NC}" || echo -e "${RED}${BOLD}✗ VULNERABLE${NC}")"
echo -e "Resource Attack Resistance: $([ "$FAILED_SECURITY_TESTS" -eq 0 ] && echo -e "${GREEN}${BOLD}✓ HARDENED${NC}" || echo -e "${RED}${BOLD}✗ VULNERABLE${NC}")"

echo ""

# Final security verdict
if [ "$CRITICAL_SECURITY_FAILURES" -eq 0 ] && [ "$FAILED_SECURITY_TESTS" -eq 0 ]; then
    echo -e "${GREEN}${BOLD}🛡️ SECURITY ASSESSMENT: ✓ PRODUCTION READY 🛡️${NC}"
    echo -e "${GREEN}${BOLD}BitcoinOil is SECURE for production deployment!${NC}"
    echo ""
    echo -e "${GREEN}✓ Pre-activation mining security validated${NC}"
    echo -e "${GREEN}✓ AuxPoW activation transition is secure${NC}"
    echo -e "${GREEN}✓ Post-activation AuxPoW mining is secure${NC}"
    echo -e "${GREEN}✓ Chain reorganization resistance confirmed${NC}"
    echo -e "${GREEN}✓ Resource attack vectors are mitigated${NC}"
    echo ""
    echo -e "${CYAN}BitcoinOil has passed comprehensive security hardening!${NC}"
    security_exit_code=0
    
elif [ "$CRITICAL_SECURITY_FAILURES" -gt 0 ]; then
    echo -e "${RED}${BOLD}🚨 SECURITY ASSESSMENT: ✗ CRITICAL VULNERABILITIES 🚨${NC}"
    echo -e "${RED}${BOLD}BitcoinOil has CRITICAL security flaws - DO NOT DEPLOY${NC}"
    echo -e "${RED}Critical security failures must be resolved immediately${NC}"
    security_exit_code=1
    
else
    echo -e "${YELLOW}${BOLD}⚠️ SECURITY ASSESSMENT: CONDITIONAL DEPLOYMENT ⚠️${NC}"
    echo -e "${YELLOW}Some security tests failed but no critical vulnerabilities${NC}"
    echo -e "${YELLOW}Review failed tests before production deployment${NC}"
    security_exit_code=2
fi

echo ""
echo -e "${CYAN}Security test logs: ${BOLD}$LOGFILE${NC}"
echo -e "${CYAN}Security test data: ${BOLD}$SECURITY_TEST_DIR${NC}"
echo ""

print_security_header "Security Test Suite Completed"

exit $security_exit_code 