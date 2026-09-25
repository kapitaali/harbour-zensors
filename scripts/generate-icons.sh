#!/usr/bin/env bash
# Generate placeholder app icons for the sizes Sailfish OS expects.
# SAILFISHAPP_ICONS in the .pro file lists: 86x86 108x108 128x128 172x172
#
# Requires ImageMagick. Replace these placeholders with real artwork
# before submitting to the Harbour.

set -euo pipefail
cd "$(dirname "$0")/.."

NAME="harbour-zensors"
SIZES=(86 108 128 172)

if ! command -v magick >/dev/null 2>&1 && ! command -v convert >/dev/null 2>&1; then
    echo "Error: ImageMagick is required (magick or convert)." >&2
    exit 1
fi

IM="magick"
command -v magick >/dev/null 2>&1 || IM="convert"

# Simple gradient square with a circle, per size
for size in "${SIZES[@]}"; do
    dir="icons/${size}x${size}"
    mkdir -p "$dir"
    echo "Generating $dir/$NAME.png (${size}x${size})"
    $IM -size "${size}x${size}" \
        gradient:'#1a5276-#2e86c1' \
        -depth 8 \
        -fill white -gravity center \
        -pointsize $((size / 5)) -annotate +0+0 "app" \
        "$dir/$NAME.png"
done

echo
echo "Done. Overwrite these placeholders with final artwork:"
for size in "${SIZES[@]}"; do
    echo "  icons/${size}x${size}/$NAME.png"
done
