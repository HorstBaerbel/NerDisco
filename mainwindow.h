#pragma once

#include "deck.h"
#include "displaythread.h"

#include <QMainWindow>
#include <QTimer>


namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    QImage currentImage() const;
    QImage realImage() const;
    ~MainWindow();

protected slots:
    void updateCurrentImage();
    void updateMenu();
    void updateSettingsFromUi();

    void loadDeckA(bool checked = false);
    void saveDeckA(bool checked = false);
    void saveAsDeckA(bool checked = false);
    void loadDeckB(bool checked = false);
    void saveDeckB(bool checked = false);
    void saveAsDeckB(bool checked = false);

    void showResponse(const QString &s);
    void processError(const QString &s);
    void processTimeout(const QString &s);

public slots:
    void showSettings();
    void exitApplication();

private:
    Ui::MainWindow *ui;
    Deck * m_deckA;
    Deck * m_deckB;
    QTimer m_displayTimer;
    QImage m_currentImage;
    QImage m_realImage;
    DisplayThread m_displayThread;
};
