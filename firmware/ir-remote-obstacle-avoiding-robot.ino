#include <IRremote.h> 
 
// IR receiver pin 
#define IR_RECEIVE_PIN 4 
 
// Motor driver pins 
#define ENA 5 
#define ENB 6 
#define IN1 7 
#define IN2 8 
#define IN3 9 
#define IN4 10 
 
// Lights and buzzer pins 
#define FRONT_LIGHT 2 
#define BACK_LIGHT 3 
#define REVERSE_BUZZER 11 
#define HORN 12 
 
// IR distance sensor pin 
#define IR_SENSOR 13 
 
// IR code values 
#define CMD_FORWARD 72 
#define CMD_BACKWARD 85 
#define CMD_RIGHT 8 
#define CMD_LEFT 18 
#define CMD_STOP 89 
#define CMD_FRONT_LIGHT_TOGGLE 71 
#define CMD_ALL_LIGHTS 7 
#define CMD_SPEED_HIGH 94 
#define CMD_SPEED_MEDIUM 6 
#define CMD_SPEED_LOW 5 
#define CMD_POWER 9 
#define CMD_HORN 64 
#define CMD_FOLLOW_MODE 26      // Follow mode 
#define CMD_IR_MODE 14          // IR mode 
 
// State variables 
int currentMode = 0;    // 0: IR, 1: Follow 
bool frontLightState = false; 
bool backLightManual = false;   // Manual backlight toggle 
bool allLightsState = false; 
bool powerState = true;   // Car starts ON 

bool hornState = false; 
int speedValue = 200; 
int lastCommand = CMD_STOP; 
bool followStopped = false;  // Tracks if Follow mode is stopped 
 
void setup() { 
  Serial.begin(9600); // For debugging (optional) 
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); 
 
  pinMode(ENA, OUTPUT); 
  pinMode(ENB, OUTPUT); 
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT); 
  pinMode(IN3, OUTPUT); 
  pinMode(IN4, OUTPUT); 
  pinMode(FRONT_LIGHT, OUTPUT); 
  pinMode(BACK_LIGHT, OUTPUT); 
  pinMode(REVERSE_BUZZER, OUTPUT); 
  pinMode(HORN, OUTPUT); 
  pinMode(IR_SENSOR, INPUT);  // IR distance sensor as input (digital) 
 
  analogWrite(ENA, speedValue); 
  analogWrite(ENB, speedValue); 
 
  stopCar(); 
} 
 
void loop() { 
  // --- Check for IR commands --- 
  if (IrReceiver.decode()) { 
    int command = IrReceiver.decodedIRData.command; 
    Serial.print("IR: "); 
    Serial.println(command); 
 
    // Handle mode switching and power command (always active) 
    if (command == CMD_FOLLOW_MODE) { 
      currentMode = 1; 
      followStopped = false;  // Reset stop state when entering Follow mode 
      stopCar(); 
      Serial.println("Switched to FOLLOW MODE"); 
    } else if (command == CMD_IR_MODE) { 
      currentMode = 0; 
      followStopped = false;  // Reset stop state 
      stopCar(); 
      Serial.println("Switched to IR MODE"); 
    } else if (command == CMD_POWER) { 
      powerState = !powerState; 
      followStopped = false;  // Reset stop state 

      if (!powerState) { 
        stopCar(); 
        digitalWrite(FRONT_LIGHT, LOW); 
        digitalWrite(BACK_LIGHT, LOW); 
        digitalWrite(HORN, LOW); 
      } 
      Serial.print("Power State: "); 
      Serial.println(powerState ? "ON" : "OFF"); 
    } else if (command == CMD_STOP && powerState) { 
      // Stop command works in both modes 
      if (currentMode == 1) { 
        followStopped = !followStopped;  // Toggle stop state in Follow mode 
        if (followStopped) { 
          stopCar(); 
          Serial.println("Follow mode STOPPED"); 
        } else { 
          Serial.println("Follow mode RESUMED"); 
        } 
      } else { 
        stopCar(); 
        lastCommand = CMD_STOP;  // Stop in IR mode 
      } 
    } else if (currentMode == 0 && powerState) { 
      // Handle other IR commands in IR mode when power is ON 
      handleCommand(command); 
    } 
    IrReceiver.resume(); 
  } 
 
  // --- Handle Follow mode --- 
  if (currentMode == 1 && powerState && !followStopped) { 
    handleFollow(); 
  } 
} 
 
// == Command Handlers == 
 
