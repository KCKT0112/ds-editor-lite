//
// Created by fluty on 2024/2/3.
//

#ifndef TIMELINEPAINTER_H
#define TIMELINEPAINTER_H

class QPainter;

class ITimelinePainter {
public:
    void setPixelsPerQuarterNote(int px);
    virtual void setTimeSignature(int numerator, int denominator);
    virtual void setQuantize(int quantize);
    void setNoSnapMode(bool enabled);
    [[nodiscard]] int denominator() const;

protected:
    void drawTimeline(QPainter *painter, double startTick, double endTick, double rectWidth, double scaleX = 1.0);
    [[nodiscard]] int pixelsPerQuarterNote() const;
    [[nodiscard]] bool isNoSnapMode() const;
    virtual void drawBar(QPainter *painter, int tick, int bar) = 0;
    virtual void drawBeat(QPainter *painter, int tick, int bar, int beat) = 0;
    virtual void drawEighth(QPainter *painter, int tick) = 0;
    virtual void drawSixteenth(QPainter *painter, int tick) = 0;
    virtual void drawThirtySecond(QPainter *painter, int tick) = 0;
    virtual void drawSixtyFourth(QPainter *painter, int tick) = 0;
    virtual void drawOneTwentyEighth(QPainter *painter, int tick) = 0;
    virtual ~ITimelinePainter() = default;

private:
    int m_minimumSpacing = 24;
    int m_pixelsPerQuarterNote = 64;
    int m_numerator = 4;
    int m_denominator = 4;
    int m_quantize = 16;
    bool m_noSnapMode = false;
};



#endif // TIMELINEPAINTER_H
