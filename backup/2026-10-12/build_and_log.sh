#!/bin/bash

LOG_FILE="compile_times.csv"
TIMESTAMP=$(date +"%Y-%m-%d %H:%M:%S")
GIT_REV=$(git rev-parse --short HEAD 2>/dev/null || echo "N/A")

if [ ! -f "$LOG_FILE" ]; then
    echo "timestamp,revision,stats" > "$LOG_FILE"
fi

echo "Cleaning..."
make clean > /dev/null 2>&1

echo "Starting build: make -j 10"

# We use 'time' on the whole block. 
# POSIX 'time -p' gives a predictable 3-line output:
# real 0.00
# user 0.00
# sys 0.00
{ time -p make -j 10 ; } 2> temp_time.txt

# Extract the numbers and format them into a single CSV line
STATS=$(awk '{print $2}' temp_time.txt | paste -sd "," -)
echo "$TIMESTAMP,$GIT_REV,$STATS" >> "$LOG_FILE"

rm temp_time.txt
echo "Build complete. Data appended to $LOG_FILE"
