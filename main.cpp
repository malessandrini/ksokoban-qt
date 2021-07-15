/*
 *  ksokoban - a Sokoban game by KDE
 *  Copyright (C) 1998-2000  Anders Widell  <awl@passagen.se>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "MainWindow.h"


static const char version[] = "0.5.0";


QString aboutData_about = QString("<p><b>ksokoban</b> ") + version
	+ "<p>" + QObject::tr("The japanese warehouse keeper game")
	+ "<p>" + QChar(0xA9) + " 1998 Anders Widell (awl@hem.passagen.se)"
	+ "<p>" + QChar(0xA9) + " 2012 Lukasz Kalamlacki"
	+ "<p>https://github.com/malessandrini/ksokoban-qt/"
	+ "<p>https://github.com/KDE/ksokoban"
	+ "<p>http://www.shlomifish.org/open-source/projects/ksokoban/"
	+ "<p>License: GNU General Public License Version 2";
QString aboutData_authors = QString("<p><b>Shlomi Fish</b>")
	+ "<p>" + QObject::tr("For porting to Qt5/KF5 and doing other cleanups")
	+ "<p>shlomif@cpan.org<p>http://www.shlomifish.org/"
	+ "<p><b>Lukasz Kalamlacki</b>"
	+ "<p>" + QObject::tr("For rewriting the original ksokoban game from kde3 to kde4")
	+ "<p>kalamlacki@gmail.com<p>http://sf.net/projects/ksokoban"
	+ "<p><b>Anders Widell</b>"
	+ "<p>" + QObject::tr("For writing the original ksokoban game")
	+ "<p>awl@hem.passagen.se<p>http://hem.passagen.se/awl/ksokoban/"
	+ "<p><b>Michele Alessandrini</b>"
	+ "<p>" + QObject::tr("For this Qt-only port")
	+ "<p>https://github.com/malessandrini/ksokoban-qt/";
QString aboutData_credits = QString("<p><b>David W. Skinner</b>")
	+ "<p>" + QObject::tr("For contributing the Sokoban levels included in this game")
	+ "<p>sasquatch@bentonrea.com<p>http://users.bentonrea.com/~sasquatch/";


int
main (int argc, char **argv)
{
  QApplication app(argc, argv);

  // values needed by QSettings
  QCoreApplication::setOrganizationName("ksokoban-qt");
  QCoreApplication::setApplicationName("ksokoban-qt");


  app.setWindowIcon(QIcon::fromTheme(QStringLiteral("ksokoban")));

  QCommandLineParser parser;
  parser.addPositionalArgument(QObject::tr("[file]"), QObject::tr("Level collection file to load"));
  parser.process(app);

  MainWindow *widget = new MainWindow();
  widget->show();

  if (parser.positionalArguments().count() > 0) {
    widget->openURL(parser.positionalArguments().at(0));
  }


  //QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

  int rc = app.exec();
  delete widget;  // or destructor will not be called and settings will not be saved
  return rc;
}
