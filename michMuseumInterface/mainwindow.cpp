#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QTextStream>
#include <QTime>
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mFadeSetting = 0;
    mFadeStep = 5;

    fadeInTimer = new QTimer(this);
    fadeInTimer->setInterval(20);
    fadeInTimer->setSingleShot(false);
    connect(fadeInTimer, SIGNAL(timeout()), SLOT(FadeIn()));
    fadeOutTimer = new QTimer(this);
    fadeOutTimer->setInterval(20);
    fadeOutTimer->setSingleShot(false);
    connect(fadeOutTimer, SIGNAL(timeout()), SLOT(FadeOut()));

    for (int i=0; i < 25; i++)
    {
        message[i] = 0;
    }
    message[0] = ':';

    mPlayer = new QMediaPlayer(this);
    connect(mPlayer, SIGNAL(stateChanged(QMediaPlayer::State)),
            SLOT(audioStateChanged(QMediaPlayer::State)));
    connect(mPlayer, SIGNAL(positionChanged(qint64)),
            SLOT(audioProgress(qint64)));

    bannerFieldCount = 2;
    sceneFieldCount = museumScene::fieldCount();

    connect(ui->actionOpen_Scene_Description, SIGNAL(triggered()),
            SLOT(openSceneDescriptionFile()));

    connect(ui->pbShowBanner, SIGNAL(clicked()),
            SLOT(showBanner()));

    connect(ui->pbPlayScene, SIGNAL(clicked()),
            SLOT(playScene()));

    settings = new QSettings("FP2014", "michMuseum",this);

    // seed random number generator
    qsrand(QTime::currentTime().msec());

    ui->sbCurrentBanner->setRange(0,0);
    ui->sbCurrentBanner->setEnabled(false);
    ui->pbShowBanner->setEnabled(false);
    ui->leCurrentBanner->setText("<No banners loaded>");
    ui->leCurrentBanner->setEnabled(false);
    ui->sbCurrentScene->setRange(0,0);
    ui->sbCurrentScene->setEnabled(false);
    ui->pbPlayScene->setEnabled(false);
    ui->leSceneName->setText("<No scenes loaded>");
    ui->leSceneName->setEnabled(false);
    ui->leMuseumName->setText("");
    ui->leMuseumName->setEnabled(false);
    ui->leMuseumLocation->setText("");
    ui->leMuseumLocation->setEnabled(false);
    ui->leAudioFile->setText("");
    ui->leAudioFile->setEnabled(false);

    QImage img(32,32, QImage::Format_ARGB32);
    img.fill(QColor(0,0,0));
    ui->lblPod0->setPixmap(QPixmap::fromImage(img));
    ui->lblPod1->setPixmap(QPixmap::fromImage(img));
    ui->lblPod2->setPixmap(QPixmap::fromImage(img));
    ui->lblPod3->setPixmap(QPixmap::fromImage(img));
    ui->lblPod4->setPixmap(QPixmap::fromImage(img));
    ui->lblPod5->setPixmap(QPixmap::fromImage(img));
    ui->lblPod6->setPixmap(QPixmap::fromImage(img));
    ui->lblPod7->setPixmap(QPixmap::fromImage(img));

    mCurrentState = STANDBY;

    mMainScreen = new QLabel();
    QFont font = mMainScreen->font();
    //qDebug("Default font size %d (%d)", font.pointSize(), font.pixelSize());
    font.setPointSize(font.pointSize() * 3);
    mMainScreen->setFont(font);
    mMainScreen->setText("Please load description");
    mMainScreen->setWordWrap(true);
    mMainScreen->setAutoFillBackground(true);
    mMainScreen->setAlignment(Qt::AlignCenter);
    mMainScreen->setMinimumSize(800,400);
    QPalette pal = mMainScreen->palette();
    pal.setColor(QPalette::Background, QColor(0,0,0));
    pal.setColor(QPalette::Foreground, QColor(255,255,255));
    mMainScreen->setPalette(pal);
    mMainScreen->show();

    // set up serial port
    arduino = new QSerialPort(this);

    testTimer = new QTimer(this);
    connect(testTimer, SIGNAL(timeout()),
            SLOT(slotTimer()));
    testTimer->setSingleShot(false);
    testTimer->setInterval(1000);
    testToggle = true;

    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    foreach (QSerialPortInfo port, ports) {
        qDebug("Serial port found: %s, made by %s, %s",
               port.portName().toLatin1().data(),
               port.manufacturer().toLatin1().data(),
               port.description().toLatin1().data());
        if (port.manufacturer().contains("Arduino"))
        {
            arduino->setPort(port);
            qDebug("Setting serial port to %s", port.portName().toLatin1().data());
            if (arduino->open(QIODevice::ReadWrite))
            {
                arduino->setBaudRate(QSerialPort::Baud115200);
                arduino->setParity(QSerialPort::NoParity);
                arduino->setStopBits(QSerialPort::OneStop);
                //testTimer->start();
            } else {
                qDebug("Failed to open %s", port.portName().toLatin1().data());
            }
        }
    }
}

