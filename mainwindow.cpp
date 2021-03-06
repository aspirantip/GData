#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    f_loadModel(false)
{
    ui->setupUi(this);
    this->setWindowTitle( "Генерация данных" );
    ui->sbNoiseLevel->setValue( 10 );


    brHit   = QBrush(Qt::green);
    brNoise = QBrush(Qt::gray);
    brClean = QBrush(Qt::white);


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

    // Initialization state machine
    // ====================================================
    createStates();
    createTransitions();
    stateMachine.setInitialState(initState);
    QTimer::singleShot(0, &stateMachine, SLOT(start()));


    // Files and TextStream
    // ====================================================
    fl_train_data.setFileName("train_data.txt");
    fl_test_data.setFileName("test_data.txt");
    fl_train_data.open(QIODevice::WriteOnly | QIODevice::Text);
    fl_test_data.open(QIODevice::WriteOnly | QIODevice::Text);
    out_train_data.setDevice( &fl_train_data );
    out_test_data.setDevice( &fl_test_data );


    // Neural Network Model
    // ====================================================
    ClassModel = new NNModel(this);



    createConnections();

}

MainWindow::~MainWindow()
{
    delete ui;

    out_train_data.flush();
    out_test_data.flush();
    fl_train_data.close();
    fl_test_data.close();
}

void MainWindow::createConnections()
{
    connect(&runningState, &QState::entered, this, &MainWindow::startGenerationDataSet);
    connect(&stateMachine, &QStateMachine::finished, this, &MainWindow::closeApplication);

    connect(ui->chbVisualization, &QCheckBox::stateChanged, this, &MainWindow::changeStateVisualization);

    connect(ui->pbLoadModel, &QPushButton::clicked, this, &MainWindow::loadNeuralNetworkModel);
    connect(ui->pbClassify, &QPushButton::clicked, this, &MainWindow::classifyImage);

    connect(ui->sbThreshold, SIGNAL(valueChanged(double)), ClassModel, SLOT(setThreshold(double)));
}

void MainWindow::createStates()
{
    qDebug() << "createStates()";

    initState = new QState(&stateMachine);
    initState->assignProperty(ui->chbVisualization, "checked", true);
    initState->assignProperty(ui->pbDataGenStop,    "enabled", false);


    normalState = new QState(&stateMachine);
    runningState.setParent(normalState);
    runningState.assignProperty(ui->pbDataGenStart, "enabled", false);
    runningState.assignProperty(ui->pbDataGenStop,  "enabled", true);
    runningState.assignProperty(this, "running", true);

    stoppedState = new QState(normalState);
    stoppedState->assignProperty(ui->pbDataGenStart, "enabled", true);
    stoppedState->assignProperty(ui->pbDataGenStop,  "enabled", false);
    stoppedState->assignProperty(this, "running", false);

    finalState = new QFinalState(&stateMachine);

}

void MainWindow::createTransitions()
{
    qDebug() << "createTransitions()";

    initState->addTransition(initState, SIGNAL(propertiesAssigned()), stoppedState);
    normalState->addTransition(ui->pbExit, SIGNAL(clicked()), finalState);
    runningState.addTransition(ui->pbDataGenStop, SIGNAL(clicked()), stoppedState);
    runningState.addTransition(this, SIGNAL(stoppedGenerationDataSet()), stoppedState);
    stoppedState->addTransition(ui->pbDataGenStart, SIGNAL(clicked()), &runningState);
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
    //qDebug() << "cleanSystem";

    for (uint8_t ch = 0; ch < nmChambers; ++ch)
    {
        for (uint8_t lr = 0; lr < nmLayers; ++lr)
        {
            for (uint8_t tb = 0; tb < nmTubes; ++tb)
            {
                vTrackSystem[ch][lr][tb]->setBrush( brClean );
            }
        }
    }

    ui->gv_canvas->update();
}

QGraphicsLineItem* MainWindow::createTrack(const QLineF line)
{
    deleteTrack();

    QPen penLine;
    penLine.setColor( Qt::red );
    penLine.setWidth( 1 );

    return scene->addLine( line, penLine);
}

