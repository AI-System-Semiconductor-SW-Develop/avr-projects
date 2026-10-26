# ATmega128A 초음파 장애물 회피 RC카

Bluetooth 명령으로 수동 조종하거나, 전방·좌·우 초음파 센서로 장애물을 감지해 주행 방향을 선택하는 RC카입니다. FND 두 개에 주행시간과 상태를 표시하고, 주행 종료 후 시간·후진·좌회전·우회전 횟수를 확인합니다.

- **개발 언어:** C
- **MCU / 클록:** ATmega128A / 16MHz
- **개발 환경:** Atmel Studio 7, AVR GCC
- **팀원:** 고지현, 곽혜민
- **발표자료:** [AVR 기반 초음파 자율주행 RC카](docs/presentation.pptx)
- **전체 프로젝트 목록:** [AVR Projects](../README.md)

## 주요 기능

| 기능 | 구현 내용 |
| --- | --- |
| 수동 주행 | Bluetooth UART 명령으로 전진·후진·좌회전·우회전·정지 |
| 자동 주행 | 초음파 거리 임계값과 좌우 거리 비교로 장애물 회피 |
| 모드 변경 | PB4 버튼으로 수동/자동 전환, 전환 시 정지 함수 호출 |
| 거리 측정 | Echo 상승·하강 에지 인터럽트와 Timer3으로 펄스 폭 측정 |
| 측정 실패 처리 | 센서 측정 타임아웃, 전방 거리값이 0이면 정지 |
| 주행 표시 | 4자리 FND 2개로 시간과 상태 표시 |
| 종료 기록 | 수동 모드에서 E 명령으로 종료한 뒤 버튼으로 기록 페이지 변경 |

## 동작 구조

```text
Bluetooth ── UART1 ── 수동 명령 ──┐
                                ├── 모터 방향 / Timer1 PWM
초음파 3개 ── INT4~6 / Timer3 ── 자동 판단
                                │
버튼 ── 모드 / 기록 페이지 변경  └── FND 상태·기록
                                      └── Timer2 자릿수 갱신
```

### 자동 주행 기준

| 조건 | 동작 |
| --- | --- |
| 전방 측정값 0 | 정지 |
| 전방 거리 15cm 미만 | 후진 |
| 전방 거리 15cm 이상, 30cm 미만 | 좌우 중 더 넓은 쪽으로 회전. 양쪽 모두 0이면 정지, 같으면 우회전 |
| 전방 거리 30cm 이상 | 전진 |

센서는 좌→중앙→우 순서로 측정하며 각 측정 후 20ms 지연을 둡니다. 자동 주행은 거리 기준의 규칙 기반 제어입니다.

### Bluetooth 명령

UART1 통신 속도는 **9600bps**이며, 한 글자 명령의 대소문자를 모두 지원합니다.

| 명령 | 동작 | PWM 비교값 |
| --- | --- | --- |
| F | 전진 | 500 |
| B | 후진 | 600 |
| L | 좌회전 | 700 |
| R | 우회전 | 700 |
| S | 정지 | 0 |
| E | 정지 및 주행 종료 | 0 |

Timer1의 TOP은 1023이며, 표의 값은 PWM 출력 비교값입니다. 자동 모드의 비교값은 전진 180, 후진 150, 회전 160입니다. Bluetooth 명령은 수동 모드에서만 처리합니다.

### 주행 기록

종료 후 버튼을 누르면 주행시간 → 후진 횟수 → 좌회전 횟수 → 우회전 횟수 순으로 표시합니다. 횟수는 해당 상태에 진입할 때 증가하며 같은 상태의 반복 명령은 중복 집계하지 않습니다. 시간은 초기화 후 Timer2에서 누적하는 시간이며, 정지 상태도 포함합니다. 기록은 RAM에 보관하므로 전원 차단 후 유지되지 않습니다.

## 핀 구성

핀 표기는 ATmega128A의 포트 기준입니다.

| 연결 | MCU 포트 |
| --- | --- |
| 왼쪽 / 중앙 / 오른쪽 초음파 TRIG | PA0 / PA1 / PA2 |
| 왼쪽 / 중앙 / 오른쪽 초음파 ECHO | PE4(INT4) / PE5(INT5) / PE6(INT6) |
| 모터 PWM | PB5(OC1A), PB6(OC1B) |
| 모터 방향 | PF0~PF3 |
| 버튼 | PB4 |
| FND 세그먼트 데이터 | PORTC |
| FND 첫 번째 / 두 번째 자릿수 선택 | PD4~PD7 / PF4~PF7 |
| PC 디버깅 UART0 RX / TX | PE0 / PE1 |
| Bluetooth UART1 RX / TX | PD2 / PD3 |

모터 방향과 두 번째 FND가 PORTF를 공유합니다. 모터 제어는 상위 비트를 보존하고 인터럽트 상태를 저장·복원하며 하위 비트만 갱신합니다.

## 소스 구성

| 파일 | 역할 |
| --- | --- |
| `main.c` | 초기화, 모드 변경, 수동 명령과 자동 주행 판단 |
| `ultrasonic.c/.h` | 초음파 거리 측정, Echo 인터럽트, 타임아웃 |
| `pwm.c/.h` | 모터 PWM과 방향 제어 |
| `fnd.c/.h` | FND 표시, 주행시간·횟수 기록 |
| `button.c/.h` | 버튼 입력과 디바운싱 |
| `uart0.c/.h` | PC 디버깅 송수신, 표준 출력 연결 |
| `uart1.c/.h` | Bluetooth 송수신 |
| `define.h` | 클록, 모드, 거리 임계값 |

## 빌드 및 실행

### Atmel Studio

1. ATmega128A를 지원하는 AVR 툴체인과 디바이스 팩을 준비합니다.
2. `firmware/07.AUTO_CAR.cproj`를 엽니다.
3. 프로젝트의 MCU가 ATmega128A인지 확인하고 빌드합니다.
4. 사용 보드의 클록·배선을 확인한 뒤 보드에 맞는 프로그래머로 펌웨어를 기록합니다.
5. 초기 모드는 수동입니다. Bluetooth 명령과 버튼으로 동작을 확인합니다.

프로젝트는 ATmega_DFP 1.7.374를 사용합니다. 다른 버전에서는 디바이스 팩 경로를 조정합니다.

### AVR GCC 명령행

`avr-gcc`가 PATH에 있는 환경에서 `firmware` 폴더로 이동해 실행합니다.

```sh
avr-gcc -mmcu=atmega128a -std=gnu99 -Os -Wall -Wextra -Werror -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -Wl,--gc-sections main.c button.c fnd.c pwm.c uart0.c uart1.c ultrasonic.c -o auto_car.elf
avr-objcopy -O ihex -R .eeprom auto_car.elf auto_car.hex
```

PowerShell에서는 `-Wl,--gc-sections` 인수를 따옴표로 감쌉니다.
