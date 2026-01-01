//
// Created by fluty on 2026/1/2.
//

#ifndef MOVECLIPBETWEENTRACKSACTION_H
#define MOVECLIPBETWEENTRACKSACTION_H

#include "Modules/History/IAction.h"

class Clip;
class Track;

class MoveClipBetweenTracksAction final : public IAction {
public:
    static MoveClipBetweenTracksAction *build(Clip *clip, Track *fromTrack, Track *toTrack);
    void execute() override;
    void undo() override;

private:
    Clip *m_clip = nullptr;
    Track *m_fromTrack = nullptr;
    Track *m_toTrack = nullptr;
};

#endif // MOVECLIPBETWEENTRACKSACTION_H
