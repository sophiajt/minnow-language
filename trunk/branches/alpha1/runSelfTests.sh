#!/bin/bash
for filename in $( ls samples/*.mno); do
    output=${filename/.mno/.out}
    echo "Compiling... $filename"
    ./minnow $filename -o $output
done
