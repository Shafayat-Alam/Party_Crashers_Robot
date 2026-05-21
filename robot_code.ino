// #include <NewPing.h>


#include <Servo.h>


// pin constants definition
const int motorOne_InputTwo = 4;
const int motorOne_InputOne = 5;
const int motorOne_Enable = 6;


const int motorTwo_Enable = 11;
const int motorTwo_InputOne = 12;
const int motorTwo_InputTwo = 13;


const int trigFront = 7;
const int echoFront = 8;
const int trigBottom = 2;
const int echoBottom = 3;


const int SWITCH = 1;


const int buzzer = A5;


// objects initialization
Servo servoLeft;
Servo servoRight;


// state variables
String gripStatus = "released";
String mode = "forward";
bool started = false;
int min = 999;


class Timer {
public:
  double timeout;
  double start;
  Timer() {
    timeout = 0;
    start = millis();
  }


  void reset() {
    start = millis();
  }


  void setTimeout(double t) {
    timeout = t;
  }
  double elapsed() {
    return millis() - start;
  }
  bool tick() {
    bool ticked = millis() - start > timeout;
    if (ticked) {
      start = millis();
    }
    return ticked;
  }
};


class LEDHandler {
public:
  const int LED_RED = A0;
  const int LED_BLUE = A1;
  bool red_on = false;
  bool blue_on = false;
  Timer timer;
  LEDHandler() {
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    timer.setTimeout(100);
    Serial.println("LED Handler Initialized");
  }


  void LED_OFF() {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_BLUE, LOW);
  }


  void LED_ON() {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_BLUE, HIGH);
  }


  void RED_ON() {
    digitalWrite(LED_RED, HIGH);
    red_on = true;
  }
  void BLUE_ON() {
    digitalWrite(LED_BLUE, HIGH);
    blue_on = true;
  }
  void RED_OFF() {
    digitalWrite(LED_RED, LOW);
    red_on = false;
  }
  void BLUE_OFF() {
    digitalWrite(LED_BLUE, LOW);
    blue_on = false;
  }


  bool LEDS_ON() {
    return red_on && blue_on;
  }


  void LED_TOGGLE_BLINK() {
    if (LEDS_ON()) {
      BLUE_OFF();
    }
    if (timer.tick()) {
      if (red_on) {
        RED_OFF();
        BLUE_ON();
      } else {
        BLUE_OFF();
        RED_ON();
      }
    }
  }
};


class Siren {
public:
  int frequency;
  int frequency_k = 2;
  bool playing = false;
  Timer timer;
  Siren() {
    frequency = 700;
    timer.setTimeout(15);
  }


  isPlaying() {
    return playing;
  }


  void start() {
    playing = true;
  }


  void play() {
    if (!playing) {
      return;
    } else {
      tone(buzzer, frequency);
      frequency += frequency_k;
    }
    if (frequency < 700 || frequency > 800) {
      frequency_k *= -1;
    }
    // if (timer.tick()) {
    //   frequency += frequency_k;
    // }
    Serial.println(frequency);
  }


  void stop() {
    playing = false;
    noTone(buzzer);
  }
};


class Beep {
public:
  int frequency;
  int timeout;
  Timer timer, soundTimer;
  bool playing = false;
  LEDHandler led;


  Beep() {
    frequency = 50;
    timeout = 100;
    timer.setTimeout(timeout);
    soundTimer.setTimeout(50);
  }
  void setFrequency(int f){
    frequency = f;
  }


  void setTimeout(int t1, int t2) {
    timeout = t1;
    timer.setTimeout(timeout);
    soundTimer.setTimeout(t2);
  }
  void play() {
    if (!playing && timer.tick()) {
      playing = true;
      soundTimer.reset();
    } else {
      if (playing) {
        tone(buzzer, frequency);
        led.RED_ON();        
        if (soundTimer.tick()) {
          playing = false;
          timer.reset();
          noTone(buzzer);
          led.RED_OFF();
        }
      }
    }
  }
};


LEDHandler led_handler;
Siren siren;
// NewPing sonar(trigFront, echoFront, 200);


