#ifndef MUSEUMSCENE_H
#define MUSEUMSCENE_H
#include <QString>
#include <QColor>

class museumScene
{
public:
    museumScene();
    QString sceneName() const;
    QString museumName() const;
    QString museumLocation() const;
    QColor lightColor(int index) const;
    QString audioFile() const;

    bool populateFromString(QString string, QChar separator=',');

    static int fieldCount() {return 13;}

private:
    QString mMuseumName;
    QString mMuseumLocation;
    QString mSceneName;
    QString mAudioFile;
    QColor mLights[8];
};

#endif // MUSEUMSCENE_H
