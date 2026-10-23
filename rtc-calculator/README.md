# ATmega128A RTC·LCD 계산기

DS1307 RTC의 시간·날짜를 LCD1602에 표시하고, 버튼과 4×4 키패드로 설정 및 계산기 기능을 사용하는 임베디드 시스템입니다. 숫자·연산자 스택으로 사칙연산을 처리하고 키패드 입력에 부저음을 연결했습니다.

- **개발 언어:** C
- **MCU / 클록:** ATmega128A / 16MHz
- **개발 환경:** Atmel Studio 7, AVR GCC
- **팀원:** 김선호, 곽혜민 — 발표자료 표기 기준
- **발표자료:** [RTC/LCD/계산기 통합 시스템 구현](docs/presentation.pptx)
- **전체 프로젝트 목록:** [AVR Projects](../README.md)

이 문서는 팀 프로젝트의 전체 구현을 설명합니다. 개인별 세부 담당 범위는 현재 자료만으로 확정하지 않았습니다.

## 주요 기능

| 기능 | 구현 내용 |
| --- | --- |
| 시계·날짜 표시 | I²C로 DS1307 레지스터를 읽고 BCD 값을 변환하여 LCD에 출력 |
| 시간·날짜 설정 | 별도 모드에서 키패드로 입력하고 RTC에 반영 |
| LCD 출력 | LCD1602를 4비트 방식으로 초기화하고 명령·문자 전송 |
| 계산기 | 숫자·연산자 스택으로 사칙연산과 우선순위 처리, 괄호 입력 |
| 입력 수정 | 버튼으로 전체 지우기, 한 글자 지우기 |
| 자동 복귀 | 계산기에서 60초 미입력 시 시계 모드로 복귀 |
| 입력음 | 키패드 입력에 따라 Timer3 출력 비교값 변경 |

## 동작 구조

```text
DS1307 ── I²C ── 시간·날짜 읽기 / 설정 ──┐
                                        ├── LCD1602 표시
키패드·버튼 ── 모드 / 수식 입력 ── 스택 계산
      │
      └── Timer0 입력 검사 / 미입력 시간 관리
                    └── Timer3 부저 출력
```

`main.c`의 `mode` 값으로 시계(0), 달력(1), 시간 설정(2), 날짜 설정(3), 계산기(4)를 구분합니다. 앞의 네 모드는 함수 포인터 배열로 호출하고 계산기는 `lcd_cal()`에서 입력을 처리합니다. Timer0 인터럽트는 코드상 약 20ms마다 키패드를 검사합니다.

## 조작 방법

전원을 켜면 시계 모드로 시작합니다. 버튼의 역할은 현재 모드에 따라 달라집니다.

| 화면 | 입력 | 동작 |
| --- | --- | --- |
| 일반 시계·달력 | BUTTON3 / PD6 | 시계와 달력 전환 |
| 일반 시계·달력 | BUTTON2 / PD5 | 시간 설정 진입 |
| 일반 화면 | BUTTON1 / PD4 | 계산기 진입 |
| 계산기 | 키패드 숫자·`+ - * /` | 수식 입력 |
| 계산기 | 키패드 `=` | 계산 결과 출력 |
| 계산기 | BUTTON0 / PD3 | 입력 전체 지우기 |
| 계산기 | BUTTON1 / PD4 | 한 글자 지우기 |
| 계산기 | BUTTON2 / PD5 | 여는 괄호 입력 |
| 계산기 | BUTTON3 / PD6 | 닫는 괄호 입력 |
| 계산기 | 60초 미입력 | 시계로 복귀 |

시간·날짜 설정의 세부 입력 처리는 `ds1307.c`의 `change_clock_mode()`와 `change_calender_mode()`를 참조합니다. 계산 결과는 소수점 두 자리로 표시하지만, 숫자 입력 파서는 정수 입력을 기준으로 작성되어 있습니다.

## 핀 구성

아래는 소스에 정의된 MCU 포트 기준입니다. 실제 보드 커넥터와 배선은 발표자료를 함께 확인합니다.

| 연결 | MCU 포트 |
| --- | --- |
| DS1307 SCL / SDA | PD0 / PD1 |
| LCD 데이터 4비트 | PC4~PC7 |
| LCD RS / RW / E | PA0 / PA1 / PA2 |
| 키패드 출력 / 입력 | PF0~PF3 / PF4~PF7 |
| BUTTON0~4 | PD3~PD7 |
| 부저 | PE3 / OC3A |
| UART0 RX / TX | PE0 / PE1 |

## 소스 구성

| 파일 | 역할 |
| --- | --- |
| `main.c` | 초기화, 모드 관리, Timer0 인터럽트 |
| `ds1307.c/.h` | I²C, RTC 읽기·쓰기, 날짜·시간 설정 및 변환 |
| `lcd.c/.h` | LCD 초기화·출력, 계산기 입력 화면 |
| `calculator.c/.h` | 숫자·연산자 스택과 수식 계산 |
| `keypad.c/.h` | 4×4 키패드 스캔과 입력 판별 |
| `button.c/.h` | 버튼 입력·디바운싱 |
| `buzzer.c` | Timer3 기반 입력음 |
| `uart0.c/.h` | 디버깅 UART |
| `i2c_m_loopback.c/.h` | 원본에 포함된 별도 I²C 실습 코드, 현재 `.cproj` 빌드 대상에서 제외 |

`i2c_m_loopback.c`를 전체 소스와 함께 무조건 빌드하지 않습니다. 프로젝트에 등록된 파일 목록을 기준으로 빌드합니다.

## 빌드 및 실행

### Atmel Studio

1. ATmega128A를 지원하는 AVR 툴체인과 디바이스 팩을 준비합니다.
2. `firmware/13. RTC_LDC_CAL_PROJECT.cproj`를 엽니다.
3. 디바이스 팩 경로, MCU와 보드 클록 설정을 확인하고 빌드합니다.
4. 실제 배선과 프로그래머 설정을 확인한 뒤 보드에 기록합니다.
5. RTC 시계 표시, 모드 변경, 키패드 입력, 기본 사칙연산 순서로 확인합니다.

원본 파일명과 프로젝트 설정을 유지했습니다. 디바이스 팩 경로는 설치 환경에 따라 조정이 필요할 수 있습니다.

### AVR GCC 명령행

`avr-gcc`가 PATH에 있는 환경에서 `firmware`로 이동해 실행합니다.

```sh
avr-gcc -mmcu=atmega128a -std=gnu99 -Os -Wall -Wextra -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums main.c button.c buzzer.c calculator.c ds1307.c keypad.c lcd.c uart0.c -lm -o rtc_calculator.elf
avr-objcopy -O ihex -R .eeprom rtc_calculator.elf rtc_calculator.hex
```