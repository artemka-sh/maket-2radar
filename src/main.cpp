#include <Servo.h>
#include <Arduino.h>

#define PIN_TOP_LEFT 5
#define PIN_TOP_RIGHT 2
#define PIN_HORIZ_RIGHT 3
#define PIN_HORIZ_LEFT 4

#define PIN_LIGHT_1 9
#define PIN_LIGHT_2 10
#define PIN_LED_FINISH 12

// Глобальные модификаторы (применяются ТОЛЬКО к горизонтальным моторам)
float globalSpeedMultiplier = 0.75;
float topAccelRatio = 0.5;

// === НАСТРОЙКИ ГОРИЗОНТАЛЬНЫХ МОТОРОВ ===
int stopSpeed = 90; // Жесткое удержание для ВСЕХ моторов
int speedForward = 50;
int speedBackward = 130;
int rampTime = 1200;
int moveTime = 500;
int pauseTime = 1000;

// === НАСТРОЙКИ ВЕРТИКАЛЬНЫХ МОТОРОВ (Идеальные калибровочные данные) ===
int upTension = 71;    // Граница натяга ВВЕРХ
int upSpeed = 59;      // Оптимальная рабочая сила ВВЕРХ
int downTension = 119; // Граница натяга ВНИЗ
int downSpeed = 123;   // Рабочая сила ВНИЗ

int upTime = 1500;
int topPauseTime = 3000;
int downTime = 1500;

int lastProgram = 0;

Servo motorLeftTop;
Servo motorRightTop;
Servo motorHorizontalRight;
Servo motorHorizontalLeft;

// Объявление функций
int calcSpeed(int baseSpeed);
void moveSmoothly(int startSpeed, int targetSpeed, unsigned long durationMs);
void smartRampTop(int startSpeed, int targetSpeed, unsigned long durationMs);
void moveTopWithRatio(int targetSpeed, unsigned long totalDurationMs, float ratio);
void program1();
void program2();
void program3();
void program4();

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

    // ВСЕ моторы стартуют в жестком удержании на 90
    motorLeftTop.write(stopSpeed);
    motorRightTop.write(stopSpeed);
    motorHorizontalRight.write(stopSpeed);
    motorHorizontalLeft.write(stopSpeed);

    // Инициализация "рандома"
    long noiseSeed = 0;
    for (int i = A0; i <= A6; i++) {
        noiseSeed += analogRead(i);
    }
    randomSeed(noiseSeed);

    delay(1000);
}

void loop() {
    // Выбор новой программы без повторений
    int currentProgram = random(1, 5);
    while (currentProgram == lastProgram) {
        currentProgram = random(1, 5);
    }
    lastProgram = currentProgram;

    digitalWrite(PIN_LED_FINISH, HIGH);

    switch (currentProgram) {
        case 1: program1(); break;
        case 2: program2(); break;
        case 3: program3(); break;
        case 4: program4(); break;
    }

    digitalWrite(PIN_LED_FINISH, HIGH);
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

// Программа 3: Ускоренная базовая
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

    moveSmoothly(stopSpeed, calcSpeed(speedBackward), rampTime * 1.5);
    delay(moveTime);
    moveSmoothly(calcSpeed(speedBackward), stopSpeed, rampTime);
}

// Пересчет скорости (ТОЛЬКО ДЛЯ ГОРИЗОНТАЛЬНЫХ МОТОРОВ)
int calcSpeed(int baseSpeed) {
    if (baseSpeed == stopSpeed) return stopSpeed;
    return stopSpeed + ((baseSpeed - stopSpeed) * globalSpeedMultiplier);
}

// Плавное движение ДЛЯ ГОРИЗОНТАЛЬНЫХ МОТОРОВ
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

// Плавное движение S-кривой ДЛЯ ВЕРТИКАЛЬНЫХ МОТОРОВ
void smartRampTop(int startSpeed, int targetSpeed, unsigned long durationMs) {
    unsigned long startTime = millis();

    while (true) {
        unsigned long elapsed = millis() - startTime;
        if (elapsed >= durationMs) break;

        float progress = (float)elapsed / durationMs;
        float easeProgress = progress * progress * (3.0f - 2.0f * progress);

        int currentSpeed = startSpeed + (targetSpeed - startSpeed) * easeProgress;

        motorLeftTop.write(currentSpeed);
        motorRightTop.write(currentSpeed);
        delay(20);
    }
    motorLeftTop.write(targetSpeed);
    motorRightTop.write(targetSpeed);
}

// Точно по вашей калибровке: Прыжок -> Задержка -> Разгон -> Торможение -> Возврат в 90
void moveTopWithRatio(int targetSpeed, unsigned long totalDurationMs, float ratio) {
    // Определяем правильную точку натяга
    bool isUp = (targetSpeed < stopSpeed);
    int tension = isUp ? upTension : downTension;

    // Рассчитываем время разгона и торможения
    unsigned long accelTime = totalDurationMs * ratio;
    unsigned long decelTime = totalDurationMs - accelTime;

    // 1. Мгновенно выбираем слабину (прыжок из 90 в точку натяга)
    motorLeftTop.write(tension);
    motorRightTop.write(tension);
    delay(50); // Микро-пауза, чтобы шестерни сцепились без удара

    // 2. Плавный разгон (Натяг -> Рабочая скорость)
    smartRampTop(tension, targetSpeed, accelTime);

    // 3. Плавное торможение (Рабочая скорость -> Натяг)
    smartRampTop(targetSpeed, tension, decelTime);

    // 4. Мгновенно возвращаем в жесткое удержание (90)
    motorLeftTop.write(stopSpeed);
    motorRightTop.write(stopSpeed);
}