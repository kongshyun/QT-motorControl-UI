# 🎛️ Nema23 Stepper Motor Controller

![Qt](https://img.shields.io/badge/Qt-5%2F6-green.svg)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)

ESP32를 통해 Nema23 스테퍼 모터를 정밀 제어하는 Qt 기반 GUI 애플리케이션입니다.

## ✨ 주요 기능

- 🎯 **정밀 제어**: 회전수/시간 기반 모터 제어
- 🛡️ **안전 기능**: 구동 중 UI 잠금, 비상 정지 확인
- 📊 **실시간 모니터링**: 진행률과 상태 추적
- 🏗️ **SOLID 아키텍처**: 확장 가능한 모듈러 설계

## 🔧 통신 프로토콜

ESP32와 UART 통신 (115200 baud)을 통해 모터를 제어합니다:

```
연결: HELLO → READY
회전수 모드: RPM:60 ROT:10 → TURN:X → DONE
시간 모드: RPM:60 TIME:30 → TURN:X → DONE  
정지: STOP → STOPPED
```

## 🛠️ 빌드 방법

```bash
# qmake로 Makefile 생성
qmake stepperESP32.pro

# 컴파일
make

# Windows (MinGW)
mingw32-make
```

## 🚀 빠른 시작

```bash
git clone https://github.com/kongshyun/Nema23-Motor-Controller.git
cd Nema23-Motor-Controller/stepperESP32
qmake stepperESP32.pro
make  # 또는 mingw32-make (Windows)
```

### 사용법
1. **연결**: ESP32 포트 선택 → Connect
2. **설정**: 모드 선택 → RPM/값 입력 → SET  
3. **실행**: GO 버튼 클릭
4. **정지**: STOP 버튼 (확인 후)

## 🔧 개발 환경

- **언어**: C++17, Qt 5/6
- **통신**: QSerialPort (115200 baud)  
- **아키텍처**: SOLID 원칙, Strategy/Factory 패턴

## 📄 라이선스

MIT 라이선스 - 자세한 내용은 `LICENSE` 파일 참조

---

⭐ 도움이 되었다면 스타를 눌러주세요!