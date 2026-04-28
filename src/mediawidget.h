#pragma once
#include <QWidget>
#include <QPainter>

class MediaWidget : public QWidget {
    Q_OBJECT
public:
    explicit MediaWidget(QWidget *parent = nullptr);
    void setTitle(const QString &title);

signals:
    void closeRequested();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void drawTitleBar(QPainter &p) const;

    QString m_title;
};
