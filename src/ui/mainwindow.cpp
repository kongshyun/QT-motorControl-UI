// MainWindow - 메인 UI 컨트롤러 구현
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QPainterPath>
#include <QTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , timer(new QTimer(this))
    , timeUpdateTimer(new QTimer(this))
#if TEST_MODE_RANDOM_DATA
    , testDataTimer(new QTimer(this))
#endif
    , serialHandler(new SerialHandler(this))
    , isSettingConfirmed(false)
    , isGetButtonPressed(false)
    , currentMode(MotorMode::ROTATION)
    , isMotorRunning(false)
    , isMotorPaused(false)
    , totalTimeSeconds(0)
    , elapsedTimeSeconds(0)
    , currentRotationCount(0)
    , targetRotationCount(0)
    , currentMotorLoad(0.0)
    , graphStartTime(0)
    , completionDialogShown(false)
{
    ui->setupUi(this);

    connect(timer, &QTimer::timeout, this, &MainWindow::updateDateTime);
    timer->start(TIMER_INTERVAL_MS); //1초마다 실행
    updateDateTime();// 초기 날짜/시간 표시
    
    // 시간 모드용 타이머 설정
    connect(timeUpdateTimer, &QTimer::timeout, this, &MainWindow::updateTimeProgress);
    timeUpdateTimer->setInterval(TIMER_INTERVAL_MS); // 1초마다 실행
    
#if TEST_MODE_RANDOM_DATA
    // 테스트용 랜덤 데이터 생성 타이머 설정
    connect(testDataTimer, &QTimer::timeout, this, &MainWindow::generateTestData);
    testDataTimer->setInterval(TIMER_INTERVAL_MS); // 1초마다 실행
#endif
    log("연결 대기");
    connect(ui->portComboBox,
            &QComboBox::currentTextChanged,
            this,
            &MainWindow::handlePortComboBoxChanged);


    connect(serialHandler, &SerialHandler::dataReceived,
            this, &MainWindow::handleSerialResponse);


    populateSerialPorts();

    // 슬라이더와 스핀박스 연동
    connect(ui->speedSlider, &QSlider::valueChanged, this, [=](int value){
        ui->speedSpinBox->setValue(value);
        // 설정값 변경 시 GET 상태 초기화
        isGetButtonPressed = false;
        ui->setButton->setEnabled(false);
    });
    
    connect(ui->speedSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value){
        ui->speedSlider->setValue(value);
        // 설정값 변경 시 GET 상태 초기화
        isGetButtonPressed = false;
        ui->setButton->setEnabled(false);
    });

    // Initialize mode selection with radio buttons
    motorControl.setCommandStrategy(MotorCommandFactory::createCommand(currentMode));
    updateUIForMode(currentMode);
    
    // 초기 UI 상태 설정 (모터 정지 상태)
    setUIEnabled(true);
    
    // 초기에는 SET 버튼 비활성화
    ui->setButton->setEnabled(false);
    
    // 초기에는 CLOSE, RELOAD 버튼 비활성화
    ui->closeButton->setEnabled(false);
    ui->reloadButton->setEnabled(false);
    
    // 시간 표시 UI 초기화
    updateTimeDisplay();
    
    // 회전 모드 표시 UI 초기화
    updateRotationDisplay();
    
    // 새로운 UI 초기화
    updateModeVisibility();
    updateCircularProgress();  // 시작 시 UI 초기화
    
    // 시간 콤보박스 초기화
    initializeTimeComboBoxes();
    
    // Initialize direction radio buttons (default to CW)
    ui->cwModeRadio->setChecked(true);
    
    // 설정값 변경 감지를 위한 연결
    connect(ui->rotationSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](){
        isGetButtonPressed = false;
        ui->setButton->setEnabled(false);
    });
    
    connect(ui->hoursComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](){
        isGetButtonPressed = false;
        ui->setButton->setEnabled(false);
    });
    
    connect(ui->minutesComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](){
        isGetButtonPressed = false;
        ui->setButton->setEnabled(false);
    });
    
    connect(ui->secondsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](){
        isGetButtonPressed = false;
        ui->setButton->setEnabled(false);
    });


}

QString MainWindow::getMessageBoxStyle()
{
    return QString(
        "QMessageBox {"
        "    background-color: #f3f3f3;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font: 400 9pt 'Segoe UI Variable', 'Segoe UI';"
        "}"
        "QMessageBox QLabel {"
        "    color: #1c1c1c;"
        "    font: 400 10pt 'Segoe UI Variable', 'Segoe UI';"
        "    background: transparent;"
        "}"
        "QMessageBox QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                stop:0 #0067c0, stop:1 #005a9e);"
        "    border: none;"
        "    border-radius: 4px;"
        "    color: #ffffff;"
        "    font: 500 9pt 'Segoe UI Variable', 'Segoe UI';"
        "    min-width: 64px;"
        "    min-height: 32px;"
        "}"
        "QMessageBox QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                stop:0 #106ebe, stop:1 #0067c0);"
        "}"
        "QMessageBox QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                stop:0 #005a9e, stop:1 #004578);"
        "}"
        "QMessageBox QPushButton[text='Cancel'] {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                stop:0 #fafafa, stop:1 #e5e5e5);"
        "    border: 1px solid #d1d1d1;"
        "    color: #1c1c1c;"
        "}"
        "QMessageBox QPushButton[text='Cancel']:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                stop:0 #f0f0f0, stop:1 #d8d8d8);"
        "    border: 1px solid #c7c7c7;"
        "}"
        "QMessageBox QPushButton[text='Cancel']:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                stop:0 #e0e0e0, stop:1 #c0c0c0);"
        "    border: 1px solid #b3b3b3;"
        "}"
    );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::populateSerialPorts()
{
    ui->portComboBox->clear();
    ui->portComboBox->addItem("Select Port");
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        ui->portComboBox->addItem(port.portName());
    }
}