// IR movement/lights/horn/speed command handler (IR mode only) 
void handleCommand(int command) { 
  switch (command) { 
    case CMD_FORWARD:    forward(); lastCommand = CMD_FORWARD; break; 
    case CMD_BACKWARD:   backward(); lastCommand = CMD_BACKWARD; break; 
    case CMD_RIGHT:      left(); lastCommand = CMD_LEFT; break;   // Intentionally mapped 
    case CMD_LEFT:       right(); lastCommand = CMD_RIGHT; break; // Intentionally mapped 
    case CMD_STOP:       stopCar(); lastCommand = CMD_STOP; break; // Already handled 
above 
    case CMD_FRONT_LIGHT_TOGGLE: toggleFrontLight(); break; 
    case CMD_ALL_LIGHTS: toggleAllLights(); break; 

    case CMD_HORN:       toggleHorn(); break; 
    case CMD_SPEED_HIGH: setSpeed(255); break; 
    case CMD_SPEED_MEDIUM: setSpeed(180); break; 
    case CMD_SPEED_LOW:  setSpeed(100); break; 
  } 
} 
 
// Follow mode handler (using digital IR sensor) 
void handleFollow() { 
  int sensorState = digitalRead(IR_SENSOR); 
  Serial.print("IR Sensor State: "); 
  Serial.println(sensorState); 
 
  if (sensorState == LOW) {  // Hand far (LOW signal, e.g., >30cm) 
    forward();  // Move forward to follow hand 
  } else {  // Hand close (HIGH signal, e.g., <10cm) 
    backward();  // Move backward to maintain distance 
  } 
 
  delay(100);  // Small delay to stabilize readings; adjust as needed 
} 
 
// === Movement Functions === 
void forward() { 
  analogWrite(ENA, speedValue); 
  analogWrite(ENB, speedValue); 
  digitalWrite(IN1, HIGH); 
  digitalWrite(IN2, LOW); 
  digitalWrite(IN3, HIGH); 
  digitalWrite(IN4, LOW); 
 
  updateBackLight(false); // Back light OFF in forward (unless manual ON) 
  digitalWrite(REVERSE_BUZZER, LOW); 
} 
 
void backward() { 
  analogWrite(ENA, speedValue); 
  analogWrite(ENB, speedValue); 
  digitalWrite(IN1, LOW); 
  digitalWrite(IN2, HIGH); 
  digitalWrite(IN3, LOW); 
  digitalWrite(IN4, HIGH); 
 
  updateBackLight(true); // Back light ON when reversing 
  digitalWrite(REVERSE_BUZZER, HIGH); 
} 
 
void left() { 

  analogWrite(ENA, speedValue); 
  analogWrite(ENB, speedValue); 
  digitalWrite(IN1, LOW); 
  digitalWrite(IN2, HIGH); 
  digitalWrite(IN3, HIGH); 
  digitalWrite(IN4, LOW); 
 
  updateBackLight(false); 
  digitalWrite(REVERSE_BUZZER, LOW); 
} 
 
void right() { 
  analogWrite(ENA, speedValue); 
  analogWrite(ENB, speedValue); 
  digitalWrite(IN1, HIGH); 
  digitalWrite(IN2, LOW); 
  digitalWrite(IN3, LOW); 
  digitalWrite(IN4, HIGH); 
 
  updateBackLight(false); 
  digitalWrite(REVERSE_BUZZER, LOW); 
} 
 
void stopCar() { 
  digitalWrite(IN1, LOW); 
  digitalWrite(IN2, LOW); 
  digitalWrite(IN3, LOW); 
  digitalWrite(IN4, LOW); 
 
  updateBackLight(false); 
  digitalWrite(REVERSE_BUZZER, LOW); 
} 
 
// === Lights & Horn === 
void toggleFrontLight() { 
  frontLightState = !frontLightState; 
  digitalWrite(FRONT_LIGHT, frontLightState ? HIGH : LOW); 
} 
 
void toggleBackLightManual() { 
  backLightManual = !backLightManual; 
  digitalWrite(BACK_LIGHT, backLightManual ? HIGH : LOW); 
} 
 
void updateBackLight(bool reversing) { 
  if (reversing) { 
    digitalWrite(BACK_LIGHT, HIGH); // Always ON in reverse 
  } else { 

    digitalWrite(BACK_LIGHT, backLightManual ? HIGH : LOW); // Respect manual toggle 
  } 
} 
 
void toggleAllLights() { 
  allLightsState = !allLightsState; 
  digitalWrite(FRONT_LIGHT, allLightsState ? HIGH : LOW); 
  digitalWrite(BACK_LIGHT, allLightsState ? HIGH : LOW); 
} 
 
void toggleHorn() { 
  hornState = !hornState; 
  digitalWrite(HORN, hornState ? HIGH : LOW); 
} 
 
void setSpeed(int val) { 
  speedValue = val; 
  analogWrite(ENA, speedValue); 
  analogWrite(ENB, speedValue); 
}