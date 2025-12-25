//
// Created by fluty on 2024/2/3.
//

#include "TimeIndicatorView.h"

#include <QPainter>
#include <QLinearGradient>
#include <cmath>

TimeIndicatorView::TimeIndicatorView(QObject *parent) : QObject(parent) {
    setFixedScaleY(true);
    
    // Setup trail opacity animation
    m_trailOpacityAnimation.setTargetObject(this);
    m_trailOpacityAnimation.setPropertyName("trailOpacity");
    m_trailOpacityAnimation.setDuration(300);
    m_trailOpacityAnimation.setEasingCurve(QEasingCurve::OutCubic);
}

void TimeIndicatorView::setPixelsPerQuarterNote(int px) {
    m_pixelsPerQuarterNote = px;
    updateLengthAndPos();
}

void TimeIndicatorView::setPosition(double tick) {
    m_time = tick;
    updateLengthAndPos();
}

void TimeIndicatorView::setOffset(int tick) {
    m_offset = tick;
    updateLengthAndPos();
}

double TimeIndicatorView::trailOpacity() const {
    return m_trailOpacity;
}

void TimeIndicatorView::setTrailOpacity(double opacity) {
    if (qAbs(m_trailOpacity - opacity) > 0.001) {
        // Prepare for bounding rect change when opacity changes
        prepareGeometryChange();
        m_trailOpacity = opacity;
        update();
    }
}

void TimeIndicatorView::setPlaybackStatus(PlaybackStatus status) {
    if (m_playbackStatus != status) {
        // Prepare for bounding rect change
        prepareGeometryChange();
        m_playbackStatus = status;
        
        // Animate trail opacity
        m_trailOpacityAnimation.stop();
        if (status == Playing) {
            m_trailOpacityAnimation.setStartValue(m_trailOpacity);
            m_trailOpacityAnimation.setEndValue(1.0);
        } else {
            m_trailOpacityAnimation.setStartValue(m_trailOpacity);
            m_trailOpacityAnimation.setEndValue(0.0);
        }
        m_trailOpacityAnimation.start();
    }
}

void TimeIndicatorView::afterSetScale() {
    updateLengthAndPos();
}

void TimeIndicatorView::afterSetVisibleRect() {
    updateLengthAndPos();
}

void TimeIndicatorView::updateLengthAndPos() {
    auto x = tickToItemX(m_time - m_offset);
    // Prepare for geometry change (position and bounding rect)
    prepareGeometryChange();
    setPos(x, 0);
    auto line = QLineF(0, visibleRect().top(), 0, visibleRect().bottom());
    setLine(line);
    update();
}

double TimeIndicatorView::tickToItemX(double tick) const {
    return tick * scaleX() * m_pixelsPerQuarterNote / 480;
}

QRectF TimeIndicatorView::boundingRect() const {
    // Get the base bounding rect from the line
    QRectF baseRect = QGraphicsLineItem::boundingRect();
    
    // Always extend the rect to include the trail area
    // This ensures proper repainting during animation and prevents visual artifacts
    baseRect.adjust(-TRAIL_WIDTH, 0, 0, 0);
    
    return baseRect;
}

void TimeIndicatorView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                              QWidget *widget) {
    // Draw the trail effect with animated opacity
    if (m_trailOpacity > 0.0) {
        drawTrail(painter);
    }

    // Draw the main indicator line
    QGraphicsLineItem::paint(painter, option, widget);
}

void TimeIndicatorView::drawTrail(QPainter *painter) {
    const double power = 2.0; // Power function exponent for smooth fade

    QLineF indicatorLine = line();
    double lineX = indicatorLine.p1().x(); // Should be 0 in item coordinates
    double topY = indicatorLine.p1().y();
    double bottomY = indicatorLine.p2().y();
    double height = bottomY - topY;

    // Get the indicator line color from pen
    QColor indicatorColor = pen().color();

    // Create gradient from right (indicator color) to left (transparent)
    // Gradient goes from lineX (right, indicator color) to lineX - TRAIL_WIDTH (left, transparent)
    QLinearGradient gradient(lineX, 0, lineX - TRAIL_WIDTH, 0);
    
    // Use power function for smooth fade: alpha = (1 - t)^power
    // where t goes from 0 (at line) to 1 (at left edge)
    // Apply overall opacity animation and multiply by 0.7 for higher visibility
    const int stops = 32; // Number of gradient stops for smooth transition
    for (int i = 0; i <= stops; ++i) {
        double t = static_cast<double>(i) / stops; // t from 0 to 1
        double alpha = std::pow(1.0 - t, power) * m_trailOpacity * 0.7; // Power function fade with opacity
        QColor color(indicatorColor.red(), indicatorColor.green(), indicatorColor.blue(),
                     static_cast<int>(alpha * 255));
        gradient.setColorAt(t, color);
    }

    // Draw the trail rectangle
    QRectF trailRect(lineX - TRAIL_WIDTH, topY, TRAIL_WIDTH, height);
    painter->setPen(Qt::NoPen);
    painter->setBrush(gradient);
    painter->drawRect(trailRect);
}