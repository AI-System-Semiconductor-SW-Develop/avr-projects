# AVR Projects

ATmega128A를 활용해 센서·모터 제어와 RTC·LCD·키패드 연동을 구현한 교육과정 팀 프로젝트 모음입니다. 각 프로젝트는 독립된 펌웨어이며 별도로 빌드합니다.

| 프로젝트 | 주요 내용 | 자료 |
| --- | --- | --- |
| 초음파 장애물 회피 RC카 | Bluetooth 수동 조종, 초음파 센서 3개 기반 자동 주행, FND 주행 기록 표시 | [프로젝트 설명](auto-car/README.md) · [소스](auto-car/firmware) · [발표자료](auto-car/docs/presentation.pptx) |
| RTC·LCD 계산기 | DS1307 시계·날짜 표시 및 설정, LCD1602 4비트 제어, 키패드와 스택 기반 계산 | [프로젝트 설명](rtc-calculator/README.md) · [소스](rtc-calculator/firmware) · [발표자료](rtc-calculator/docs/presentation.pptx) |

## 기술 구성

| 구분 | RC카 | RTC·LCD 계산기 |
| --- | --- | --- |
| 공통 환경 | ATmega128A · C · 16MHz · Atmel Studio / AVR GCC | ATmega128A · C · 16MHz · Atmel Studio / AVR GCC |
| 입력 | 초음파 센서 3개, Bluetooth, 버튼 | 4×4 키패드, 버튼, DS1307 |
| 출력 | DC 모터, 4자리 FND 2개 | LCD1602, 부저 |
| 주요 기술 | UART, PWM, 외부 인터럽트, 타이머 | I²C, LCD 4비트 인터페이스, 타이머, 스택 |

개별 README에서 동작 구조, 핀 구성, 소스 역할과 실행 방법을 확인할 수 있습니다. 팀원은 각 발표자료 기준으로 표기했으며, 프로젝트 전체 기능을 개인 기여로 표시하지 않았습니다.

## 저장소 구성

```text
avr-projects/
├── README.md
├── .gitignore
├── .gitattributes
├── auto-car/
│   ├── README.md
│   ├── firmware/       # C 소스·헤더 및 Atmel Studio 프로젝트
│   └── docs/presentation.pptx
└── rtc-calculator/
    ├── README.md
    ├── firmware/
    └── docs/presentation.pptx
```

## 자료 받기

발표 PPT는 원본을 보존하기 위해 Git LFS로 관리합니다. Git LFS가 설치된 환경에서 저장소를 복제하고 다음 명령으로 발표자료를 받습니다.

```sh
git lfs install
git lfs pull
```

소스 빌드에는 PPT가 필요하지 않습니다. 빌드 산출물과 개인 IDE 설정은 저장소에서 제외합니다.


