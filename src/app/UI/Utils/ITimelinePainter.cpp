//
// Created by fluty on 2024/2/3.
//

#include "ITimelinePainter.h"

#include <QPainter>
#include <QPen>
#include <QColor>
#include <QtGlobal>

void ITimelinePainter::drawTimeline(QPainter *painter, double startTick, double endTick,
                                    double rectWidth, double scaleX) {
    auto ticksPerPixel = (endTick - startTick) / rectWidth;
    
    // Calculate ticks for different grid levels
    // Quarter note = 480 ticks, so:
    // 1/2 = 960 ticks, 1/4 = 480 ticks, 1/8 = 240 ticks, 1/16 = 120 ticks,
    // 1/32 = 60 ticks, 1/64 = 30 ticks, 1/128 = 15 ticks
    int barTicks = 1920 * m_numerator / m_denominator;
    int beatTicks = 1920 / m_denominator;
    int quarterTicks = 480;
    int eighthTicks = 240;
    int sixteenthTicks = 120;
    int thirtySecondTicks = 60;
    int sixtyFourthTicks = 30;
    int oneTwentyEighthTicks = 15;
    
    // Calculate pixel spacing for each grid level
    auto oneBarLength = barTicks / ticksPerPixel;
    auto quarterLength = quarterTicks / ticksPerPixel;
    auto eighthLength = eighthTicks / ticksPerPixel;
    auto sixteenthLength = sixteenthTicks / ticksPerPixel;
    auto thirtySecondLength = thirtySecondTicks / ticksPerPixel;
    auto sixtyFourthLength = sixtyFourthTicks / ticksPerPixel;
    auto oneTwentyEighthLength = oneTwentyEighthTicks / ticksPerPixel;
    
    // Determine which grid levels to draw based on spacing and quantize
    // Use a more sophisticated approach: show grids that are close to minimum spacing
    // and respect the quantize setting
    bool canDrawBar = oneBarLength >= m_minimumSpacing;
    bool canDrawBeat = beatTicks / ticksPerPixel >= m_minimumSpacing;
    bool canDrawQuarter = quarterLength >= m_minimumSpacing;
    bool canDrawEighth = m_quantize >= 8 && eighthLength >= m_minimumSpacing;
    bool canDrawSixteenth = m_quantize >= 16 && sixteenthLength >= m_minimumSpacing;
    bool canDrawThirtySecond = m_quantize >= 32 && thirtySecondLength >= m_minimumSpacing;
    bool canDrawSixtyFourth = m_quantize >= 64 && sixtyFourthLength >= m_minimumSpacing;
    bool canDrawOneTwentyEighth = m_quantize >= 128 && oneTwentyEighthLength >= m_minimumSpacing;
    
    // Calculate animation alpha based on how close we are to the threshold
    // This creates a smooth fade-in effect when grids appear
    // The grid will smoothly appear as we zoom in, creating an "expanding" effect
    const double minimumSpacing = m_minimumSpacing;
    auto calculateAlpha = [minimumSpacing](double length, double threshold) -> double {
        if (length >= threshold * 1.2)
            return 1.0;
        if (length < threshold * 0.6)
            return 0.0;
        // Smooth transition between 0.6x and 1.2x threshold
        // Use a smooth curve for more natural appearance
        double ratio = (length - threshold * 0.6) / (threshold * 0.6);
        // Apply easing curve for smoother transition
        ratio = ratio * ratio * (3.0 - 2.0 * ratio); // Smoothstep function
        return qBound(0.0, ratio, 1.0);
    };
    
    // Draw bars (always draw, even when zoomed out)
    // Calculate appropriate bar hop size to ensure minimum spacing
    int drawBarHopSize = 1;
    auto curLineSpacing = oneBarLength;
    while (curLineSpacing < m_minimumSpacing && drawBarHopSize < 1024) { // Limit to prevent infinite loop
        drawBarHopSize *= 2;
        curLineSpacing *= 2;
    }
    auto prevLineTick =
        static_cast<int>(startTick / barTicks / drawBarHopSize) * barTicks * drawBarHopSize;
    for (int i = prevLineTick; i <= endTick; i += barTicks * drawBarHopSize) {
        auto bar = i / barTicks + 1;
        drawBar(painter, i, bar);
    }
    
    // Draw beats (only if spacing is sufficient and bars are drawn at normal interval)
    if (canDrawBeat && drawBarHopSize == 1) {
        auto prevLineTick = static_cast<int>(startTick / beatTicks) * beatTicks;
        for (int i = prevLineTick; i <= endTick; i += beatTicks) {
            if (i % barTicks != 0) { // Don't draw if it's a bar line
                auto bar = i / barTicks + 1;
                auto beat = (i % barTicks) / beatTicks + 1;
                drawBeat(painter, i, bar, beat);
            }
        }
    }
    
    // Draw eighth notes with animation
    if (canDrawEighth) {
        double alpha = calculateAlpha(eighthLength, m_minimumSpacing);
        if (alpha > 0.0) {
            QPen originalPen = painter->pen();
            QPen pen = originalPen;
            QColor color = pen.color();
            color.setAlphaF(color.alphaF() * alpha);
            pen.setColor(color);
            // Use dashed line in no-snap mode for sub-beat lines
            if (m_noSnapMode) {
                pen.setStyle(Qt::DashLine);
            }
            painter->setPen(pen);
            
            auto prevLineTick = static_cast<int>(startTick / eighthTicks) * eighthTicks;
            for (int i = prevLineTick; i <= endTick; i += eighthTicks) {
                if (i % barTicks != 0 && i % beatTicks != 0 && i % quarterTicks != 0) {
                    drawEighth(painter, i);
                }
            }
            
            painter->setPen(originalPen); // Restore original pen
        }
    }
    
    // Draw sixteenth notes with animation
    if (canDrawSixteenth) {
        double alpha = calculateAlpha(sixteenthLength, m_minimumSpacing);
        if (alpha > 0.0) {
            QPen originalPen = painter->pen();
            QPen pen = originalPen;
            QColor color = pen.color();
            color.setAlphaF(color.alphaF() * alpha);
            pen.setColor(color);
            // Use dashed line in no-snap mode for sub-beat lines
            if (m_noSnapMode) {
                pen.setStyle(Qt::DashLine);
            }
            painter->setPen(pen);
            
            auto prevLineTick = static_cast<int>(startTick / sixteenthTicks) * sixteenthTicks;
            for (int i = prevLineTick; i <= endTick; i += sixteenthTicks) {
                if (i % eighthTicks != 0) { // Don't draw if it's an eighth line
                    drawSixteenth(painter, i);
                }
            }
            
            painter->setPen(originalPen);
        }
    }
    
    // Draw thirty-second notes with animation
    if (canDrawThirtySecond) {
        double alpha = calculateAlpha(thirtySecondLength, m_minimumSpacing);
        if (alpha > 0.0) {
            QPen originalPen = painter->pen();
            QPen pen = originalPen;
            QColor color = pen.color();
            color.setAlphaF(color.alphaF() * alpha);
            pen.setColor(color);
            // Use dashed line in no-snap mode for sub-beat lines
            if (m_noSnapMode) {
                pen.setStyle(Qt::DashLine);
            }
            painter->setPen(pen);
            
            auto prevLineTick = static_cast<int>(startTick / thirtySecondTicks) * thirtySecondTicks;
            for (int i = prevLineTick; i <= endTick; i += thirtySecondTicks) {
                if (i % sixteenthTicks != 0) { // Don't draw if it's a sixteenth line
                    drawThirtySecond(painter, i);
                }
            }
            
            painter->setPen(originalPen);
        }
    }
    
    // Draw sixty-fourth notes with animation
    if (canDrawSixtyFourth) {
        double alpha = calculateAlpha(sixtyFourthLength, m_minimumSpacing);
        if (alpha > 0.0) {
            QPen originalPen = painter->pen();
            QPen pen = originalPen;
            QColor color = pen.color();
            color.setAlphaF(color.alphaF() * alpha);
            pen.setColor(color);
            // Use dashed line in no-snap mode for sub-beat lines
            if (m_noSnapMode) {
                pen.setStyle(Qt::DashLine);
            }
            painter->setPen(pen);
            
            auto prevLineTick = static_cast<int>(startTick / sixtyFourthTicks) * sixtyFourthTicks;
            for (int i = prevLineTick; i <= endTick; i += sixtyFourthTicks) {
                if (i % thirtySecondTicks != 0) { // Don't draw if it's a thirty-second line
                    drawSixtyFourth(painter, i);
                }
            }
            
            painter->setPen(originalPen);
        }
    }
    
    // Draw one-twenty-eighth notes with animation
    if (canDrawOneTwentyEighth) {
        double alpha = calculateAlpha(oneTwentyEighthLength, m_minimumSpacing);
        if (alpha > 0.0) {
            QPen originalPen = painter->pen();
            QPen pen = originalPen;
            QColor color = pen.color();
            color.setAlphaF(color.alphaF() * alpha);
            pen.setColor(color);
            // Use dashed line in no-snap mode for sub-beat lines
            if (m_noSnapMode) {
                pen.setStyle(Qt::DashLine);
            }
            painter->setPen(pen);
            
            auto prevLineTick = static_cast<int>(startTick / oneTwentyEighthTicks) * oneTwentyEighthTicks;
            for (int i = prevLineTick; i <= endTick; i += oneTwentyEighthTicks) {
                if (i % sixtyFourthTicks != 0) { // Don't draw if it's a sixty-fourth line
                    drawOneTwentyEighth(painter, i);
                }
            }
            
            painter->setPen(originalPen);
        }
    }
}

int ITimelinePainter::pixelsPerQuarterNote() const {
    return m_pixelsPerQuarterNote;
}

void ITimelinePainter::setPixelsPerQuarterNote(int px) {
    m_pixelsPerQuarterNote = px;
}

void ITimelinePainter::setTimeSignature(int numerator, int denominator) {
    m_numerator = numerator;
    m_denominator = denominator;
}

void ITimelinePainter::setQuantize(int quantize) {
    m_quantize = quantize;
}

int ITimelinePainter::denominator() const {
    return m_denominator;
}

void ITimelinePainter::setNoSnapMode(bool enabled) {
    m_noSnapMode = enabled;
}

bool ITimelinePainter::isNoSnapMode() const {
    return m_noSnapMode;
}