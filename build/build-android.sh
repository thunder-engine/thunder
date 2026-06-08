#!/usr/bin/env bash

export QT_INSTALL_DIR=~/Qt
export QT_VERSION=6.7.3
export QTCREATOR_VERSION=13.0.1
export QT_BIN=$QT_INSTALL_DIR/$QT_VERSION/clang_64/bin

#./build/install-qt.sh -d $QT_INSTALL_DIR --version $QTCREATOR_VERSION qtcreator

qbs --version
qbs setup-android --ndk-dir /d/Environment/Android/sdk/ndk/29.0.14206865 android
qbs setup-toolchains --detect
qbs config --list

qbs build --all-products config:release qbs.toolchainType:clang profile:android
