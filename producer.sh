#!/usr/bin/env bash

# Andre Augusto Giannotti Scota (https://sites.google.com/view/a2gs/)

# Script exit if a command fails:
#set -e

# Script exit if a referenced variable is not declared:
#set -u

# If one command in a pipeline fails, its exit code will be returned as the result of the whole pipeline:
#set -o pipefail

# Activate tracing:
#set -x

if [ -z "$RANDOM" ]; then
	echo "Sh RANDOM variable doesnt exist."
	exit 1
fi

if [ "$#" -eq 2 ]; then

	while [ true ];
	do
		if [ ! -p "$1" ]; then
			echo "EXIT namedpipe [$1] with message [$2] (namedpipe doesnt exist)"
			exit 1;
		fi

		echo "$2" > "$1"
		sleep $(( RANDOM % 6 ))
	done

else
	echo "Usage: $0 [NAMEDPIPE_FILE] [MSG]"
fi
