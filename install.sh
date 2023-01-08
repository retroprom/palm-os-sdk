#!/bin/bash

set -e

if [ -z "$1" ]; then
	echo "Usage: $0 <destdir>"
	exit 1
fi

SDKS="$(dirname "$(realpath $0)")"

mkdir -p "$1/m68k-palmos/lib/pkgconfig"
for pc in pkgconfig/*.pc.in; do
	(echo "prefix=${SDKS}"; cat "$pc") > "$1/m68k-palmos/lib/pkgconfig/$(basename -s .in "$pc")"
done

