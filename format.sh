#!/bin/bash

declare -a PATHS=(
	"./server/src/"
	"./imgui/src/"
	"./shared/messages.hpp"
)

declare -a IGNORE_PATHS=(
	"./imgui/src/implot"
)

build_exclude_conditions() {
	local conditions=""
	for ignore_path in "${IGNORE_PATHS[@]}"; do
		conditions+=" -path '$ignore_path' -prune -o"
	done
	# Remove trailing ' -o'
	echo "${conditions% -o}"
}

find_files() {
	local path exclude_conditions
	exclude_conditions=$(build_exclude_conditions)
	local files=""

	for path in "${PATHS[@]}"; do
		if [ -d "$path" ]; then
			files+=$(find "$path" $exclude_conditions -type f \( -iname "*.hpp" -o -iname "*.h" -o -iname "*.c" -o -iname "*.cpp" \) -print)" "
		elif [ -f "$path" ]; then
			files+="$path "
		else
			echo "Warning: '$path' is not a valid file or directory"
		fi
	done
	echo "$files"
}

format_files() {
	local files="$1"
	for file in $files; do
		clang-format -i -style=file "$file"
	done
}

files=$(find_files)
format_files "$files"