MainWindow::~MainWindow()
{
    if (arduino->isOpen())
    {
        // black out the lights
        clearMessage();

        arduino->write(message,25);
        arduino->flush();
        arduino->close();
    }
    delete ui;
    delete mMainScreen;
}

void MainWindow::toggleFullScreen()
{
    mMainScreen->setWindowState(mMainScreen->windowState() ^ Qt::WindowFullScreen);
}

void MainWindow::openSceneDescriptionFile()
{
    QString defaultDir = settings->value("defaultDir", QString()).toString();
    //qDebug("defaultDir: %s", defaultDir.toLatin1().data());
    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Select scene description file...",
                                                    defaultDir + "/blah",
                                                    "Scene files (*.scn)");
    if (filename.isEmpty())
    {
        return;
    }

    QFile sceneFile(filename);
    if (!sceneFile.open(QIODevice::ReadOnly))
    {
        qDebug("Unable to open %s", filename.toLatin1().data());
        return;
    }
    // save location to settings to expedite future loads
    QFileInfo info(filename);
    mDir = info.absoluteDir();
    settings->setValue("defaultDir", info.absoluteDir().absolutePath());

    // clear old scenes & banners
    scenes.clear();
    banners.clear();

    // read in the new ones
    QTextStream ts(&sceneFile);
    QString currLine;
    while (!ts.atEnd())
    {
        currLine = ts.readLine().trimmed();
        if (!currLine.isNull())
        {
            if (currLine.startsWith("b", Qt::CaseInsensitive))
            {
                // it's a banner
                QStringList bannerSplit = currLine.split(":", QString::SkipEmptyParts);
                if (bannerSplit.count() == bannerFieldCount)
                {
                    banners.append(bannerSplit[1]);
                    //qDebug("Added \"%s\" (size %d) to banners",
                    //       bannerSplit[1].toLatin1().data(),
                    //       bannerSplit[1].size());
                } else {
                    qDebug("Wrong number of fields for banner line, got %d, expected %d",
                           bannerSplit.count(), bannerFieldCount);
                }
            } else if (currLine.startsWith("s", Qt::CaseInsensitive))
            {
                // oh it's a scene, man
                QStringList sceneSplit = currLine.split(":", QString::SkipEmptyParts);
                if (sceneSplit.count() == sceneFieldCount)
                {
                    museumScene scene;
                    scene.populateFromString(currLine, ':');
                    info.setFile(mDir, scene.audioFile());
                    if (!info.exists())
                    {
                        qDebug("Unable to find audio file: %s", scene.audioFile().toLatin1().data());
                    }
                    scenes.append(scene);
                } else {
                    qDebug("Wrong number of fields for scene line, got %d, expected %d, offending line: %s",
                           sceneSplit.count(), sceneFieldCount, currLine.toLatin1().data());
                }
            } else if (currLine.startsWith("#")) {
                // it's a comment, ignore
            } else {
                // unknown line
                qDebug("Unknown line: \"%s\"", currLine.toLatin1().data());
            }
        } // if (!currLine.isNull())
    } // end while
    if (banners.count() > 0)
    {
        ui->sbCurrentBanner->setRange(0, banners.count() - 1);
        ui->sbCurrentBanner->setEnabled(true);
        ui->pbShowBanner->setEnabled(true);
        ui->leCurrentBanner->setText("");
        ui->leCurrentBanner->setEnabled(true);
        ui->lblBannerCount->setText(QString::number(banners.count()) + QString(" Banners"));

        selectRandomBanner();
    } else {
        ui->sbCurrentBanner->setRange(0,0);
        ui->sbCurrentBanner->setEnabled(false);
        ui->pbShowBanner->setEnabled(false);
        ui->leCurrentBanner->setText("<No banners loaded>");
        ui->leCurrentBanner->setEnabled(false);
    }

    if (scenes.count() > 0)
    {
        ui->sbCurrentScene->setRange(0,scenes.count() - 1);
        ui->sbCurrentScene->setEnabled(true);
        ui->pbPlayScene->setEnabled(true);
        ui->leSceneName->setText("");
        ui->leSceneName->setEnabled(true);
        ui->leMuseumName->setText("");
        ui->leMuseumName->setEnabled(true);
        ui->leMuseumLocation->setText("");
        ui->leMuseumLocation->setEnabled(true);
        ui->leAudioFile->setText("");
        ui->leAudioFile->setEnabled(true);
    } else {
        ui->sbCurrentScene->setRange(0,0);
        ui->sbCurrentScene->setEnabled(false);
        ui->pbPlayScene->setEnabled(false);
        ui->leSceneName->setText("<No scenes loaded>");
        ui->leSceneName->setEnabled(false);
        ui->leMuseumName->setText("");
        ui->leMuseumName->setEnabled(false);
        ui->leMuseumLocation->setText("");
        ui->leMuseumLocation->setEnabled(false);
        ui->leAudioFile->setText("");
        ui->leAudioFile->setEnabled(false);
    }

    ui->lblBannerCount->setText(QString::number(banners.count()) + QString(" Banners"));
    ui->lblSceneCount->setText(QString::number(scenes.count()) + QString(" Scenes"));
}

