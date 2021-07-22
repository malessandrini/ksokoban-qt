#!/bin/bash

DEB_NAME=ksokoban-qt_0.5-1
DEB_PATH=deb/$DEB_NAME

rm -rf $DEB_PATH

mkdir -p $DEB_PATH/usr/games
mkdir -p $DEB_PATH/usr/share/doc/ksokoban-qt
mkdir -p $DEB_PATH/usr/share/applications
for a in 16 22 32 48 64 128
do
mkdir -p $DEB_PATH/usr/share/icons/hicolor/${a}x${a}/apps
done
mkdir -p $DEB_PATH/usr/share/games/ksokoban-qt/i18n
mkdir -p $DEB_PATH/DEBIAN

cp build/ksokoban-qt $DEB_PATH/usr/games
cp AUTHORS COPYING* NEWS README* TODO doc/index.html $DEB_PATH/usr/share/doc/ksokoban-qt
cp data/org.kde.ksokoban.desktop $DEB_PATH/usr/share/applications/ksokoban.desktop
for a in 16 22 32 48 64 128
do
cp data/${a}-apps-ksokoban.png $DEB_PATH/usr/share/icons/hicolor/${a}x${a}/apps/ksokoban.png
done
cp i18n/*.qm $DEB_PATH/usr/share/games/ksokoban-qt/i18n

cat <<EOT >> $DEB_PATH/DEBIAN/control
Package: ksokoban-qt
Version: 0.5-1
Section: games
Priority: optional
Architecture: amd64
Depends: libqt5core5a, libqt5gui5, libqt5widgets5
Maintainer: Michele Alessandrini, https://github.com/malessandrini/ksokoban-qt
Description: ksokoban
 ksokoban is the japanese warehouse keeper game, where
 you have to move some gems to the proper place in
 order to complete a level.
 This is the Qt-only porting of the original KDE game.

EOT

cd deb ; dpkg-deb -b --root-owner-group $DEB_NAME
