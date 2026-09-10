#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //estado inicial
    volumenActual=0.0;
    TempTanque=20.0;


    //rangos de los controles y los qmax configurables
    ui->spincapacidad->setRange(1,10000);
    ui->spincapacidad->setValue(500);

    ui->spinQmax1->setRange(0,20000);
    ui->spinQmax1->setValue(5000);

    ui->spinQmax2->setRange(0,20000);
    ui->spinQmax2->setValue(5000);

    ui->spinQmaxout->setRange(0,50000);
    ui->spinQmaxout->setValue(50000);

    ui->spinPmax->setRange(0,100);
    ui->spinPmax->setValue(10);

    //diales de caudal regulables
    ui->dialQin1->setRange(0,ui->spinQmax1->value() );
    ui->dialQin2->setRange(0,ui->spinQmax2->value());
    ui->dialQout->setRange(0, ui->spinQmaxout->value());

    //dial del heater

    ui->dialheaterpower_2->setRange(0,ui->spinPmax->value());

    //slider de temp para cada fuente
    ui->slidertemp1_2->setRange(0,99);
    ui->slidertemp1_2->setValue(20);
    ui->slidertemp2->setRange(0,99);
    ui->slidertemp2->setValue(20);

    ui->slidermax->setRange(0,100);
    ui->slidermax->setValue(83);
    ui->slidermin->setRange(0,100);
    ui->slidermin->setValue(20);



    //tanque

    ui->progressNivel->setRange(0,100);
    ui->progressNivel->setValue(0);
    ui->progressNivel->setTextVisible(true);
    ui->progressNivel->setFormat("%p%");

    //el boton del heater va arriba del agua
    ui->btnheater->raise();

    ui->btnheater->setStyleSheet(
        "QPushButton {"
        "  background-color: #F08080;"
        "  border: 1px solid #8B0000;"
        "  border-radius: 4px;"
        "  font-weight: bold;"
        "  color: #333;"
        "  padding: 6px;"
        "}"
        "QPushButton:checked {"
        "  background-color: #FF3B30;"
        "  color: white;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #D3D3D3;"
        "  color: #888;"
        "  border: 1px solid #999;"
        "}"
        );




    //fuentes habilitadas

    ui->checkHabilitarfuente1->setChecked(true);
    ui->checkHabilitarfuente2->setChecked(true);


    //conexiones de los labes al mover los controles

    connect(ui->dialQin1, &QDial::valueChanged, this ,&MainWindow::actualizarEtiquetas);
    connect(ui->dialQin2, &QDial::valueChanged, this ,&MainWindow::actualizarEtiquetas);
    connect(ui->dialQout, &QDial::valueChanged, this ,&MainWindow::actualizarEtiquetas);
    connect(ui->dialheaterpower_2, &QDial::valueChanged, this ,&MainWindow::actualizarEtiquetas);
    connect(ui->slidertemp1_2, &QSlider::valueChanged, this ,&MainWindow::actualizarEtiquetas);
    connect(ui->slidertemp2, &QSlider::valueChanged, this ,&MainWindow::actualizarEtiquetas);
    connect(ui->slidermax, &QSlider::valueChanged, this ,&MainWindow::actualizarEtiquetas);
    connect(ui->slidermin, &QSlider::valueChanged, this ,&MainWindow::actualizarEtiquetas);

