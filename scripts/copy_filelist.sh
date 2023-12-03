#!/bin/bash
script_path=$(realpath "$0")
script_dir=$(dirname "$script_path")
cat "$script_dir/filelist.txt" | while read file; do
    target_dir=$(dirname "$file")

    if [ ! -d "$target_dir" ]; then
        mkdir -p "$target_dir"
    fi
    if [ ! -f "$file" ]; then
        if [ -f "/$1/bin/$file" ]; then echo "cp /$1/bin/$file $PWD/$file"; cp "/$1/bin/$file" "$PWD/$file"; fi    
    fi
    if [ ! -f "$file" ]; then
        if [ -f "/$1/share/qt6/$file" ]; then echo "cp /$1/share/qt6/$file $PWD/$file"; cp "/$1/share/qt6/$file" "$PWD/$file"; fi
    fi
done