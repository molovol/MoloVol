#!/bin/bash

# CLI Test Suite for MoloVol
# This script tests the CLI interface functionality and can be used to compare
# behavior between different branches (e.g., GUI vs non-GUI implementations)

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test configuration
MOLOVOL_EXEC="./MoloVol"
TEST_STRUCTURE="test_structure.pdb"
OUTPUT_DIR="test_output"
RESULTS_FILE="cli_test_results.txt"

# Initialize test results
echo "MoloVol CLI Test Results - $(date)" > $RESULTS_FILE
echo "========================================" >> $RESULTS_FILE
echo "" >> $RESULTS_FILE

# Function to run a test and capture output
run_test() {
    local test_name="$1"
    local command="$2"
    local expected_exit_code="$3"
    
    echo -e "${YELLOW}Running test: $test_name${NC}"
    echo "Test: $test_name" >> $RESULTS_FILE
    echo "Command: $command" >> $RESULTS_FILE
    
    # Create output directory if needed
    mkdir -p $OUTPUT_DIR
    
    # Run the command and capture output and exit code
    set +e  # Don't exit on error for individual tests
    output=$(eval "$command" 2>&1)
    actual_exit_code=$?
    set -e
    
    echo "Exit Code: $actual_exit_code" >> $RESULTS_FILE
    echo "Output:" >> $RESULTS_FILE
    echo "$output" >> $RESULTS_FILE
    echo "----------------------------------------" >> $RESULTS_FILE
    echo "" >> $RESULTS_FILE
    
    # Check if test passed
    if [ "$actual_exit_code" -eq "$expected_exit_code" ]; then
        echo -e "${GREEN}✓ PASSED${NC}"
        return 0
    else
        echo -e "${RED}✗ FAILED (expected exit code $expected_exit_code, got $actual_exit_code)${NC}"
        return 1
    fi
}

# Function to check if executable exists
check_executable() {
    if [ ! -f "$MOLOVOL_EXEC" ]; then
        echo -e "${RED}Error: $MOLOVOL_EXEC not found!${NC}"
        echo "Please ensure MoloVol is compiled in the current directory."
        exit 1
    fi
    
    if [ ! -x "$MOLOVOL_EXEC" ]; then
        echo -e "${RED}Error: $MOLOVOL_EXEC is not executable!${NC}"
        exit 1
    fi
}

# Function to check if test structure exists
check_test_structure() {
    if [ ! -f "$TEST_STRUCTURE" ]; then
        echo -e "${RED}Error: Test structure file $TEST_STRUCTURE not found!${NC}"
        echo "Please ensure the test structure file exists."
        exit 1
    fi
}

echo -e "${YELLOW}MoloVol CLI Test Suite${NC}"
echo "======================="
echo ""

# Pre-flight checks
check_executable
check_test_structure

# Clean up any previous test outputs
rm -rf $OUTPUT_DIR
rm -f $RESULTS_FILE

echo "Executable: $MOLOVOL_EXEC" >> $RESULTS_FILE
echo "Test Structure: $TEST_STRUCTURE" >> $RESULTS_FILE
echo "Working Directory: $(pwd)" >> $RESULTS_FILE
echo "" >> $RESULTS_FILE

# Test counter
passed_tests=0
total_tests=0

# Test 1: Help command
total_tests=$((total_tests + 1))
if run_test "Help Command" "$MOLOVOL_EXEC --help" 0; then
    passed_tests=$((passed_tests + 1))
fi

# Test 2: Version command
total_tests=$((total_tests + 1))
if run_test "Version Command" "$MOLOVOL_EXEC --version" 0; then
    passed_tests=$((passed_tests + 1))
fi

# Test 3: Basic calculation with minimal parameters
total_tests=$((total_tests + 1))
if run_test "Basic Calculation" "$MOLOVOL_EXEC -r 1.4 -g 0.5 -fs $TEST_STRUCTURE -o none" 0; then
    passed_tests=$((passed_tests + 1))
fi

