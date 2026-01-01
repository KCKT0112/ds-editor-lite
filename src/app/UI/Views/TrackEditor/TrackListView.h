//
// Created by fluty on 2024/7/11.
//

#ifndef TRACKLISTWIDGET_H
#define TRACKLISTWIDGET_H

#include <QListWidget>

class TracksGraphicsView;

class TrackListView : public QListWidget {
    Q_OBJECT
public:
    explicit TrackListView(QWidget *parent = nullptr);
    void setGraphicsView(TracksGraphicsView *view);

signals:
    void trackDragged(qsizetype fromIndex, qsizetype toIndex);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void onRowsMoved(const QModelIndex &parent, int start, int end,
                     const QModelIndex &destination, int row);

private:
    TracksGraphicsView *m_view = nullptr;
};



#endif // TRACKLISTWIDGET_H
