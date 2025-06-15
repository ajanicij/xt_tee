#!/usr/bin/env bash

set -e

# === CONFIGURATION ===
BINARY_NAME="xt_tee"
SRC_FILE="xt_tee.c"
MANPAGE_FILE="xt_tee.1"
VERSION="1.0.3"
INSTALL_PREFIX="/usr/local"

# === BUILD ===
echo "[1/4] Compiling $SRC_FILE → $BINARY_NAME..."

GIT_COMMIT=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
BUILD_DATE=$(date +%Y-%m-%d)

gcc -Wall -O2 \
  -D XT_TEE_VERSION="\"$VERSION\"" \
  -D XT_TEE_COMMIT="\"git-$GIT_COMMIT\"" \
  -D XT_TEE_BUILD_DATE="\"$BUILD_DATE\"" \
  -o "$BINARY_NAME" "$SRC_FILE"

echo "[2/4] Installing binary to $INSTALL_PREFIX/bin..."
sudo install -m 755 "$BINARY_NAME" "$INSTALL_PREFIX/bin/$BINARY_NAME"

# === MANPAGE INSTALL ===
echo "[3/4] Installing man page to $INSTALL_PREFIX/share/man/man1..."
sudo install -m 644 "$MANPAGE_FILE" "$INSTALL_PREFIX/share/man/man1/$BINARY_NAME.1"

echo "[4/4] Updating man database..."
sudo mandb >/dev/null

# === VERIFY ===
echo ""
echo "✅ Installation complete!"
echo "   Binary installed: $(command -v $BINARY_NAME)"
echo "   Man page available via: man $BINARY_NAME"
echo ""

# Optional test run
"$BINARY_NAME" --version