void MainWindow::showBanner()
{
    int desiredBanner = ui->sbCurrentBanner->value();
    ui->leCurrentBanner->setText(banners[desiredBanner]);
    mMainScreen->setText(banners[desiredBanner]);
}

void MainWindow::playScene()
{
    int sceneIndex = ui->sbCurrentScene->value();
    museumScene scene = scenes[sceneIndex];
    QFileInfo info;
    info.setFile(mDir, scene.audioFile());
    ui->leSceneName->setText(scene.sceneName());
    ui->leMuseumName->setText(scene.museumName());
    ui->leMuseumLocation->setText(scene.museumLocation());
    ui->leAudioFile->setText(scene.audioFile());

    QString mainScreenString = QString("<font size=\"6\">") + scene.sceneName() + QString("</font><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p>") +
            scene.museumName() + QString("<p>") +
            scene.museumLocation();
    mMainScreen->setText(mainScreenString);

    // write to lights
    //arduino->write(message, 25);
    fadeInTimer->start();

    // audio
    if (ui->chkDisableAudio->isChecked())
    {
        // don't play audio
    } else {
        mCurrentState = PLAYING;
        qDebug("Playing");
        mPlayer->setMedia(QUrl::fromLocalFile(info.absoluteFilePath()));
        mPlayer->setVolume(100);
        mPlayer->play();
    }
}

void MainWindow::selectRandomScene()
{
    if (scenes.count() <= 0)
    {
        return;
    }
    fadeInTimer->stop();
    fadeOutTimer->stop();

    if (mCurrentState == PLAYING)
    {
        mPlayer->stop();
    }

    randomizeScene();

    int randomScene = qrand() % scenes.count();
    ui->sbCurrentScene->setValue(randomScene);
    playScene();
}

