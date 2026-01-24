#!/usr/bin/env sh
# Create a simple test patch ZIP for the updater and print the SHA256.
# Run from repository root: ./tools/patches/create_test_patch.sh

set -e

PATCH_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$PATCH_DIR"

# Create content to include in the patch
cat > README_patch.txt <<'EOF'
Test patch for Depth of Dejenol
Version: 1.0.1-test

This file is included in the test patch used by the updater.
EOF

# Create zip (overwrite if exists)
if command -v zip >/dev/null 2>&1; then
  zip -r -FS test_patch.zip README_patch.txt > /dev/null
else
  # Fallback using python if zip is not available
  python3 - <<PY
import zipfile
with zipfile.ZipFile('test_patch.zip', 'w', compresslevel=9) as zf:
    zf.write('README_patch.txt')
PY
fi

# Show resulting file and SHA256
if command -v sha256sum >/dev/null 2>&1; then
  sha256sum test_patch.zip
elif command -v shasum >/dev/null 2>&1; then
  shasum -a 256 test_patch.zip
else
  python3 - <<PY
import hashlib
with open('test_patch.zip','rb') as f:
    h = hashlib.sha256(f.read()).hexdigest()
print(h + '  test_patch.zip')
PY
fi

echo "Created: $PATCH_DIR/test_patch.zip"
