#!/bin/bash

# Check if a file name is provided as an argument
if [ -z "$1" ]; then
  echo "Usage: $0 <filename.txt>"
  exit 1
fi

# Assign the first argument to a variable
input_file="$1"

# Check if the file exists
if [ ! -f "$input_file" ]; then
  echo "File not found: $input_file"
  exit 1
fi

# Run make clean and make
make clean > /dev/null && make > /dev/null

# Run the encoder and decoder
./encoder "$input_file"
./decoder "$input_file.comp"

# Compare the original and decompressed files
diff "$input_file" "$input_file.comp.txt"

# # Check the diff result
# if [ $? -eq 0 ]; then
#   echo "The files are identical."
# else
#   echo "The files differ."
# fi
