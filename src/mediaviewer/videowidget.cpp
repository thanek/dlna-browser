#include "mediaviewer/videowidget.h"
#include "dlna/dlnautils.h"
#include "ui/faicon.h"

#include <QPainter>
#include <QPainterPath>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFontMetrics>

// ─── ControlOverlay ────────────────────────────────────────────────────────

ControlOverlay::ControlOverlay(QWidget *parent)
    : QWidget(parent)
    , m_hideTimer(new QTimer(this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setMouseTracking(true);
    m_hideTimer->setSingleShot(true);
    m_hideTimer->setInterval(3000);
    connect(m_hideTimer, &QTimer::timeout, this, [this]() {
        m_controlsVisible = false;
        update();
    });
}

void ControlOverlay::setTitle(const QString &title)
{
    m_title = title;
    update();
}

void ControlOverlay::setPosition(qint64 pos, qint64 duration)
{
    m_position = pos;
    m_duration = duration;
    update();
}

void ControlOverlay::setPlaying(bool playing)
{
    m_playing = playing;
    update();
}

void ControlOverlay::setMuted(bool muted)
{
    m_muted = muted;
    update();
}

void ControlOverlay::setAudioMode(bool audio, const QPixmap &art)
{
    m_audioMode = audio;
    m_albumArt = art;
    update();
}

void ControlOverlay::showControls()
{
    m_controlsVisible = true;
    m_hideTimer->start();
    update();
}

// ─── Geometry helpers ───────────────────────────────────────────────────────

QRectF ControlOverlay::titleBarRect() const
{
    QFontMetrics fm(font());
    int textW = fm.horizontalAdvance(m_title);
    int bw = qMin(textW + 100, width() - 40);
    int bh = fm.height() + 24;
    return QRectF((width() - bw) / 2.0, 20, bw, bh);
}

QRectF ControlOverlay::controlBarRect() const
{
    int bw = qMin(600, width() - 80);
    int bh = 72;
    return QRectF((width() - bw) / 2.0, height() - bh - 20, bw, bh);
}

QRectF ControlOverlay::progressBarRect() const
{
    QRectF cb = controlBarRect();
    return QRectF(cb.left() + 16, cb.top() + 14, cb.width() - 32, 6);
}

QRectF ControlOverlay::playButtonRect() const
{
    QRectF cb = controlBarRect();
    return QRectF(cb.left() + 12, cb.top() + 28, 28, 28);
}

QRectF ControlOverlay::muteButtonRect() const
{
    QRectF cb = controlBarRect();
    return QRectF(cb.right() - 40, cb.top() + 28, 28, 28);
}

// ─── Painting ────────────────────────────────────────────────────────────────

void ControlOverlay::paintPlayIcon(QPainter &p, QRectF r, bool playing)
{
    uint glyph = playing ? Fa::Pause : Fa::Play;
    QPixmap px = FaIcon::pixmap(glyph, Qt::white, int(r.height()));
    p.drawPixmap(r.toRect(), px);
}


void ControlOverlay::paintTitleBar(QPainter &p)
{
    if (m_title.isEmpty()) return;
    QRectF r = titleBarRect();
    QPainterPath path;
    path.addRoundedRect(r, 20,  20);
    p.fillPath(path, QColor(0, 0, 0, 170));
    p.setPen(Qt::white);
    QFont f = font();
    f.setPointSize(14);
    p.setFont(f);
    p.drawText(r, Qt::AlignCenter, m_title);
}

void ControlOverlay::paintAudioCircle(QPainter &p)
{
    int d = int(qMin(width(), height())*0.75);
    QRect rect = QRect((width() - d) / 2, (height() - d) / 2, d, d);
    QPainterPath path;
    path.addRoundedRect(rect, 24, 24);
    p.fillPath(path, QColor(0x46, 0x46, 0x46));

    if (!m_albumArt.isNull()) {
        p.save();
        p.setClipPath(path);
        p.drawPixmap(rect, m_albumArt.scaled(rect.size(),
                     Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        p.restore();
    } else {
        constexpr uint MusicNote = 0xf001;
        int iconSize = d / 2;
        QPixmap px = FaIcon::pixmap(MusicNote, QColor(0xff, 0xff, 0xff), iconSize);
        p.drawPixmap(rect.center().x() - iconSize / 2,
                     rect.center().y() - iconSize / 2, px);
    }
}

void ControlOverlay::paintControlBar(QPainter &p)
{
    QRectF cb = controlBarRect();
    QPainterPath path;
    path.addRoundedRect(cb, 20, 20);
    p.fillPath(path, QColor(0, 0, 0, 200));

    // Progress bar track
    QRectF pb = progressBarRect();
    QPainterPath track;
    track.addRoundedRect(pb, 3, 3);
    p.fillPath(track, QColor(0x55, 0x55, 0x55));

    // Progress bar fill
    if (m_duration > 0) {
        double frac = double(m_position) / m_duration;
        QRectF fill = pb;
        fill.setWidth(pb.width() * frac);
        QPainterPath fillPath;
        fillPath.addRoundedRect(fill, 3, 3);
        p.fillPath(fillPath, QColor(AccentColor));
    }

    // Time text
    QString timeStr = DlnaUtils::formatTime(m_position);
    if (m_duration > 0)
        timeStr += " / " + DlnaUtils::formatTime(m_duration);
    QFont f = font();
    f.setPointSize(11);
    p.setFont(f);
    p.setPen(Qt::white);
    p.drawText(cb, Qt::AlignCenter | Qt::AlignBottom,
               QString("     %1     ").arg(timeStr));

    // Play/Pause icon
    paintPlayIcon(p, playButtonRect(), m_playing);

    // Mute icon
    QRectF mr = muteButtonRect();
    uint muteGlyph = m_muted ? Fa::VolumeMute : Fa::VolumeUp;
    QPixmap muteIcon = FaIcon::pixmap(muteGlyph, Qt::white, int(mr.height()));
    p.drawPixmap(mr.toRect(), muteIcon);
}

void ControlOverlay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    if (m_audioMode)
        paintAudioCircle(p);

    if (!m_controlsVisible) return;

    paintTitleBar(p);
    paintControlBar(p);
}

void ControlOverlay::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        QPointF pos = e->position();

        if (playButtonRect().contains(pos)) {
            emit playPauseClicked();
            showControls();
            return;
        }
        if (muteButtonRect().contains(pos)) {
            emit muteClicked();
            showControls();
            return;
        }
        QRectF pb = progressBarRect();
        // Expand hit area vertically for the progress bar
        QRectF pbHit = pb.adjusted(0, -12, 0, 12);
        if (pbHit.contains(pos) && m_duration > 0) {
            double frac = (pos.x() - pb.left()) / pb.width();
            frac = qBound(0.0, frac, 1.0);
            emit seekRequested(frac);
            showControls();
            return;
        }
    }
}

