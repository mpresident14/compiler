#!/bin/bash

TEST="$1"
EXPECTED_OUTPUT=$(echo "$2" | sed 's/,/\n/g')

OUTPUT=$($TEST)
if [[ "$OUTPUT" == "$EXPECTED_OUTPUT" ]]; then
  echo "Pass!"
  exit(0)
else
  echo "Failure..."
  echo "Expected:"
  echo "$EXPECTED_OUTPUT"
  echo
  echo "Got"
  echo "$OUTPUT"
  exit(1)
fi
