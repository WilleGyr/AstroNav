#include "music.h"
#include <QFile>
#include <QFileInfo>
#include <QResource>
#include <QDir>
#include <QDebug>
#include <QSoundEffect>
#include <Qt3DCore>
#include <QUrl>

BackgroundMusic::BackgroundMusic(QObject *parent)
    : QObject(parent)
    , m_player(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
    , m_looping(true)
{
    m_player->setAudioOutput(m_audioOutput);

    // Set default volume to 50%
    m_audioOutput->setVolume(0.5);

    // Handle looping
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia && m_looping) {
            m_player->setPosition(0);
            m_player->play();
        }
    });

    // Handle errors
    connect(m_player, &QMediaPlayer::errorOccurred, this, [](QMediaPlayer::Error error, const QString &errorString) {
        qWarning() << "Media player error:" << error << errorString;
    });
}

BackgroundMusic::~BackgroundMusic()
{
    // Stop playback before destruction
    stop();
}

void BackgroundMusic::play()
{
    m_player->play();
}

void BackgroundMusic::pause()
{
    m_player->pause();
}

void BackgroundMusic::stop()
{
    m_player->stop();
}

void BackgroundMusic::setVolume(float volume)
{
    // Ensure volume is between 0.0 and 1.0
    volume = qBound(0.0f, volume, 1.0f);
    m_audioOutput->setVolume(volume);
}

void BackgroundMusic::setLooping(bool loop)
{
    m_looping = loop;
}

bool BackgroundMusic::loadMusic(const QString &filePath)
{
    QUrl url;

    // Check if the file exists on the filesystem
    QFileInfo fileInfo(filePath);
    if (fileInfo.exists() && fileInfo.isFile()) {
        url = QUrl::fromLocalFile(filePath);
    }
    // Check if it's a resource file
    else if (filePath.startsWith(":/") || filePath.startsWith("qrc:/")) {
        url = QUrl(filePath);
    }
    // Try to interpret as a URL
    else {
        url = QUrl(filePath);
    }

    if (url.isValid()) {
        m_player->setSource(url);
        return true;
    }

    qWarning() << "Failed to load music from:" << filePath;
    return false;
}

void BackgroundMusic::playSoundEffect(Qt3DCore::QEntity* parentEntity, const QString& filepath) {
    QSoundEffect* clickSound = new QSoundEffect(parentEntity);
    clickSound->setSource(QUrl(filepath));
    clickSound->setVolume(0.8f);
    clickSound->play();
}
