#!/bin/bash

# Marshall Asch, Feburary 26th, 2020
#
# This script is a wrapper arround clang-format that will reformat all of the
# c++ files to maintain a consistant code style.
#

files=$(ls -1 *.h *.cc)

if ! command -v clang-format >/dev/null;
then
    echo "Clang format is not installed, can not run"
    exit 1
fi

while IFS= read -r file
do
    clang-format -style=file "$file" > .temp.format
    cat .temp.format > "$file"
done <<< "$files"

rm .temp.format
