//
// Created by fluty on 2026/1/2.
//

#include "MoveClipBetweenTracksAction.h"

#include "Model/AppModel/Clip.h"
#include "Model/AppModel/Track.h"

MoveClipBetweenTracksAction *MoveClipBetweenTracksAction::build(Clip *clip, Track *fromTrack,
                                                                 Track *toTrack) {
    const auto a = new MoveClipBetweenTracksAction;
    a->m_clip = clip;
    a->m_fromTrack = fromTrack;
    a->m_toTrack = toTrack;
    return a;
}

void MoveClipBetweenTracksAction::execute() {
    if (!m_clip || !m_fromTrack || !m_toTrack)
        return;

    // Remove clip from source track
    m_fromTrack->removeClip(m_clip);

    // Add clip to target track
    m_toTrack->insertClip(m_clip);
}

void MoveClipBetweenTracksAction::undo() {
    if (!m_clip || !m_fromTrack || !m_toTrack)
        return;

    // Remove clip from target track
    m_toTrack->removeClip(m_clip);

    // Add clip back to source track
    m_fromTrack->insertClip(m_clip);
}