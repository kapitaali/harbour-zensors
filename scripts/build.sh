#!/usr/bin/env bash
# Build (and optionally sign) the Sailfish OS RPM package with sfdk.
#
# Usage:
#   ./scripts/build.sh              # normal build
#   ./scripts/build.sh --sign       # signed package (requires sfdk signing config)
#
# Signing setup (once):
#   sfdk config --global --push package.signing.user "Full Name"
#   sfdk config --global --push package.signing-passphrase-file "$HOME/passphrase.txt"
#
# Verify afterwards:
#   rpm -K RPMS/harbour-myapp-*.rpm

set -euo pipefail
cd "$(dirname "$0")/.."

sfdk build "$@"

echo
echo "Packages in RPMS/:"
ls -l RPMS/ 2>/dev/null || echo "(no RPMS directory - build may use a different output path)"