//qmax le da el maximo al dial
    connect(ui->spinQmax1,QOverload<int>::of(&QSpinBox::valueChanged), this , [this](int v){ui->dialQin1->setRange(0,v);} );
    connect(ui->spinQmax2,QOverload<int>::of(&QSpinBox::valueChanged), this , [this](int v){ui->dialQin2->setRange(0,v);} );
    connect(ui->spinQmaxout,QOverload<int>::of(&QSpinBox::valueChanged), this , [this](int v){ui->dialQout->setRange(0,v);} );
    connect(ui->spinPmax,QOverload<int>::of(&QSpinBox::valueChanged), this , [this](int v){ui->dialheaterpower_2->setRange(0,v);} );


    //timer

    timer = new QTimer(this);
    connect (timer,&QTimer::timeout,this ,&MainWindow::actualizarSimulacion );
    timer->start(intervalo_ms);

    actualizarEtiquetas();


}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::actualizarSimulacion()
{
    const double horas = intervalo_ms /1000.0 /3600.0;


    const double capacidad =ui->spincapacidad->value(); //capacidad leida en el momento

    const double nivelmax = capacidad * ui->slidermax->value() / 100.0;
    const double nivelMin = capacidad * ui->slidermin->value() / 100.0;

    //caudales de entrada

    double qin1 = ui->checkHabilitarfuente1->isChecked() ? ui->dialQin1->value(): 0.0;
    double qin2 = ui->checkHabilitarfuente2->isChecked() ? ui->dialQin2->value(): 0.0;


    if(volumenActual >= nivelmax)
    {

        qin1=0.0;
        qin2=0.0;
    }

//litros que ingresan por tick

    const double litros1 =qin1 *horas;
    const double litros2 =qin2 *horas;

    const double t1= ui->slidertemp1_2->value();
    const double t2= ui->slidertemp2->value();

    const double volumenEntrante= litros1 + litros2;

        if(volumenEntrante>0.0)
        {
        TempTanque= (volumenActual *TempTanque +litros1 *t1 +litros2 * t2) / (volumenActual + volumenEntrante);

        }
        volumenActual += volumenEntrante;

        //para la capacidad del tanque que no se pase

        if(volumenActual>capacidad)
        {
            volumenActual=capacidad;
        }

        //salida

        const double litrosSalen= ui->dialQout->value() * horas;

        volumenActual -=litrosSalen;

        if(volumenActual<0.0)
        {
            volumenActual=0.0;
        }

        //heater

        if(volumenActual<nivelMin)
        {
            ui->btnheater->setChecked(false);
            ui->btnheater->setEnabled(false);
        }
        else{
            ui->btnheater->setEnabled(true);

        }

        if(ui->btnheater->isChecked() && volumenActual > 0.0)
        {
            const double potencia =ui->dialheaterpower_2->value(); //kw

            const double deltaT= (potencia * horas * 3600.0)/ (volumenActual* C_AGUA);

            TempTanque+=deltaT;

            if(TempTanque>100.0)
            {
                TempTanque=100.0;

            }


        }


        actualizarEtiquetas();





}
void MainWindow::actualizarEtiquetas()
{
    //caudales
    ui->lblQin1->setText(QString::number(ui->dialQin1->value()) + " L/h");
    ui->lblQin2->setText(QString::number(ui->dialQin2->value()) + " L/h");
    ui->lblQout->setText(QString::number(ui->dialQout->value()) + " L/h");

    //qintotal

    int qinTotal = 0;

    if(ui->checkHabilitarfuente1->isChecked()) qinTotal += ui->dialQin1->value();
    if(ui->checkHabilitarfuente2->isChecked()) qinTotal+= ui->dialQin2->value();
    ui->lblQintotal->setText(QString::number(qinTotal)+" L/H");

    //temperaturas de las fuentes
    ui->lbltemp1valor->setText(QString::number(ui->slidertemp1_2->value()) + " °C");
    ui->lbltemp2valor->setText(QString::number(ui->slidertemp2->value()) + " °C");


    //niveles min y max

    ui->lblminvalor->setText(QString::number(ui->slidermin->value()));
    ui->lblmaxvalor->setText(QString::number(ui->slidermax->value()));

    //heater

    ui->lblheaterpower->setText(QString::number(ui->dialheaterpower_2->value()) + " kw");
    const double factor = ui->spinPmax->value() *3600.0 / C_AGUA;
    ui->lblheaterpowercalculado->setText(QString::number(factor , 'f',1) + "  °C*L/hr");

    //estado del tanque

    const double capacidad= ui->spincapacidad->value();
    const int porcentaje= (capacidad > 0) ?(int)(volumenActual *100/ capacidad) :0;

    ui->progressNivel->setValue(porcentaje);
    ui->lblcantidadactual->setText(QString::number(volumenActual,'f',3) + " L");
    ui->lbltemptanque->setText(QString::number(TempTanque,'f',4));


}


