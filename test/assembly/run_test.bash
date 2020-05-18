#!/bin/bash

TEST=$1
EXPECTED_OUTPUT=$2

OUTPUT=$($TEST)
if [[ "$OUTPUT" == "$EXPECTED_OUTPUT" ]]; then
  echo "Pass!"
else
  echo "Failure:"
  echo "   Expected $EXPECTED_OUTPUT"
  echo "   Got $OUTPUT"
fi
