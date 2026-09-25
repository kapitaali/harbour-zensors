#!/usr/bin/env bash
# Rename the template app: ./scripts/rename.sh harbour-oldapp harbour-newapp
# Mass-renames every occurrence of the old harbour-* identifier.

set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "Usage: $0 <old-name> <new-name>" >&2
    echo "  both names must start with 'harbour-'" >&2
    exit 1
fi

OLD="$1"
NEW="$2"

if [[ "$OLD" != harbour-* || "$NEW" != harbour-* ]]; then
    echo "Error: both names must start with 'harbour-'" >&2
    exit 1
fi

cd "$(dirname "$0")/.."

echo "Renaming $OLD -> $NEW ..."

# Rename files and directories
find . -depth -name "*${OLD}*" | while read -r path; do
    newpath="${path//"$OLD"/$NEW}"
    echo "  mv $path -> $newpath"
    mv "$path" "$newpath"
done

# Rename content in text files
find . -type f \( -name "*.pro" -o -name "*.qml" -o -name "*.cpp" -o -name "*.h" \
    -o -name "*.spec" -o -name "*.desktop" -o -name "*.ts" -o -name "*.xml" \
    -o -name "*.changes" -o -name "*.changes.run" -o -name "*.sh" -o -name "*.md" \) \
    | while read -r file; do
    if grep -q "$OLD" "$file" 2>/dev/null; then
        echo "  sed $file"
        sed -i "s/${OLD}/${NEW}/g" "$file"
    fi
done

# Derive display name (harbour-zensors -> Myapp) for Name=/Exec hints
echo
echo "Done. Remember to update:"
echo "  - harbour-zensors.desktop: Name=, OrganizationName, ApplicationName"
echo "  - rpm spec: Summary, URL, %description"
echo "  - translations display strings (qsTr)"
