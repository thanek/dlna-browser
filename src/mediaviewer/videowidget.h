#pragma once
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoSink>
#include <QVideoFrame>
#include <QTimer>
#include <QPixmap>
#include <QNetworkAccessManager>
#include "dlna/dlnaitem.h"
#include "mediaviewer/autohideoverlay.h"
#include "mediaviewer/mediawidget.h"

class ControlOverlay : public AutoHideOverlay {
    Q_OBJECT
public:
    explicit ControlOverlay(QWidget *parent = nullptr);

    void setTitle(const QString &title);
    void setPosition(qint64 pos, qint64 duration);
    void setPlaying(bool playing);
    void setMuted(bool muted);
    void setAudioMode(bool audio, const QPixmap &art = {});
    void showControls();

signals:
    void playPauseClicked();
    void muteClicked();
    void seekRequested(double fraction);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;

private:
    void paintAudioCircle(QPainter &p);
    void paintTitleBar(QPainter &p);
    void paintControlBar(QPainter &p);
    void paintPlayIcon(QPainter &p, QRectF r, bool playing);

    QRectF controlBarRect() const;
    QRectF titleBarRect() const;
    QRectF progressBarRect() const;
    QRectF playButtonRect() const;
    QRectF muteButtonRect() const;

    QString m_title;
    qint64 m_position = 0;
    qint64 m_duration = 0;
    bool m_playing = false;
    bool m_muted = false;
    bool m_audioMode = false;
    QPixmap m_albumArt;

    static constexpr int ControlBarMaxWidth = 600;
    static constexpr QColor AccentColor{0x88, 0xc0, 0xd0};
};

class VideoWidget : public MediaWidget {
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    void loadItem(const DlnaItem &item);
    void stop();

protected:
    void resizeEvent(QResizeEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;

private:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPlayWatchdog();
    void togglePlayPause();
    void fetchAlbumArt(const QUrl &url);
    void paintEvent(QPaintEvent *) override;
    static QRectF letterboxRect(QSize content, QSize view);

    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    QVideoSink *m_videoSink;
    QVideoFrame m_currentFrame;
    ControlOverlay *m_overlay;
    QNetworkAccessManager *m_nam;
    QTimer *m_playWatchdog;
    QUrl m_currentSource;
    bool m_audioMode = false;
    bool m_pendingPlay = false;
    int m_playRetries = 0;

    static constexpr int WatchdogMs = 2000;
};
