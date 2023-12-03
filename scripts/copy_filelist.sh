#!/bin/bash
script_path=$(realpath "$0")
script_dir=$(dirname "$script_path")
ls -l $script_dir
sed -i 's/\r$//' "$script_dir/filelist.txt"
cat "$script_dir/filelist.txt" | while read file; do
	echo "check $PWD/$file"
	target_dir=$(dirname "$PWD/$file")

	if [ ! -d "$target_dir" ]; then
		mkdir -p "$target_dir"
	fi
	if [ ! -f "$PWD/$file" ]; then
		echo "check /$1/bin/$file"
		ls -l "/$1/bin/$file"
		if [ -f "/$1/bin/$file" ]; then
			echo "cp /$1/bin/$file $PWD/$file"
			cp "/$1/bin/$file" "$PWD/$file"
		fi
	fi
	if [ ! -f "$PWD/$file" ]; then
		echo "check /$1/share/qt6/$file"
		ls -l "/$1/share/qt6/$file"
		if [ -f "/$1/share/qt6/$file" ]; then
			echo "cp /$1/share/qt6/$file $PWD/$file"
			cp "/$1/share/qt6/$file" "$PWD/$file"
		fi
	fi
done
