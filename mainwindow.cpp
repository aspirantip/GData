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
    //ui->gv_canvas->scale(1.5, 1.5);

    l_track = new QGraphicsLineItem();


    nmChambers  = 2;
    nmLayers    = 3;
    nmTubes     = 10;
    diamTube    = 30.;
    radTube     = diamTube/2.;


    drawSystem();



    getInstance();

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
    for (uint8_t ch = 0; ch < nmChambers; ++ch)
    {
        for (uint8_t lr = 0; lr < nmLayers; ++lr)
        {
            for (uint8_t tb = 0; tb < nmTubes; ++tb)
            {
                vTrackSystem[ch][lr][tb]->setBrush(QBrush(Qt::white));
            }
        }
    }

    delete l_track;

    ui->gv_canvas->update();
}

void MainWindow::createTrack(QLineF line)
{
    QPen penLine;
    penLine.setColor(Qt::red);
    penLine.setWidth(1);

    l_track = scene->addLine( line, penLine);
}

QList<QGraphicsEllipseItem *> MainWindow::getMaskTrack(QGraphicsLineItem* track)
{
    QGraphicsItem* crItem;
    QGraphicsEllipseItem el;
    QList<QGraphicsEllipseItem *> lstHits;

    QList<QGraphicsItem *> lstItems = track->collidingItems();
    foreach(crItem, lstItems)
    {
        if (crItem->type() == el.type()) {
            QGraphicsEllipseItem *ellipse = qgraphicsitem_cast<QGraphicsEllipseItem *>(crItem);

            lstHits << ellipse;
        }
    }

    return lstHits;
}

void MainWindow::drawMaskTack(QList<QGraphicsEllipseItem *> lstHits)
{
    foreach (QGraphicsEllipseItem* crEllipseItem, lstHits)
        crEllipseItem->setBrush(QBrush(Qt::green));

    ui->gv_canvas->update();
}

void MainWindow::getInstance()
{
    createTrack( QLineF(20.0, 10.0, 200.0, 300.0) );
    QList<QGraphicsEllipseItem *> maskTrack = getMaskTrack( l_track );
    drawMaskTack( maskTrack );


    // text format instance
    // ==============================================================
    for (uint8_t ch = 0; ch < nmChambers; ++ch)
    {
        for (uint8_t lr = 0; lr < nmLayers; ++lr)
        {
            for (uint8_t tb = 0; tb < nmTubes; ++tb)
            {
                bool f_hit = false;
                foreach (QGraphicsEllipseItem * tube, maskTrack) {
                    if (vTrackSystem[ch][lr][tb] == tube){
                        f_hit = true;
                        break;
                    }
                }

                f_hit ? std::cout << 1 << " " : std::cout << 0 << " ";
             }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

}
