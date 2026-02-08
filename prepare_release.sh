#!/bin/sh
set -e
# set -x

read_char() {
  stty -icanon -echo
  eval "$1=\$(dd bs=1 count=1 2>/dev/null)"
  stty icanon echo
}


rm -f ./libsession_util_nodejs*.tar.gz
python -m venv .venv
. .venv/bin/activate
pip install git-archive-all

PACKAGE_VERSION=$(node -p "require('./package.json').version")
GIT_COMMIT=$(git rev-parse HEAD)

HEADER_PACKAGE_VERSION=$(grep 'LIBSESSION_NODEJS_VERSION' src/version.h | sed -E 's/.*"([0-9.]+)".*/\1/')
HEADER_GIT_COMMIT=$(grep 'LIBSESSION_NODEJS_COMMIT' src/version.h | sed -E 's/.*"([A-Za-z0-9.]+)".*/\1/')

echo "Package: $PACKAGE_VERSION; Commit: $GIT_COMMIT"
if [ "$PACKAGE_VERSION" != "$HEADER_PACKAGE_VERSION" ]; then
    echo "Error: Version mismatch! package.json version is $PACKAGE_VERSION, but src/version.h has $HEADER_PACKAGE_VERSION. Build the project first before packaging 'pnpm install'"
    exit 1
fi

if [ "$GIT_COMMIT" != "$HEADER_GIT_COMMIT" ]; then
    echo "Error: Version mismatch! Git commit is $GIT_COMMIT, but src/version.h has $HEADER_GIT_COMMIT. Build the project first before packaging 'pnpm install'"
    exit 1
fi

echo "Is '$PACKAGE_VERSION' the correct version? If yes, press 'y' to create the release. Press anything else to exit."
read_char char_read
case "$char_read" in
    y) break ;;
    *) echo "Exiting..."; exit 1 ;;
esac


echo "Continuing..."

echo "Building tar archive of source..."
python3 build_release_archive.py libsession_util_nodejs-v$PACKAGE_VERSION.tar.gz --include src/version.h

echo "tar archive size:"
du -sh libsession_util_nodejs*.tar.gz

echo "Creating draft release on github $PACKAGE_VERSION..."
GH_RELEASE_URL=$(gh release create v$PACKAGE_VERSION -t v$PACKAGE_VERSION --latest --generate-notes)
echo "Uploading tar archive to release $PACKAGE_VERSION..."
gh release upload v$PACKAGE_VERSION libsession_util_nodejs-v$PACKAGE_VERSION.tar.gz
echo "GH_RELEASE_URL: $GH_RELEASE_URL"
echo "Update session-desktop with this new libsession-nodejs version with: "
echo "pnpm remove libsession_util_nodejs --ignore-scripts; pnpm add https://github.com/session-foundation/libsession-util-nodejs/releases/download/v$PACKAGE_VERSION/libsession_util_nodejs-v$PACKAGE_VERSION.tar.gz"
echo "Done"
