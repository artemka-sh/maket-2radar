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
float globalSpeedMultiplier = 0.75; //сделали чуть медленней чем было
float topAccelRatio = 0.5;

// Настройки горизонтальных моторов
int stopSpeed = 90;
int speedForward = 50;
int speedBackward = 130;
int rampTime = 1200;
int moveTime = 500;
int pauseTime = 1000;

// Настройки вертикальных моторов
int upSpeed = 27;
int downSpeed = 153;
int upTime = 1500;
int topPauseTime = 3000;
int downTime = 1500;

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

    while (true) {
        unsigned long elapsed = millis() - startTime;
        if (elapsed >= durationMs) {
            break;
        }
        float progress = (float)elapsed / durationMs;

        // Магическая формула S-кривой (SmoothStep): 3*x^2 - 2*x^3
        // Дает идеальную плавность и на старте, и на финише движения.
        float easeProgress = progress * progress * (3.0f - 2.0f * progress);

        int currentSpeed = startSpeed + (targetSpeed - startSpeed) * easeProgress;

        motorLeftTop.write(currentSpeed);
        motorRightTop.write(currentSpeed);

        // Изменил delay с 15 на 20!
        // Стандартные сервоприводы работают на частоте 50Гц (обновление каждые 20мс).
        // Если слать им сигнал чаще (каждые 15мс), дешевые контроллеры сервы могут "давиться"
        // и пропускать такты, что тоже вызывает физические рывки.
        delay(20);
    }

    motorLeftTop.write(targetSpeed);
    motorRightTop.write(targetSpeed);
}

// Движение верхних моторов с компенсацией слабого левого мотора - они не одинаковы
void moveTopWithRatio(int targetRawSpeed, unsigned long totalDurationMs, float ratio) {
    int targetSpeed = calcSpeed(targetRawSpeed);
    unsigned long accelTime = totalDurationMs * ratio;

    // Плавный разгон
    rampTop(stopSpeed, targetSpeed, accelTime);

    // Плавное, правильное торможение
    rampTop(targetSpeed, stopSpeed, totalDurationMs - accelTime);

    // ВАЖНО: Никаких мгновенных включений delay(100) здесь быть не должно!
    // Убеждаемся, что оба точно остановлены
    motorLeftTop.write(stopSpeed);
    motorRightTop.write(stopSpeed);
}