// ---------- 슬롯 구현 ----------

void MainWindow::on_getButton_clicked()
{
    int speed = ui->speedSpinBox->value();  // speedSpinBox에서 값 가져오기
    int value;
    QString settingText;
    logInfo("설정값 불러오기");

    if (currentMode == MotorMode::ROTATION) {
        value = ui->rotationSpinBox->value();
        logInfo(QString("회전수 모드: RPM=%1, 회전수=%2").arg(speed).arg(value));
    } else if (currentMode == MotorMode::TIME) {
        int hours = ui->hoursComboBox->currentText().toInt();
        int minutes = ui->minutesComboBox->currentText().toInt();
        int seconds = ui->secondsComboBox->currentText().toInt();
        value = getTotalSeconds();
        logInfo(QString("시간 모드: %1시 %2분 %3초 = 총 %4초").arg(hours).arg(minutes).arg(seconds).arg(value));
    }
    
    isSettingConfirmed = false;
    isGetButtonPressed = true;
    
    // GET 버튼을 누르면 SET 버튼 활성화
    ui->setButton->setEnabled(true);
}


void MainWindow::on_setButton_clicked()
{
    // GET 버튼을 먼저 누르지 않았다면 SET 불가
    if (!isGetButtonPressed) {
        logError("먼저 [GET] 버튼을 눌러 설정값을 불러오세요");
        return;
    }
    
    confirmedSpeed = ui->speedSpinBox->value();  // speedSpinBox에서 값 가져오기
    
    if (currentMode == MotorMode::ROTATION) {
        confirmedValue = ui->rotationSpinBox->value();
    } else if (currentMode == MotorMode::TIME) {
        confirmedValue = getTotalSeconds();
    }
    
    isSettingConfirmed = true;

    QString direction = ui->cwModeRadio->isChecked() ? "CW" : "CCW";
    logStatus("설정 확인 완료", QString("방향: %1, GO버튼 클릭시 시작").arg(direction));
}


void MainWindow::log(const QString &message)
{
    ui->textEditConnect->setPlainText(message);
}
void MainWindow::on_goButton_clicked()
{
    if (!isSettingConfirmed) {
        logError("SET 버튼을 누르세요");
        return;
    }

    if (!motorControl.isValidInput(confirmedSpeed, confirmedValue)) {
        logError("유효하지 않은 설정값입니다");
        return;
    }

    // Get motor direction from radio buttons
    MotorDirection direction = ui->cwModeRadio->isChecked() ? MotorDirection::CW : MotorDirection::CCW;
    
    QString command = motorControl.buildCommand(confirmedSpeed, confirmedValue, direction);
    serialHandler->sendCommand(command);
    logCommand(command, "GO 버튼으로 전송");
    logStatus("모터 구동 시작");
    
    // 새로운 GO 시작 시 그래프 완전 초기화
    clearAllGraphData();
    graphStartTime = QDateTime::currentSecsSinceEpoch();
    // currentMotorLoad는 이전 값 유지 (clearAllGraphData에서 초기화하지 않음)
    completionDialogShown = false;  // 완료 대화상자 플래그 초기화
    
    // 모드별 설정
    if (currentMode == MotorMode::TIME) {
        totalTimeSeconds = confirmedValue;
        elapsedTimeSeconds = 0;
        timeUpdateTimer->start();  // 시간 업데이트 타이머 시작
        
        // 시간 설정 로그
        ui->textEditInputLog->appendPlainText(QString("⏰ 시간 설정: 총 %1초 (목표 시간)").arg(totalTimeSeconds));
        
        // 시간 모드 그래프 설정
        ui->motorLoadGraphWidget->setMotorMode("시간 모드");
        ui->motorLoadGraphWidget->setMotorSpeed(confirmedSpeed);
    } else if (currentMode == MotorMode::ROTATION) {
        // 회전 모드에서 그래프 초기화
        targetRotationCount = confirmedValue;
        currentRotationCount = 0;
        
        // 회전 모드 그래프 설정
        ui->motorLoadGraphWidget->setMotorMode("회전 모드");
        ui->motorLoadGraphWidget->setMotorSpeed(confirmedSpeed);
    }
    
    // 내장 그래프 시작
    ui->motorLoadGraphWidget->startUpdating();
    
#if TEST_MODE_RANDOM_DATA
    // 테스트 모드에서 랜덤 데이터 생성 시작
    testDataTimer->start();
    ui->textEditInputLog->appendPlainText("🧪 [TEST] 랜덤 데이터 생성 모드 시작");
#endif
    
    // 모터 구동 시작 - UI 비활성화
    isMotorRunning = true;
    isMotorPaused = false;  // 새로 시작할 때는 일시정지 상태 해제
    setUIEnabled(false);
    updateMotorStatus("구동중", "#FF4500");  // 밝은 주황색 (OrangeRed)
    updateTimeDisplay();  // 시간 표시 업데이트
    updateRotationDisplay();  // 회전 표시 업데이트

    isSettingConfirmed = false;
    isGetButtonPressed = false;
}

void MainWindow::updateDateTime()
{
    QDateTime current = QDateTime::currentDateTime();

    // 요일 한글 변환
    QMap<int, QString> weekdays;
    weekdays[1] = "MON";
    weekdays[2] = "TUE";
    weekdays[3] = "WED";
    weekdays[4] = "THU";
    weekdays[5] = "FRI";
    weekdays[6] = "SAT";
    weekdays[7] = "SUN";

    QString weekday = weekdays[current.date().dayOfWeek()];

    // 날짜/시간 문자열 생성 (24시간제)
    QString dateStr = QString::asprintf("%04d.%02d.%02d %s",
                                        current.date().year(),
                                        current.date().month(),
                                        current.date().day(),
                                        qPrintable(weekday));
                                        
    QString timeStr = QString::asprintf("%02d:%02d:%02d",
                                        current.time().hour(),  // 24시간제 사용
                                        current.time().minute(),
                                        current.time().second());

    // HTML로 날짜와 시간을 다른 스타일로 표시
    QString dateTimeStr = QString("<span style='color: #666; font-size: 12pt;'>%1</span> "
                                  "<span style='color: #0066CC; font-size: 13pt; font-weight: bold;'>%2</span>")
                                  .arg(dateStr).arg(timeStr);

    ui->dateTimeLabel->setText(dateTimeStr);
}

