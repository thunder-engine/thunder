#!/usr/bin/env bash

export QT_INSTALL_DIR=~/Qt
export QT_VERSION=6.7.3
export QTCREATOR_VERSION=13.0.1
export QT_BIN=$QT_INSTALL_DIR/$QT_VERSION/clang_64/bin
export PATH="$QT_INSTALL_DIR/Qt Creator.app/Contents/MacOS:$QT_BIN:$PATH"

./build/install-qt.sh -d $QT_INSTALL_DIR --version $QT_VERSION qtbase qtsvg qtimageformats qttools qtxmlpatterns qtdeclarative
./build/install-qt.sh -d $QT_INSTALL_DIR --version $QTCREATOR_VERSION qtcreator

qbs --version
qbs setup-toolchains --detect
qbs config defaultProfile xcode
qbs config --list

qbs install --all-products config:release

qbs resolve config:release profile:xcode-iphoneos-arm64
qbs install --all-products config:release profile:xcode-iphoneos-arm64
qbs resolve config:release profile:xcode-appletvos-arm64
qbs install --all-products config:release profile:xcode-appletvos-arm64
