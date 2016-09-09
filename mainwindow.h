#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>


#include <iostream>

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
    QGraphicsLineItem* track;
    QLineF l_track;

    uint8_t nmChambers;
    uint8_t nmLayers;
    uint8_t nmTubes;
    qreal   diamTube;
    qreal   radTube;

    QVector< QVector < QVector < QGraphicsEllipseItem * > > > vTrackSystem;



    void createConnections();
    void cleanSystem();                                         // очищаем систему

    void createTrack(QLineF line);
    void deleteTrack();
    QList<QGraphicsEllipseItem *> getMaskTrack(QGraphicsLineItem* track);
    QList<QGraphicsEllipseItem *> maskTrack;

    void drawSystem();                                          // рисуем систему
    void drawMaskTack(QList<QGraphicsEllipseItem *> lstHits);   // рисуем маску трека


    void getInstance(bool f_track, uint8_t levelNoise);


public slots:
    void startGenerationDataSet();
    void stopGenerationDataSet();
    void changeStateVisualization();

    void closeApplication();


};

#endif // MAINWINDOW_H