# Test 4: Calculation with output directory
total_tests=$((total_tests + 1))
if run_test "Calculation with Output Dir" "$MOLOVOL_EXEC -r 1.4 -g 0.5 -fs $TEST_STRUCTURE -do $OUTPUT_DIR -o none" 0; then
    passed_tests=$((passed_tests + 1))
fi

# Test 5: Two-probe mode
total_tests=$((total_tests + 1))
if run_test "Two-Probe Mode" "$MOLOVOL_EXEC -r 1.4 -r2 2.8 -g 0.5 -fs $TEST_STRUCTURE -o none" 0; then
    passed_tests=$((passed_tests + 1))
fi

# Test 6: Include HETATM
total_tests=$((total_tests + 1))
if run_test "Include HETATM" "$MOLOVOL_EXEC -r 1.4 -g 0.5 -fs $TEST_STRUCTURE -ht -o none" 0; then
    passed_tests=$((passed_tests + 1))
fi

# Test 7: Calculate surfaces
total_tests=$((total_tests + 1))
if run_test "Calculate Surfaces" "$MOLOVOL_EXEC -r 1.4 -g 0.5 -fs $TEST_STRUCTURE -sf -o none" 0; then
    passed_tests=$((passed_tests + 1))
fi

# Test 8: Export report
total_tests=$((total_tests + 1))
if run_test "Export Report" "$MOLOVOL_EXEC -r 1.4 -g 0.5 -fs $TEST_STRUCTURE -do $OUTPUT_DIR -xr -o none" 0; then
    passed_tests=$((passed_tests + 1))
fi

# Test 9: Quiet mode
total_tests=$((total_tests + 1))
if run_test "Quiet Mode" "$MOLOVOL_EXEC -r 1.4 -g 0.5 -fs $TEST_STRUCTURE -q -o none" 0; then
    passed_tests=$((passed_tests + 1))
fi

# Test 10: Different output options
total_tests=$((total_tests + 1))
if run_test "Output Options - Volume Only" "$MOLOVOL_EXEC -r 1.4 -g 0.5 -fs $TEST_STRUCTURE -o vol" 0; then
    passed_tests=$((passed_tests + 1))
fi

# Test 11: Unicode output
total_tests=$((total_tests + 1))
if run_test "Unicode Output" "$MOLOVOL_EXEC -r 1.4 -g 0.5 -fs $TEST_STRUCTURE -un -o vol" 0; then
    passed_tests=$((passed_tests + 1))
fi

# Test 12: Error case - missing required parameter
total_tests=$((total_tests + 1))
if run_test "Missing Required Parameter" "$MOLOVOL_EXEC -r 1.4 -fs $TEST_STRUCTURE" 1; then
    passed_tests=$((passed_tests + 1))
fi

# Test 13: Error case - invalid file
total_tests=$((total_tests + 1))
if run_test "Invalid Structure File" "$MOLOVOL_EXEC -r 1.4 -g 0.5 -fs nonexistent_file.pdb -o none" 0; then
    passed_tests=$((passed_tests + 1))
fi

# Test 14: Complex calculation with most options (excluding unit cell analysis for simple test structure)
total_tests=$((total_tests + 1))
if run_test "Complex Calculation" "$MOLOVOL_EXEC -r 1.4 -r2 2.8 -g 0.3 -d 8 -fs $TEST_STRUCTURE -do $OUTPUT_DIR -ht -sf -xr -xt -xc -o all -un" 0; then
    passed_tests=$((passed_tests + 1))
fi

echo ""
echo "==============================="
echo -e "${YELLOW}Test Summary${NC}"
echo "==============================="
echo "Tests passed: $passed_tests/$total_tests"

if [ "$passed_tests" -eq "$total_tests" ]; then
    echo -e "${GREEN}All tests passed! ✓${NC}"
    echo ""
    echo "Test Summary: $passed_tests/$total_tests tests passed" >> $RESULTS_FILE
    echo "Overall Result: PASSED" >> $RESULTS_FILE
    exit 0
else
    echo -e "${RED}Some tests failed! ✗${NC}"
    echo ""
    echo "Test Summary: $passed_tests/$total_tests tests passed" >> $RESULTS_FILE
    echo "Overall Result: FAILED" >> $RESULTS_FILE
    exit 1
fi