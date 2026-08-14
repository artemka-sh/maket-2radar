#include <Servo.h>
#include <Arduino.h>

#define PIN_TOP_LEFT 5
#define PIN_TOP_RIGHT 2
#define PIN_HORIZ_RIGHT 3
#define PIN_HORIZ_LEFT 4

#define PIN_LIGHT_1 9
#define PIN_LIGHT_2 10
#define PIN_LED_FINISH 12

// Глобальные модификаторы
float globalSpeedMultiplier = 0.8; //сделали чуть медленней чем было
float topAccelRatio = 0.5;

// Настройки горизонтальных моторов
int stopSpeed = 90;
int speedForward = 50;
int speedBackward = 130;
int rampTime = 1200;
int moveTime = 500;
int pauseTime = 1000;

// Настройки вертикальных моторов
int upSpeed = 25;
int downSpeed = 155;
int upTime = 600;
int topPauseTime = 3000;
int downTime = 500;

int lastProgram = 0;

Servo motorLeftTop;
Servo motorRightTop;
Servo motorHorizontalRight;
Servo motorHorizontalLeft;


int calcSpeed(int baseSpeed);
void moveSmoothly(int startSpeed, int targetSpeed, unsigned long durationMs);
void rampTop(int startSpeed, int targetSpeed, unsigned long durationMs);
void moveTopWithRatio(int targetRawSpeed, unsigned long totalDurationMs, float ratio);
void program1();
void program2();
void program3();
void program4();
void program5();

void setup() {
    pinMode(PIN_LIGHT_1, OUTPUT);
    pinMode(PIN_LIGHT_2, OUTPUT);
    pinMode(PIN_LED_FINISH, OUTPUT);
    digitalWrite(PIN_LED_FINISH, LOW);

    digitalWrite(PIN_LIGHT_1, HIGH);
    digitalWrite(PIN_LIGHT_2, HIGH);

    motorLeftTop.attach(PIN_TOP_LEFT);
    motorRightTop.attach(PIN_TOP_RIGHT);
    motorHorizontalRight.attach(PIN_HORIZ_RIGHT);
    motorHorizontalLeft.attach(PIN_HORIZ_LEFT);

    motorLeftTop.write(stopSpeed);
    motorRightTop.write(stopSpeed);
    motorHorizontalRight.write(stopSpeed);
    motorHorizontalLeft.write(stopSpeed);

    // Собираем шум со свободных пинов для инициализации "рандома")
    long noiseSeed = 0;
    for (int i = A0; i <= A6; i++) {
        noiseSeed += analogRead(i);
    }
    randomSeed(noiseSeed);

    delay(1000);
}

void loop() {
    // Выбор новой программы без повторений
    int currentProgram = random(1, 6);
    while (currentProgram == lastProgram) {
        currentProgram = random(1, 6);
    }
    lastProgram = currentProgram;

    digitalWrite(PIN_LED_FINISH, HIGH);


    switch (currentProgram) {
        case 1: program1(); break;
        case 2: program2(); break;
        case 3: program3(); break;
        case 4: program4(); break;
        // case 5: program5(); break;
    }

    digitalWrite(PIN_LED_FINISH, HIGH);

    // Выключаем свет после завершения
    delay(1000);
}


// Программа 1: Базовая
void program1() {
    moveSmoothly(stopSpeed, calcSpeed(speedForward), rampTime);
    delay(moveTime);
    moveSmoothly(calcSpeed(speedForward), stopSpeed, rampTime);
    delay(pauseTime);

    moveSmoothly(stopSpeed, calcSpeed(speedBackward), rampTime);
    delay(moveTime);
    moveSmoothly(calcSpeed(speedBackward), stopSpeed, rampTime);
    delay(pauseTime);

    moveTopWithRatio(upSpeed, upTime, topAccelRatio);
    delay(topPauseTime);
    moveTopWithRatio(downSpeed, downTime, topAccelRatio);
}

// Программа 2: Сначала подъем, потом горизонталь
void program2() {
    moveTopWithRatio(upSpeed, upTime, topAccelRatio);
    delay(topPauseTime);
    moveTopWithRatio(downSpeed, downTime, topAccelRatio);
    delay(pauseTime);

    moveSmoothly(stopSpeed, calcSpeed(speedForward), rampTime);
    delay(moveTime);
    moveSmoothly(calcSpeed(speedForward), stopSpeed, rampTime);
    delay(pauseTime);

    moveSmoothly(stopSpeed, calcSpeed(speedBackward), rampTime);
    delay(moveTime);
    moveSmoothly(calcSpeed(speedBackward), stopSpeed, rampTime);
}

