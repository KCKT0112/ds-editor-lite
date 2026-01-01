//
// Created by fluty on 2024/7/11.
//

#include "TrackListView.h"

#include "TracksGraphicsView.h"
#include "Controller/TrackController.h"
#include "Global/TracksEditorGlobal.h"

#include <QScroller>
#include <QWheelEvent>
#include <QDropEvent>

TrackListView::TrackListView(QWidget *parent) : QListWidget(parent) {
    setObjectName("TrackListWidget");
    setFixedWidth(TracksEditorGlobal::trackListWidth);
    setViewMode(ListMode);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollMode(ScrollPerPixel);
    setSelectionMode(SingleSelection);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDefaultDropAction(Qt::MoveAction);
    setDropIndicatorShown(true);
    setDragDropOverwriteMode(false);
    QScroller::grabGesture(this, QScroller::TouchGesture);

    // Let Qt handle UI drag & drop, then sync data model via signal
    connect(model(), &QAbstractItemModel::rowsMoved, this, &TrackListView::onRowsMoved);
}

void TrackListView::setGraphicsView(TracksGraphicsView *view) {
    m_view = view;
}

void TrackListView::mousePressEvent(QMouseEvent *event) {
    QListWidget::mousePressEvent(event);
    event->ignore();
}

void TrackListView::wheelEvent(QWheelEvent *event) {
    if (!m_view)
        return;

    const auto modifiers = event->modifiers();
    if (modifiers == Qt::AltModifier)
        m_view->onWheelVerScale(event);
    else if (modifiers == Qt::NoModifier)
        m_view->onWheelVerScroll(event);
}

void TrackListView::onRowsMoved(const QModelIndex &parent, int start, int end,
                                  const QModelIndex &destination, int row) {
    Q_UNUSED(parent)
    Q_UNUSED(end)
    Q_UNUSED(destination)

    qDebug() << "TrackListView::onRowsMoved start:" << start << "row:" << row << "count:" << count();

    // Qt has already moved the UI items
    // Calculate the from/to indices for the data model
    qsizetype fromIndex = start;
    qsizetype toIndex;

    if (row < start) {
        // Moved up
        toIndex = row;
    } else {
        // Moved down
        toIndex = row - 1;
    }

    qDebug() << "TrackListView::onRowsMoved calculated fromIndex:" << fromIndex << "toIndex:" << toIndex;

    // Emit signal for TrackEditorView to handle
    // It will update the data model and sync the ViewModel
    emit trackDragged(fromIndex, toIndex);
}