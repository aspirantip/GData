#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle( "Генерация данных" );


    createConnections();


    scene = new QGraphicsScene();
    ui->gv_canvas->setScene( scene );
    ui->gv_canvas->update();
    ui->gv_canvas->scale(1.5, 1.5);

    nmChambers  = 2;
    nmLayers    = 3;
    nmTubes     = 10;
    diamTube    = 30.;
    radTube     = diamTube/2.;

    QPen p_line;
    p_line.setColor(Qt::red);
    p_line.setWidth(1);
    QGraphicsLineItem* l_track = scene->addLine( QLineF(20.0, 10.0, 200.0, 200.0), p_line);

    drawSystem();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createConnections()
{

}

void MainWindow::drawSystem()
{
    qDebug() << "MainWindow::drawSystem()";

    uint chambersPositionY = 50;
    uint chambersPositionX = 50;

    // рисуем систему детектора/детекторов
    for (uint8_t chamber = 0; chamber < nmChambers; ++chamber)
    {
        QPointF st_pointSystem(chambersPositionX, chambersPositionY);   // точка, относительно которой строится система
        QPointF st_point;                                               // точка, относительно которой строится элемент в системе
        st_point = st_pointSystem;

        //QPen pen(Qt::black, 1, Qt::DotLine);
        QVector < QVector <QGraphicsEllipseItem *> > vLayers;
        for (uint8_t lr = 0; lr < nmLayers; ++lr)
        {
            QVector <QGraphicsEllipseItem *> layer;
            vLayers.append( layer );

            st_point.setX( st_pointSystem.x() );

            // для среднего слоя делаем смещение по координате Х на половину диаметра трубки
            if ( lr == 1 )
                st_point.setX( st_point.x() + radTube);

            for (uint tb = 0; tb < nmTubes; ++tb)
            {
                vLayers[lr].append(scene->addEllipse( st_point.x(), st_point.y(), diamTube, diamTube));
                //system_el[lr].append(scene->addEllipse( st_point.x(), st_point.y(), diamTube, diamTube));
                st_point.setX( st_point.x() + diamTube );
            }


            uint y_shift = sqrt( pow(diamTube, 2) - pow(radTube, 2) );
            st_point.setY( st_point.y() + y_shift);
        }
        vTrackSystem.insert( chamber, vLayers );

        chambersPositionY += 100;  //Меняем позицию для новой камеры по Y на 100
    }

    ui->gv_canvas->update();
}

void MainWindow::cleanSystem()
{

}

void MainWindow::drawMaskTack(QVector<QPoint> vMask)
{

}

void MainWindow::drawMaskTack(QList<QGraphicsEllipseItem *> lstHits)
{

}
