#ifndef AUTO_CAR_DEFINE_H
#define AUTO_CAR_DEFINE_H


#define F_CPU 16000000UL          /* 16MHz */

/* 동작 모드 */
#define MANUAL_MODE   0           /* 블루투스 수동 조종 */
#define AUTO_MODE     1           /* 초음파 자동 주행   */

/* 자동주행 거리*/
#define DIST_VERY_NEAR  15        /* 이보다 가까우면 후진          */
#define DIST_NEAR       30        /* 이보다 가까우면 넓은 쪽으로 회전 */


#endif /* AUTO_CAR_DEFINE_H */