// Программа 3: Ускоренная базовая (разгон и паузы срезаны вдвое)
void program3() {
    moveSmoothly(stopSpeed, calcSpeed(speedForward), rampTime / 2);
    delay(moveTime);
    moveSmoothly(calcSpeed(speedForward), stopSpeed, rampTime / 2);
    delay(pauseTime / 2);

    moveSmoothly(stopSpeed, calcSpeed(speedBackward), rampTime / 2);
    delay(moveTime);
    moveSmoothly(calcSpeed(speedBackward), stopSpeed, rampTime / 2);
    delay(pauseTime / 2);

    moveTopWithRatio(upSpeed, upTime, topAccelRatio);
    delay(topPauseTime / 2);
    moveTopWithRatio(downSpeed, downTime, topAccelRatio);
}

// Программа 4: "Пульс" (короткие рывки)
void program4() {
    for(int i = 0; i < 3; i++) {
        moveSmoothly(stopSpeed, calcSpeed(speedForward), 300);
        delay(200);
        moveSmoothly(calcSpeed(speedForward), stopSpeed, 300);
        delay(300);
    }

    delay(pauseTime);

    moveTopWithRatio(upSpeed, upTime, topAccelRatio);
    delay(topPauseTime);
    moveTopWithRatio(downSpeed, downTime, topAccelRatio);
    delay(pauseTime);

    // Долгий и плавный возврат
    moveSmoothly(stopSpeed, calcSpeed(speedBackward), rampTime * 1.5);
    delay(moveTime);
    moveSmoothly(calcSpeed(speedBackward), stopSpeed, rampTime);
}

// // Программа 5: "Радар" (медленный поиск и резкий бросок)
// void program5() {
//     int slowForward = stopSpeed + ((calcSpeed(speedForward) - stopSpeed) / 3);
//
//     // Долгое и медленное сканирование
//     moveSmoothly(stopSpeed, slowForward, 2500);
//     delay(1000);
//     moveSmoothly(slowForward, stopSpeed, 1500);
//     delay(1000);
//
//     // Резкий бросок обратно
//     moveSmoothly(stopSpeed, calcSpeed(speedBackward), 200);
//     delay(500);
//     moveSmoothly(calcSpeed(speedBackward), stopSpeed, 400);
//
//     moveTopWithRatio(upSpeed, upTime, topAccelRatio);
//     delay(topPauseTime);
//     moveTopWithRatio(downSpeed, downTime, topAccelRatio);
// }

// Пересчет скорости с учетом множителя
int calcSpeed(int baseSpeed) {
    if (baseSpeed == stopSpeed) return stopSpeed;
    return stopSpeed + ((baseSpeed - stopSpeed) * globalSpeedMultiplier);
}

// Плавное движение для горизонтальных моторов с поддержкой света
void moveSmoothly(int startSpeed, int targetSpeed, unsigned long durationMs) {
    unsigned long startTime = millis();
    while (millis() - startTime < durationMs) {
        float progress = (float)(millis() - startTime) / durationMs;
        if (progress > 1.0f) progress = 1.0f;

        int currentSpeed = startSpeed + (targetSpeed - startSpeed) * progress;
        motorHorizontalRight.write(currentSpeed);
        motorHorizontalLeft.write(currentSpeed);

        delay(15);
    }
    motorHorizontalRight.write(targetSpeed);
    motorHorizontalLeft.write(targetSpeed);
}

// Базовый разгон/тормоз для верхних моторов
void rampTop(int startSpeed, int targetSpeed, unsigned long durationMs) {
    unsigned long startTime = millis();
    while (millis() - startTime < durationMs) {
        float progress = (float)(millis() - startTime) / durationMs;
        if (progress > 1.0f) progress = 1.0f;

        int currentSpeed = startSpeed + (targetSpeed - startSpeed) * progress;
        motorLeftTop.write(currentSpeed);
        motorRightTop.write(currentSpeed);

        delay(15);
    }
    motorLeftTop.write(targetSpeed);
    motorRightTop.write(targetSpeed);
}

// Движение верхних моторов с компенсацией слабого левого мотора - они не одинаковы
void moveTopWithRatio(int targetRawSpeed, unsigned long totalDurationMs, float ratio) {
    int targetSpeed = calcSpeed(targetRawSpeed);
    unsigned long accelTime = totalDurationMs * ratio;

    rampTop(stopSpeed, targetSpeed, accelTime);
    rampTop(targetSpeed, stopSpeed, totalDurationMs - accelTime);

    // Компенсация: даем левому мотору дополнительные 100 мс на рабочей скорости,
    // чтобы он успел довернуть механизм
    motorLeftTop.write(targetSpeed);
    delay(100);
    motorLeftTop.write(stopSpeed);
}