void MainWindow::updateTimeProgress()
{
    // 시간 모드에서 타이머로 시간을 직접 관리
    if (currentMode == MotorMode::TIME && isMotorRunning && !isMotorPaused) {
        // 경과 시간 1초 증가
        elapsedTimeSeconds++;

        // 목표 시간 도달 확인
        if (elapsedTimeSeconds >= totalTimeSeconds) {
            elapsedTimeSeconds = totalTimeSeconds;
            timeUpdateTimer->stop();
            
            // 모터에 정지 신호 전송
            serialHandler->sendCommand("STOP");
            logStatus("설정 시간 완료", "모터 자동 정지");
            
            // 상태 변경
            isMotorRunning = false;
            updateMotorStatus("완료", "blue");
            
            // 완료 대화상자 표시
            showTimeCompletionDialog();
        }
        
        // UI 업데이트
        updateTimeDisplay();
    }
}

#if TEST_MODE_RANDOM_DATA
void MainWindow::generateTestData()
{
    // 모터가 구동 중일 때 테스트 데이터 생성 (모든 모드)
    if (isMotorRunning && !isMotorPaused) {
        // 랜덤 모터 부하량 생성 (30~90% 범위)
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<double> dis(30.0, 90.0);
        
        double randomLoad = dis(gen);
        currentMotorLoad = randomLoad;
        
        // TEST 모드에서도 LOAD 메시지 시뮬레이션하여 그래프 업데이트
        updateMotorLoadGraph();
        
        // 회전수 증가 (가끔씩, 실제 한 바퀴 도는 시간을 시뮬레이션)
        static int rotationCounter = 0;
        rotationCounter++;
        if (rotationCounter >= 3) { // 3초마다 한 바퀴 완료로 시뮬레이션
            rotationCounter = 0;
            if (currentRotationCount < targetRotationCount) {
                currentRotationCount++;
                ui->textEditInputLog->appendPlainText(QString("🔄 [TEST] 회전 완료: %1/%2").arg(currentRotationCount).arg(targetRotationCount));
                
                // 목표 회전수 달성 시
                if (currentRotationCount >= targetRotationCount) {
                    testDataTimer->stop();
                    isMotorRunning = false;
                    setUIEnabled(true);
                    updateMotorStatus("완료", "blue");
                    ui->textEditInputLog->appendPlainText("✅ [TEST] 목표 회전수 달성 - 테스트 완료");
                    
                    // 완료 대화상자 표시
                    showRotationCompletionDialog();
                }
            }
        }
        
        // 디스플레이 업데이트 (그래프는 LOAD 메시지에서만 업데이트)
        updateRotationDisplay();
        
        // 진행률 업데이트
        if (targetRotationCount > 0) {
            // 진행률은 새로운 UI에서 updateCircularProgress()가 처리
            updateCircularProgress();
        }
        
        // 테스트 로그 (매 5초마다만 출력)
        static int logCounter = 0;
        if (++logCounter >= 5) {
            logCounter = 0;
            ui->textEditInputLog->appendPlainText(QString("📊 [TEST] 부하량: %1% | 회전: %2/%3")
                .arg(currentMotorLoad, 0, 'f', 1)
                .arg(currentRotationCount)
                .arg(targetRotationCount));
        }
    }
}
#endif

void MainWindow::handlePortComboBoxChanged(const QString &portName)
{
    selectedPortName = portName;
    log("포트 선택됨: " + portName);
    ui->textEditConnect->moveCursor(QTextCursor::End);
}

void MainWindow::on_connectButton_clicked()
{
    if(selectedPortName.isEmpty() || selectedPortName =="Serial Port"){
        log("✅ 포트를 선택하세요.");
        return;
    }
    if(serialHandler->openSerialPort(selectedPortName)){
        log("포트를 열었습니다. 모터 연결 확인 중...");
        serialHandler->sendCommand("HELLO");
        qDebug()<<"전송메세지 : HELLO ";
    }else{
        log("❌ 포트 열기 실패: " + selectedPortName);
    }
    ui->textEditConnect->moveCursor(QTextCursor::End);

}

void MainWindow::on_disconnectButton_clicked()
{
    if(serialHandler->isOpen()){
        serialHandler->closeSerialPort();
        log("✅ 포트 연결이 끊어졌습니다.");
        
        // UI 상태 업데이트
        ui->connectButton->setEnabled(true);
        ui->disconnectButton->setEnabled(false);
        ui->statusLabel->setStyleSheet("QLabel { background-color: gray; border-color: none; }");
        updateMotorStatus("연결 끊김", "#808080");  // 회색
        
        // 모터 동작 중이면 정지
        if(isMotorRunning){
            resetToInitialState();
        }
    }
    ui->textEditConnect->moveCursor(QTextCursor::End);
}



