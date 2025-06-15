#!/bin/bash
set -e

XT_TEE=$1
echo "hello" | "$XT_TEE" > output.txt

if grep -q "hello" output.txt; then
    echo "Test passed"
    exit 0
else
    echo "Test failed"
    exit 1
fi