void ControlOverlay::mouseMoveEvent(QMouseEvent *)
{
    showControls();
}

// ─── VideoWidget ─────────────────────────────────────────────────────────────

VideoWidget::VideoWidget(QWidget *parent)
    : MediaWidget(parent)
    , m_player(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
    , m_videoSink(new QVideoSink(this))
    , m_overlay(new ControlOverlay(this))
    , m_nam(new QNetworkAccessManager(this))
    , m_playWatchdog(new QTimer(this))
{
    m_playWatchdog->setSingleShot(true);
    m_playWatchdog->setInterval(2000);
    connect(m_playWatchdog, &QTimer::timeout, this, &VideoWidget::onPlayWatchdog);
    setMouseTracking(true);

    // QVideoSink receives frames — we draw them ourselves in paintEvent,
    // so the ControlOverlay (a normal child widget) sits on top without issue.
    m_player->setAudioOutput(m_audioOutput);
    m_player->setVideoSink(m_videoSink);

    m_overlay->setGeometry(rect());
    m_overlay->showControls();

    connect(m_videoSink, &QVideoSink::videoFrameChanged, this, [this](const QVideoFrame &frame) {
        m_currentFrame = frame;
        update();
    });

    connect(m_overlay, &ControlOverlay::playPauseClicked, this, [this]() {
        if (m_player->playbackState() == QMediaPlayer::PlayingState)
            m_player->pause();
        else
            m_player->play();
    });
    connect(m_overlay, &ControlOverlay::muteClicked, this, [this]() {
        m_audioOutput->setMuted(!m_audioOutput->isMuted());
        m_overlay->setMuted(m_audioOutput->isMuted());
    });
    connect(m_overlay, &ControlOverlay::seekRequested, this, [this](double frac) {
        if (m_player->duration() > 0)
            m_player->setPosition(qint64(m_player->duration() * frac));
    });

    connect(m_player, &QMediaPlayer::positionChanged, this, [this](qint64 pos) {
        if (pos > 0) m_playWatchdog->stop();
        m_overlay->setPosition(pos, m_player->duration());
    });
    connect(m_player, &QMediaPlayer::durationChanged, this, [this](qint64 dur) {
        m_overlay->setPosition(m_player->position(), dur);
    });
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState s) {
        m_overlay->setPlaying(s == QMediaPlayer::PlayingState);
    });
    connect(m_player, &QMediaPlayer::mediaStatusChanged,
            this, &VideoWidget::onMediaStatusChanged);
}

