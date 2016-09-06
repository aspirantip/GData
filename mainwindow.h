#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QGraphicsScene* scene;
    QGraphicsLineItem* l_track;

    uint8_t nmChambers;
    uint8_t nmLayers;
    uint8_t nmTubes;
    qreal   diamTube;
    qreal   radTube;

    QVector< QVector < QVector < QGraphicsEllipseItem * > > > vTrackSystem;



    void createConnections();
    void cleanSystem();                                         // очищаем систему

    void createTrack(QLineF line);
    QList<QGraphicsEllipseItem *> getMaskTrack(QGraphicsLineItem* track);

    void drawSystem();                                          // рисуем систему
    void drawMaskTack(QList<QGraphicsEllipseItem *> lstHits);   // рисуем маску трека




};

#endif // MAINWINDOW_H
