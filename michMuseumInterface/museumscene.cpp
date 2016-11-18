#include "museumscene.h"

museumScene::museumScene()
{
}

QString museumScene::sceneName() const
{
    return mSceneName;
}

QString museumScene::museumName() const
{
    return mMuseumName;
}

QString museumScene::museumLocation() const
{
    return mMuseumLocation;
}

QColor museumScene::lightColor(int index) const
{
    if (index < 0 || index > 7)
    {
        return QColor(0,0,0);
    } else {
        return mLights[index];
    }
}

QString museumScene::audioFile() const
{
    return mAudioFile;
}

bool museumScene::populateFromString(QString string, QChar separator)
{
    QStringList fields = string.split(separator);
    if (fields.count() != fieldCount())
    {
        qDebug("Error parsing string, expected %d fields, got %d",
               fieldCount(), fields.count());
        return false;
    } else {
        mSceneName = fields[1];
        mMuseumName = fields[2];
        mMuseumLocation = fields[3];
        mAudioFile = fields[4];
        bool ok;
        for (int i=0; i < 8; i++)
        {
            unsigned int color = (fields[5 + i].toUInt(&ok, 16)) | 0xff000000;
            if (ok)
            {
                mLights[i] = QColor::fromRgba(color);
            } else {
                return false;
            }
        }
        return true;
    }
}
