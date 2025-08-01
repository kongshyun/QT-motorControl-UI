// MotorLoadGraphWidget - 모터 부하량 실시간 그래프
#ifndef MOTORLOADGRAPHWIDGET_H
#define MOTORLOADGRAPHWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QCloseEvent>
#include <QVector>
#include "qcustomplot.h"

class MotorLoadGraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MotorLoadGraphWidget(QWidget *parent = nullptr);
    ~MotorLoadGraphWidget();

    void addDataPoint(double time, double load);
    void clearData();
    void startUpdating();
    void stopUpdating();
    void preserveGraph();       // 그래프 데이터 보존 모드
    void setMotorMode(const QString &mode);
    void setMotorSpeed(int rpm);

protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    void windowClosed();

private slots:
    void updateGraph();

private:
    QCustomPlot *customPlot;
    QTimer *updateTimer;
    
    QVector<double> timeData;
    QVector<double> loadData;
    
    QString motorMode;
    int currentRPM;
    
    void setupGraph(bool isEmbedded = false);
    void setupAxes(bool isEmbedded = false);
    void setupLegend();
};

#endif // MOTORLOADGRAPHWIDGET_H