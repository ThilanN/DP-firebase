#include <ESP32Servo.h>

#define ENTRY_INDEX 0
#define EXIT_INDEX 1
#define NUM_GATES 2
#define MAX_PARKING_SLOTS 5

const uint8_t TRIG_PINS[] = {5, 23};      // Entry, Exit
const uint8_t ECHO_PINS[] = {18, 19};
const uint8_t PIR_PINS[] = {21, 22};
const uint8_t SERVO_PINS[] = {13, 25};
const uint8_t LED_RED_PINS[] = {14, 26};
const uint8_t LED_GREEN_PINS[] = {27, 33};

long duration[NUM_GATES];
int distance[NUM_GATES];
bool carDetected[NUM_GATES];
Servo servos[NUM_GATES];

int availableSlots = MAX_PARKING_SLOTS;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < NUM_GATES; i++) {
    pinMode(TRIG_PINS[i], OUTPUT);
    pinMode(ECHO_PINS[i], INPUT);
    pinMode(PIR_PINS[i], INPUT);
    pinMode(LED_RED_PINS[i], OUTPUT);
    pinMode(LED_GREEN_PINS[i], OUTPUT);
    digitalWrite(LED_RED_PINS[i], HIGH);    // Red ON (Barrier closed initially)
    digitalWrite(LED_GREEN_PINS[i], LOW);   // Green OFF

    ESP32PWM::allocateTimer(i);
    servos[i].setPeriodHertz(50);
    servos[i].attach(SERVO_PINS[i], 500, 2500);
    servos[i].write(0); // Barrier down initially
  }

  Serial.println("Smart Parking System with Entry/Exit Started");
  Serial.print("Total Available Slots: ");
  Serial.println(availableSlots);
}

void loop() {
  for (int i = 0; i < NUM_GATES; i++) {
    digitalWrite(TRIG_PINS[i], LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PINS[i], HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PINS[i], LOW);

    duration[i] = pulseIn(ECHO_PINS[i], HIGH);
    distance[i] = duration[i] * 0.034 / 2;
    bool motion = digitalRead(PIR_PINS[i]);
    carDetected[i] = (distance[i] < 50) || motion;

    if (i == ENTRY_INDEX) {
      // Entry logic
      if (carDetected[i]) {
        if (availableSlots > 0) {
          openBarrier(i, "ENTRY");
          availableSlots--;
          delay(3000); // Simulate entry time
          closeBarrier(i, "ENTRY");
        } else {
          // Available slots == 0, so explicitly force red ON and green OFF
          digitalWrite(LED_RED_PINS[i], HIGH);
          digitalWrite(LED_GREEN_PINS[i], LOW);
          servos[i].write(0); // Ensure barrier stays down
          Serial.println("ENTRY: Car Detected but Parking Full - Access Denied");
        }
      } else {
        closeBarrier(i, "ENTRY");
      }
    } else {
      // Exit logic
      if (carDetected[i]) {
        openBarrier(i, "EXIT");
        if (availableSlots < MAX_PARKING_SLOTS) {
          availableSlots++;
        }
        delay(3000); // Simulate exit time
        closeBarrier(i, "EXIT");
      } else {
        closeBarrier(i, "EXIT");
      }
    }

    // Output in DATA format with gate identifier
    Serial.print("DATA,");
    Serial.print(i == 0 ? "Entry" : "Exit");
    Serial.print(",");
    Serial.print(distance[i]);
    Serial.print(",");
    Serial.print(motion);
    Serial.print(",");
    Serial.print(!carDetected[i]); // Gate open when no car detected
    Serial.println();

    // Status for debugging
    Serial.print("GATE: ");
    Serial.print(i == 0 ? "ENTRY" : "EXIT");
    Serial.print(" | Distance: ");
    Serial.print(distance[i]);
    Serial.print(" cm | Motion: ");
    Serial.print(motion);
    Serial.print(" | Detected: ");
    Serial.print(carDetected[i]);
    Serial.print(" | Available Slots: ");
    Serial.println(availableSlots);
  }

  delay(5000); // Loop delay
}

// For Common Anode RGB LED
void openBarrier(int index, const char* gateName) {
  servos[index].write(90);  
  digitalWrite(LED_RED_PINS[index], HIGH);    // OFF 
  digitalWrite(LED_GREEN_PINS[index], LOW);   // ON
  Serial.print(gateName);
  Serial.println(" Barrier Opened");
}

void closeBarrier(int index, const char* gateName) {
  servos[index].write(0);
  digitalWrite(LED_RED_PINS[index], LOW);     // ON
  digitalWrite(LED_GREEN_PINS[index], HIGH);  // OFF
  Serial.print(gateName);
  Serial.println(" Barrier Closed");
}