void VideoWidget::loadItem(const DlnaItem &item)
{
    m_audioMode = (item.type == DlnaItemType::Audio);
    m_currentFrame = {};
    m_overlay->setAudioMode(m_audioMode);
    QString displayTitle = item.title;
    if (m_audioMode && !item.artist.isEmpty())
        displayTitle = item.artist + " — " + item.title;
    m_overlay->setTitle(displayTitle);
    m_overlay->setPosition(0, 0);
    m_overlay->setPlaying(false);
    m_overlay->setMuted(m_audioOutput->isMuted());
    m_overlay->showControls();

    if (m_audioMode && !item.thumbnailUrl.isEmpty())
        fetchAlbumArt(item.thumbnailUrl);

    m_currentSource = item.resourceUrl;
    m_playRetries = 0;
    m_pendingPlay = true;
    m_player->setSource(item.resourceUrl);
    setFocus();
}

void VideoWidget::stop()
{
    m_pendingPlay = false;
    m_playRetries = 0;
    m_playWatchdog->stop();
    m_currentSource.clear();
    m_player->stop();
    m_player->setSource({});
}

void VideoWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    m_overlay->setGeometry(rect());
}

void VideoWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::black);

    if (!m_audioMode && m_currentFrame.isValid()) {
        QImage img = m_currentFrame.toImage();
        if (img.isNull()) return;

        p.setRenderHint(QPainter::SmoothPixmapTransform);

        int angle = 0;
        switch (m_currentFrame.rotation()) {
        case QtVideo::Rotation::Clockwise90:  angle = 90;  break;
        case QtVideo::Rotation::Clockwise180: angle = 180; break;
        case QtVideo::Rotation::Clockwise270: angle = 270; break;
        default: break;
        }

        const bool swapped = (angle % 180 != 0);
        QSize displaySize = swapped ? img.size().transposed() : img.size();
        QRectF dest = letterboxRect(displaySize, size());

        p.save();
        p.translate(dest.center());
        p.rotate(angle);
        double hw = swapped ? dest.height() / 2.0 : dest.width() / 2.0;
        double hh = swapped ? dest.width() / 2.0 : dest.height() / 2.0;
        p.drawImage(QRectF(-hw, -hh, hw * 2, hh * 2), img, img.rect());
        p.restore();
    }
}

QRectF VideoWidget::letterboxRect(QSize content, QSize view)
{
    double scale = qMin(double(view.width()) / content.width(),
                        double(view.height()) / content.height());
    double w = content.width() * scale;
    double h = content.height() * scale;
    return QRectF((view.width() - w) / 2.0, (view.height() - h) / 2.0, w, h);
}

void VideoWidget::mouseMoveEvent(QMouseEvent *)
{
    m_overlay->showControls();
}

void VideoWidget::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Escape:
        m_player->stop();
        emit closeRequested();
        break;
    case Qt::Key_Space:
    case Qt::Key_Return:
        if (m_player->playbackState() == QMediaPlayer::PlayingState)
            m_player->pause();
        else
            m_player->play();
        m_overlay->showControls();
        break;
    case Qt::Key_Left:
    case Qt::Key_Minus:
        m_player->setPosition(qMax(0LL, m_player->position() - 5000));
        m_overlay->showControls();
        break;
    case Qt::Key_Right:
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        m_player->setPosition(qMin(m_player->duration(), m_player->position() + 5000));
        m_overlay->showControls();
        break;
    case Qt::Key_M:
        m_audioOutput->setMuted(!m_audioOutput->isMuted());
        m_overlay->setMuted(m_audioOutput->isMuted());
        m_overlay->showControls();
        break;
    default:
        MediaWidget::keyPressEvent(e);
    }
}

void VideoWidget::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (m_pendingPlay &&
        (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia)) {
        m_pendingPlay = false;
        m_player->play();
        m_playWatchdog->start();
    } else if (status == QMediaPlayer::EndOfMedia) {
        m_player->pause();
        m_overlay->setPlaying(false);
        m_overlay->showControls();
    }
}

void VideoWidget::onPlayWatchdog()
{
    // macOS/AVFoundation sometimes enters PlayingState but stalls at position 0;
    // reload the source to reset the audio pipeline (max 2 retries).
    if (m_player->playbackState() != QMediaPlayer::PlayingState) return;
    if (m_player->position() != 0) return;
    if (m_currentSource.isEmpty() || ++m_playRetries > 2) return;

    m_pendingPlay = true;
    m_player->stop();
    m_player->setSource(m_currentSource);
}

void VideoWidget::fetchAlbumArt(const QUrl &url)
{
    QNetworkRequest req(url);
    req.setTransferTimeout(5000);
    QNetworkReply *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;
        QPixmap px;
        if (px.loadFromData(reply->readAll()))
            m_overlay->setAudioMode(true, px);
    });
}
