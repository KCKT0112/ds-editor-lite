//
// Created by fluty on 2026/1/2.
//

#include "MoveTrackAction.h"

#include "Model/AppModel/AppModel.h"

MoveTrackAction *MoveTrackAction::build(const qsizetype fromIndex, const qsizetype toIndex,
                                         AppModel *model) {
    const auto a = new MoveTrackAction;
    a->m_fromIndex = fromIndex;
    a->m_toIndex = toIndex;
    a->m_model = model;
    return a;
}

void MoveTrackAction::execute() {
    if (!m_model)
        return;

    const auto &tracksList = m_model->tracks();

    qDebug() << "MoveTrackAction::execute fromIndex:" << m_fromIndex << "toIndex:" << m_toIndex
             << "tracksCount:" << tracksList.size();

    // Validate indices
    if (m_fromIndex < 0 || m_fromIndex >= tracksList.size()) {
        qWarning() << "MoveTrackAction::execute invalid fromIndex";
        return;
    }

    if (m_toIndex < 0 || m_toIndex > tracksList.size()) {
        qWarning() << "MoveTrackAction::execute invalid toIndex";
        return;
    }

    if (m_fromIndex == m_toIndex)
        return;

    // Save the track pointer for undo
    m_track = tracksList.at(m_fromIndex);

    // Remove from old position
    m_model->removeTrackAt(m_fromIndex);

    // Calculate adjusted target index after removal
    // When moving down (toIndex > fromIndex), after removal the list is shorter
    // but we still want to insert at the visual position, which is now toIndex-1
    qsizetype adjustedToIndex = m_toIndex;
    if (m_toIndex > m_fromIndex) {
        adjustedToIndex = m_toIndex - 1;
    }

    qDebug() << "MoveTrackAction::execute adjustedToIndex:" << adjustedToIndex;

    // Insert at new position
    m_model->insertTrack(m_track, adjustedToIndex);
}

void MoveTrackAction::undo() {
    if (!m_model || !m_track)
        return;

    const auto &tracksList = m_model->tracks();

    // Find current position of the track
    qsizetype currentIndex = tracksList.indexOf(m_track);
    if (currentIndex < 0)
        return;

    // Remove from current position
    m_model->removeTrackAt(currentIndex);

    // Calculate adjusted original index after removal
    qsizetype adjustedFromIndex = m_fromIndex;
    if (currentIndex < m_fromIndex) {
        adjustedFromIndex = m_fromIndex - 1;
    }

    // Insert back to original position
    m_model->insertTrack(m_track, adjustedFromIndex);
}
