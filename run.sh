#!/bin/bash

# Small helper script to build/run/debug the kurai-webserver
# Usage examples:
#   ./run.sh            # build Release and run (default)
#   ./run.sh -b         # build (default Release)
#   ./run.sh -b Debug   # build with specified type (Debug or Release)
#   ./run.sh -r         # only run (no build)
#   ./run.sh -d         # clean, build Debug, and run under gdb

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$ROOT_DIR/build_linux"
BIN_PATH="$BUILD_DIR/core/kurai-webserver"

export CC=gcc
export CXX=g++

usage() {
	cat <<EOF
Usage: $0 [options]

Options:
  -b [Release|Debug]   Build the project. Optionally provide build type (default: Release).
  -r                   Run the built application (no build).
  -d                   Debug: remove build directory, build Debug, then run under gdb.
  -h, --help           Show this help and exit.

No flags: build Release and run the application.
You can combine flags (for example: -b Debug -r will build Debug then run).
EOF
	exit 0
}

ACTION_BUILD=0
ACTION_RUN=0
ACTION_DEBUG=0
BUILD_TYPE="Release"

# Simple arg parsing: allow -b with optional build type as next token.
while [[ $# -gt 0 ]]; do
	case "$1" in
		-b|--build)
			ACTION_BUILD=1
			if [[ -n "${2-}" && ${2:0:1} != "-" ]]; then
				BUILD_TYPE="$2"
				shift
			else
				BUILD_TYPE="Release"
			fi
			shift
			;;
		-r|--run)
			ACTION_RUN=1
			shift
			;;
		-d|--debug)
			ACTION_DEBUG=1
			shift
			;;
		-h|--help)
			usage
			;;
		*)
			echo "Unknown arg: $1" >&2
			usage
			;;
	esac
done

# Default behavior: no flags => build Release and run
if [[ $ACTION_BUILD -eq 0 && $ACTION_RUN -eq 0 && $ACTION_DEBUG -eq 0 ]]; then
	ACTION_BUILD=1
	ACTION_RUN=1
	BUILD_TYPE="Release"
fi

build_app() {
	local bt="$1"
	echo "[run.sh] Building (type=$bt) ..."
	mkdir -p "$BUILD_DIR"
	pushd "$BUILD_DIR" >/dev/null
	cmake -G "Ninja" -DCMAKE_BUILD_TYPE="$bt" ..
	ninja
	popd >/dev/null
	echo "[run.sh] Build finished."
}

run_app() {
	if [[ ! -x "$BIN_PATH" ]]; then
		echo "Executable not found: $BIN_PATH" >&2
		echo "You may need to build first (use -b)." >&2
		return 1
	fi
	echo "[run.sh] Running $BIN_PATH"
	"$BIN_PATH"
}

debug_app() {
	echo "[run.sh] Debug flow: removing $BUILD_DIR and building Debug..."
	rm -rf "$BUILD_DIR"
	build_app Debug
	if [[ ! -x "$BIN_PATH" ]]; then
		echo "Executable not found after debug build: $BIN_PATH" >&2
		return 1
	fi
	echo "[run.sh] Launching gdb: $BIN_PATH"
	# Run gdb with the binary
	gdb --args "$BIN_PATH"
}

# Execute requested actions
if [[ $ACTION_DEBUG -eq 1 ]]; then
	debug_app
	exit $?
fi

if [[ $ACTION_BUILD -eq 1 ]]; then
	build_app "$BUILD_TYPE"
fi

if [[ $ACTION_RUN -eq 1 ]]; then
	run_app
fi
