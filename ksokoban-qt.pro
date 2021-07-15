QT += core gui widgets

CONFIG += c++11 release
CONFIG -= debug_and_release debug_and_release_target

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
	Bookmark.h \
	History.h \
	HtmlPrinter.h \
	ImageData.h \
	InternalCollections.h \
	LevelCollection.h \
	LevelMap.h \
	MainWindow.h \
	Map.h \
	MapDelta.h \
	ModalLabel.h \
	Move.h \
	MoveSequence.h \
	PathFinder.h \
	PlayField.h \
	Queue.h \
	StaticImage.h

SOURCES += \
	Bookmark.cpp \
	History.cpp \
	HtmlPrinter.cpp \
	ImageData.cpp \
	InternalCollections.cpp \
	LevelCollection.cpp \
	LevelMap.cpp \
	MainWindow.cpp \
	Map.cpp \
	MapDelta.cpp \
	ModalLabel.cpp \
	Move.cpp \
	MoveSequence.cpp \
	PathFinder.cpp \
	PlayField.cpp \
	StaticImage.cpp \
	main.cpp

RESOURCES += \
	ksokoban.qrc
