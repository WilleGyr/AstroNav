#ifndef MUSIC_H
#define MUSIC_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QString>
#include <QUrl>
#include <Qt3DCore>

class BackgroundMusic : public QObject
{
    Q_OBJECT

public:
    explicit BackgroundMusic(QObject *parent = nullptr);
    ~BackgroundMusic();

    void play();
    void pause();
    void stop();
    void setVolume(float volume);
    void setLooping(bool loop);
    bool loadMusic(const QString &filePath);
    void playSoundEffect(Qt3DCore::QEntity* parentEntity, const QString& filepath);


private:
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    bool m_looping;
};



#endif // MUSIC_H