void setup() {
  Serial.begin(9600);
  for (int i = 4; i <= 10; i++) {
    if (i == 8)
      continue;
    pinMode(i, OUTPUT);
  }
  pinMode(SWITCH, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(trigFront, OUTPUT);
  pinMode(echoFront, INPUT);
  pinMode(trigBottom, OUTPUT);
  pinMode(echoBottom, INPUT);
  pinMode(6, OUTPUT);
  pinMode(11, OUTPUT);
  // digitalWrite(motorOne_Enable, HIGH);
  // digitalWrite(motorTwo_Enable, HIGH);
  servoLeft.attach(10);
  servoRight.attach(9);
  gripRelease();
  // siren.start();
}
void forward() {
  // Serial.println("Forward");
  digitalWrite(motorOne_InputOne, HIGH);
  digitalWrite(motorOne_InputTwo, LOW);
  digitalWrite(motorTwo_InputOne, LOW);
  digitalWrite(motorTwo_InputTwo, HIGH);
  // digitalWrite(motorOne_Enable, 0);
  // digitalWrite(motorTwo_Enable, 0);
}
void right() {
  // Serial.println("Turn Right");
  digitalWrite(motorOne_InputOne, HIGH);
  digitalWrite(motorOne_InputTwo, LOW);
  digitalWrite(motorTwo_InputOne, HIGH);
  digitalWrite(motorTwo_InputTwo, LOW);
}
void left() {
  // Serial.println("Turn Right");
  digitalWrite(motorOne_InputOne, LOW);
  digitalWrite(motorOne_InputTwo, HIGH);
  digitalWrite(motorTwo_InputOne, LOW);
  digitalWrite(motorTwo_InputTwo, HIGH);
}
void reverse() {
  // Serial.println("Reverse");
  digitalWrite(motorOne_InputOne, LOW);
  digitalWrite(motorOne_InputTwo, HIGH);
  digitalWrite(motorTwo_InputOne, HIGH);
  digitalWrite(motorTwo_InputTwo, LOW);
}


void stop() {
  // Serial.println("Reverse");
  digitalWrite(motorOne_InputOne, LOW);
  digitalWrite(motorOne_InputTwo, LOW);
  digitalWrite(motorTwo_InputOne, LOW);
  digitalWrite(motorTwo_InputTwo, LOW);
}




long microsecondsToInches(double microseconds) {
  return microseconds / 74 / 2;
}


double microsecondsToCentimeters(double microseconds) {
  return microseconds / 29 / 2;
}


void gripLock() {
  if (gripStatus == "locked") {
    return;
  }
  int d = 50;
  int offset = 5;
  // for (int i = offset; i < 180 - offset; i++){
  //   servoLeft.write(180 - offset - i);
  //   servoRight.write(i);
  //   delay(d);
  // }
  servoLeft.write(offset);
  servoRight.write(180 - offset);
  // delay(500);
  stop();
  gripStatus = "locked";
  delay(500);
}


void gripRelease() {
  if (gripStatus == "released") {
    return;
  }
  int d = 50;
  int offset = 15;
  servoLeft.write(180 - offset);
  servoRight.write(offset);
  // delay(500);
  gripStatus = "released";
}




double get_duration(int trig, int echo) {
  // return 1;
  pinMode(trig, OUTPUT);
  digitalWrite(trig, LOW);
  delay(5);
  digitalWrite(trig, HIGH);
  delay(10);
  digitalWrite(trig, LOW);
  pinMode(echo, INPUT);
  // return 1;
  return microsecondsToCentimeters(pulseIn(echo, HIGH));
}


bool switch_pressed() {
  return digitalRead(SWITCH);
}


Timer reverseTimer;
Beep beep;
// scanTimer.setTimeout(1000);


void loop() {
  {
    if (!started) {
      gripLock();
      Serial.println(switch_pressed());
      led_handler.BLUE_ON();
      gripRelease();
      // scanTimer.setTimeout(1000);
      // siren.play();
      reverseTimer.setTimeout(1500);
      started = true;
      beep.setFrequency(400);
      beep.setTimeout(200, 100);
    }
    // analogWrite(motorOne_Enable, 0);     //allows motor one&#39;s speed to go
    // analogWrite(motorTwo_Enable, 0);     //allows motor two&#39;s speed to go
    double duration, cmFront, cmBottom;  //establishes variables for duration of ping
    cmFront = get_duration(trigFront, echoFront);
    cmBottom = get_duration(trigBottom, echoBottom);


    // Serial.println(cmBottom);
    // beep.play();
    // return;
    if (mode == "forward" || mode == "cup") {
      forward();
      if (cmBottom > 10) {
        led_handler.LED_OFF();
        led_handler.RED_ON();
        reverse();
        delay(1500);
        mode = "stop";
        stop();
        delay(1000);
      }
    }
    if (mode == "forward" && mode != "cup"){
      beep.play();
    }
    if (mode == "stop") {
      stop();
      beep.play();
      return;
    }
    if (cmFront < 6) {
      gripLock();
      // led_handler.LED_OFF();
      // tone(buzzer, 100);
      siren.start();
      led_handler.LED_TOGGLE_BLINK();
      mode = "cup";
    } else {
      gripRelease();
      if (mode == "cup") {
        mode = "reverse";
        reverseTimer.reset();
        siren.stop();
        tone(buzzer, 50);
      } else {
        // led_handler.LED_OFF();
        led_handler.BLUE_ON();
        // mode = "forward";
      }
    }
    if (mode == "reverse") {
      reverse();
      tone(buzzer, 150);
      if (reverseTimer.tick()) {
        Serial.println("Tickkkkkkkkkkk");
        // int a = 1 / 0;
        mode = "stop";
        beep.setTimeout(500, 1000);
        noTone(buzzer);
      }


    }
    Serial.println(mode);
    siren.play();
  }
}
