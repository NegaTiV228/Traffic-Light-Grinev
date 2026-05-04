#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#define SCREEN_ADDRESS 0x3C
#define OLED_RESET    -1
#define I2C_SDA 33
#define I2C_SCL 35
Adafruit_SSD1306 display(128, 64, &Wire, -1);
RTC_DS1307 rtc;
// светофоры
const int NSR = 42; //Север-Юг красный свет
const int WER = 41; //Запад-Восток красный свет
const int NSY =  3; //Север-Юг жёлтый свет
const int WEY =  4; //Запад-Восток жёлтый свет
const int NSG =  0; //Север-Юг зелёный свет
const int WEG = 45; //Запад-Восток зелёный свет
const int WR  = 39;  //Пешеход красный свет
const int WG  = 40;  //Пешеход зелёный свет
// диагностика
const int DP1 = 37; //Точка самодиагностики 1
const int DP2 = 36; //Точка самодиагностики 2
const int DP3 = 21; //Точка самодиагностики 3
const int DP4 = 34; //Точка самодиагностики 4
const int DP5 = 20; //Точка самодиагностики 5
const int DP6 = 26; //Точка самодиагностики 6
const int DP7 = 19; //Точка самодиагностики 7
const int DP8 = 18; //Точка самодиагностики 8
const int DPA = 38; //Точка самодиагностики A
const int DPB = 17; //Точка самодиагностики B
const int DPC = 16; //Точка самодиагностики C
const int DPD = 15; //Точка самодиагностики D
const int DPE = 14; //Точка самодиагностики E
const int DPF = 13; //Точка самодиагностики F
// переферия
const int BUZ =  9; //Звуковое оповещение
const int SW  = 12; //кнопка энкодера
const int DT  = 11; //выход B энкодера
const int CLK = 10; //выход A энкодера

enum State {
  STATE_1,
  STATE_2,
  STATE_3,
  STATE_4,
  STATE_5,
  STATE_6,
  STATE_7,
  STATE_8,
  STATE_9
};

//длительности режимов 
unsigned long TIME_ST_1 = 5000    ;  // длительность режима 1 (движение С-Ю)
unsigned long TIME_ST_2 = 3000    ;  // длительность режима 2 (завершение С-Ю)
unsigned long TIME_ST_3 = 2000    ;  // длительность режима 3 (переход с С-Ю на З-В)
unsigned long TIME_ST_4 = 5000    ;  // длительность режима 4 (движение З-В)
unsigned long TIME_ST_5 = 3000    ;  // длительность режима 5 (завершение З-В)
unsigned long TIME_ST_6 = 2000    ;  // длительность режима 6 (переход с З-В на пешеходный)
unsigned long TIME_ST_7 = 4000    ;  // длительность режима 7 (пешеходный режим)
unsigned long TIME_ST_8 = 3000    ;  // длительность режима 8 (переход с пешеходного на С-Ю)

State currentState = STATE_1;  // Глобальная переменная
unsigned long lastStateChangeTime = 0;
bool displayInitialized = false;
bool rtcInitialized = false;
unsigned long lastTimeDisplay = 0;

