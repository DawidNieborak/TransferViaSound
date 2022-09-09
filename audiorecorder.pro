TEMPLATE = app
TARGET = audiorecorder

QT += multimedia

win32:INCLUDEPATH += $$PWD

HEADERS = \
    Sound.h \
    asciibreaker.h \
    audiorecorder.h \
    audiolevel.h \
    decoder.h \
    extractor.h

SOURCES = \
    asciibreaker.cpp \
    decoder.cpp \
    extractor.cpp \
    main.cpp \
    audiorecorder.cpp \
    audiolevel.cpp

FORMS += audiorecorder.ui

target.path = $$[QT_INSTALL_EXAMPLES]/multimedia/audiorecorder
INSTALLS += target

QT+=widgets
include(../shared/shared.pri)


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/lib/release/ -lsfml-audio.2.5.1
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/lib/debug/ -lsfml-audio.2.5.1
else:unix: LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/lib/ -lsfml-audio.2.5.1

INCLUDEPATH += $$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/include
DEPENDPATH += $$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/include


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/lib/release/ -lsfml-graphics.2.5.1
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/lib/debug/ -lsfml-graphics.2.5.1
else:unix: LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/lib/ -lsfml-graphics.2.5.1

INCLUDEPATH += $$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/include
DEPENDPATH += $$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/lib/release/ -lsfml-system.2.5.1
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/lib/debug/ -lsfml-system.2.5.1
else:unix: LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/lib/ -lsfml-system.2.5.1

INCLUDEPATH += $$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/include
DEPENDPATH += $$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/lib/release/ -lsfml-window.2.5.1
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/lib/debug/ -lsfml-window.2.5.1
else:unix: LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/lib/ -lsfml-window.2.5.1

INCLUDEPATH += $$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/include
DEPENDPATH += $$PWD/../../../../../../../usr/local/Cellar/sfml/2.5.1_1/include


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/cpr/1.8.3/lib/release/ -lcpr.1.8.3
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/cpr/1.8.3/lib/debug/ -lcpr.1.8.3
else:unix: LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/cpr/1.8.3/lib/ -lcpr.1.8.3

INCLUDEPATH += $$PWD/../../../../../../../usr/local/Cellar/cpr/1.8.3/include
DEPENDPATH += $$PWD/../../../../../../../usr/local/Cellar/cpr/1.8.3/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/fftw/3.3.10_1/lib/release/ -lfftw3.3
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/fftw/3.3.10_1/lib/debug/ -lfftw3.3
else:unix: LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/fftw/3.3.10_1/lib/ -lfftw3.3

INCLUDEPATH += $$PWD/../../../../../../../usr/local/Cellar/fftw/3.3.10_1/include
DEPENDPATH += $$PWD/../../../../../../../usr/local/Cellar/fftw/3.3.10_1/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/jsoncpp/1.9.5/lib/release/ -ljsoncpp.25
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/jsoncpp/1.9.5/lib/debug/ -ljsoncpp.25
else:unix: LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/jsoncpp/1.9.5/lib/ -ljsoncpp.25

INCLUDEPATH += $$PWD/../../../../../../../usr/local/Cellar/jsoncpp/1.9.5/include
DEPENDPATH += $$PWD/../../../../../../../usr/local/Cellar/jsoncpp/1.9.5/include
