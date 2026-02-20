//
// Created by fluty on 2024/2/4.
//

#ifndef PLAYBACKVIEW_H
#define PLAYBACKVIEW_H

#include <QSize>
#include <QWidget>

#include "Global/PlaybackGlobal.h"

class EditLabel;
class QPushButton;
class ComboBox;

using namespace PlaybackGlobal;

class PlaybackView final : public QWidget {
    Q_OBJECT
public:
    explicit PlaybackView(QWidget *parent = nullptr);

signals:
    void setTempoTriggered(double tempo);
    void setTimeSignatureTriggered(int numerator, int denominator);
    void playTriggered();
    void pauseTriggered();
    void stopTriggered();
    void setPositionTriggered(double tick);
    void setQuantizeTriggered(int value);

public slots:
    void updateView();
    void onTempoChanged(double tempo);
    void onTimeSignatureChanged(int numerator, int denominator);
    void onPositionChanged(double tick);
    void onPlaybackStatusChanged(PlaybackStatus status);

private:
    EditLabel *m_elTempo;
    EditLabel *m_elTimeSignature;
    QPushButton *m_btnStop;
    QPushButton *m_btnPlay;
    QPushButton *m_btnPause;
    QPushButton *m_btnPlayPause;
    QPushButton *m_btnLoop;
    EditLabel *m_elTime;
    ComboBox *m_cbQuantize;

    double m_tempo = 120;
    int m_numerator = 4;
    int m_denominator = 4;
    int m_tick = 0;
    PlaybackStatus m_status = Stopped;

    int m_contentHeight = 28;
    QSize m_iconSize = QSize(16, 16);

    QString toFormattedTickTime(int ticks) const;
    int fromTickTimeString(const QStringList &splitStr) const;

    const QStringList quantizeStrings = {"1/2", "1/4", "1/8", "1/16", "1/32", "1/64", "1/128"};
    const QList<int> quantizeValues = {2, 4, 8, 16, 32, 64, 128};

    void updateTempoView();
    void updateTimeSignatureView();
    void updateTimeView();
    void updatePlaybackControlView();
    void updateLoopButtonView();
};

#endif // PLAYBACKVIEW_H