unsigned long getStateDelay(State state) {
  switch(state) {
    case STATE_1: return TIME_ST_1;
    case STATE_2: return TIME_ST_2;
    case STATE_3: return TIME_ST_3;
    case STATE_4: return TIME_ST_4;
    case STATE_5: return TIME_ST_5;
    case STATE_6: return TIME_ST_6;
    case STATE_7: return TIME_ST_7;
    case STATE_8: return TIME_ST_8;
    default: return 100000;
  }
}
// Получение текущего времени в виде строки
  String getCurrentTime() {
  if (!rtcInitialized) return "RTC Error";
  
  DateTime now = rtc.now();
  
  char timeStr[20];
  sprintf(timeStr, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  
  return String(timeStr);
}
void displayTimeOnOLED() {
  if (!displayInitialized) return;
  int cursorX = display.getCursorX();
  int cursorY = display.getCursorY();
  display.setCursor(0, 0);
  display.print(getCurrentTime());
  display.setCursor(cursorX, cursorY);
}
void setup() {
Wire.begin(I2C_SDA, I2C_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    delay(100);
  }
  if (!rtc.begin()) {
  rtcInitialized = false;
} else {
  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  rtcInitialized = true;  // <--- ЭТА СТРОКУ ДОБАВИТЬ!
}
   if (!rtc.begin()) {
  rtcInitialized = false;
} else {
  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  rtcInitialized = true;  // <--- ЭТА СТРОКУ ДОБАВИТЬ!
}

display.clearDisplay();
display.setTextSize(2);
display.setTextColor(SSD1306_WHITE);
display.display();

// светодиоды
 pinMode(NSR, OUTPUT);
 pinMode(WER, OUTPUT);
 pinMode(NSY, OUTPUT);
 pinMode(WEY, OUTPUT);
 pinMode(NSG, OUTPUT);
 pinMode(WEG, OUTPUT);
 pinMode(WR , OUTPUT);
 pinMode(WG , OUTPUT);

// диагностика
 pinMode(DP1, INPUT_PULLDOWN);
 pinMode(DP2, INPUT_PULLDOWN);
 pinMode(DP3, INPUT_PULLDOWN);
 pinMode(DP4, INPUT_PULLDOWN);
 pinMode(DP5, INPUT_PULLDOWN);
 pinMode(DP6, INPUT_PULLDOWN);
 pinMode(DP7, INPUT_PULLDOWN);
 pinMode(DP8, INPUT_PULLDOWN);
 pinMode(DPA, INPUT_PULLDOWN);
 pinMode(DPB, INPUT_PULLDOWN);
 pinMode(DPC, INPUT_PULLDOWN);
 pinMode(DPD, INPUT_PULLDOWN);
 pinMode(DPE, INPUT_PULLDOWN);
 pinMode(DPF, INPUT_PULLDOWN);
// переферия
 pinMode(BUZ, OUTPUT);
 pinMode(SW , INPUT );
 pinMode(DT , INPUT );
 pinMode(CLK, INPUT );
setState(STATE_1);
}

// Функция для переключения на следующий режим
void nextState() {
  int nextMode = (currentState + 1) % 8; 
  currentState = (State)nextMode;
  setState(currentState);
  lastStateChangeTime = millis();
}

void setState(State state) {
  
  switch(state) {
    case STATE_1:
      Serial.println("движение С-Ю");
      digitalWrite(NSR, LOW ); 
      digitalWrite(WER, HIGH); 
      digitalWrite(NSY, LOW ); 
      digitalWrite(WEY, LOW ); 
      digitalWrite(NSG, HIGH); 
      digitalWrite(WEG, LOW ); 
      digitalWrite(WR , HIGH); 
      digitalWrite(WG , LOW ); 
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Flow N-S");
      display.setCursor(0, 30);
      display.println(getCurrentTime());
      display.display();
      break;

    case STATE_2:
      Serial.println("завершение С-Ю");
      digitalWrite(NSR, LOW); 
      digitalWrite(WER, HIGH); 
      digitalWrite(NSY, LOW); 
      digitalWrite(WEY, LOW); 
      digitalWrite(WEG, LOW); 
      digitalWrite(WR , HIGH); 
      digitalWrite(WG , LOW);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("End N-S");
      display.setCursor(0, 30);
      display.println(getCurrentTime());
      display.display();
      break;

    case STATE_3:
      Serial.println("переход с С-Ю на З-В");
      digitalWrite(NSR, LOW ); 
      digitalWrite(WER, HIGH); 
      digitalWrite(NSY, HIGH); 
      digitalWrite(WEY, HIGH); 
      digitalWrite(NSG, LOW ); 
      digitalWrite(WEG, LOW ); 
      digitalWrite(WR , HIGH); 
      digitalWrite(WG , LOW ); 
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("N-S to W-E");
      display.setCursor(0, 30);
      display.println(getCurrentTime());
      display.display();
      break;

    case STATE_4:
      Serial.println("движение З-В");
      digitalWrite(NSR, HIGH); 
      digitalWrite(WER, LOW ); 
      digitalWrite(NSY, LOW ); 
      digitalWrite(WEY, LOW ); 
      digitalWrite(NSG, LOW ); 
      digitalWrite(WEG, HIGH); 
      digitalWrite(WR , HIGH); 
      digitalWrite(WG , LOW ); 
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Flow W-E");
      display.setCursor(0, 30);
      display.println(getCurrentTime());
      display.display();
      break;

    case STATE_5:
      Serial.println("завершение З-В");
      digitalWrite(NSR, HIGH); 
      digitalWrite(WER, LOW ); 
      digitalWrite(NSY, LOW ); 
      digitalWrite(WEY, LOW ); 
      digitalWrite(NSG, LOW ); 
      digitalWrite(WR , HIGH); 
      digitalWrite(WG , LOW ); 
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("End W-E");
      display.setCursor(0, 30);
      display.println(getCurrentTime());
      display.display();
      break;

    case STATE_6:
      Serial.println("переход с З-В на пешеходный");
      digitalWrite(NSR, HIGH); 
      digitalWrite(WER, LOW ); 
      digitalWrite(NSY, LOW ); 
      digitalWrite(WEY, HIGH); 
      digitalWrite(NSG, LOW ); 
      digitalWrite(WEG, LOW ); 
      digitalWrite(WR , HIGH); 
      digitalWrite(WG , LOW ); 
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("N-S to W");
      display.setCursor(0, 30);
      display.println(getCurrentTime());
      display.display();
      break;

    case STATE_7:
      Serial.println("пешеходный режим");
      digitalWrite(NSR, HIGH); 
      digitalWrite(WER, HIGH); 
      digitalWrite(NSY, LOW ); 
      digitalWrite(WEY, LOW ); 
      digitalWrite(NSG, LOW ); 
      digitalWrite(WEG, LOW ); 
      digitalWrite(WR , LOW ); 
      digitalWrite(WG , HIGH);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Walker");
      display.setCursor(0, 30);
      display.println(getCurrentTime());
      display.display();
      break;

    case STATE_8:
      Serial.println("переход с пешеходного на С-Ю");
      digitalWrite(NSR, HIGH); 
      digitalWrite(WER, HIGH); 
      digitalWrite(NSY, HIGH); 
      digitalWrite(WEY, LOW ); 
      digitalWrite(NSG, LOW ); 
      digitalWrite(WEG, LOW ); 
      digitalWrite(WR , LOW ); 
      digitalWrite(WG , HIGH); 
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("W to N-S");
      display.setCursor(0, 30);
      display.println(getCurrentTime());
      display.display();
      break;

    case STATE_9:
      Serial.println("Авария");
      digitalWrite(NSR, LOW ); 
      digitalWrite(WER, LOW ); 
      digitalWrite(NSG, LOW ); 
      digitalWrite(WEG, LOW ); 
      digitalWrite(WR , LOW ); 
      digitalWrite(WG , HIGH); 
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Achtung");
      display.setCursor(0, 30);
      display.println(getCurrentTime());
      display.display();
      break;
  }
}

// Функция для неблокирующего мигания
void Blink(int pin, unsigned long interval = 500) {
  static unsigned long lastBlink[48] = {0};  // Массив времени пина
  static bool ledState[48] = {false};        // Массив состояния пина
  unsigned long currentTime = millis();
  
  if (currentTime - lastBlink[pin] >= interval) {  
    lastBlink[pin] = currentTime;
    ledState[pin] = !ledState[pin];
    digitalWrite(pin, ledState[pin]);
  }
}

// Функция для установки задержки режима
void setStateDelay(int stateNumber, unsigned long delayMs) {
  switch(stateNumber) {
    case 1: TIME_ST_1 = delayMs; break;
    case 2: TIME_ST_2 = delayMs; break;
    case 3: TIME_ST_3 = delayMs; break;
    case 4: TIME_ST_4 = delayMs; break;
    case 5: TIME_ST_5 = delayMs; break;
    case 6: TIME_ST_6 = delayMs; break;
    case 7: TIME_ST_7 = delayMs; break;
    case 8: TIME_ST_8 = delayMs; break;
    default: Serial.println("Ошибка"); break;
  }
  Serial.print("Задержка для режима ");
  Serial.print(stateNumber);
  Serial.print(" установлена: ");
  Serial.print(delayMs);
  Serial.println(" мс");
}

void loop() {
    // Обновление времени на дисплее каждую секунду
  if(displayInitialized && (millis() - lastTimeDisplay >= 1000)) {
    lastTimeDisplay = millis();
    int cursorX = display.getCursorX();
    int cursorY = display.getCursorY();
    display.setCursor(70, 0);
    display.print(getCurrentTime());
    display.display();
    display.setCursor(cursorX, cursorY);
    if(displayInitialized && rtcInitialized && (millis() - lastTimeDisplay >= 1000)) { 
  lastTimeDisplay = millis();
  display.setCursor(70, 0);
  display.print(getCurrentTime());
  display.display();
}
  }
  // Чтение команд от консоли
  if (Serial.available() > 0) {
    String input = Serial.readString();
    input.trim();

     // Команда для переключения режима
    if (input.startsWith("m")) {
      int modeNumber = input.substring(1).toInt();
      if (modeNumber >= 1 && modeNumber <= 9) {
        State newState = (State)(modeNumber - 1);
        if (newState != currentState) {
          currentState = newState;
          setState(currentState);
          lastStateChangeTime = millis(); 
          Serial.println("Ручное переключение");
        }
      } else {
        Serial.print("Ошибка");
        Serial.println(modeNumber);
      }
    }

    // Команда для установки задержки: t<режим>,<задержка> например: t1,10000
    else if (input.startsWith("t")) {
      int commaPos = input.indexOf(',');
      if (commaPos > 0) {
        int modeNumber = input.substring(1, commaPos).toInt();
        unsigned long delayMs = input.substring(commaPos + 1).toInt();
        if (modeNumber >= 1 && modeNumber <= 9 && delayMs > 0) {
          setStateDelay(modeNumber, delayMs);
        } else {}
      } else {}
    
    } else {
      Serial.print("Ошибка");
      Serial.println(input);
    }
  }

// мигания отдельно, должны быть в loop
  switch(currentState) {
    case STATE_2:
      Blink(NSG); 
      break;
    
    case STATE_5:
      Blink(WEG);  
      break;

    case STATE_7:
      Blink(BUZ);  
      break;

    case STATE_8:
      Blink(WG);
      Blink(BUZ, 250);  
      break;
      
    case STATE_9:
      Blink(WEY);
      Blink(NSY);  
      break;

    default:
      break;
  }

// Автоматическое переключение режимов
  unsigned long currentTime = millis();
    unsigned long currentDelay = getStateDelay(currentState);
    
    if (currentTime - lastStateChangeTime >= currentDelay) {
      nextState();
    }

  delay(10);  // для стабильности
}

