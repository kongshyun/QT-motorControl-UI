// MotorLoadGraphWidget - 모터 부하량 실시간 그래프 구현
#include "motorloadgraphwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QScreen>
#include <algorithm>

MotorLoadGraphWidget::MotorLoadGraphWidget(QWidget *parent)
    : QWidget(parent)
    , customPlot(nullptr)
    , updateTimer(new QTimer(this))
    , currentRPM(0)
{
    setWindowTitle("모터 부하량 실시간 그래프");
    
    // 부모가 있으면 embedded 모드, 없으면 standalone 모드
    bool isEmbedded = (parent != nullptr);
    
    if (!isEmbedded) {
        resize(800, 500);
        
        // 화면 중앙에 위치
        QScreen *screen = QApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }
    
    // 레이아웃 설정
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(isEmbedded ? 2 : 10, isEmbedded ? 2 : 10, isEmbedded ? 2 : 10, isEmbedded ? 2 : 10);
    mainLayout->setSpacing(isEmbedded ? 2 : 5);
    
    // 정보 표시 레이아웃 (embedded 모드에서는 더 작게)
    if (!isEmbedded) {  // standalone 모드에서만 정보 표시
        QHBoxLayout *infoLayout = new QHBoxLayout();
        
        QLabel *modeLabel = new QLabel("모드:");
        modeLabel->setStyleSheet("font: 500 10pt 'JetBrains Mono'; color: #333;");
        QLabel *modeValueLabel = new QLabel("회전");
        modeValueLabel->setObjectName("modeValueLabel");
        modeValueLabel->setStyleSheet("font: 700 10pt 'JetBrains Mono'; color: #0066cc;");
        
        QLabel *rpmLabel = new QLabel("RPM:");
        rpmLabel->setStyleSheet("font: 500 10pt 'JetBrains Mono'; color: #333;");
        QLabel *rpmValueLabel = new QLabel("0");
        rpmValueLabel->setObjectName("rpmValueLabel");
        rpmValueLabel->setStyleSheet("font: 700 10pt 'JetBrains Mono'; color: #ff6600;");
        
        infoLayout->addWidget(modeLabel);
        infoLayout->addWidget(modeValueLabel);
        infoLayout->addSpacing(20);
        infoLayout->addWidget(rpmLabel);
        infoLayout->addWidget(rpmValueLabel);
        infoLayout->addStretch();
        
        mainLayout->addLayout(infoLayout);
    }
    
    // QCustomPlot 생성 및 설정
    customPlot = new QCustomPlot(this);
    mainLayout->addWidget(customPlot);
    
    setupGraph(isEmbedded);
    
    // 타이머 설정 (100ms마다 업데이트)
    connect(updateTimer, &QTimer::timeout, this, &MotorLoadGraphWidget::updateGraph);
    updateTimer->setInterval(100);
}

MotorLoadGraphWidget::~MotorLoadGraphWidget()
{
    if (updateTimer->isActive()) {
        updateTimer->stop();
    }
}

void MotorLoadGraphWidget::setupGraph(bool isEmbedded)
{
    // 그래프 기본 설정
    customPlot->setBackground(QBrush(QColor(250, 250, 250)));
    
    // 축 설정
    setupAxes(isEmbedded);
    
    // 그래프 추가
    customPlot->addGraph();
    QPen graphPen(QColor(76, 175, 80), isEmbedded ? 2 : 3); // 선 굵기
    customPlot->graph(0)->setPen(graphPen);
    customPlot->graph(0)->setBrush(QBrush(QColor(76, 175, 80, 50))); // 반투명 채우기 (더 진하게)
    customPlot->graph(0)->setName("모터 부하량");
    
    // 데이터 포인트 스타일 설정
    customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QColor(76, 175, 80), QColor(255, 255, 255), isEmbedded ? 4 : 6));
    
    // 범례 설정 (embedded에서는 숨김)
    if (!isEmbedded) {
        setupLegend();
    } else {
        customPlot->legend->setVisible(false);
    }
    
    // 그리드 설정
    customPlot->xAxis->grid()->setPen(QPen(QColor(200, 200, 200), 1, Qt::DashLine));
    customPlot->yAxis->grid()->setPen(QPen(QColor(200, 200, 200), 1, Qt::DashLine));
    customPlot->xAxis->grid()->setSubGridVisible(false);
    customPlot->yAxis->grid()->setSubGridVisible(false);
    
    // 상호작용 설정 (embedded에서는 제한)
    if (isEmbedded) {
        customPlot->setInteractions(QCP::iNone); // 상호작용 비활성화
    } else {
        customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    }
}

