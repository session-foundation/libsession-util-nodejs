#!/bin/sh
set -e
# set -x

VERSION_FILE="src/version.ts"
rm $VERSION_FILE
export LIBSESSION_NODEJS_VERSION=$(node -p "require('./package.json').version")
# sadly, this is most likely only working on unix
LIBSESSION_NODEJS_COMMIT_SHA=$(git rev-parse HEAD)

echo "Updating LIBSESSION_NODEJS_VERSION: $LIBSESSION_NODEJS_VERSION"
echo "Updating LIBSESSION_NODEJS_COMMIT_SHA: $LIBSESSION_NODEJS_COMMIT_SHA"

echo "export const LIBSESSION_NODEJS_VERSION = '$LIBSESSION_NODEJS_VERSION';" > $VERSION_FILE
echo "export const LIBSESSION_NODEJS_COMMIT_SHA = '$LIBSESSION_NODEJS_COMMIT_SHA';" >> $VERSION_FILE

echo "\n'$VERSION_FILE' updated to:"
cat $VERSION_FILE

