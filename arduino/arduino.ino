#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

int joystickPins[4] = {A0, A1, A2, A3}; // 搖桿的 X 和 Y 軸接腳
int servoChannels[4] = {0, 1, 2, 3};    // 伺服馬達的通道
int servoPositions[4] = {90, 90, 35, 90}; // 伺服馬達的初始位置

int joystickCenter = 512; // 搖桿中心點值
int deadZone = 50; // 死區範圍，避免輕微偏移干擾

unsigned long lastUpdateTimes[4] = {0, 0, 0, 0}; // 紀錄每個伺服的最後更新時間
const unsigned long updateInterval = 20; // 控制每個伺服更新的最小間隔 (ms)

unsigned long lastSerialTime = 0; // 紀錄上次串口發送的時間
const unsigned long serialInterval = 400; // 串口數據發送間隔 (ms)

void setup() {
  Serial.begin(9600);
  pwm.begin();
  pwm.setPWMFreq(60); // 設定伺服驅動頻率
}

void loop() {
  unsigned long currentTime = millis();

  // 用來存儲搖桿狀態
  int joystickInputs[4];

  for (int i = 0; i < 4; i++) {
    int joystickValue = analogRead(joystickPins[i]); // 讀取搖桿數值
    joystickInputs[i] = mapJoystick(joystickValue);  // 根據搖桿數值來確定方向 (-1, 0, 1)

    // 只在達到更新間隔時更新伺服
    if (currentTime - lastUpdateTimes[i] >= updateInterval) {
      updateServo(servoChannels[i], servoPositions[i], joystickInputs[i], i);
      lastUpdateTimes[i] = currentTime;
    }
  }

  // 每 50ms 發送一次串口數據
  if (currentTime - lastSerialTime >= serialInterval) {
    Serial.print(joystickInputs[0]);
    Serial.print(",");
    Serial.print(joystickInputs[1]);
    Serial.print(",");
    Serial.print(joystickInputs[2]);
    Serial.print(",");
    Serial.println(joystickInputs[3]);
    lastSerialTime = currentTime;
  }
}

int mapJoystick(int joystickValue) {
  // 判斷搖桿數值並返回 -1, 0 或 1
  if (joystickValue < joystickCenter - deadZone) {
    return -1; // 搖桿值小於中心點減去死區，返回 -1
  } else if (joystickValue > joystickCenter + deadZone) {
    return 1; // 搖桿值大於中心點加上死區，返回 1
  } else {
    return 0; // 搖桿值在死區範圍內，返回 0
  }
}

void updateServo(int channel, int &position, int joystickInput, int index) {
  // 根據搖桿輸入來調整伺服馬達位置
  if (joystickInput == 1) {
    position = min(position + 1, 180); // 增加角度，最多到 180
  } else if (joystickInput == -1) {
    position = max(position - 1, 0); // 減少角度，最少到 0
  }

  // 將角度轉換為脈衝並設定伺服馬達
  int pulse = map(position, 0, 180, 150, 600); // 設定脈衝範圍
  pwm.setPWM(channel, 0, pulse);
}
