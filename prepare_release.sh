#!/bin/sh
set -e
# set -x

read_char() {
  stty -icanon -echo
  eval "$1=\$(dd bs=1 count=1 2>/dev/null)"
  stty icanon echo
}


rm -f ./libsession_util_nodejs*.tar.gz
virtualenv venv
. venv/bin/activate
pip install git-archive-all
PACKAGE_VERSION=$(node -p "require('./package.json').version")
yarn update_version
echo "PACKAGE_VERSION: $PACKAGE_VERSION"
echo "Is '$PACKAGE_VERSION' the correct version? If yes, press 'y' to create the release. Press anything else to exit."
read_char char_read
case "$char_read" in
    y) break ;;
    *) echo "Exiting..."; exit 1 ;;
esac


echo "Continuing..."
# see test.yml for explanation of this sed command
yarn dirty_sed

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
echo "yarn remove libsession_util_nodejs --ignore-scripts; yarn add https://github.com/session-foundation/libsession-util-nodejs/releases/download/v$PACKAGE_VERSION/libsession_util_nodejs-v$PACKAGE_VERSION.tar.gz"
echo "Done"
