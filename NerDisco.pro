greaterThan(QT_MAJOR_VERSION, 4) {
    QT       += widgets serialport quick multimedia
} else {
    include($$QTSERIALPORT_PROJECT_ROOT/src/serialport/qt4support/serialport.prf)
}

TARGET = nerdisco
TEMPLATE = app

HEADERS += \
    mainwindow.h \
    settingsdialog.h \
    deck.h \
    liveview.h \
    settings.h \
    displaythread.h \
    QTextEditLineNumberArea.h \
    CodeEdit.h \
    ColorOperations.h \
    AudioThread.h \
    QTextEditStatusArea.h

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    deck.cpp \
    liveview.cpp \
    settings.cpp \
    displaythread.cpp \
    QTextEditLineNumberArea.cpp \
    CodeEdit.cpp \
    AudioThread.cpp \
    QTextEditStatusArea.cpp \
    ColorOperations.cpp

FORMS += \
    mainwindow.ui \
    settingsdialog.ui \
    codedeck.ui

RESOURCES += \
    resources.qrc