void MainWindow::selectRandomBanner()
{
    int randomBanner = qrand() % banners.count();
    ui->sbCurrentBanner->setValue(randomBanner);
    showBanner();
}

void MainWindow::audioStateChanged(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::StoppedState)
    {
        ui->prgAudioProgress->setValue(100);
        fadeOutTimer->start();
        qDebug("Standby");
        mCurrentState = STANDBY;
        selectRandomBanner();
    }
}

void MainWindow::audioProgress(qint64 position)
{
    ui->prgAudioProgress->setValue(position / (double)(mPlayer->duration()) * 100.);
}

void MainWindow::randomizeScene()
{
    QVector<int> times;
    times.append(30);
    times.append(30);
    times.append(30);
    times.append(30);
    times.append(30);
    times.append(30);
    times.append(50);
    times.append(50);
    times.append(50);
    times.append(50);
    times.append(50);
    times.append(75);
    times.append(75);
    times.append(75);
    times.append(75);
    times.append(100);
    times.append(100);
    times.append(100);
    times.append(200);
    times.append(200);
    times.append(300);
    times.append(500);
    times.append(1000);

    foreach (int time, times)
    {
        usleep(time * 1000); // delay
        int sceneIndex = qrand() % scenes.count();
        museumScene scene = scenes[sceneIndex];

        QString mainScreenString = QString("<font size=\"6\">") + scene.sceneName() + QString("</font><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p>") +
                scene.museumName() + QString("<p>") +
                scene.museumLocation();
        mMainScreen->setText(mainScreenString);
        qApp->processEvents();
    }

}

void MainWindow::slotTimer()
{
    char c[1];
    if (testToggle)
    {
        c[0] = ':';
    } else {
        c[0] = '.';
    }
    arduino->write(c,1);
    arduino->flush();
    testToggle = !testToggle;
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
    {
        // toggle full screen
        qDebug("Escape key pressed");
        toggleFullScreen();
    } else if (e->key() == Qt::Key_Space)
    {
        //if (mCurrentState == STANDBY)
        {
            qDebug("Space key pressed, selecting random scene");
            selectRandomScene();
        }
    }
    e->accept();
}

void MainWindow::clearMessage()
{
    // sets data portion of message to all 0 (all lights black)
    memset(message+1,0,24);
}

