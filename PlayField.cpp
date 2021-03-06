/*
 *  ksokoban - a Sokoban game by KDE
 *  Copyright (C) 1998  Anders Widell  <d95-awi@nada.kth.se>
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

#include <QWidget>
#include <QPixmap>
#include <QKeyEvent>
#include <QApplication>
#include <QMessageBox>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QFontDatabase>
#include <QSettings>
#include "PlayField.h"
#include "ModalLabel.h"
#include "LevelMap.h"
#include "Move.h"
#include "History.h"
#include "PathFinder.h"
#include "MapDelta.h"
#include "MoveSequence.h"
#include "StaticImage.h"
#include "HtmlPrinter.h"
#include "Bookmark.h"
#include "LevelCollection.h"

PlayField::PlayField(QWidget *parent)
  : QWidget(parent), imageData_(nullptr), lastLevel_(-1),
    moveSequence_(nullptr), moveInProgress_(false), dragInProgress_(false),
    xOffs_(0), yOffs_(0),
    wheelDelta_(0), debug_counter(0),
    sizeAllCursor(Qt::SizeAllCursor), crossCursor(Qt::CrossCursor),
	levelText_(tr("Level:")), stepsText_(tr("Steps:")),
	pushesText_(tr("Pushes:")),
	pnumXpm_(nullptr), ptxtXpm_(nullptr), snumXpm_(nullptr), stxtXpm_(nullptr),
	lnumXpm_(nullptr), ltxtXpm_(nullptr), collXpm_(nullptr),
    statusFont_(QFontDatabase::systemFont(QFontDatabase::GeneralFont).family(), 18, QFont::Bold),
    statusMetrics_(statusFont_) {

  //setAttribute(Qt::WA_PaintOutsidePaintEvent);
  setFocusPolicy(Qt::StrongFocus);
  setFocus();
  setMouseTracking(true);

  highlightX_ = highlightY_ = 0;

  QSettings sett;
  
  imageData_ = new StaticImage;

  animDelay_ = sett.value("animDelay", 2).toInt();
  if (animDelay_ < 0 || animDelay_ > 3) animDelay_ = 2;

  history_ = new History;

  background_.setTexture(imageData_->background());
  //floor_ = QColor(0x66,0x66,0x66, 255);

  levelMap_  = new LevelMap;
  mapDelta_ = new MapDelta(levelMap_);
  mapDelta_->end();

  levelChange();
}

PlayField::~PlayField() {
  QSettings sett;
  sett.setValue("animDelay", animDelay_);

  delete mapDelta_;
  delete history_;
  delete levelMap_;
  delete imageData_;
  if(ltxtXpm_) 
	delete ltxtXpm_;
  if(lnumXpm_)
	delete lnumXpm_;
  if(stxtXpm_)
    delete stxtXpm_;
  if(snumXpm_)
    delete snumXpm_;
  if(ptxtXpm_)
    delete ptxtXpm_;
  if(pnumXpm_)
    delete pnumXpm_;
  if(collXpm_)
    delete collXpm_;
}

void
PlayField::changeCursor(const QCursor* c) {
  if (cursor_ == c) return;

  cursor_ = c;
  if (c == nullptr) unsetCursor();
  else setCursor(*c);
}

int
PlayField::level() const {
  if (levelMap_ == nullptr) return 0;
  return levelMap_->level();
}

const QString &
PlayField::collectionName() {
  static QString error = QStringLiteral("????");
  if (levelMap_ == nullptr) return error;
  return levelMap_->collectionName();
}

int
PlayField::totalMoves() const {
  if (levelMap_ == nullptr) return 0;
  return levelMap_->totalMoves();
}

int
PlayField::totalPushes() const{
  if (levelMap_ == nullptr) return 0;
  return levelMap_->totalPushes();
}

void
PlayField::levelChange() {
  stopMoving();
  stopDrag();
  history_->clear();
  setSize(width(), height());

  updateLevelXpm();
  updateStepsXpm();
  updatePushesXpm();
  highlight();
}

void
PlayField::paintSquare(int x, int y, QPainter &paint) {
  if (levelMap_->xpos() == x && levelMap_->ypos() == y) {
    if (levelMap_->goal(x, y))
      imageData_->saveman(paint, x2pixel(x), y2pixel(y));
    else {
      imageData_->man(paint, x2pixel(x), y2pixel(y));
	  //printf("imageData_->man() %d; %d\n",x2pixel(x), y2pixel(y));
	} 
    return;
  }
  if (levelMap_->empty(x, y)) {
    if (levelMap_->floor(x, y)) {
      if (levelMap_->goal(x, y))
		imageData_->goal(paint, x2pixel(x), y2pixel(y));
      else{
		//paint.fillRect(x2pixel(x), y2pixel(y), size_, size_, floor_);
		imageData_->floor(paint, x2pixel(x), y2pixel(y));
		//printf("executing paint.fillRect(): %d; %d,%d,%d,%d\n",++debug_counter, x2pixel(x), y2pixel(y), size_, size_);
	  }
    } else {
      paint.fillRect(x2pixel(x), y2pixel(y), size_, size_, background_);
    }
    return;
  }
  if (levelMap_->wall(x, y)) {
    imageData_->wall(paint, x2pixel(x), y2pixel(y), x+y*(MAX_X+1),
		     levelMap_->wallLeft(x, y),
		     levelMap_->wallRight(x, y));
    return;
  }


  if (levelMap_->object(x, y)) {
    if (highlightX_ == x && highlightY_ == y) {
      if (levelMap_->goal(x, y))
	imageData_->brightTreasure(paint, x2pixel(x), y2pixel(y));
      else
	imageData_->brightObject(paint, x2pixel(x), y2pixel(y));
    } else {
      if (levelMap_->goal(x, y))
	imageData_->treasure(paint, x2pixel(x), y2pixel(y));
      else
	imageData_->object(paint, x2pixel(x), y2pixel(y));
    }
    return;
  }
}

void
PlayField::paintDelta() {
  repaint();
}



void
PlayField::paintEvent(QPaintEvent *e) {
  QPainter paint;
  paint.begin(this);
  // the following line is a workaround for a bug in Qt 2.0.1
  // (and possibly earlier versions)
  paint.setBrushOrigin(0, 0);

  paint.setClipRegion(e->region());
  paint.setClipping(true);

  paintPainter(paint, e->rect());
  paint.end();
}

void
PlayField::paintPainterClip(QPainter &paint, int x, int y, int w, int h) {
  QRect rect(x, y, w, h);

  paint.setClipRect(rect);
  paint.setClipping(true);
  paintPainter(paint, rect);
}

void
PlayField::paintPainter(QPainter &paint, const QRect &rect) {
  if (size_ <= 0) return;
  int minx = pixel2x(rect.x());
  int miny = pixel2y(rect.y());
  int maxx = pixel2x(rect.x()+rect.width()-1);
  int maxy = pixel2y(rect.y()+rect.height()-1);

  if (minx < 0) minx = 0;
  if (miny < 0) miny = 0;
  if (maxx >= levelMap_->width()) maxx = levelMap_->width()-1;
  if (maxy >= levelMap_->height()) maxy = levelMap_->height()-1;

  {
    int x1, x2, y1, y2;
    y1 = y2pixel(miny);
    if (y1 > rect.y()) paint.fillRect(rect.x(), rect.y(), rect.width(), y1-rect.y(), background_);

    int bot=rect.y()+rect.height();
    if (bot > height()-collRect_.height()) bot = height()-collRect_.height();

    y2 = y2pixel(maxy+1);
    if (y2 < bot) paint.fillRect(rect.x(), y2, rect.width(), bot-y2, background_);

    x1 = x2pixel(minx);
    if (x1 > rect.x()) paint.fillRect(rect.x(), y1, x1-rect.x(), y2-y1, background_);

    x2 = x2pixel(maxx+1);
    if (x2 < rect.x()+rect.width()) paint.fillRect(x2, y1, rect.x()+rect.width()-x2, y2-y1, background_);

    // paint.eraseRect
  }

  for (int y=miny; y<=maxy; y++) {
    for (int x=minx; x<=maxx; x++) {
      paintSquare(x, y, paint);
    }
  }

  if (collRect_.intersects(rect) && collXpm_)  paint.drawPixmap(collRect_.x(), collRect_.y(), *collXpm_);
  if (ltxtRect_.intersects(rect) && ltxtXpm_)  paint.drawPixmap(ltxtRect_.x(), ltxtRect_.y(), *ltxtXpm_);
  if (lnumRect_.intersects(rect) && lnumXpm_)  paint.drawPixmap(lnumRect_.x(), lnumRect_.y(), *lnumXpm_);
  if (stxtRect_.intersects(rect) && stxtXpm_)  paint.drawPixmap(stxtRect_.x(), stxtRect_.y(), *stxtXpm_);
  if (snumRect_.intersects(rect) && snumXpm_)  paint.drawPixmap(snumRect_.x(), snumRect_.y(), *snumXpm_);
  if (ptxtRect_.intersects(rect) && ptxtXpm_)  paint.drawPixmap(ptxtRect_.x(), ptxtRect_.y(), *ptxtXpm_);
  if (pnumRect_.intersects(rect) && pnumXpm_)  paint.drawPixmap(pnumRect_.x(), pnumRect_.y(), *pnumXpm_);
}

void
PlayField::resizeEvent(QResizeEvent *e) {
  setSize(e->size().width(), e->size().height());
}

void
PlayField::mouseMoveEvent(QMouseEvent *e) {
  lastMouseXPos_ = e->x();
  lastMouseYPos_ = e->y();

  if (!dragInProgress_) return highlight();

  int old_x = dragX_, old_y = dragY_;

  dragX_ = lastMouseXPos_ - mousePosX_;
  dragY_ = lastMouseYPos_ - mousePosY_;

  {
    int x = pixel2x(dragX_ + size_/2);
    int y = pixel2y(dragY_ + size_/2);
    if (x >= 0 && x < levelMap_->width() &&
	y >= 0 && y < levelMap_->height() &&
	pathFinder_.canDragTo(x, y)) {
      x = x2pixel(x);
      y = y2pixel(y);

      if (dragX_ >= x - size_/4 &&
	  dragX_ <  x + size_/4 &&
	  dragY_ >= y - size_/4 &&
	  dragY_ <  y + size_/4) {
	dragX_ = x;
	dragY_ = y;
      }
    }
  }

  if (dragX_ == old_x && dragY_ == old_y) return;

  repaint();
  
}

void
PlayField::highlight() {
  // FIXME: the line below should not be needed
  if (size_ == 0) return;

  int x=pixel2x(lastMouseXPos_);
  int y=pixel2y(lastMouseYPos_);

  if (x < 0 || y < 0 || x >= levelMap_->width() || y >= levelMap_->height())
    return;

  if (x == highlightX_ && y == highlightY_) return;

  if (pathFinder_.canDrag(x, y)) {
    highlightX_ = x;
    highlightY_ = y;
    repaint();
  } else {
    if (pathFinder_.canWalkTo(x, y)) changeCursor(&crossCursor);
    else changeCursor(nullptr);
    if (highlightX_ >= 0) {
      repaint();
    }
  }
}

void
PlayField::stopMoving() {
  killTimers();
  delete moveSequence_;
  moveSequence_ = nullptr;
  moveInProgress_ = false;
  updateStepsXpm();
  updatePushesXpm();

  repaint();
  pathFinder_.updatePossibleMoves();
}


void
PlayField::startMoving(Move *m) {
  startMoving(new MoveSequence(m, levelMap_));
}

void
PlayField::startMoving(MoveSequence *ms) {
  static const int delay[4] = {0, 15, 35, 60};

  assert(moveSequence_ == nullptr && !moveInProgress_);
  moveSequence_ = ms;
  moveInProgress_ = true;
  if (animDelay_) timers.append(startTimer(delay[animDelay_]));
  timerEvent(nullptr);
}

void
PlayField::timerEvent(QTimerEvent *) {
  assert(moveInProgress_);
  if (moveSequence_ == nullptr) {
    killTimers();
    moveInProgress_ = false;
    return;
  }

  bool more=false;

  mapDelta_->start();
  if (animDelay_) more = moveSequence_->next();
  else {
    while (moveSequence_->next()) if (levelMap_->completed()) break;
    more = true;   // FIXME: clean this up
    stopMoving();
  }
  mapDelta_->end();

  if (more) {
    paintDelta();
    if (levelMap_->completed()) {
      stopMoving();
	  ModalLabel::message(tr("Level completed"), this);
      nextLevel();
      return;
    }
  } else stopMoving();
}

void
PlayField::step(int _x, int _y) {
  if (!canMoveNow()) return;

  int oldX=levelMap_->xpos();
  int oldY=levelMap_->ypos();
  int x=oldX, y=oldY;

  int dx=0, dy=0;
  if (_x>oldX) dx=1;
  if (_x<oldX) dx=-1;
  if (_y>oldY) dy=1;
  if (_y<oldY) dy=-1;

  while (!(x==_x && y==_y) && levelMap_->step(x+dx, y+dy)) {
    x += dx;
    y += dy;
  }

  if (x!=oldX || y!=oldY) {
    Move *m = new Move(oldX, oldY);
    m->step(x, y);
    m->finish();
    history_->add(m);
    m->undo(levelMap_);

    startMoving(m);

  }
}

void
PlayField::push(int _x, int _y) {
  if (!canMoveNow()) return;

  int oldX=levelMap_->xpos();
  int oldY=levelMap_->ypos();
  int x=oldX, y=oldY;

  int dx=0, dy=0;
  if (_x>oldX) dx=1;
  if (_x<oldX) dx=-1;
  if (_y>oldY) dy=1;
  if (_y<oldY) dy=-1;

  while (!(x==_x && y==_y) && levelMap_->step(x+dx, y+dy)) {
    x += dx;
    y += dy;
  }
  int objX=x, objY=y;
  while (!(x==_x && y==_y) && levelMap_->push(x+dx, y+dy)) {
    x += dx;
    y += dy;
  }

  if (x!=oldX || y!=oldY) {
    Move *m = new Move(oldX, oldY);

    if (objX!=oldX || objY!=oldY) m->step(objX, objY);

    if (objX!=x || objY!=y) {
      m->push(x, y);

      objX += dx;
      objY += dy;
    }
    m->finish();
    history_->add(m);

    m->undo(levelMap_);

    startMoving(m);
  }
}

void
PlayField::keyPressEvent(QKeyEvent * e) {
  int x=levelMap_->xpos();
  int y=levelMap_->ypos();

  switch (e->key()) {
  case Qt::Key_Up:
    if (e->modifiers() & Qt::ControlModifier) step(x, 0);
    else if (e->modifiers() & Qt::ShiftModifier) push(x, 0);
    else push(x, y-1);
    break;
  case Qt::Key_Down:
    if (e->modifiers() & Qt::ControlModifier) step(x, MAX_Y);
    else if (e->modifiers() & Qt::ShiftModifier) push(x, MAX_Y);
    else push(x, y+1);
    break;
  case Qt::Key_Left:
    if (e->modifiers() & Qt::ControlModifier) step(0, y);
    else if (e->modifiers() & Qt::ShiftModifier) push(0, y);
    else push(x-1, y);
    break;
  case Qt::Key_Right:
    if (e->modifiers() & Qt::ControlModifier) step(MAX_X, y);
    else if (e->modifiers() & Qt::ShiftModifier) push(MAX_X, y);
    else push(x+1, y);
    break;
#if 0
  case Qt::Key_Q:
    qApp->closeAllWindows();
    break;
#endif
  case Qt::Key_Backspace:
  case Qt::Key_Delete:
    if (e->modifiers() & Qt::ControlModifier) redo();
    else undo();
    break;

#if 0
  case Qt::Key_X:
    levelMap_->random();
    levelChange();
    repaint(false);
    break;

  case Qt::Key_R:
    level(levelMap_->level());
    return;
    break;
  case Qt::Key_N:
    nextLevel();
    return;
    break;
  case Qt::Key_P:
    previousLevel();
    return;
    break;
  case Qt::Key_U:
    undo();
    return;
    break;
  case Qt::Key_I:
    history_->redo(levelMap_);
    repaint(false);
    return;
    break;

  case Qt::Key_S:
    {
      QString buf;
      history_->save(buf);
      printf("%s\n", (char *) buf);
    }
    return;
    break;

  case Qt::Key_L:
    stopMoving();
    history_->clear();
    level(levelMap_->level());
    {
      char buf[4096]="r1*D1*D1*r1*@r1*D1*";
      //scanf("%s", buf);
      history_->load(levelMap_, buf);
    }
    updateStepsXpm();
    updatePushesXpm();
    repaint(false);
    return;
    break;
#endif


  case Qt::Key_Print:
    HtmlPrinter::printHtml(levelMap_);
    break;

  default:
    e->ignore();
    return;
  }
}

void
PlayField::stopDrag() {
  if (!dragInProgress_) return;

  changeCursor(nullptr);

  repaint();
  dragInProgress_ = false;
}

void
PlayField::dragObject(int xpixel, int ypixel) {
  int x=pixel2x(xpixel - mousePosX_ + size_/2);
  int y=pixel2y(ypixel - mousePosY_ + size_/2);

  if (x == highlightX_ && y == highlightY_) return;

  printf("drag %d,%d to %d,%d\n", highlightX_, highlightY_, x, y);
  pathFinder_.drag(highlightX_, highlightY_, x, y);
  stopDrag();
}


void
PlayField::mousePressEvent(QMouseEvent *e) {
  if (!canMoveNow()) return;

  if (dragInProgress_) {
    if (e->button() == Qt::LeftButton) dragObject(e->x(), e->y());
    else stopDrag();
    return;
  }

  int x=pixel2x(e->x());
  int y=pixel2y(e->y());

  if (x < 0 || y < 0 || x >= levelMap_->width() || y >= levelMap_->height())
    return;

  if (e->button() == Qt::LeftButton && pathFinder_.canDrag(x, y)) {
    repaint();
    highlightX_ = x;
    highlightY_ = y;
    pathFinder_.updatePossibleDestinations(x, y);

    dragX_ = x2pixel(x);
    dragY_ = y2pixel(y);
    mousePosX_ = e->x() - dragX_;
    mousePosY_ = e->y() - dragY_;
    dragInProgress_ = true;
  }

  Move *m;
  switch (e->button()) {
  case Qt::LeftButton:
    m = pathFinder_.search(levelMap_, x, y);
    if (m != nullptr) {
      history_->add(m);

      startMoving(m);
    }
    break;
  case Qt::MiddleButton:
    undo();
    return;
    break;
  case Qt::RightButton:
    push(x, y);
    break;

  default:
    return;
  }
}

void
PlayField::wheelEvent(QWheelEvent *e) {
  wheelDelta_ += e->angleDelta().y();

  if (wheelDelta_ >= 120) {
    wheelDelta_ %= 120;
    redo();
  } else if (wheelDelta_ <= -120) {
    wheelDelta_ = -(-wheelDelta_ % 120);
    undo();
  }
}

void
PlayField::mouseReleaseEvent(QMouseEvent *e) {
  if (dragInProgress_) dragObject(e->x(), e->y());
}


void
PlayField::focusInEvent(QFocusEvent *) {
  //printf("PlayField::focusInEvent\n");
}

void
PlayField::focusOutEvent(QFocusEvent *) {
  //printf("PlayField::focusOutEvent\n");
}

void
PlayField::leaveEvent(QEvent *) {
  stopDrag();
}

void
PlayField::setSize(int w, int h) {
  int sbarHeight = statusMetrics_.height();
  int sbarNumWidth = statusMetrics_.boundingRect(QStringLiteral("88888")).width()+8;
  int sbarLevelWidth = statusMetrics_.boundingRect(levelText_).width()+8;
  int sbarStepsWidth = statusMetrics_.boundingRect(stepsText_).width()+8;
  int sbarPushesWidth = statusMetrics_.boundingRect(pushesText_).width()+8;

  pnumRect_.setRect(w-sbarNumWidth, h-sbarHeight, sbarNumWidth, sbarHeight);
  ptxtRect_.setRect(pnumRect_.x()-sbarPushesWidth, h-sbarHeight, sbarPushesWidth, sbarHeight);
  snumRect_.setRect(ptxtRect_.x()-sbarNumWidth, h-sbarHeight, sbarNumWidth, sbarHeight);
  stxtRect_.setRect(snumRect_.x()-sbarStepsWidth, h-sbarHeight, sbarStepsWidth, sbarHeight);
  lnumRect_.setRect(stxtRect_.x()-sbarNumWidth, h-sbarHeight, sbarNumWidth, sbarHeight);
  ltxtRect_.setRect(lnumRect_.x()-sbarLevelWidth, h-sbarHeight, sbarLevelWidth, sbarHeight);
  collRect_.setRect(0, h-sbarHeight, ltxtRect_.x(), sbarHeight);

  //printf("collRect_:%d;%d;%d;%d\n",collRect_.x(), collRect_.y(), collRect_.width(), collRect_.height());
  if(ltxtXpm_) 
	delete ltxtXpm_;
  if(lnumXpm_)
	delete lnumXpm_;
  if(stxtXpm_)
    delete stxtXpm_;
  if(snumXpm_)
    delete snumXpm_;
  if(ptxtXpm_)
    delete ptxtXpm_;
  if(pnumXpm_)
    delete pnumXpm_;
  if(collXpm_)
    delete collXpm_;
  ltxtXpm_ =  new QPixmap(ltxtRect_.size());	
  lnumXpm_ =  new QPixmap(lnumRect_.size());
  stxtXpm_ =  new QPixmap(stxtRect_.size());
  snumXpm_ =  new QPixmap(snumRect_.size());
  ptxtXpm_ =  new QPixmap(ptxtRect_.size());
  pnumXpm_ =  new QPixmap(pnumRect_.size());
  collXpm_ = new QPixmap(collRect_.size());
  
  h -= sbarHeight;

  int cols = levelMap_->width();
  int rows = levelMap_->height();

  // FIXME: the line below should not be needed
  if (cols == 0 || rows == 0) return;

  int xsize = w / cols;
  int ysize = h / rows;

  if (xsize < 8) xsize = 8;
  if (ysize < 8) ysize = 8;

  size_ = imageData_->resize(xsize > ysize ? ysize : xsize);

  xOffs_ = (w - cols*size_) / 2;
  yOffs_ = (h - rows*size_) / 2;


  updateCollectionXpm();
  updateTextXpm();
  updateLevelXpm();
  updateStepsXpm();
  updatePushesXpm();
}

void
PlayField::nextLevel() {
  if (levelMap_->level()+1 >= levelMap_->noOfLevels()) {
	ModalLabel::message(tr("\
This is the last level in\n\
the current collection."), this);
    return;
  }
  if (levelMap_->level() >= levelMap_->completedLevels()) {
	ModalLabel::message(tr("\
You have not completed\n\
this level yet."), this);
    return;
  }

  level(levelMap_->level()+1);
  levelChange();
  repaint();
}

void
PlayField::previousLevel() {
  if (levelMap_->level() <= 0) {
	ModalLabel::message(tr("\
This is the first level in\n\
the current collection."), this);
    return;
  }
  level(levelMap_->level()-1);
  levelChange();
  repaint();
}

void
PlayField::undo() {
  if (!canMoveNow()) return;

  startMoving(history_->deferUndo(levelMap_));
}

void
PlayField::redo() {
  if (!canMoveNow()) return;

  startMoving(history_->deferRedo(levelMap_));
}

void
PlayField::restartLevel() {
  stopMoving();
  history_->clear();
  level(levelMap_->level());
  updateStepsXpm();
  updatePushesXpm();
  repaint();
}

void
PlayField::changeCollection(LevelCollection *collection) {
  if (levelMap_->collection() == collection) return;
  levelMap_->changeCollection(collection);
  levelChange();
  //erase(collRect_);
  repaint();
}

void
PlayField::updateCollectionXpm() {
  if (!collXpm_) return;
  if (collXpm_->isNull()) return;
  //printf("executing PlayField::updateCollectionXpm() w:%d, h:%d\n",collXpm_->width(), collXpm_->height());
  
  QPainter paint(collXpm_);
  paint.setBrushOrigin(- collRect_.x(), - collRect_.y());
  paint.fillRect(0, 0, collRect_.width(), collRect_.height(), background_);

  paint.setFont(statusFont_);
  paint.setPen(QColor(0,255,0));
  paint.drawText(0, 0, collRect_.width(), collRect_.height(),
		 Qt::AlignLeft, collectionName());
}

void
PlayField::updateTextXpm() {
  if (!ltxtXpm_) return;
  if (ltxtXpm_->isNull()) return;
  //printf("executing PlayField::updateTextXpm() w:%d, h:%d\n",ltxtXpm_->width(), ltxtXpm_->height());
  
  QPainter paint;

  paint.begin(ltxtXpm_);
  paint.setBrushOrigin(- ltxtRect_.x(), - ltxtRect_.y());
  paint.fillRect(0, 0, ltxtRect_.width(), ltxtRect_.height(), background_);
  paint.setFont(statusFont_);
  paint.setPen(QColor(128,128,128));
  paint.drawText(0, 0, ltxtRect_.width(), ltxtRect_.height(), Qt::AlignLeft, levelText_);
  paint.end();

  paint.begin(stxtXpm_);
  paint.setBrushOrigin(- stxtRect_.x(), - stxtRect_.y());
  paint.fillRect(0, 0, stxtRect_.width(), stxtRect_.height(), background_);
  paint.setFont(statusFont_);
  paint.setPen(QColor(128,128,128));
  paint.drawText(0, 0, stxtRect_.width(), stxtRect_.height(), Qt::AlignLeft, stepsText_);
  paint.end();

  paint.begin(ptxtXpm_);
  paint.setBrushOrigin(- ptxtRect_.x(), - ptxtRect_.y());
  paint.fillRect(0, 0, ptxtRect_.width(), ptxtRect_.height(), background_);
  paint.setFont(statusFont_);
  paint.setPen(QColor(128,128,128));
  paint.drawText(0, 0, ptxtRect_.width(), ptxtRect_.height(), Qt::AlignLeft, pushesText_);
  paint.end();
}

void
PlayField::updateLevelXpm() {
  if (!lnumXpm_) return;
  if (lnumXpm_->isNull()) return;
  //printf("executing PlayField::updateLevelXpm()\n");
  
  QPainter paint(lnumXpm_);
  paint.setBrushOrigin(- lnumRect_.x(), - lnumRect_.y());
  paint.fillRect(0, 0, lnumRect_.width(), lnumRect_.height(), background_);

  paint.setFont(statusFont_);
  paint.setPen(QColor(255,0,0));
  paint.drawText(0, 0, lnumRect_.width(), lnumRect_.height(),
                 Qt::AlignLeft, QString::asprintf("%05d", level()+1));
}

void
PlayField::updateStepsXpm() {
  if (!snumXpm_) return;
  if (snumXpm_->isNull()) return;
  //printf("executing PlayField::updateStepsXpm()\n");
  
  QPainter paint(snumXpm_);
  paint.setBrushOrigin(- snumRect_.x(), - snumRect_.y());
  paint.fillRect(0, 0, snumRect_.width(), snumRect_.height(), background_);

  paint.setFont(statusFont_);
  paint.setPen(QColor(255,0,0));
  paint.drawText(0, 0, snumRect_.width(), snumRect_.height(),
                 Qt::AlignLeft, QString::asprintf("%05d", totalMoves()));
}

void
PlayField::updatePushesXpm() {
  if (!pnumXpm_) return;
  if (pnumXpm_->isNull()) return;
  //printf("executing PlayField::updatePushesXpm()\n"); 
  
  QPainter paint(pnumXpm_);
  paint.setBrushOrigin(- pnumRect_.x(), - pnumRect_.y());
  paint.fillRect(0, 0, pnumRect_.width(), pnumRect_.height(), background_);

  paint.setFont(statusFont_);
  paint.setPen(QColor(255,0,0));
  paint.drawText(0, 0, pnumRect_.width(), pnumRect_.height(),
                 Qt::AlignLeft, QString::asprintf("%05d", totalPushes()));
}


void
PlayField::changeAnim(int num)
{
  assert(num >= 0 && num <= 3);

  animDelay_ = num;
}

// FIXME: clean up bookmark stuff

// static const int bookmark_id[] = {
//   0, 1, 8, 2, 9, 3, 5, 6, 7, 4
// };

void
PlayField::setBookmark(Bookmark *bm) {
  if (!levelMap_->goodLevel()) return;

  if (collection()->id() < 0) {
	QMessageBox::critical(this, tr("Error"), tr("Sorry, bookmarks for external levels\n"
				  "is not implemented yet."));
    return;
  }

  bm->set(collection()->id(), levelMap_->level(), levelMap_->totalMoves(), history_);
}

void
PlayField::goToBookmark(Bookmark *bm) {
  level(bm->level());
  levelChange();
  if (!bm->goTo(levelMap_, history_)) fprintf(stderr, "Warning: bad bookmark\n");
  //updateLevelXpm();
  updateStepsXpm();
  updatePushesXpm();
  repaint();
}

bool
PlayField::canMoveNow() {
  if (moveInProgress_) return false;
  if (!levelMap_->goodLevel()) {
	ModalLabel::message(tr("This level is broken"), this);
    return false;
  }
  return true;
}

void
PlayField::killTimers() {
  for(QList<int>::Iterator it=timers.begin(); it!=timers.end(); it++) {
	killTimer(*it);
  }
  timers.clear();
}