void MainWindow::handleSerialResponse(const QString &data)
{
    QString trimmed = data.trimmed();
    // 수신된 메시지만 출력
    logReceived(trimmed);

    // 여러 줄로 구성된 메시지를 각 줄별로 처리
    QStringList lines = trimmed.split('\n');
    
    for (const QString& line : lines) {
        QString processedLine = line.trimmed();
        if (processedLine.isEmpty()) continue;

        if (motorControl.processResponse(processedLine)) {
            log(" 모터 제어기와 연결되었습니다.");
            serialHandler->sendCommand("HI");
            ui->portComboBox->setEnabled(false);
            ui->connectButton->setEnabled(false);
            ui->disconnectButton->setEnabled(true);
            ui->statusLabel->setStyleSheet("QLabel { background-color: rgb(0,220,0); border:none;}");
            updateMotorStatus("연결됨", "blue");
        }

        // 모든 모드에서 LOAD 메시지 처리
        if (processedLine.startsWith("LOAD:")) {
            // 모터 부하량 업데이트: "LOAD:75.5%" 또는 "LOAD:75.5" 형태
            QString loadStr = processedLine.section(":", 1, 1);
            // % 기호 제거 (있을 경우)
            if (loadStr.endsWith("%")) {
                loadStr.chop(1);
            }
            currentMotorLoad = loadStr.toDouble();
            updateMotorLoadGraph();
        }
        
        // 시간 모드에서 진행률 처리 (참고용, 실제 시간은 타이머로 관리)
        if (currentMode == MotorMode::TIME && processedLine.startsWith("TURN:")) {
            // ESP32 시간 정보는 참고용으로만 사용, 실제 시간은 타이머로 관리
        } else if (currentMode == MotorMode::ROTATION) {
            // 회전 모드에서 TURN 메시지 처리
            if (processedLine.startsWith("TURN:")) {
                // 회전수 업데이트: "TURN:5" 형태
                currentRotationCount = processedLine.section(":", 1, 1).toInt();
                updateRotationDisplay();
                // 진행률은 새로운 UI에서 updateCircularProgress()가 처리
                updateCircularProgress();
            }
        }
        
        // 모터 완료 또는 정지 시 UI 재활성화
        if (processedLine == "DONE") {
            isMotorRunning = false;
            isMotorPaused = false;  // 완료 시 일시정지 상태 해제
            logStatus("모터 구동 완료", "DONE 신호 수신");
            timeUpdateTimer->stop();  // 시간 업데이트 타이머 정지
#if TEST_MODE_RANDOM_DATA
            testDataTimer->stop();   // 테스트 타이머 정지
#endif
            if (currentMode == MotorMode::TIME) {
                elapsedTimeSeconds = totalTimeSeconds;  // 완료 시 시간을 최대값으로 설정
                updateTimeDisplay();
            }
            setUIEnabled(true);
            updateMotorStatus("완료", "blue");
            
            // 그래프 데이터 보존
            if (ui->motorLoadGraphWidget) {
                ui->motorLoadGraphWidget->preserveGraph();
            }
            
            // 완료 대화상자 표시
            if (currentMode == MotorMode::TIME) {
                showTimeCompletionDialog();
            } else if (currentMode == MotorMode::ROTATION) {
                showRotationCompletionDialog();
            }
        } else if (processedLine == "STOPPED") {
            isMotorRunning = false;
            isMotorPaused = true;  // 일시정지 상태로 설정
            logStatus("모터 일시정지", "STOPPED 신호 수신");
            timeUpdateTimer->stop();  // 시간 업데이트 타이머 정지
#if TEST_MODE_RANDOM_DATA
            testDataTimer->stop();   // 테스트 타이머 정지
#endif
            updateMotorStatus("일시정지", "#FFA500");  // 주황색
            
            // 그래프 데이터 보존
            if (ui->motorLoadGraphWidget) {
                ui->motorLoadGraphWidget->preserveGraph();
            }
            
            setPausedUIState();  // 일시정지 UI 상태로 변경
        }
    }
}


void MainWindow::on_rotationModeRadio_toggled(bool checked)
{
    if (checked) {
        currentMode = MotorMode::ROTATION;
        motorControl.setCommandStrategy(MotorCommandFactory::createCommand(currentMode));
        updateUIForMode(currentMode);
        // 모드 변경 시 GET 상태 초기화
        isGetButtonPressed = false;
        ui->setButton->setEnabled(false);
        updateTimeDisplay();  // 시간 표시 업데이트
        updateRotationDisplay();  // 회전 표시 업데이트
        updateModeVisibility();  // 모드별 UI 표시 업데이트
        updateCircularProgress();  // 원형 진행률 업데이트
    }
}

void MainWindow::on_timeModeRadio_toggled(bool checked)
{
    if (checked) {
        currentMode = MotorMode::TIME;
        motorControl.setCommandStrategy(MotorCommandFactory::createCommand(currentMode));
        updateUIForMode(currentMode);
        // 모드 변경 시 GET 상태 초기화
        isGetButtonPressed = false;
        ui->setButton->setEnabled(false);
        
        // 시간 모드 전환 시 디버깅 로그
        ui->textEditInputLog->appendPlainText(QString("🔄 시간 모드로 전환: 총=%1초, 경과=%2초")
                                             .arg(totalTimeSeconds).arg(elapsedTimeSeconds));
        
        updateTimeDisplay();  // 시간 표시 업데이트
        updateRotationDisplay();  // 회전 표시 업데이트
        updateModeVisibility();  // 모드별 UI 표시 업데이트
        updateCircularProgress();  // 원형 진행률 업데이트
    }
}

void MainWindow::updateUIForMode(MotorMode mode)
{
    if (mode == MotorMode::ROTATION) {
        ui->labelRotation->setText("Rotation:");
        ui->rotationSpinBox->setSuffix(" 회전");
        ui->rotationSpinBox->setMaximum(9999);
        ui->rotationSpinBox->setValue(1);
        ui->rotationSpinBox->setVisible(true);
        
        // 시간 콤보박스들 숨기기
        ui->hoursComboBox->setVisible(false);
        ui->minutesComboBox->setVisible(false);
        ui->secondsComboBox->setVisible(false);
        ui->timeLabel1->setVisible(false);
        ui->timeLabel2->setVisible(false);
        ui->timeUnitLabel->setVisible(false);
    } else if (mode == MotorMode::TIME) {
        ui->labelRotation->setText("Run time:");
        ui->rotationSpinBox->setVisible(false);
        
        // 시간 콤보박스들 보이기
        ui->hoursComboBox->setVisible(true);
        ui->minutesComboBox->setVisible(true);
        ui->secondsComboBox->setVisible(true);
        ui->timeLabel1->setVisible(true);
        ui->timeLabel2->setVisible(true);
        ui->timeUnitLabel->setVisible(true);
    }
}