inline void MainWindow::deleteTrack()
{
    //qDebug() << "\tdeleteTrack()";

    if (track != nullptr){
        //qDebug() << "\ttrack != nullprt";

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
        crEllipseItem->setBrush( brHit );

    ui->gv_canvas->update();
}

void MainWindow::drawNoiseHits(QList<QGraphicsEllipseItem *> lstHits)
{
    foreach (QGraphicsEllipseItem* crEllipseItem, lstHits)
        crEllipseItem->setBrush( brNoise );

    ui->gv_canvas->update();
}

void MainWindow::getInstance(const bool f_track, const uint8_t levelNoise, std::vector<float> &image)
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

    // очищаем контейнеры
    // ==============================================================

    lstHitsTrack.clear();
    lstHitsNoise.clear();


    // type of sample
    // ==============================================================
    if (f_track){
        const int field (diamTube*nmTubes + radTube);
        float x1 = rand() % field + 50;
        float x2 = rand() % field + 50;
        const float y1 = 0.0;
        const float y2 = 300.0;
        l_track = QLineF(x1, y1, x2, y2);

        track = createTrack( l_track );
        lstHitsTrack = getMaskTrack( track );
        deleteTrack();
    }


    // add noise
    // ==============================================================
    if (levelNoise > 0){
        bool f_dbg_noise = false;
        if (f_dbg_noise){
            qDebug() << "noise info:";
            qDebug() << "=====================================";
        }

        for (uint8_t ind = 0; ind < levelNoise; ++ind){
            uint8_t nmChamber = rand() % nmChambers;
            uint8_t nmLayer   = rand() % nmLayers;
            uint8_t nmTube    = rand() % nmTubes;
            if (f_dbg_noise){
                qDebug() << "||   hit: " << ind;
                qDebug() << "||   ================================";
                qDebug() << "||   number of chamber: " << nmChamber;
                qDebug() << "||   number of layer:   " << nmLayer;
                qDebug() << "||   number of tube:    " << nmTube;
                qDebug() << "||   ================================";
                qDebug() << "||";
            }

            // проверяем, что данная трубка не входит в список listHitsTrack
            // если все норм то добавляем данную трубку в lstHitsNoise
            bool f_hitTrack = false;
            foreach(QGraphicsEllipseItem* hit, lstHitsTrack){
                if (vTrackSystem[nmChamber][nmLayer][nmTube] == hit){
                    f_hitTrack = true;
                    break;
                }
            }
            if ( !f_hitTrack )
                lstHitsNoise.append( vTrackSystem[nmChamber][nmLayer][nmTube] );

        }

        if (f_dbg_noise)
            qDebug() << "=====================================\n";
    }



    // save image to vector
    // ==============================================================
    for (uint8_t ch = 0; ch < nmChambers; ++ch)
    {
        for (uint8_t lr = 0; lr < nmLayers; ++lr)
        {
            for (uint8_t tb = 0; tb < nmTubes; ++tb)
            {
                bool f_hit = false;
                foreach (QGraphicsEllipseItem* hit_track, lstHitsTrack) {
                    if (vTrackSystem[ch][lr][tb] == hit_track){
                        f_hit = true;
                        break;
                    }
                }

                foreach (QGraphicsEllipseItem* hit_noise, lstHitsNoise) {
                    if (vTrackSystem[ch][lr][tb] == hit_noise){
                        f_hit = true;
                        break;
                    }
                }

                f_hit ? image.push_back(1) : image.push_back(0);
            }
        }
    }
}

void MainWindow::showImage(const bool f_track)
{
    cleanSystem();

    if (f_track)
        track = createTrack( l_track );
    else
        deleteTrack();

    drawNoiseHits( lstHitsNoise );
    drawMaskTack ( lstHitsTrack );
}

void MainWindow::saveImage(const bool f_track, const uint32_t indImage, const std::vector<float> image)
{
    QString image_txt;
    foreach (float value, image) {
        value ? image_txt += "1 " : image_txt += "0 ";
    }

    // сохранение убрать и упаковать в отдельную функцию
    // ===========================================================
    image_txt.chop(1);
    QString sample("|labels " + QString::number(f_track) + " |features " + image_txt + "\n");
    qDebug() << sample << "\n";


    // здесь разбиваем данные на обучающую и тестовую выборку
    // ===========================================================
    qDebug() << "indImage = "  << indImage;
    if (indImage%3 == 0)
        out_test_data << sample;
    else
        out_train_data << sample;
}

