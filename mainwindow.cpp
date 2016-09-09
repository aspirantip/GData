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

    track = new QGraphicsLineItem();


    nmChambers  = 2;
    nmLayers    = 3;
    nmTubes     = 10;
    diamTube    = 30.;
    radTube     = diamTube/2.;


    drawSystem();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createConnections()
{
    // PushButton
    connect(ui->pbDataGenStart, &QPushButton::clicked, this, &MainWindow::startGenerationDataSet);
    connect(ui->pbDataGenStop,  &QPushButton::clicked, this, &MainWindow::stopGenerationDataSet);
    connect(ui->pbExit, &QPushButton::clicked, this, &MainWindow::closeApplication);


    connect(ui->chbVisualization, &QCheckBox::stateChanged, this, &MainWindow::changeStateVisualization);
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
    qDebug() << "cleanSystem";

    deleteTrack();


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

    ui->gv_canvas->update();
}

void MainWindow::createTrack(QLineF line)
{
    deleteTrack();

    QPen penLine;
    penLine.setColor( Qt::red );
    penLine.setWidth( 1 );

    track = scene->addLine( line, penLine);
}

void MainWindow::deleteTrack()
{
    if (track != nullptr){
        delete track;
        track = nullptr;
    }
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

void MainWindow::getInstance(bool f_track, uint8_t levelNoise)
{
    /*
     * 1) задать линию
     *      а) обе точки прямой задаются случайным образом
     *      б) устанавливается минимально возможный шаг для смещения линии и
     *         создается N (например 100) образцов с лучайным шумом
     *         и случайным количеством отсутствующих хитов (тут тоже надо подойти с умом)
     * 2) ищем маску
     * 3) конвертируем в текстовый формат или формат Mat OpenCV
     * 4) сохраняем в БД
     *
     *  добавить образцы просто с шумом и образцы где трек прошел только через часть детекторов
     */

    const int field (diamTube*nmTubes + radTube);


    cleanSystem();
    maskTrack.clear();


    // type of sample
    // ==============================================================
    if (f_track){
        float x1 = rand() % field + 50;
        float x2 = rand() % field + 50;
        const float y1 = 0.0;
        const float y2 = 300.0;
        l_track = QLineF(x1, y1, x2, y2);
        createTrack( l_track );
        maskTrack = getMaskTrack( track );
    }


    // add noise
    // ==============================================================



    // visualization
    // ==============================================================
    if (ui->chbVisualization->isChecked())
        drawMaskTack( maskTrack );
    else
        deleteTrack();


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
            //std::cout << std::endl;
        }
        //std::cout << std::endl;
    }

    std::cout << std::endl;

}

void MainWindow::startGenerationDataSet()
{
    const uint32_t numInstance = 1;
    bool f_track = true;
    uint8_t levelNoise = 0;

    for (uint32_t cnt = 0; cnt < numInstance; ++cnt)
    {
        f_track = rand()%2;
        std::cout << "f_track = " << f_track << "\n";

        std::cout << "#" << cnt << "\t";
        getInstance(f_track, levelNoise);

    }
     std::cout << std::endl;

}

void MainWindow::stopGenerationDataSet()
{

}

void MainWindow::changeStateVisualization()
{
    qDebug() << "changeStateVisualization()";

    if (ui->chbVisualization->isChecked()){
        createTrack( l_track );
        drawMaskTack( maskTrack );
    }
    else
        cleanSystem();
}

void MainWindow::closeApplication()
{
    close();
}