void MainWindow::setUIEnabled(bool enabled)
{
    // 모터 구동 중에는 설정 변경 불가
    ui->speedSlider->setEnabled(enabled);
    ui->rotationSpinBox->setEnabled(enabled);
    ui->rotationModeRadio->setEnabled(enabled);
    ui->timeModeRadio->setEnabled(enabled);
    ui->goButton->setEnabled(enabled);
    ui->getButton->setEnabled(enabled);
    
    // 시간 콤보박스들도 포함
    ui->hoursComboBox->setEnabled(enabled);
    ui->minutesComboBox->setEnabled(enabled);
    ui->secondsComboBox->setEnabled(enabled);
    
    // SET 버튼은 GET 버튼을 누른 후에만 활성화
    if (enabled) {
        ui->setButton->setEnabled(isGetButtonPressed);
    } else {
        ui->setButton->setEnabled(false);
    }
    
    // STOP 버튼은 모터 구동 중에만 활성화
    ui->stopButton->setEnabled(!enabled && isMotorRunning);
}

void MainWindow::setPausedUIState()
{
    // 일시정지 상태에서는 모든 설정 입력 비활성화
    ui->speedSlider->setEnabled(false);
    ui->rotationSpinBox->setEnabled(false);
    ui->rotationModeRadio->setEnabled(false);
    ui->timeModeRadio->setEnabled(false);
    ui->hoursComboBox->setEnabled(false);
    ui->minutesComboBox->setEnabled(false);
    ui->secondsComboBox->setEnabled(false);
    
    // GET, SET, GO 버튼 비활성화
    ui->getButton->setEnabled(false);
    ui->setButton->setEnabled(false);
    ui->goButton->setEnabled(false);
    
    // STOP 버튼 비활성화 (이미 일시정지됨)
    ui->stopButton->setEnabled(false);
    
    // CLOSE, RELOAD 버튼만 활성화
    ui->closeButton->setEnabled(true);
    ui->reloadButton->setEnabled(true);
}

void MainWindow::on_stopButton_clicked()
{
    // 바로 일시정지 신호 전송 (경고창 없음)
    serialHandler->sendCommand("STOP");
    logCommand("STOP", "일시정지 요청");
    
    // 일시정지 상태로 변경
    isMotorPaused = true;
    isMotorRunning = false;
    updateMotorStatus("일시정지", "#FFA500");  // 주황색
    // DEBUG 로그 제거 (깔끔한 로그를 위해)
        
#if TEST_MODE_RANDOM_DATA
    testDataTimer->stop();   // 테스트 타이머 정지
#endif
        
    // 일시정지 상태에서는 설정값 변경 불가, close/reload만 가능
    setPausedUIState();
}

void MainWindow::initializeTimeComboBoxes()
{
    // 시간 콤보박스 (0-23)
    ui->hoursComboBox->clear();
    for (int i = 0; i < 24; i++) {
        ui->hoursComboBox->addItem(QString::number(i));
    }
    ui->hoursComboBox->setCurrentIndex(0);
    
    // 분 콤보박스 (0-59)
    ui->minutesComboBox->clear();
    for (int i = 0; i < 60; i++) {
        ui->minutesComboBox->addItem(QString("%1").arg(i, 2, 10, QChar('0')));
    }
    ui->minutesComboBox->setCurrentIndex(0);
    
    // 초 콤보박스 (0-59)
    ui->secondsComboBox->clear();
    for (int i = 0; i < 60; i++) {
        ui->secondsComboBox->addItem(QString("%1").arg(i, 2, 10, QChar('0')));
    }
    ui->secondsComboBox->setCurrentIndex(10); // 기본값 10초
}

int MainWindow::getTotalSeconds() const
{
    int hours = ui->hoursComboBox->currentText().toInt();
    int minutes = ui->minutesComboBox->currentText().toInt();
    int seconds = ui->secondsComboBox->currentText().toInt();
    
    return hours * 3600 + minutes * 60 + seconds;
}

void MainWindow::updateMotorStatus(const QString &status, const QString &color)
{
    if (status == "구동중") {
        // 구동 중일 때는 더 큰 테두리
        ui->motorStatusLED->setStyleSheet(QString("QLabel { background-color: %1; border: 3px solid #FF0000; border-radius: 12px;}").arg(color));
    } else if (status == "일시정지") {
        // 일시정지 상태일 때는 밝은 주황색 테두리
        ui->motorStatusLED->setStyleSheet(QString("QLabel { background-color: %1; border: 2px solid #FFA500; border-radius: 12px;}").arg(color));
    } else {
        // 일반적인 스타일 (연결됨, 완료 등)
        ui->motorStatusLED->setStyleSheet(QString("QLabel { background-color: %1; border: 2px solid #666; border-radius: 12px;}").arg(color));
    }
    ui->motorStatusText->setText(status);
}

void MainWindow::updateTimeDisplay()
{
    if (currentMode == MotorMode::TIME) {
        // 시간 모드에서만 시간 표시
        ui->timeProgressFrame->setVisible(true);
        
        // 시간 표시는 updateTimeProgressDisplay()에서 처리하므로 중복 호출 제거
        // 단일 진입점으로 updateCircularProgress()만 호출
        updateCircularProgress();
    } else {
        // 회전 모드에서는 시간 표시 숨기기 (새로운 UI는 updateModeVisibility()에서 처리)
    }
}