void MotorLoadGraphWidget::setupAxes(bool isEmbedded)
{
    int labelFontSize = isEmbedded ? 7 : 10;
    int tickFontSize = isEmbedded ? 6 : 9;
    
    // X축 (시간) 설정
    customPlot->xAxis->setLabel(isEmbedded ? "시간(초)" : "시간 (초)");
    customPlot->xAxis->setLabelFont(QFont("JetBrains Mono", labelFontSize));
    customPlot->xAxis->setTickLabelFont(QFont("JetBrains Mono", tickFontSize));
    customPlot->xAxis->setRange(0, isEmbedded ? 30 : 60); // 초기 고정 윈도우 크기
    customPlot->xAxis->setLabelColor(QColor(60, 60, 60));
    customPlot->xAxis->setTickLabelColor(QColor(80, 80, 80));
    
    // Y축 (부하량) 설정
    customPlot->yAxis->setLabel(isEmbedded ? "부하(%)" : "부하량 (%)");
    customPlot->yAxis->setLabelFont(QFont("JetBrains Mono", labelFontSize));
    customPlot->yAxis->setTickLabelFont(QFont("JetBrains Mono", tickFontSize));
    customPlot->yAxis->setRange(0, 50); // 초기 범위를 50%로 줄임
    customPlot->yAxis->setLabelColor(QColor(60, 60, 60));
    customPlot->yAxis->setTickLabelColor(QColor(80, 80, 80));
    
    // 축 스타일 설정
    int axisLineWidth = isEmbedded ? 1 : 2;
    customPlot->xAxis->setBasePen(QPen(QColor(100, 100, 100), axisLineWidth));
    customPlot->yAxis->setBasePen(QPen(QColor(100, 100, 100), axisLineWidth));
    customPlot->xAxis->setTickPen(QPen(QColor(100, 100, 100), 1));
    customPlot->yAxis->setTickPen(QPen(QColor(100, 100, 100), 1));
    customPlot->xAxis->setSubTickPen(QPen(QColor(150, 150, 150), 1));
    customPlot->yAxis->setSubTickPen(QPen(QColor(150, 150, 150), 1));
    
    // embedded 모드에서는 라벨 패딩 줄이기
    if (isEmbedded) {
        customPlot->xAxis->setLabelPadding(2);
        customPlot->yAxis->setLabelPadding(2);
        customPlot->xAxis->setTickLabelPadding(1);
        customPlot->yAxis->setTickLabelPadding(1);
    }
}

void MotorLoadGraphWidget::setupLegend()
{
    customPlot->legend->setVisible(true);
    customPlot->legend->setFont(QFont("JetBrains Mono", 9));
    customPlot->legend->setTextColor(QColor(60, 60, 60));
    customPlot->legend->setBrush(QBrush(QColor(255, 255, 255, 200)));
    customPlot->legend->setBorderPen(QPen(QColor(180, 180, 180)));
    customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignRight);
}

void MotorLoadGraphWidget::addDataPoint(double time, double load)
{
    timeData.append(time);
    loadData.append(load);
    
    // 최대 1000개 데이터 포인트 유지 (메모리 관리)
    if (timeData.size() > 1000) {
        timeData.removeFirst();
        loadData.removeFirst();
    }
}

