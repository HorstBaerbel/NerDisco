greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets serialport multimedia opengl
} else {
    include($$QTSERIALPORT_PROJECT_ROOT/src/serialport/qt4support/serialport.prf)
}

TARGET = nerdisco
TEMPLATE = app

HEADERS += \
    QTextEditLineNumberArea.h \
    CodeEdit.h \
    ColorOperations.h \
    QTextEditStatusArea.h \
    AudioInterface.h \
    Deck.h \
    DisplayThread.h \
    LiveView.h \
    MainWindow.h \
    Settings.h \
    SettingsDialog.h \
    AudioConversion.h \
    AudioProcessing.h \
    MIDIControlConnection.h \
    MIDIControlMapper.h \
    MIDIInterface.h \
    MIDIWorker.h \
    SignalJoiner.h \
    SwapThread.h \
    rtmidi/RtMidi.h

SOURCES += \
    QTextEditLineNumberArea.cpp \
    CodeEdit.cpp \
    QTextEditStatusArea.cpp \
    ColorOperations.cpp \
    NerDisco.cpp \
    AudioInterface.cpp \
    Deck.cpp \
    DisplayThread.cpp \
    LiveView.cpp \
    MainWindow.cpp \
    Settings.cpp \
    SettingsDialog.cpp \
    AudioConversion.cpp \
    AudioProcessing.cpp \
    MIDIControlConnection.cpp \
    MIDIControlMapper.cpp \
    MIDIInterface.cpp \
    MIDIWorker.cpp \
    SignalJoiner.cpp \
    SwapThread.cpp \
    rtmidi/RtMidi.cpp

FORMS += \
    MainWindow.ui \
    SettingsDialog.ui \
    Deck.ui

RESOURCES += \
    resources.qrc