void MainWindow::updateRotationDisplay()
{
    if (currentMode == MotorMode::ROTATION) {
        // 회전 모드에서만 회전수와 부하량 표시
        ui->rotationProgressFrame->setVisible(true);
        
        
        // 새로운 UI 업데이트 호출
        updateRotationProgress();
    } else {
        // 시간 모드에서는 회전 관련 UI 처리 (새로운 UI는 updateModeVisibility()에서 처리)
    }
}

void MainWindow::updateMotorLoadGraph()
{
    if (isMotorRunning && !isMotorPaused) {
        // 상대적인 시간 (초) 계산 - 그래프 시작부터 경과된 시간
        double currentTime = 0.0;
        if (graphStartTime > 0) {
            currentTime = (QDateTime::currentSecsSinceEpoch() - graphStartTime);
        }
        
        // 그래프에 데이터 추가
        if (ui->motorLoadGraphWidget) {
            ui->motorLoadGraphWidget->addDataPoint(currentTime, currentMotorLoad);
        }
    }
}


void MainWindow::showTimeCompletionDialog()
{
    // 이미 대화상자가 표시되었으면 무시
    if (completionDialogShown) {
        return;
    }
    completionDialogShown = true;
    
    // 완료 시간을 시:분:초 형식으로 변환
    int hours = totalTimeSeconds / 3600;
    int minutes = (totalTimeSeconds % 3600) / 60;
    int seconds = totalTimeSeconds % 60;
    
    QString timeStr;
    if (hours > 0) {
        timeStr = QString("%1시간 %2분 %3초").arg(hours).arg(minutes).arg(seconds);
    } else if (minutes > 0) {
        timeStr = QString("%1분 %2초").arg(minutes).arg(seconds);
    } else {
        timeStr = QString("%1초").arg(seconds);
    }
    
    // 완료 확인 대화상자
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("모터 구동 완료");
    msgBox.setText(QString("%1간 구동이 완료되었습니다.\n종료하시겠습니까?").arg(timeStr));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Information);
    
    // Windows 11 스타일 적용
    msgBox.setStyleSheet(getMessageBoxStyle());
    
    int reply = msgBox.exec();
    
    if (reply == QMessageBox::Ok) {
        // 모든 상태 초기화하여 처음으로 돌아가기
        resetToInitialState();
    }
    // Cancel을 누르면 완료 상태 유지
}

void MainWindow::showRotationCompletionDialog()
{
    // 완료된 회전수 표시
    QString rotationStr;
    if (targetRotationCount == 1) {
        rotationStr = "1회전";
    } else {
        rotationStr = QString("%1회전").arg(targetRotationCount);
    }
    
    // 완료 확인 대화상자
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("모터 구동 완료");
    msgBox.setText(QString("%1 구동이 완료되었습니다.\n종료하시겠습니까?").arg(rotationStr));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Information);
    
    // Windows 11 스타일 적용
    msgBox.setStyleSheet(getMessageBoxStyle());
    
    int reply = msgBox.exec();
    
    if (reply == QMessageBox::Ok) {
        // 모든 상태 초기화하여 처음으로 돌아가기
        resetToInitialState();
    }
    // Cancel을 누르면 완료 상태 유지
}

void MainWindow::resetToInitialState()
{
    // 모든 상태 변수 초기화
    isMotorRunning = false;
    isMotorPaused = false;
    isSettingConfirmed = false;
    isGetButtonPressed = false;
    totalTimeSeconds = 0;
    elapsedTimeSeconds = 0;
    currentRotationCount = 0;
    targetRotationCount = 0;
    // currentMotorLoad는 초기화하지 않음 (이전 값 유지)
    graphStartTime = 0;
    completionDialogShown = false;
    
    // 타이머 정지
    timeUpdateTimer->stop();
#if TEST_MODE_RANDOM_DATA
    testDataTimer->stop();
#endif
    
    // 모터 상태 초기화
    motorControl.reset();
    // 진행률 초기화는 새로운 UI에서 처리
    updateCircularProgress();
    
    // 그래프는 보존하고 UI만 활성화
    if (ui->motorLoadGraphWidget) {
        ui->motorLoadGraphWidget->preserveGraph(); // 데이터 확실히 보존
    }
    
    // UI 완전 활성화
    setUIEnabled(true);
    updateMotorStatus("정지됨", "gray");
    updateTimeDisplay();
    updateRotationDisplay();
    
    // 로그 메시지
    logStatus("초기화 완료", "새로운 작업 시작 가능");
}


void MainWindow::on_closeButton_clicked()
{
    // DEBUG 로그 제거
    if (!isMotorPaused) {
        logError("일시정지 상태에서만 완전 종료가 가능합니다");
        return;
    }
    
    // 완전 종료 확인 대화상자
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("모터 완전 종료 확인");
    msgBox.setText("현재 작업을 완전히 종료하시겠습니까?\n모든 설정값이 초기화됩니다.");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.setIcon(QMessageBox::Warning);
    
    int reply = msgBox.exec();
    
    if (reply == QMessageBox::Ok) {
        // 완전 종료 신호 전송 (필요시)
        serialHandler->sendCommand("CLOSE");
        logCommand("CLOSE", "모터 작업 완전 종료");
        
        // 모든 상태 초기화
        isMotorRunning = false;
        isMotorPaused = false;
        isSettingConfirmed = false;
        isGetButtonPressed = false;
        timeUpdateTimer->stop();  // 타이머 정지
        
        // 모터 상태 초기화
        motorControl.reset();
        
        // frameOutput 모든 값 초기화
        resetFrameOutputValues();
        
        // UI 완전 활성화
        setUIEnabled(true);
        updateMotorStatus("정지됨", "gray");
        
        logStatus("초기화 완료", "새로운 작업 시작 가능");
    }
}


