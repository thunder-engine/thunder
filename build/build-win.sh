#!/usr/bin/env bash

export MSVC=2015
export QT_INSTALL_DIR=/c/Qt
export QT_VERSION=5.12.10
export QTCREATOR_VERSION=5.0.3
export QT_BIN=$QT_INSTALL_DIR/$QT_VERSION/clang_64/bin
export PATH="$QT_INSTALL_DIR/Qt Creator.app/Contents/MacOS:$QT_BIN:$PATH"

./build/install-qt.sh -d $QT_INSTALL_DIR --version $QT_VERSION qtbase qtmultimedia qtscript qtsvg qtimageformats qttools qtxmlpatterns qtdeclarative
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
