#!/usr/bin/env bash
# Render the Zensors icon for the sizes Sailfish OS expects.
# SAILFISHAPP_ICONS in the .pro file lists: 86x86 108x108 128x128 172x172
#
# Artwork lives in icons/zensors.svg and is produced by scripts/make-icon.py,
# which is the place to change the mark. This script only rasterises it:
# the SVG is rendered once at 4x and downsampled, so the small sizes stay
# smooth instead of being drawn straight from the path data.
#
# Requires ImageMagick and python3.

set -euo pipefail
cd "$(dirname "$0")/.."

NAME="harbour-zensors"
SIZES=(86 108 128 172)
MASTER=1024

if ! command -v magick >/dev/null 2>&1 && ! command -v convert >/dev/null 2>&1; then
    echo "Error: ImageMagick is required (magick or convert)." >&2
    exit 1
fi
command -v python3 >/dev/null 2>&1 || { echo "Error: python3 is required." >&2; exit 1; }

IM="magick"
command -v magick >/dev/null 2>&1 || IM="convert"

echo "Generating icons/zensors.svg"
python3 scripts/make-icon.py icons/zensors.svg

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

# Render big once; ImageMagick's own SVG renderer antialiases as it scales.
echo "Rendering master at ${MASTER}x${MASTER}"
$IM -background none icons/zensors.svg -resize "${MASTER}x${MASTER}!" "$TMP/master.png"

for size in "${SIZES[@]}"; do
    dir="icons/${size}x${size}"
    mkdir -p "$dir"
    out="$dir/$NAME.png"
    $IM "$TMP/master.png" -filter Lanczos -resize "${size}x${size}" -depth 8 "$out"
    # The launcher expects opaque squares; alpha would show through as black.
    $IM "$out" -alpha off -quality 95 "$out.tmp" && mv "$out.tmp" "$out"
    printf "  %-34s %s\n" "$out" "$(identify -format '%wx%h' "$out")"
done

echo
echo "Done. Preview all four at once:"
echo "  magick icons/86x86/$NAME.png icons/108x108/$NAME.png icons/128x128/$NAME.png icons/172x172/$NAME.png +append icons/preview.png"
