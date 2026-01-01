//
// Created by fluty on 2026/1/2.
//

#ifndef MOVETRACKACTION_H
#define MOVETRACKACTION_H

#include "Modules/History/IAction.h"

#include <QtTypes>

class Track;
class AppModel;

class MoveTrackAction final : public IAction {
public:
    static MoveTrackAction *build(qsizetype fromIndex, qsizetype toIndex, AppModel *model);
    void execute() override;
    void undo() override;

private:
    qsizetype m_fromIndex = -1;
    qsizetype m_toIndex = -1;
    Track *m_track = nullptr;
    AppModel *m_model = nullptr;
};

#endif // MOVETRACKACTION_H