void MainWindow::on_reloadButton_clicked()
{
    // DEBUG 로그 제거
    if (!isMotorPaused) {
        logError("일시정지 상태에서만 재개가 가능합니다");
        return;
    }
    
    // 바로 재개 신호 전송 (경고창 없음)
    serialHandler->sendCommand("RELOAD");
    logCommand("RELOAD", "작업 재개 요청");
    
    // 구동 상태로 복원
    isMotorRunning = true;
    isMotorPaused = false;
    
    // 시간 모드에서 타이머 재시작
    if (currentMode == MotorMode::TIME) {
        timeUpdateTimer->start();
    }
        
#if TEST_MODE_RANDOM_DATA
    // 회전 모드에서 테스트 타이머 재시작
    if (currentMode == MotorMode::ROTATION) {
        testDataTimer->start();
    }
#endif
        
    // UI를 구동 중 상태로 변경
    setUIEnabled(false);
    updateMotorStatus("구동중", "#FF4500");  // 밝은 주황색
}

void MainWindow::on_infoButton_clicked()
{
    QMessageBox infoBox(this);
    infoBox.setWindowTitle("프로그램 정보");
    infoBox.setIcon(QMessageBox::Information);
    
    // 여기에 정보를 입력해주세요
    QString infoText = "StepperRT - 스테퍼 모터 제어 프로그램\n\n"
                       "개발자: FREKERSPACE\n"
                       "이메일: freker@xxx.com\n"
                       "제작일: 2025년\n"
                       "이 프로그램은 ESP32를 통해 Nema23 스테퍼 모터를\n"
                       "제어하기 위한 GUI 애플리케이션입니다.";
    
    infoBox.setText(infoText);
    infoBox.setStandardButtons(QMessageBox::Ok);
    
    // 스타일 적용
    infoBox.setStyleSheet(
        "QMessageBox {"
        "    background-color: white;"
        "}"
        "QMessageBox QLabel {"
        "    color: #333;"
        "    font: 10pt 'Segoe UI';"
        "    min-width: 300px;"
        "}"
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    padding: 6px 20px;"
        "    font: bold 10pt;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
    );
    
    infoBox.exec();
}

// 새로운 UI 업데이트 메서드들 구현
void MainWindow::updateCircularProgress()
{
    // 현재 모드에 따라 해당하는 원형 진행률만 업데이트
    if (currentMode == MotorMode::ROTATION) {
        updateRotationProgress();
    } else if (currentMode == MotorMode::TIME) {
        updateTimeProgressDisplay();
    }
    updateLoadProgress();
}

void MainWindow::updateRotationProgress()
{
    if (currentMode != MotorMode::ROTATION) return;
    
    int progress = 0;
    if (targetRotationCount > 0) {
        progress = (currentRotationCount * 100) / targetRotationCount;
    }
    
    // 원형 진행률 그리기
    QWidget* circularWidget = ui->circularProgressWidget;
    if (circularWidget) {
        drawCircularProgress(circularWidget, progress, QColor(78, 157, 235));
    }
    
    // 텍스트 업데이트
    ui->rotationPercentLabel->setText(QString("%1%").arg(progress));
    ui->rotationCountDisplay->setText(QString("%1 / %2 회전").arg(currentRotationCount).arg(targetRotationCount));
    ui->rotationSpeedDisplay->setText(QString("%1 RPM").arg(confirmedSpeed));
    ui->rotationLinearProgress->setValue(progress);
}

void MainWindow::updateTimeProgressDisplay()
{
    if (currentMode != MotorMode::TIME) {
        // DEBUG 로그 제거
        return;
    }
    
    // 안전한 범위 확인
    if (elapsedTimeSeconds < 0) elapsedTimeSeconds = 0;
    if (elapsedTimeSeconds > totalTimeSeconds) elapsedTimeSeconds = totalTimeSeconds;
    
    // 진행률 계산
    int progress = 0;
    if (totalTimeSeconds > 0) {
        progress = (elapsedTimeSeconds * 100) / totalTimeSeconds;
        progress = qBound(0, progress, 100); // 0-100% 범위 제한
    }
    
    // 남은 시간 계산 (음수 방지)
    int remainingSeconds = qMax(0, totalTimeSeconds - elapsedTimeSeconds);
    int remainHours = remainingSeconds / 3600;
    int remainMinutes = (remainingSeconds % 3600) / 60;
    int remainSecs = remainingSeconds % 60;
    
    // 경과 시간 계산
    int elapsedHours = elapsedTimeSeconds / 3600;
    int elapsedMinutes = (elapsedTimeSeconds % 3600) / 60;
    int elapsedSecs = elapsedTimeSeconds % 60;
    

    
    // 원형 진행률 그리기
    QWidget* circularWidget = ui->timeCircularProgressWidget;
    if (circularWidget) {
        drawCircularProgress(circularWidget, progress, QColor(0, 85, 255));
    }
    
    // UI 텍스트 업데이트
    ui->timePercentLabel->setText(QString("%1%").arg(progress));
    ui->timeRemainingDisplay->setText(QString("%1:%2:%3")
                                       .arg(remainHours, 2, 10, QChar('0'))
                                       .arg(remainMinutes, 2, 10, QChar('0'))
                                       .arg(remainSecs, 2, 10, QChar('0')));
    ui->elapsedTimeLabel->setText(QString("경과: %1:%2:%3")
                                   .arg(elapsedHours, 2, 10, QChar('0'))
                                   .arg(elapsedMinutes, 2, 10, QChar('0'))
                                   .arg(elapsedSecs, 2, 10, QChar('0')));
    ui->timeLinearProgress->setValue(progress);
}

void MainWindow::updateLoadProgress()
{
    // 부하량 진행률 UI 업데이트만 처리 (그래프 데이터 추가는 updateMotorLoadGraph에서)
    // 필요시 여기에 부하량 관련 UI 업데이트 코드 추가 가능
}