void MainWindow:: FadeIn()
{
    museumScene scene = scenes[ui->sbCurrentScene->value()];
    if (mFadeSetting < 255)
    {
        QImage img(32,32, QImage::Format_ARGB32);

        img.fill(QColor(scene.lightColor(0).red() * mFadeSetting / 255.,
                        scene.lightColor(0).green() * mFadeSetting / 255.,
                         scene.lightColor(0).blue() * mFadeSetting / 255.));
        message[1] = scene.lightColor(0).red() * mFadeSetting / 255.;
        message[2] = scene.lightColor(0).green() * mFadeSetting / 255.;
        message[3] = scene.lightColor(0).blue() * mFadeSetting / 255.;
        ui->lblPod0->setPixmap(QPixmap::fromImage(img));
        img.fill(QColor(scene.lightColor(1).red() * mFadeSetting / 255.,
                        scene.lightColor(1).green() * mFadeSetting / 255.,
                         scene.lightColor(1).blue() * mFadeSetting / 255.));
        message[4] = scene.lightColor(1).red() * mFadeSetting / 255.;
        message[5] = scene.lightColor(1).green() * mFadeSetting / 255.;
        message[6] = scene.lightColor(1).blue() * mFadeSetting / 255.;
        ui->lblPod1->setPixmap(QPixmap::fromImage(img));
        img.fill(QColor(scene.lightColor(2).red() * mFadeSetting / 255.,
                        scene.lightColor(2).green() * mFadeSetting / 255.,
                         scene.lightColor(2).blue() * mFadeSetting / 255.));
        message[7] = scene.lightColor(2).red() * mFadeSetting / 255.;
        message[8] = scene.lightColor(2).green() * mFadeSetting / 255.;
        message[9] = scene.lightColor(2).blue() * mFadeSetting / 255.;
        ui->lblPod2->setPixmap(QPixmap::fromImage(img));
        img.fill(QColor(scene.lightColor(3).red() * mFadeSetting / 255.,
                        scene.lightColor(3).green() * mFadeSetting / 255.,
                         scene.lightColor(3).blue() * mFadeSetting / 255.));
        message[10] = scene.lightColor(3).red() * mFadeSetting / 255.;
        message[11] = scene.lightColor(3).green() * mFadeSetting / 255.;
        message[12] = scene.lightColor(3).blue() * mFadeSetting / 255.;
        ui->lblPod3->setPixmap(QPixmap::fromImage(img));
        img.fill(QColor(scene.lightColor(4).red() * mFadeSetting / 255.,
                        scene.lightColor(4).green() * mFadeSetting / 255.,
                         scene.lightColor(4).blue() * mFadeSetting / 255.));
        message[13] = scene.lightColor(4).red() * mFadeSetting / 255.;
        message[14] = scene.lightColor(4).green() * mFadeSetting / 255.;
        message[15] = scene.lightColor(4).blue() * mFadeSetting / 255.;
        ui->lblPod4->setPixmap(QPixmap::fromImage(img));
        img.fill(QColor(scene.lightColor(5).red() * mFadeSetting / 255.,
                        scene.lightColor(5).green() * mFadeSetting / 255.,
                         scene.lightColor(5).blue() * mFadeSetting / 255.));
        message[16] = scene.lightColor(5).red() * mFadeSetting / 255.;
        message[17] = scene.lightColor(5).green() * mFadeSetting / 255.;
        message[18] = scene.lightColor(5).blue() * mFadeSetting / 255.;
        ui->lblPod5->setPixmap(QPixmap::fromImage(img));
        img.fill(QColor(scene.lightColor(6).red() * mFadeSetting / 255.,
                        scene.lightColor(6).green() * mFadeSetting / 255.,
                         scene.lightColor(6).blue() * mFadeSetting / 255.));
        message[19] = scene.lightColor(6).red() * mFadeSetting / 255.;
        message[20] = scene.lightColor(6).green() * mFadeSetting / 255.;
        message[21] = scene.lightColor(6).blue() * mFadeSetting / 255.;
        ui->lblPod6->setPixmap(QPixmap::fromImage(img));
        img.fill(QColor(scene.lightColor(7).red() * mFadeSetting / 255.,
                        scene.lightColor(7).green() * mFadeSetting / 255.,
                         scene.lightColor(7).blue() * mFadeSetting / 255.));
        message[22] = scene.lightColor(7).red() * mFadeSetting / 255.;
        message[23] = scene.lightColor(7).green() * mFadeSetting / 255.;
        message[24] = scene.lightColor(7).blue() * mFadeSetting / 255.;
        ui->lblPod7->setPixmap(QPixmap::fromImage(img));

        arduino->write(message, 25);
        arduino->flush();
        mFadeSetting += mFadeStep;
    } else {
        mFadeSetting = 255;
        fadeInTimer->stop();
    }
}