void MainWindow::startGenerationDataSet()
{
    ui->statusBar->showMessage( tr("Data generation ..."));

    uint32_t numInstance = ui->sbNumberInstance->value();
    const float_t one_percent = 100.0/numInstance;


    bool f_track = true;
    float onePercent = (nmChambers * nmLayers * nmTubes)/100.0;
    float maxLevel = ui->sbNoiseLevel->value();
    uint8_t topLevelNoise = onePercent * maxLevel;
    uint8_t levelNoise = 0;



    for (uint32_t cnt = 0; cnt < numInstance; ++cnt)
    {
        f_track = rand()%2;

        if (topLevelNoise)
            levelNoise = rand() % topLevelNoise + 1;


        // debug information
        // ==============================================================
        qDebug() << "Image #" << cnt << "\t";
        qDebug() << "f_track = " << f_track;
        qDebug() << "levelNoise = " << levelNoise;
        qDebug() << "topLevelNoise = " << topLevelNoise;


        std::vector<float> image;
        getInstance(f_track, levelNoise, image);
        if (ui->chbVisualization->isChecked())
            showImage(f_track);
        if (ui->chbWriteImage->isChecked())
            saveImage(f_track, cnt, image);


        ui->prbProgress->setValue( (cnt+1)*one_percent );
    }

    out_test_data.flush();
    out_train_data.flush();

    ui->statusBar->showMessage(tr("Data generation is complete."));
    emit stoppedGenerationDataSet();

}

void MainWindow::stopGenerationDataSet()
{

}

void MainWindow::changeStateVisualization()
{
    qDebug() << "changeStateVisualization()";

    if (ui->chbVisualization->isChecked()){
        if (lstHitsTrack.size() > 0)
            track = createTrack( l_track );
        drawMaskTack( lstHitsTrack );
        drawNoiseHits( lstHitsNoise );
    }
    else{

        deleteTrack();
        cleanSystem();
    }

}

void MainWindow::loadNeuralNetworkModel()
{
    qDebug() << "MainWindow::loadNeuralNetworkModel()";

    // 1# Load model
    // =============================================
    //const std::string modelFilePath = "/home/plotnikov/cntk/Examples/Image/MNIST/Output/Models/01_OneHidden";
    const std::string modelFilePath = "/home/plotnikov/cntk/Projects/Models/LR_ImageTrack.dnn";
    f_loadModel = ClassModel->loadModel( modelFilePath );

    if (f_loadModel){
        ClassModel->setThreshold( ui->sbThreshold->value() );
        ui->statusBar->showMessage(tr("Модель загружена: %1").arg(modelFilePath.data()));
    }

}

void MainWindow::classifyImage()
{
    qDebug() << "MainWindow::ClassifyImage()";


    if ( !f_loadModel ){
        QMessageBox::critical(this, tr("Critical error"),
                             tr("Model of neural network have not been loaded."));
        return;
    }


    // 2# create image
    // =============================================
    /*
    std::vector<float> image {0, 1, 1, 1, 0, 0, 0, 0, 0, 0,
                              1, 0, 0, 1, 0, 0, 0, 0, 0, 0,
                              0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
                              0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
                              0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
                              0, 0, 0, 0, 0, 0, 1, 0, 0, 0};
    */

    /*
    std::vector<float> image {0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
                              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                              0, 0, 1, 0, 0, 1, 0, 0, 0, 0,
                              0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
                              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                              0, 0, 0, 0, 0, 0, 0, 0, 1, 0};
    */

    std::vector<float> image;
    size_t size_ivector = ClassModel->getSizeInputVector();


    bool f_track = rand()%2;
    float onePercent = (nmChambers * nmLayers * nmTubes)/100.0;
    float maxLevel = ui->sbNoiseLevel->value();
    uint8_t topLevelNoise = onePercent * maxLevel;
    uint8_t levelNoise = 0;


    if (topLevelNoise)
        levelNoise = rand() % topLevelNoise + 1;


    // debug information
    // ==============================================================
    qDebug() << "\nImage of system:" << "\t";
    qDebug() << "f_track = " << f_track;
    qDebug() << "levelNoise = " << levelNoise;
    qDebug() << "topLevelNoise = " << topLevelNoise;


    getInstance(f_track, levelNoise, image);
    showImage(f_track);


    // проверка размерностей векторов
    if (size_ivector != image.size()){
        QMessageBox::critical(this, tr("Критическая ошибка"),
                             tr("Размерность входного вектора модели нейронной сети и вектора образа не совпадает. (%1/%2)").arg(size_ivector).arg(image.size()));
        return;
    }



    // 3# classify image
    // =============================================

    bool result = ClassModel->ClassificationImage( image );

    if (result)
        ui->statusBar->showMessage("Образ с треком.");
    else
        ui->statusBar->showMessage("Образ без трека.");
}

void MainWindow::closeApplication()
{
    close();
}


