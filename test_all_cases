#!/bin/bash

TEST_DIRS=("tests/task1" "tests/task2" "tests/task3" "tests/task4")

# Initialize counters
total_tests=0
passed_tests=0
failed_tests=0
failed_tests_list=()

# Loop through each test directory
for dir in "${TEST_DIRS[@]}"; do
    echo "Running tests in $dir"

    # Find all .bin files in the current test directory
    for bin_file in "$dir"/*.bin; do
        # Extract the base name (e.g., "t1" from "t1.bin")
        test_name=$(basename "$bin_file" .bin)
        res_file="$dir/$test_name.res"

        # Run the test with the simulator
        echo "Running test: $test_name"
        ./riscv_sim "$bin_file"

        # Check if the output matches the expected result
        if diff -q output.bin "$res_file" > /dev/null; then
            echo "✅ Test passed: $test_name"
            passed_tests=$((passed_tests + 1))
        else
            echo "❌ Test failed: $test_name"
            failed_tests=$((failed_tests + 1))
            failed_tests_list+=("$dir/$test_name")
        fi

        # Increment total test counter
        total_tests=$((total_tests + 1))
    done
done

# Print summary
echo "===================================="
echo "Test Summary:"
echo "Total tests: $total_tests"
echo "Passed tests: $passed_tests"
echo "Failed tests: $failed_tests"
echo "===================================="

# List failed tests
if [ $failed_tests -gt 0 ]; then
    echo "Failed Tests:"
    for test in "${failed_tests_list[@]}"; do
        echo "❌ $test"
    done
else
    echo "All tests passed! 🎉"
fi

# Exit with the number of failed tests as the exit code
exit $failed_tests
