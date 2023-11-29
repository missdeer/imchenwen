#!/bin/bash

location="https://iina.io/dylibs/universal"
IFS=$'\n' read -r -d '' -a files < <(curl "${location}/filelist.txt" && printf '\0')
for file in "${files[@]}"; do
	set -x
	curl -L "${location}/${file}" -o libmpv/$file
	{ set +x; } 2>/dev/null
	install_name_tool -add_rpath @executable_path/../Libs libmpv/$file
	install_name_tool -add_rpath $PWD/libmpv libmpv/$file
done

ln -sf $PWD/libmpv/libmpv.2.dylib $PWD/libmpv/libmpv.dylib
