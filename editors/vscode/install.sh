#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXT_DIR="$SCRIPT_DIR/tmake-lang"

# check if code command is available
if ! command -v code &> /dev/null; then
    echo "error: 'code' command not found. make sure VS Code is installed and 'code' is in your PATH."
    exit 1
fi

# check if npm is available (needed to compile the extension)
if ! command -v npm &> /dev/null; then
    echo "error: 'npm' command not found. make sure Node.js and npm are installed."
    exit 1
fi

echo "installing tmake VS Code extension..."

# install dependencies
cd "$EXT_DIR" || exit 1
npm install

# compile typescript
npm run compile

# install the extension by symlinking into the VS Code extensions directory
VSCODE_EXT_DIR="$HOME/.vscode/extensions/tmake.tmake-lang-0.1.0"

if [ -d "$VSCODE_EXT_DIR" ] || [ -L "$VSCODE_EXT_DIR" ]; then
    echo "removing existing installation..."
    rm -rf "$VSCODE_EXT_DIR"
fi

ln -s "$EXT_DIR" "$VSCODE_EXT_DIR"

echo "done! restart VS Code or reload the window to activate the extension."
