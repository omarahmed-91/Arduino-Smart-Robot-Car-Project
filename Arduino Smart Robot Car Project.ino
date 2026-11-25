
#include <Servo.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

// Bluetooth HC-05
// Arduino RX = D13
// Arduino TX = D12
SoftwareSerial Bluetooth(13, 12);

// LCD 
// RS, E, D4, D5, D6, D7
LiquidCrystal lcd(A3, A4, A5, A0, 4, A1);

// Motor Driver L298N
#define IN1 10
#define IN2 9
#define IN3 8
#define IN4 7
#define ENA 11
#define ENB 6

// Motor speeds
#define AutoSpeed 65
#define BluetoothSpeed 80

int currentSpeed = AutoSpeed;

// Ultrasonic Sensor
#define Echo 2
#define Trig 3

// Servo Motor
#define ServoPin 5
Servo myservo;

int spoint = 85;

// Button
#define Button A2

// Modes
bool isObstacleMode = false;
bool isBluetoothMode = false;

// Button variables
bool lastButtonState = HIGH;
bool buttonHandled = false;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;




void setup() {

  Serial.begin(9600);
  Bluetooth.begin(9600);

  lcd.begin(16, 2);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);

  pinMode(Button, INPUT_PULLUP);

  myservo.attach(ServoPin);
  myservo.write(spoint);

  Stop();

  lcd.setCursor(0, 0);
  lcd.print("Robot Car");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");

  delay(1500);

  displayMode();

  Serial.println("Robot Car Ready");
}



void loop() {

  checkButton();

  if (isObstacleMode) {

    currentSpeed = AutoSpeed;
    Obstacle();

  }

  else if (isBluetoothMode) {

    currentSpeed = BluetoothSpeed;
    Bluetoothcontrol();

  }

  else {

    Stop();

  }
}

// Display Mode on LCD

void displayMode() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Mode:");

  lcd.setCursor(0, 1);

  if (isObstacleMode) {
    lcd.print("OBSTACLE Eco MODE");
  }

  else if (isBluetoothMode) {
    lcd.print("BLUETOOTH MODE");
  }

  else {
    lcd.print("STOP");
  }
}

// Button Control

void checkButton() {

  bool reading = digitalRead(Button);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if (millis() - lastDebounceTime > debounceDelay) {

    if (reading == LOW && !buttonHandled) {

      buttonHandled = true;

      Stop();

      if (!isObstacleMode && !isBluetoothMode) {

        isObstacleMode = true;
        isBluetoothMode = false;

        Serial.println("OBSTACLE MODE");
      }

      else if (isObstacleMode) {

        isObstacleMode = false;
        isBluetoothMode = true;

        Serial.println("BLUETOOTH MODE");
      }

      else {

        isObstacleMode = false;
        isBluetoothMode = false;

        Serial.println("STOP MODE");
      }

      displayMode();
    }

    if (reading == HIGH) {
      buttonHandled = false;
    }
  }

  lastButtonState = reading;
}


// Obstacle Avoidance Mode

void Obstacle() {

  int distance = ultrasonic();

  if (distance > 25) {

    forward();

  }

  else {

    Stop();
    delay(200);

    backward();
    delay(200);

    Stop();
    delay(200);

    int rightDistance = rightsee();

    delay(200);

    int leftDistance = leftsee();

    delay(200);

    myservo.write(spoint);
    delay(200);

    if (rightDistance > leftDistance) {

      right();
      delay(350);

    }

    else {

      left();
      delay(350);

    }

    Stop();
    delay(200);
  }
}


// Ultrasonic Sensor

int ultrasonic() {

  digitalWrite(Trig, LOW);
  delayMicroseconds(2);

  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);

  digitalWrite(Trig, LOW);

  long duration = pulseIn(Echo, HIGH, 30000);

  if (duration == 0) {
    return 400;
  }

  int distance = duration * 0.034 / 2;

  return distance;
}


// Servo Right Scan

int rightsee() {

  myservo.write(20);
  delay(500);

  int distance = ultrasonic();

  return distance;
}


// Servo Left Scan

int leftsee() {

  myservo.write(180);
  delay(500);

  int distance = ultrasonic();

  return distance;
}


// Bluetooth Control

void Bluetoothcontrol() {

  if (Bluetooth.available()) {

    char command = Bluetooth.read();

    Serial.println(command);

    switch (command) {

      case 'U':
      case 'F':
        forward();
        break;

      case 'D':
      case 'B':
        backward();
        break;

      case 'L':
        left();
        break;

      case 'R':
        right();
        break;

      case 'S':
        Stop();
        break;

      default:
        break;
    }
  }
}


// Motor Direction

void setDir(bool aFwd, bool bFwd) {

  digitalWrite(IN1, aFwd ? LOW : HIGH);
  digitalWrite(IN2, aFwd ? HIGH : LOW);

  digitalWrite(IN3, bFwd ? HIGH : LOW);
  digitalWrite(IN4, bFwd ? LOW : HIGH);
}

// Forward
void forward() {

  setDir(true, true);

  analogWrite(ENA, currentSpeed);
  analogWrite(ENB, currentSpeed);
}

// Backward
void backward() {

  setDir(false, false);

  analogWrite(ENA, currentSpeed);
  analogWrite(ENB, currentSpeed);
}

// Right

void right() {

  setDir(true, false);

  analogWrite(ENA, currentSpeed);
  analogWrite(ENB, currentSpeed);
}

// Left

void left() {

  setDir(false, true);

  analogWrite(ENA, currentSpeed);
  analogWrite(ENB, currentSpeed);
}

// Stop
void Stop() {

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
