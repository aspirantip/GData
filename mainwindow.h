#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QState>
#include <QStateMachine>
#include <QFinalState>
#include <QTimer>
#include <QFile>


#include <iostream>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Q_PROPERTY(bool running READ getRunning WRITE setRunning);

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

    QBrush brHit;
    QBrush brClean;
    QBrush brNoise;

    uint32_t indInstance;

    QVector< QVector < QVector < QGraphicsEllipseItem * > > > vTrackSystem;

    QFile fl_train_data;
    QFile fl_test_data;
    QTextStream out_train_data;
    QTextStream out_test_data;

    // for state machine
    QStateMachine stateMachine;
    QState* initState;
    QState* normalState;
    QState runningState;
    QState* stoppedState;
    QFinalState* finalState;
    bool f_running;             //State of process of data generation


    void createConnections();
    void createStates();
    void createTransitions();
    void cleanSystem();                                         // очищаем систему

    QGraphicsLineItem *createTrack(const QLineF line);
    inline void deleteTrack();

    QList<QGraphicsEllipseItem *> getMaskTrack(QGraphicsLineItem* track);
    QList<QGraphicsEllipseItem *> lstHitsTrack;
    QList<QGraphicsEllipseItem *> lstHitsNoise;

    void drawSystem();                                          // рисуем систему
    void drawMaskTack(QList<QGraphicsEllipseItem *> lstHits);   // рисуем маску трека
    void drawNoiseHits(QList<QGraphicsEllipseItem *> lstHits);  // рисуем трубки с шумовым сигналом


    void getInstance(const bool f_track, const uint8_t levelNoise);


    bool getRunning() const { return f_running; }
    void setRunning(bool state) {f_running = state; }




public slots:
    void startGenerationDataSet();
    void stopGenerationDataSet();
    void changeStateVisualization();

    void closeApplication();

signals:
    void stoppedGenerationDataSet();

};

#endif // MAINWINDOW_H
