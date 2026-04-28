#include "videowidget.h"
#include "dlnaparser.h"

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

QString ControlOverlay::formatTime(qint64 ms)
{
    return DlnaParser::formatTime(ms);
}

// ─── Painting ────────────────────────────────────────────────────────────────

void ControlOverlay::paintPlayIcon(QPainter &p, QRectF r, bool playing)
{
    p.save();
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);
    if (playing) {
        // Pause: two rectangles
        double bw = r.width() * 0.28;
        double gap = r.width() * 0.18;
        p.drawRect(QRectF(r.left(), r.top(), bw, r.height()));
        p.drawRect(QRectF(r.left() + bw + gap, r.top(), bw, r.height()));
    } else {
        // Play: triangle
        QPolygonF tri;
        tri << r.topLeft()
            << QPointF(r.right(), r.center().y())
            << r.bottomLeft();
        p.drawPolygon(tri);
    }
    p.restore();
}

void ControlOverlay::paintMuteIcon(QPainter &p, QRectF r, bool muted)
{
    p.save();
    p.setPen(QPen(Qt::white, 2));
    p.setBrush(Qt::NoBrush);
    double cx = r.center().x();
    double cy = r.center().y();
    double sz = qMin(r.width(), r.height()) * 0.4;
    // Speaker body (square + triangle)
    p.setBrush(Qt::white);
    p.setPen(Qt::NoPen);
    QRectF body(cx - sz * 1.1, cy - sz * 0.5, sz * 0.6, sz);
    p.drawRect(body);
    QPolygonF cone;
    cone << QPointF(cx - sz * 0.5, cy - sz * 0.5)
         << QPointF(cx + sz * 0.5, cy - sz)
         << QPointF(cx + sz * 0.5, cy + sz)
         << QPointF(cx - sz * 0.5, cy + sz * 0.5);
    p.drawPolygon(cone);
    if (muted) {
        p.setPen(QPen(QColor(0xe0, 0x60, 0x60), 2));
        p.drawLine(QPointF(cx + sz * 0.8, cy - sz * 0.8),
                   QPointF(cx - sz * 0.8, cy + sz * 0.8));
    }
    p.restore();
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
        // Music note (simplified)
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0x19, 0x19, 0x19));
        int ns = d / 5;
        QRect note(rect.center().x() - ns / 2,
                   rect.center().y() - ns * 2, ns, ns * 3);
        p.drawEllipse(note.left() - ns / 2, note.bottom() - ns / 2, ns, ns / 2);
        p.drawRect(QRect(note.left(), note.top(), ns / 4, note.height()));
        p.drawRect(QRect(note.right() - ns / 4, note.top() - ns / 2, ns / 4, ns * 2));
        p.drawRect(QRect(note.left(), note.top(), ns, ns / 4));
        p.drawEllipse(note.right() - ns / 2, note.bottom() - ns / 4 - ns / 2, ns, ns / 2);
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
    QString timeStr = formatTime(m_position);
    if (m_duration > 0)
        timeStr += " / " + formatTime(m_duration);
    QFont f = font();
    f.setPointSize(11);
    p.setFont(f);
    p.setPen(Qt::white);
    p.drawText(cb, Qt::AlignCenter | Qt::AlignBottom,
               QString("     %1     ").arg(timeStr));

    // Play/Pause icon
    paintPlayIcon(p, playButtonRect(), m_playing);

    // Mute icon
    paintMuteIcon(p, muteButtonRect(), m_muted);
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
{
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
    m_overlay->setTitle(item.title);
    m_overlay->setPosition(0, 0);
    m_overlay->setPlaying(false);
    m_overlay->setMuted(m_audioOutput->isMuted());
    m_overlay->showControls();

    if (m_audioMode && !item.thumbnailUrl.isEmpty())
        fetchAlbumArt(item.thumbnailUrl);

    m_player->setSource(item.resourceUrl);
    m_player->play();
    setFocus();
}

void VideoWidget::stop()
{
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
        if (!img.isNull()) {
            p.setRenderHint(QPainter::SmoothPixmapTransform);
            p.drawImage(letterboxRect(img.size(), size()), img, img.rect());
        }
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
    if (status == QMediaPlayer::EndOfMedia) {
        m_player->pause();
        m_overlay->setPlaying(false);
        m_overlay->showControls();
    }
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
