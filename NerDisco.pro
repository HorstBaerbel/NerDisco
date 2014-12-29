greaterThan(QT_MAJOR_VERSION, 4) {
    QT       += widgets serialport quick multimedia
} else {
    include($$QTSERIALPORT_PROJECT_ROOT/src/serialport/qt4support/serialport.prf)
}

TARGET = nerdisco
TEMPLATE = app

HEADERS += \
    QTextEditLineNumberArea.h \
    CodeEdit.h \
    ColorOperations.h \
    AudioThread.h \
    QTextEditStatusArea.h \
    Deck.h \
    DisplayThread.h \
    LiveView.h \
    MainWindow.h \
    Settings.h \
    SettingsDialog.h

SOURCES += \
    QTextEditLineNumberArea.cpp \
    CodeEdit.cpp \
    AudioThread.cpp \
    QTextEditStatusArea.cpp \
    ColorOperations.cpp \
    Deck.cpp \
    DisplayThread.cpp \
    LiveView.cpp \
    MainWindow.cpp \
    Settings.cpp \
    SettingsDialog.cpp \
    NerDisco.cpp

FORMS += \
    CodeDeck.ui \
    MainWindow.ui \
    SettingsDialog.ui

RESOURCES += \
    resources.qrc