void MainWindow::drawCircularProgress(QWidget* widget, int percentage, QColor color)
{
    if (!widget) return;
    
    QPixmap pixmap(widget->size());
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    int side = qMin(widget->width(), widget->height()) - 4;
    QRect rect((widget->width() - side) / 2, (widget->height() - side) / 2, side, side);
    
    // 배경 원
    painter.setPen(QPen(QColor(220, 220, 220), 3));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(rect);
    
    // 진행률 원호
    if (percentage > 0) {
        painter.setPen(QPen(color, 3, Qt::SolidLine, Qt::RoundCap));
        int startAngle = 90 * 16; // 12시 방향부터 시작
        int spanAngle = -(percentage * 360 * 16) / 100; // 시계방향
        painter.drawArc(rect, startAngle, spanAngle);
    }
    
    // 기존 라벨 찾거나 새로 생성
    QLabel* backgroundLabel = widget->findChild<QLabel*>("backgroundLabel");
    if (!backgroundLabel) {
        backgroundLabel = new QLabel(widget);
        backgroundLabel->setObjectName("backgroundLabel");
        backgroundLabel->setGeometry(0, 0, widget->width(), widget->height());
        backgroundLabel->lower(); // 다른 위젯들 뒤로 보내기
        backgroundLabel->show();
    }
    
    // 픽스맵 업데이트
    backgroundLabel->setPixmap(pixmap);
}


void MainWindow::updateModeVisibility()
{
    // 현재 모드에 따라 프레임들의 투명도 조정
    if (currentMode == MotorMode::ROTATION) {
        ui->rotationProgressFrame->setStyleSheet(
            "QFrame { border: 2px solid rgb(78, 157, 235); border-radius: 8px; background-color:none; }");
        ui->timeProgressFrame->setStyleSheet(
            "QFrame { border: 1px solid rgb(200, 200, 200); border-radius: 8px; background-color:none; }");
    } else {
        ui->rotationProgressFrame->setStyleSheet(
            "QFrame { border: 1px solid rgb(200, 200, 200); border-radius: 8px; background-color:none; }");
        ui->timeProgressFrame->setStyleSheet(
            "QFrame { border: 2px solid rgb(0, 85, 255); border-radius: 8px; background-color:none; }");
    }
}

void MainWindow::resetFrameOutputValues()
{
    // 회전 모드 진행률 초기화
    ui->rotationPercentLabel->setText("0%");
    ui->rotationCountDisplay->setText("0 / 0 회전");
    ui->rotationSpeedDisplay->setText("0 RPM");
    ui->rotationLinearProgress->setValue(0);
    
    // 시간 모드 진행률 초기화
    ui->timePercentLabel->setText("0%");
    ui->timeRemainingDisplay->setText("00:00:00");
    ui->elapsedTimeLabel->setText("경과: 00:00:00");
    ui->timeLinearProgress->setValue(0);
    
    // 초기화 로그
    ui->textEditInputLog->appendPlainText("🔄 시간 모드 UI 초기화 완료");
    
    // 원형 진행률 표시기들 초기화 (투명한 상태로)
    if (ui->circularProgressWidget) {
        drawCircularProgress(ui->circularProgressWidget, 0, QColor(78, 157, 235));
    }
    if (ui->timeCircularProgressWidget) {
        drawCircularProgress(ui->timeCircularProgressWidget, 0, QColor(0, 85, 255));
    }
    
    // 모터 로드 그래프는 유지 (그래프 데이터 보존)
    if (ui->motorLoadGraphWidget) {
        ui->motorLoadGraphWidget->stopUpdating();
    }
    
    // 내부 상태 변수들도 초기화
    currentRotationCount = 0;
    targetRotationCount = 0;
    totalTimeSeconds = 0;
    elapsedTimeSeconds = 0;
    // currentMotorLoad는 초기화하지 않음 (이전 값 유지)
    
    // 그래프 데이터 초기화
    graphStartTime = 0;
    
    ui->textEditInputLog->appendPlainText("🔄 출력 데이터가 모두 초기화되었습니다.");
}

void MainWindow::clearAllGraphData()
{
    // 그래프 데이터 완전 초기화 (새로운 GO 시작 시에만 호출)
    if (ui->motorLoadGraphWidget) {
        ui->motorLoadGraphWidget->clearData();
        ui->motorLoadGraphWidget->stopUpdating();
    }
    
    // currentMotorLoad는 초기화하지 않음 (이전 값 유지)
    graphStartTime = 0;
}

// 통일된 로그 출력 함수들
void MainWindow::logCommand(const QString &command, const QString &details)
{
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    QString message = QString("<span style='color: #888;'>[%1]</span> %2").arg(timestamp).arg(command);
    if (!details.isEmpty()) {
        message += QString(" (%1)").arg(details);
    }
    ui->textEditInputLog->appendHtml(message);
}

void MainWindow::logReceived(const QString &data)
{
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    QString message = QString("<span style='color: #888;'>[%1]</span> 수신: %2").arg(timestamp).arg(data);
    ui->textEditInputLog->appendHtml(message);
}

void MainWindow::logStatus(const QString &status, const QString &details)
{
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    QString icon = "ℹ️";
    if (status.contains("완료") || status.contains("성공")) icon = "✅";
    else if (status.contains("정지") || status.contains("일시정지")) icon = "⏸️";
    else if (status.contains("구동") || status.contains("시작")) icon = "▶️";
    else if (status.contains("재개")) icon = "🔄";
    
    QString message = QString("<span style='color: #888;'>[%1]</span> %2 %3").arg(timestamp).arg(icon).arg(status);
    if (!details.isEmpty()) {
        message += QString(" - %1").arg(details);
    }
    ui->textEditInputLog->appendHtml(message);
}

void MainWindow::logError(const QString &error)
{
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    QString message = QString("<span style='color: #888;'>[%1]</span> ❌ ERROR: %2").arg(timestamp).arg(error);
    ui->textEditInputLog->appendHtml(message);
}

void MainWindow::logInfo(const QString &info)
{
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    QString message = QString("<span style='color: #888;'>[%1]</span> 💡  %2").arg(timestamp).arg(info);
    ui->textEditInputLog->appendHtml(message);
}

