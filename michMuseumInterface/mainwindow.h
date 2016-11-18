#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QList>
#include <QString>
#include <QWidget>
#include <QLabel>
#include <QAction>
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include "museumscene.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum stateType {
        STANDBY,
        PLAYING
    };

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void toggleFullScreen();
    void openSceneDescriptionFile();
    void showBanner();
    void playScene();
    void selectRandomScene();
    void selectRandomBanner();
    void audioStateChanged(QMediaPlayer::State state);
    void audioProgress(qint64 position);
    void randomizeScene();

    void slotTimer();
    void FadeIn();
    void FadeOut();

protected:
    void keyPressEvent(QKeyEvent* e);

private:
    void clearMessage();

    QTimer* fadeInTimer;
    QTimer* fadeOutTimer;

    Ui::MainWindow *ui;
    QList<museumScene> scenes;
    QStringList banners;
    int bannerFieldCount;
    int sceneFieldCount;
    QSettings* settings;
    stateType mCurrentState;
    QDir mDir;
    QMediaPlayer* mPlayer;

    QLabel* mMainScreen;
    QSerialPort* arduino;

    QTimer* testTimer;
    bool testToggle;
    char message[25];
    int mFadeSetting;
    int mFadeStep;
};

#endif // MAINWINDOW_H
