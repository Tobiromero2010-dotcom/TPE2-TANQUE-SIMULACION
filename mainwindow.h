#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;


private slots:

    void actualizarSimulacion(); //para cada tick del timer se ejecuta
    void actualizarEtiquetas(); //cuando moves un control refresca los labels




private:
    Ui::MainWindow *ui;
    QTimer *timer;


    //estado del tanque
    double volumenActual; //litros que hay en el tanque ahora
    double TempTanque; //temperatura del agua en el tanque


    //las constantes
    static constexpr double C_AGUA = 4.186; //calor especifico
    static constexpr int intervalo_ms = 100; //cada cuanto corre la simulacion

};
#endif // MAINWINDOW_H
