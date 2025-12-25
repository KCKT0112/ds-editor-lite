//
// Created by fluty on 2024/2/3.
//

#ifndef TIMEINDICATORVIEW_H
#define TIMEINDICATORVIEW_H

#include <QGraphicsLineItem>
#include <QPropertyAnimation>

#include "UI/Utils/IScalableItem.h"
#include "Global/PlaybackGlobal.h"

using namespace PlaybackGlobal;

class TimeIndicatorView : public QObject, public QGraphicsLineItem, public IScalableItem {
    Q_OBJECT
    Q_PROPERTY(double trailOpacity READ trailOpacity WRITE setTrailOpacity)

public:
    explicit TimeIndicatorView(QObject *parent = nullptr);
    void setPixelsPerQuarterNote(int px);
    void setPlaybackStatus(PlaybackStatus status);

    double trailOpacity() const;
    void setTrailOpacity(double opacity);

public slots:
    void setPosition(double tick);
    void setOffset(int tick);

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
    void afterSetScale() override;
    void afterSetVisibleRect() override;

private:
    static constexpr double TRAIL_WIDTH = 15.0; // Maximum width of the trail in pixels
    
    double m_time = 0;
    double m_pixelsPerQuarterNote = 64;
    int m_offset = 0;
    PlaybackStatus m_playbackStatus = Stopped;
    double m_trailOpacity = 0.0;

    QPropertyAnimation m_trailOpacityAnimation;

    void updateLengthAndPos();
    [[nodiscard]] double tickToItemX(double tick) const;
    void drawTrail(QPainter *painter);
};

#endif // TIMEINDICATORVIEW_H
