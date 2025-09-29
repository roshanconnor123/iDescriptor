#pragma once
#include <QGraphicsView>
#include <QMainWindow>

// A custom QGraphicsView that keeps the content fitted with aspect ratio on
// resize
class ResponsiveGraphicsView : public QGraphicsView
{
public:
    ResponsiveGraphicsView(QGraphicsScene *scene, QWidget *parent = nullptr)
        : QGraphicsView(scene, parent)
    {
    }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        if (scene() && !scene()->items().isEmpty()) {
            fitInView(scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
        }
        QGraphicsView::resizeEvent(event);
    }
};

#ifdef Q_OS_MAC
void setupMacOSWindow(QMainWindow *window);
#endif