void MotorLoadGraphWidget::updateGraph()
{
    if (timeData.isEmpty() || loadData.isEmpty()) {
        return;
    }
    
    // 데이터 설정
    customPlot->graph(0)->setData(timeData, loadData);
    
    // 슬라이딩 윈도우 X축 조정
    if (!timeData.isEmpty()) {
        double currentTime = timeData.last();
        bool isEmbedded = (parent() != nullptr);
        double timeWindow = isEmbedded ? 30.0 : 60.0; // 고정 시간 윈도우
        
        // 항상 고정된 시간 윈도우로 표시 (왼쪽으로 스크롤 효과)
        if (currentTime > timeWindow) {
            // 데이터가 윈도우를 넘어서면 슬라이딩 시작
            double minTime = currentTime - timeWindow;
            double maxTime = currentTime;
            customPlot->xAxis->setRange(minTime, maxTime);
        } else {
            // 아직 윈도우를 채우지 못했으면 0부터 현재 시간까지
            customPlot->xAxis->setRange(0, timeWindow);
        }
    }
    
    // Y축 동적 조정 - 데이터에 맞게 범위 설정
    if (!loadData.isEmpty()) {
        double maxLoad = *std::max_element(loadData.begin(), loadData.end());
        double minLoad = *std::min_element(loadData.begin(), loadData.end());
        
        // 최대값에 여유를 두고 범위 설정 (최소 20% 여유)
        double yMax = qMax(50.0, maxLoad * 1.2); // 최소 50%, 실제 최대값의 120%
        double yMin = qMax(0.0, minLoad - 5.0);  // 최소값에서 5% 여유
        
        customPlot->yAxis->setRange(yMin, yMax);
    }
    
    // 그래프 새로 그리기
    customPlot->replot();
}

void MotorLoadGraphWidget::clearData()
{
    timeData.clear();
    loadData.clear();
    customPlot->graph(0)->setData(timeData, loadData);
    
    // 축을 초기 범위로 리셋
    bool isEmbedded = (parent() != nullptr);
    double timeWindow = isEmbedded ? 30.0 : 60.0;
    customPlot->xAxis->setRange(0, timeWindow);
    customPlot->yAxis->setRange(0, 50); // Y축도 초기 범위로 리셋
    
    customPlot->replot();
}

void MotorLoadGraphWidget::startUpdating()
{
    updateTimer->start();
}

void MotorLoadGraphWidget::stopUpdating()
{
    updateTimer->stop();
    
    // 타이머 중지 전 마지막으로 그래프를 다시 그려서 데이터 보존
    if (customPlot && !timeData.isEmpty() && !loadData.isEmpty()) {
        customPlot->replot();
    }
}

void MotorLoadGraphWidget::preserveGraph()
{
    // 그래프 데이터를 확실히 보존하고 다시 그리기
    updateTimer->stop();
    
    if (customPlot && !timeData.isEmpty() && !loadData.isEmpty()) {
        // 데이터를 다시 설정하고 그래프 업데이트
        customPlot->graph(0)->setData(timeData, loadData);
        
        // Y축 범위 유지
        if (!loadData.isEmpty()) {
            double maxLoad = *std::max_element(loadData.begin(), loadData.end());
            double minLoad = *std::min_element(loadData.begin(), loadData.end());
            double yMax = qMax(50.0, maxLoad * 1.2);
            double yMin = qMax(0.0, minLoad - 5.0);
            customPlot->yAxis->setRange(yMin, yMax);
        }
        
        // 강제로 다시 그리기
        customPlot->replot();
    }
}

void MotorLoadGraphWidget::setMotorMode(const QString &mode)
{
    motorMode = mode;
    QLabel *modeValueLabel = findChild<QLabel*>("modeValueLabel");
    if (modeValueLabel) {
        modeValueLabel->setText(mode);
    }
    
    // 윈도우 제목 업데이트
    setWindowTitle(QString("모터 부하량 실시간 그래프 - %1 모드").arg(mode));
}

void MotorLoadGraphWidget::setMotorSpeed(int rpm)
{
    currentRPM = rpm;
    QLabel *rpmValueLabel = findChild<QLabel*>("rpmValueLabel");
    if (rpmValueLabel) {
        rpmValueLabel->setText(QString("%1").arg(rpm));
    }
}

void MotorLoadGraphWidget::closeEvent(QCloseEvent *event)
{
    stopUpdating();
    emit windowClosed();
    event->accept();
}
