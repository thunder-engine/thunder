#!/usr/bin/env bash

export MSVC=2022
export QT_INSTALL_DIR=/c/Qt
export QT_VERSION=6.7.3
export QTCREATOR_VERSION=13.0.1
export QT_BIN=$QT_INSTALL_DIR/$QT_VERSION/clang_64/bin
export PATH="$QT_BIN:$PATH"

./build/install-qt.sh -d $QT_INSTALL_DIR --version $QT_VERSION qtbase qtsvg qtimageformats qttools qtxmlpatterns qtdeclarative
./build/install-qt.sh -d $QT_INSTALL_DIR --version $QTCREATOR_VERSION qtcreator

qbs --version
qbs setup-toolchains --detect
qbs setup-qt $QT_PATH/qmake.exe qt
qbs config --list

qbs install --all-products config:release profile:qt
