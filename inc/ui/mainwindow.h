// MainWindow - 메인 UI 컨트롤러
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/*
  테스트 모드 설정 - qmake에서 DEFINES += TEST_MODE_RANDOM_DATA=1로 설정 가능
  - 테스트 모드 ON: #define TEST_MODE_RANDOM_DATA 1
  - 테스트 모드 OFF: #define TEST_MODE_RANDOM_DATA 0 (ESP32 준비 완료 후)
*/
#ifndef TEST_MODE_RANDOM_DATA
#define TEST_MODE_RANDOM_DATA 0
#endif

#include <QMainWindow>
#include <QTimer>
#include <QDateTime>
#include <QSerialPortInfo>
#include <QString>
#include <QMessageBox>
#if TEST_MODE_RANDOM_DATA
#include <random>
#endif
#include "serialhandler.h"
#include "motorcontrol.h"
#include "motorcommandfactory.h"
#include "motorloadgraphwidget.h"

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
    ~MainWindow();

private slots:
    void updateDateTime();
    void updateTimeProgress();  // 시간 진행 업데이트
#if TEST_MODE_RANDOM_DATA
    void generateTestData();    // 테스트용 랜덤 데이터 생성
#endif
    void on_getButton_clicked();
    void on_setButton_clicked();
    void on_goButton_clicked();
    void handlePortComboBoxChanged(const QString &portName);
    void on_connectButton_clicked();
    void on_disconnectButton_clicked();
    void on_rotationModeRadio_toggled(bool checked);
    void on_timeModeRadio_toggled(bool checked);
    void on_stopButton_clicked();
    void on_closeButton_clicked();
    void on_reloadButton_clicked();
    void on_infoButton_clicked();

    void handleSerialResponse(const QString &data);
    
private:
    // 상수 정의
    static constexpr int TIMER_INTERVAL_MS = 1000;       // 타이머 간격 (1초)
    static constexpr int MAX_GRAPH_POINTS = 1000;        // 그래프 최대 데이터 포인트
    static constexpr int DEFAULT_BAUD_RATE = 115200;     // 기본 전송 속도
    
    // 메시지박스 스타일시트 상수
    static QString getMessageBoxStyle();
    
private:
    Ui::MainWindow *ui;
    QTimer *timer;
    QTimer *timeUpdateTimer;  // 시간 모드용 1초 타이머
#if TEST_MODE_RANDOM_DATA
    QTimer *testDataTimer;    // 테스트용 랜덤 데이터 생성 타이머
#endif
    SerialHandler *serialHandler;
    QString selectedPortName;


    //내부 상태 관리용 변수
    bool isSettingConfirmed;
    bool isGetButtonPressed;  // GET 버튼이 눌렸는지 확인
    int confirmedSpeed;
    int confirmedValue;
    MotorMode currentMode;
    bool isMotorRunning;
    bool isMotorPaused;  // 모터 일시정지 상태
    
    // 시간 모드 관련 변수
    int totalTimeSeconds;     // 전체 목표 시간 (초)
    int elapsedTimeSeconds;   // 경과 시간 (초)
    
    // 회전 모드 관련 변수
    int currentRotationCount; // 현재 회전수
    int targetRotationCount;  // 목표 회전수
    double currentMotorLoad;  // 현재 모터 부하량 (%)
    qint64 graphStartTime;    // 그래프 시작 시간 (초 단위 타임스탬프)
    bool completionDialogShown;  // 완료 대화상자 표시 여부

    MotorControl motorControl;
    
    void populateSerialPorts();
    void log(const QString &message);
    void updateUIForMode(MotorMode mode);
    void setUIEnabled(bool enabled);
    void setPausedUIState();  // 일시정지 상태 UI 설정
    void initializeTimeComboBoxes();
    int getTotalSeconds() const;
    void updateMotorStatus(const QString &status, const QString &color);
    void updateTimeDisplay();  // 시간 모드에서 남은 시간 표시 업데이트
    void showTimeCompletionDialog();  // 시간 완료 대화상자 표시
    void showRotationCompletionDialog();  // 회전 완료 대화상자 표시
    void resetToInitialState();  // 모든 상태를 초기 상태로 리셋
    void updateRotationDisplay();  // 회전 모드 디스플레이 업데이트
    void updateMotorLoadGraph();  // 모터 부하량 그래프 업데이트
    
    // 새로운 UI 업데이트 메서드들
    void updateCircularProgress();  // 원형 진행률 표시기 업데이트
    void updateRotationProgress();  // 회전 모드 진행률 업데이트
    void updateTimeProgressDisplay();      // 시간 모드 진행률 UI 업데이트  
    void updateLoadProgress();      // 부하량 진행률 업데이트
    void drawCircularProgress(QWidget* widget, int percentage, QColor color);  // 원형 진행률 그리기
    void updateModeVisibility();    // 모드별 UI 요소 표시/숨김
    void resetFrameOutputValues();  // frameOutput 모든 값 초기화
    void clearAllGraphData();       // 그래프 데이터 완전 초기화 (새로운 GO 시작 시)
    
    // 로그 출력 함수들
    void logCommand(const QString &command, const QString &details = "");
    void logReceived(const QString &data);
    void logStatus(const QString &status, const QString &details = "");
    void logError(const QString &error);
    void logInfo(const QString &info);
    
};
#endif // MAINWINDOW_H
