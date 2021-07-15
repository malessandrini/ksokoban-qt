/*
 *  ksokoban - a Sokoban game by KDE
 *  Copyright (C) 1998  Anders Widell  <awl@hem.passagen.se>
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

#include <stdio.h>
#include <assert.h>

#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QKeyEvent>
#include <QSignalMapper>
#include <QSettings>
#include <QString>
#include <QFrame>
#include <QTemporaryFile>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QDialogButtonBox>

#include "MainWindow.h"
#include "PlayField.h"
#include "LevelCollection.h"


extern QString aboutData_about, aboutData_authors, aboutData_credits;


void
MainWindow::createCollectionMenu(QMenu* collection_) {
  level_act = new QAction*[internalCollections_.collections()];
  for (int i=0; i<internalCollections_.collections(); i++) {
    QAction *qact = new QAction(internalCollections_[i]->name(), this);
	level_act[i] = qact;
	qact->setCheckable(true);
    	connect(qact, &QAction::triggered, this, [this,i ]() {changeCollection(i);});
	collection_->addAction(qact);
  }
  checkedCollection_ = 0;

  QSettings sett;
  int id = sett.value("collection", 13).toInt();

  currentCollection_ = 0;
  for (int i=0; i<internalCollections_.collections(); i++) {
    if (internalCollections_[i]->id() == id) currentCollection_ = i;
  }

  changeCollection(currentCollection_);
}


MainWindow::MainWindow() : QMainWindow(nullptr), externalCollection_(nullptr) {
  int i;
  QAction *qact;

  //setEraseColor(QColor(0,0,0));

  QSettings sett;
  int width = sett.value("width", 750).toInt();
  int height = sett.value("height", 562).toInt();
  resize(width, height);

  playField_ = new PlayField(this);
  setCentralWidget(playField_);
  //playField_->show();

  menu_ = menuBar();//  new KMenuBar(this);

  game_ = menu_->addMenu(tr("&Game"));
  
  qact = new QAction(tr("&Load Levels..."), this);
  connect(qact, &QAction::triggered, this, &MainWindow::loadLevels);
  game_->addAction(qact);
  
  qact = new QAction(tr("&Next Level"), this);
  qact->setShortcut(Qt::Key_N);
  connect(qact, &QAction::triggered, playField_, &PlayField::nextLevel);
  game_->addAction(qact);
  
  qact = new QAction(tr("&Previous Level"), this);
  qact->setShortcut(Qt::Key_P);
  connect(qact, &QAction::triggered, playField_, &PlayField::previousLevel);
  game_->addAction(qact);
  
  qact = new QAction(tr("Re&start Level"), this);
  qact->setShortcut(Qt::Key_Escape);
  connect(qact, &QAction::triggered, playField_, &PlayField::restartLevel);
  game_->addAction(qact);

  createCollectionMenu(game_->addMenu(tr("&Level Collection")));
  
  qact = new QAction(tr("&Undo"), this);
  qact->setShortcut(QKeySequence::Undo);
  connect(qact, &QAction::triggered, playField_, &PlayField::undo);
  game_->addAction(qact);
  
  qact = new QAction(tr("&Redo"), this);
  qact->setShortcut(QKeySequence::Redo);
  connect(qact, &QAction::triggered, playField_, &PlayField::redo);
  game_->addAction(qact);
 
  game_->addSeparator();
 
  qact = new QAction(tr("&Quit"), this);
  qact->setShortcut(QKeySequence::Quit);
  connect(qact, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
  game_->addAction(qact);
 
  animation_ = menu_->addMenu(tr("&Animation"));
  qa_slow = new QAction(tr("&Slow"), this);
  qa_slow->setCheckable(true);
  qa_medium = new QAction(tr("&Medium"), this);
  qa_medium->setCheckable(true);
  qa_fast = new QAction(tr("&Fast"), this);
  qa_fast->setCheckable(true);
  qa_off = new QAction(tr("&Off"), this);
  qa_off->setCheckable(true);
  animation_->addAction(qa_slow);
  animation_->addAction(qa_medium);
  animation_->addAction(qa_fast);
  animation_->addAction(qa_off);
  connect(qa_slow, &QAction::triggered, this, [this]() {updateAnim(3);});
  connect(qa_medium, &QAction::triggered, this, [this]() {updateAnim(2);});
  connect(qa_fast, &QAction::triggered, this, [this]() {updateAnim(1);});
  connect(qa_off, &QAction::triggered, this, [this]() {updateAnim(0);});
  
  checkedAnim_ = playField_->animDelay();
  updateAnimMenu(checkedAnim_);
 

  
  bookmarkMenu_ = menu_->addMenu(tr("&Bookmarks"));
  setBM_ = bookmarkMenu_->addMenu(tr("&Set Bookmark"));
  setBM_act[0] = setBM_->addAction(tr("(unused)"));
  setBM_act[0]->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_1));
  connect(setBM_act[0], &QAction::triggered, this, [this]() {setBookmark(1);});
  setBM_act[1] = setBM_->addAction(tr("(unused)"));
  setBM_act[1]->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_2));
  connect(setBM_act[1], &QAction::triggered, this, [this]() {setBookmark(2);});
  setBM_act[2] = setBM_->addAction(tr("(unused)"));
  setBM_act[2]->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_3));
  connect(setBM_act[2], &QAction::triggered, this, [this]() {setBookmark(3);});
  setBM_act[3] = setBM_->addAction(tr("(unused)"));
  setBM_act[3]->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_4));
  connect(setBM_act[3], &QAction::triggered, this, [this]() {setBookmark(4);});
  setBM_act[4] = setBM_->addAction(tr("(unused)"));
  setBM_act[4]->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_5));
  connect(setBM_act[4], &QAction::triggered, this, [this]() {setBookmark(5);});
  setBM_act[5] = setBM_->addAction(tr("(unused)"));
  setBM_act[5]->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_6));
  connect(setBM_act[5], &QAction::triggered, this, [this]() {setBookmark(6);});
  setBM_act[6] = setBM_->addAction(tr("(unused)"));
  setBM_act[6]->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_7));
  connect(setBM_act[6], &QAction::triggered, this, [this]() {setBookmark(7);});
  setBM_act[7] = setBM_->addAction(tr("(unused)"));
  setBM_act[7]->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_8));
  connect(setBM_act[7], &QAction::triggered, this, [this]() {setBookmark(8);});
  setBM_act[8] = setBM_->addAction(tr("(unused)"));
  setBM_act[8]->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_9));
  connect(setBM_act[8], &QAction::triggered, this, [this]() {setBookmark(9);});
  setBM_act[9] = setBM_->addAction(tr("(unused)"));
  setBM_act[9]->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_0));
  connect(setBM_act[9], &QAction::triggered, this, [this]() {setBookmark(10);});
  
  goToBM_ =  bookmarkMenu_->addMenu(tr("&Go to Bookmark"));
  goToBM_act[0] = goToBM_->addAction(tr("(unused)"));
  goToBM_act[0]->setShortcut(QKeySequence(Qt::Key_1));
  connect(goToBM_act[0], &QAction::triggered, this, [this]() {goToBookmark(1);});

  goToBM_act[1] = goToBM_->addAction(tr("(unused)"));
  goToBM_act[1]->setShortcut(QKeySequence(Qt::Key_2));
  connect(goToBM_act[1], &QAction::triggered, this, [this]() {goToBookmark(2);});
  goToBM_act[2] = goToBM_->addAction(tr("(unused)"));
  goToBM_act[2]->setShortcut(QKeySequence(Qt::Key_3));
  connect(goToBM_act[2], &QAction::triggered, this, [this]() {goToBookmark(3);});
  goToBM_act[3] = goToBM_->addAction(tr("(unused)"));
  goToBM_act[3]->setShortcut(QKeySequence(Qt::Key_4));
  connect(goToBM_act[3], &QAction::triggered, this, [this]() {goToBookmark(4);});
  goToBM_act[4] = goToBM_->addAction(tr("(unused)"));
  goToBM_act[4]->setShortcut(QKeySequence(Qt::Key_5));
  connect(goToBM_act[4], &QAction::triggered, this, [this]() {goToBookmark(5);});
  goToBM_act[5] = goToBM_->addAction(tr("(unused)"));
  goToBM_act[5]->setShortcut(QKeySequence(Qt::Key_6));
  connect(goToBM_act[5], &QAction::triggered, this, [this]() {goToBookmark(6);});
  goToBM_act[6] = goToBM_->addAction(tr("(unused)"));
  goToBM_act[6]->setShortcut(QKeySequence(Qt::Key_7));
  connect(goToBM_act[6], &QAction::triggered, this, [this]() {goToBookmark(7);});
  goToBM_act[7] = goToBM_->addAction(tr("(unused)"));
  goToBM_act[7]->setShortcut(QKeySequence(Qt::Key_8));
  connect(goToBM_act[7], &QAction::triggered, this, [this]() {goToBookmark(8);});
  goToBM_act[8] = goToBM_->addAction(tr("(unused)"));
  goToBM_act[8]->setShortcut(QKeySequence(Qt::Key_9));
  connect(goToBM_act[8], &QAction::triggered, this, [this]() {goToBookmark(9);});
  goToBM_act[9] = goToBM_->addAction(tr("(unused)"));
  goToBM_act[9]->setShortcut(QKeySequence(Qt::Key_0));
  connect(goToBM_act[9], &QAction::triggered, this, [this]() {goToBookmark(10);});

  for (i=1; i<=10; i++) {
    bookmarks_[i-1] = new Bookmark(i);
    updateBookmark(i);
  }

  menu_->addSeparator();
  help_ = menu_->addMenu(tr("&Help"));
  help_->addAction(tr("Ksokoban &Handbook"));  // TODO
  help_->addAction(tr("&About Ksokoban"), this, &MainWindow::aboutDialog);

  menu_->show();
  playField_->show();

  setAcceptDrops(true);
}

MainWindow::~MainWindow()
{
  QSettings sett;
  sett.setValue("width", width());
  sett.setValue("height", height());

  sett.setValue("collection", internalCollections_[checkedCollection_]->id());

  for (int i=1; i<=10; i++) {
    delete bookmarks_[i-1];
  }

  delete externalCollection_;
  delete[] level_act;

  // The following line segfaults when linked against qt 1.44
  //delete help_;
  //delete goToBM_;
  //delete setBM_;
  //delete bookmarkMenu_;
  //delete animation_;
  //delete collection_;
  //delete game_;
  //delete menu_;

  //delete playField_;
}


void
MainWindow::updateAnim(int val) {
    updateAnimMenu(val);
    playField_->changeAnim(val);
}

void
MainWindow::focusInEvent(QFocusEvent *) {
  playField_->setFocus();
}

void
MainWindow::updateAnimMenu(int id) {
  switch(checkedAnim_) {
	case 0:
		qa_off->setChecked(false);
		break;
	case 1:
	    qa_fast->setChecked(false);
	    break;
	case 2:
	    qa_medium->setChecked(false);
		break;
	case 3:
	    qa_slow->setChecked(false);
	    break;
  }
  switch(id) {
	case 0:
		qa_off->setChecked(true);
		break;
	case 1:
	    qa_fast->setChecked(true);
	    break;
	case 2:
	    qa_medium->setChecked(true);
		break;
	case 3:
	    qa_slow->setChecked(true);
	    break;
  }
  checkedAnim_ = id;
}

void
MainWindow::updateBookmark(int num) {
  int col = internalCollections_.toInternalId(bookmarks_[num-1]->collection());
  int lev = bookmarks_[num-1]->level();
  int mov = bookmarks_[num-1]->moves();

  if (col < 0 || lev < 0) return;

  QString name;
  if (col >= 0 && col < internalCollections_.collections())
    name = internalCollections_[col]->name();
  else
	name = tr("(invalid)");
  QString l;
  l.setNum(lev+1);
  name += " #" + l;
  l.setNum(mov);
  name += " (" + l + QLatin1Char(')');

  setBM_act[num-1]->setText(name);
  goToBM_act[num-1]->setText(name);
}

void
MainWindow::setBookmark(int id) {
  assert(id >= 1 && id <= 10);
  playField_->setBookmark(bookmarks_[id-1]);
  updateBookmark(id);
}

void
MainWindow::goToBookmark(int id) {
  assert(id >= 1 && id <= 10);

  Bookmark *bm = bookmarks_[id-1];
  int collection = internalCollections_.toInternalId(bm->collection());
  int level = bm->level();

  if (collection < 0 || collection >= internalCollections_.collections()) return;
  LevelCollection* colPtr = internalCollections_[collection];
  if (colPtr == nullptr) return;
  if (level < 0 || level >= colPtr->noOfLevels()) return;
  if (level > colPtr->completedLevels()) return;

  playField_->setUpdatesEnabled(false);
  changeCollection(collection);
  playField_->setUpdatesEnabled(true);
  playField_->goToBookmark(bookmarks_[id-1]);
}

void
MainWindow::changeCollection(int id)
{
  level_act[checkedCollection_]->setChecked(false);
  checkedCollection_ = id;
  level_act[checkedCollection_]->setChecked(true);

  delete externalCollection_;
  externalCollection_ = nullptr;
  playField_->changeCollection(internalCollections_[id]);
}

void
MainWindow::loadLevels() {
  QSettings sett;
  QString lastFile = sett.value("lastLevelFile").toString();
  QUrl result = QFileDialog::getOpenFileUrl(this, tr("Load Levels From File"), lastFile, QStringLiteral("*"));
  if (result.isEmpty()) return;

  openURL(result);
}


void MainWindow::aboutDialog() {
	QDialog d;
	d.setWindowTitle(tr("About Ksokoban"));
	d.setMinimumSize(640, 480);
	auto *l = new QVBoxLayout(&d);
	auto *tabw = new QTabWidget(&d);
	l->addWidget(tabw);
	auto *l1 = new QLabel(aboutData_about, &d);
	l1->setAlignment(Qt::AlignTop);
	l1->setMargin(10);
	tabw->addTab(l1, tr("&About"));
	auto *l2 = new QLabel(aboutData_authors, &d);
	l2->setAlignment(Qt::AlignTop);
	l2->setMargin(10);
	tabw->addTab(l2, tr("A&uthors"));
	auto *l3 = new QLabel(aboutData_credits, &d);
	l3->setAlignment(Qt::AlignTop);
	l3->setMargin(10);
	tabw->addTab(l3, tr("&Credits"));
	auto *bbox = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, &d);
	l->addWidget(bbox);
	connect(bbox, &QDialogButtonBox::rejected, &d, &QDialog::reject);
	d.exec();
}

void
MainWindow::openURL(const QUrl &_url) {
//   int namepos = _url.path().findRev('/') + 1; // NOTE: findRev can return -1
//   QString levelName = _url.path().mid(namepos);
  QString levelName = _url.fileName();

  QString levelFile;
  QTemporaryFile f;
  if (_url.isLocalFile()) {
    levelFile = _url.path();
  } else {
	  return;
  }

  LevelCollection *tmpCollection = new LevelCollection(levelFile, levelName);

  if (tmpCollection->noOfLevels() < 1) {
	QMessageBox::critical(this, tr("Error"), tr("No levels found in file"));
    delete tmpCollection;
    return;
  }
  if (_url.isLocalFile()) {
	QSettings sett;
	sett.setValue("lastLevelFile", _url.path());
  }

  delete externalCollection_;
  externalCollection_ = tmpCollection;

  level_act[checkedCollection_]->setChecked(false);
  playField_->changeCollection(externalCollection_);


}
/*
void
MainWindow::dragEnterEvent(QDragEnterEvent* event) {
  event->accept(KURLDrag::canDecode(event));
}
*/

void
MainWindow::dropEvent(QDropEvent*) {
}
