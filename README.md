# ksokoban-qt

![Game screenshot](doc/screenshot.png)

This is a patched version of ksokoban, in order to build it with Qt only, with no dependencies on KDE libraries. Based on original sources at [https://github.com/KDE/ksokoban](https://github.com/KDE/ksokoban).

Ksokoban is a great game, but it's not being distributed anymore with official distributions, perhaps for problems with specific KDE versions. So this patch (actually very light, with a few bug fixes, too) is meant to give new life to this game, while preserving its look and feel very closely.

The build process is simplified, too, needing only the "qmake" utility provided by Qt. Provided you have the needed Qt development environment installed, just do:

```
mkdir build
cd build
qmake ..
make
```

I'm also providing a deb package (Debian buster and newer) and a Windows installer, see "Releases" section.


Original README follows:

------------------------------------------------------------------------

ksokoban 0.5.0 - a Sokoban game by KDE

copyright 1998 Anders Widell  <d95-awi@nada.kth.se>

copyright 2012 Lukasz Kalamlacki <kalamlacki@gmail.com>

ksokoban is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License. See the file
COPYING for details.

See http://hem.passagen.se/awl/ksokoban/ for later versions of ksokoban.
See the file AUTHORS for details about where the levels come from.

------------------------------------------------------------------------

## PLAYING


The objective of the game is to push all the red gems (these should
actually have been crates, but gems looked nicer) to the goal squares,
which are marked with green glassy things on the floor.

Use the cursor keys to move about. If you move onto a gem and there is
noting blocking it on the opposite side, then you will push the gem.

Use the CONTROL key together with the cursor keys to move as far as
possible in a direction without pushing any gems. With the SHIFT key
you will move as far as possible in a direction, possibly pushing a
gem if it is in the way.

Use the left mouse button to move to any place you can reach without
pushing any gems. The middle mouse moves in a straight line, possibly
pushing a gem if it is in the way.

The U key or the right mouse button undoes the last move.
