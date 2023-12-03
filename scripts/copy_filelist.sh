#!/bin/bash
script_path=$(realpath "$0")
script_dir=$(dirname "$script_path")
ls -l $script_dir
sed -i 's/\r$//' "$script_dir/filelist.txt"
cat "$script_dir/filelist.txt" | while read file; do
	target_dir=$(dirname "$PWD/$file")

	if [ ! -d "$target_dir" ]; then
		mkdir -p "$target_dir"
	fi
	if [ ! -f "$PWD/$file" ]; then
		if [ -f "/$1/bin/$file" ]; then
			cp "/$1/bin/$file" "$PWD/$file"
		fi
	fi
	if [ ! -f "$PWD/$file" ]; then
		if [ -f "/$1/share/qt6/$file" ]; then
			cp "/$1/share/qt6/$file" "$PWD/$file"
		fi
	fi
done