void MainWindow::FadeOut()
{
    museumScene scene = scenes[ui->sbCurrentScene->value()];
    if (mFadeSetting > 5)
    {
        QImage img(32,32, QImage::Format_ARGB32);

        img.fill(QColor(scene.lightColor(0).red() * mFadeSetting / 255.,
                        scene.lightColor(0).green() * mFadeSetting / 255.,
                         scene.lightColor(0).blue() * mFadeSetting / 255.));
        message[1] = scene.lightColor(0).red() * mFadeSetting / 255.;
        message[2] = scene.lightColor(0).green() * mFadeSetting / 255.;
        message[3] = scene.lightColor(0).blue() * mFadeSetting / 255.;
        ui->lblPod0->setPixmap(QPixmap::fromImage(img));
        img.fill(QColor(scene.lightColor(1).red() * mFadeSetting / 255.,
                        scene.lightColor(1).green() * mFadeSetting / 255.,
                         scene.lightColor(1).blue() * mFadeSetting / 255.));
        message[4] = scene.lightColor(1).red() * mFadeSetting / 255.;
        message[5] = scene.lightColor(1).green() * mFadeSetting / 255.;
        message[6] = scene.lightColor(1).blue() * mFadeSetting / 255.;
        ui->lblPod1->setPixmap(QPixmap::fromImage(img));
        img.fill(QColor(scene.lightColor(2).red() * mFadeSetting / 255.,
                        scene.lightColor(2).green() * mFadeSetting / 255.,
                         scene.lightColor(2).blue() * mFadeSetting / 255.));
        message[7] = scene.lightColor(2).red() * mFadeSetting / 255.;
        message[8] = scene.lightColor(2).green() * mFadeSetting / 255.;
        message[9] = scene.lightColor(2).blue() * mFadeSetting / 255.;
        ui->lblPod2->setPixmap(QPixmap::fromImage(img));
        img.fill(QColor(scene.lightColor(3).red() * mFadeSetting / 255.,
                        scene.lightColor(3).green() * mFadeSetting / 255.,
                         scene.lightColor(3).blue() * mFadeSetting / 255.));
        message[10] = scene.lightColor(3).red() * mFadeSetting / 255.;
        message[11] = scene.lightColor(3).green() * mFadeSetting / 255.;
        message[12] = scene.lightColor(3).blue() * mFadeSetting / 255.;
        ui->lblPod3->setPixmap(QPixmap::fromImage(img));
        img.fill(QColor(scene.lightColor(4).red() * mFadeSetting / 255.,
                        scene.lightColor(4).green() * mFadeSetting / 255.,
                         scene.lightColor(4).blue() * mFadeSetting / 255.));
        message[13] = scene.lightColor(4).red() * mFadeSetting / 255.;
        message[14] = scene.lightColor(4).green() * mFadeSetting / 255.;
        message[15] = scene.lightColor(4).blue() * mFadeSetting / 255.;
        ui->lblPod4->setPixmap(QPixmap::fromImage(img));
        img.fill(QColor(scene.lightColor(5).red() * mFadeSetting / 255.,
                        scene.lightColor(5).green() * mFadeSetting / 255.,
                         scene.lightColor(5).blue() * mFadeSetting / 255.));
        message[16] = scene.lightColor(5).red() * mFadeSetting / 255.;
        message[17] = scene.lightColor(5).green() * mFadeSetting / 255.;
        message[18] = scene.lightColor(5).blue() * mFadeSetting / 255.;
        ui->lblPod5->setPixmap(QPixmap::fromImage(img));
        img.fill(QColor(scene.lightColor(6).red() * mFadeSetting / 255.,
                        scene.lightColor(6).green() * mFadeSetting / 255.,
                         scene.lightColor(6).blue() * mFadeSetting / 255.));
        message[19] = scene.lightColor(6).red() * mFadeSetting / 255.;
        message[20] = scene.lightColor(6).green() * mFadeSetting / 255.;
        message[21] = scene.lightColor(6).blue() * mFadeSetting / 255.;
        ui->lblPod6->setPixmap(QPixmap::fromImage(img));
        img.fill(QColor(scene.lightColor(7).red() * mFadeSetting / 255.,
                        scene.lightColor(7).green() * mFadeSetting / 255.,
                         scene.lightColor(7).blue() * mFadeSetting / 255.));
        message[22] = scene.lightColor(7).red() * mFadeSetting / 255.;
        message[23] = scene.lightColor(7).green() * mFadeSetting / 255.;
        message[24] = scene.lightColor(7).blue() * mFadeSetting / 255.;
        ui->lblPod7->setPixmap(QPixmap::fromImage(img));

        arduino->write(message, 25);
        arduino->flush();
        mFadeSetting -= mFadeStep;
    } else {
        mFadeSetting = 0;
        clearMessage();
        arduino->write(message, 25);
        arduino->flush();
        fadeOutTimer->stop();
    }
}
