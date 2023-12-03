#!/bin/bash
script_path=$(realpath "$0")
script_dir=$(dirname "$script_path")
cat "$script_dir/filelist.txt" | while read file; do
    target_dir=$(dirname "$file")

    if [ ! -d "$target_dir" ]; then
        mkdir -p "$target_dir"
    fi
    if [ ! -f "$file" ]; then
        if [ -f "/$1/bin/$file" ]; then cp "/$1/bin/$file" "$file"; fi    
    fi
    if [ ! -f "$file" ]; then
        if [ -f "/$1/share/qt6/$file" ]; then cp "/$1/share/qt6/$file" "$file"; fi
    fi
done