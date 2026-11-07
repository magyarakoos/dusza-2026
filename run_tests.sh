#!/bin/bash

set -e

mkdir -p results

for input in test/input_*.txt; do
  # Skip if no matching files exist
  [ -f "$input" ] || continue

  # Extract filename without extension
  filename=$(basename "$input" .txt)

  # Construct output filename
  output="results/${filename/input_/output_}.txt"

  echo "Running $input -> $output"

  ./run.sh -q "$input" "$output"
done

echo "All tests completed."
