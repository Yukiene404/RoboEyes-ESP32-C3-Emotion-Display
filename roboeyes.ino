
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <FluxGarage_RoboEyes.h>

// ESP32-C3 SuperMini pins
#define OLED_SDA    6
#define OLED_SCL    7
#define BUTTON_PIN  9

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3C

#define EMOTION_HOLD_MS 2500

Adafruit_SH1106G display(
  SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET
);

RoboEyes<Adafruit_SH1106G> roboEyes(display);

enum Emotion {
  EMO_IDLE,
  EMO_HAPPY,
  EMO_CURIOUS,
  EMO_ANNOYED,
  EMO_SAD
};

Emotion currentEmotion = EMO_IDLE;
unsigned long emotionStartTime = 0;
unsigned long lastButtonPress = 0;

bool lastButtonState = HIGH;

const Emotion emotionCycle[] = {
  EMO_HAPPY, EMO_CURIOUS, EMO_ANNOYED, EMO_SAD
};

const uint8_t CYCLE_LEN =
  sizeof(emotionCycle) / sizeof(emotionCycle[0]);

uint8_t cycleIdx = 0;

void setEmotion(Emotion e) {
  currentEmotion = e;
  emotionStartTime = millis();

  roboEyes.setHFlicker(OFF, 0);

  switch (e) {
    case EMO_IDLE:
      roboEyes.setMood(DEFAULT);
      roboEyes.setPosition(DEFAULT);
      roboEyes.setIdleMode(ON, 2, 2);
      roboEyes.setCuriosity(ON);
      break;

    case EMO_HAPPY:
      roboEyes.setMood(HAPPY);
      roboEyes.setIdleMode(OFF, 0, 0);
      roboEyes.anim_laugh();
      break;

    case EMO_CURIOUS:
      roboEyes.setMood(DEFAULT);
      roboEyes.setCuriosity(ON);
      roboEyes.setPosition(NE);
      roboEyes.setIdleMode(OFF, 0, 0);
      break;

    case EMO_ANNOYED:
      roboEyes.setMood(ANGRY);
      roboEyes.setIdleMode(OFF, 0, 0);
      roboEyes.setHFlicker(ON, 2);
      break;

    case EMO_SAD:
      roboEyes.setMood(TIRED);
      roboEyes.setIdleMode(OFF, 0, 0);
      roboEyes.setPosition(S);
      break;
  }
}

void handleButton() {
  bool state = digitalRead(BUTTON_PIN);

  if (state == LOW && lastButtonState == HIGH &&
      millis() - lastButtonPress > 250) {

    lastButtonPress = millis();

    setEmotion(emotionCycle[cycleIdx]);
    cycleIdx = (cycleIdx + 1) % CYCLE_LEN;
  }

  lastButtonState = state;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setClock(100000);

  if (!display.begin(OLED_ADDRESS, true)) {
    Serial.println("OLED initialization failed!");
    while (true) {
      delay(1000);
    }
  }

  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 60);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 2);
  roboEyes.setMood(DEFAULT);
  roboEyes.setCuriosity(ON);

  setEmotion(EMO_IDLE);
}

void loop() {
  roboEyes.update();
  handleButton();

  if (currentEmotion != EMO_IDLE &&
      millis() - emotionStartTime >= EMOTION_HOLD_MS) {
    setEmotion(EMO_IDLE);
  }
}
