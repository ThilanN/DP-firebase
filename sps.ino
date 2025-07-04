#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>

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
bool gateOpen[NUM_GATES]; // Track gate state
bool lastGateOpen[NUM_GATES] = {false, false}; 


int availableSlots = MAX_PARKING_SLOTS;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < NUM_GATES; i++) {
    pinMode(TRIG_PINS[i], OUTPUT);
    pinMode(ECHO_PINS[i], INPUT);
    pinMode(PIR_PINS[i], INPUT);
    pinMode(LED_RED_PINS[i], OUTPUT);
    pinMode(LED_GREEN_PINS[i], OUTPUT);
    digitalWrite(LED_RED_PINS[i], LOW);    // Red ON (Barrier closed initially)
    digitalWrite(LED_GREEN_PINS[i], HIGH); // Green OFF
    gateOpen[i] = false; // Initialize gates as closed

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
      if (carDetected[i] && !gateOpen[i]) {
        if (availableSlots > 0) {
          gateOpen[i] = true;
          lastGateOpen[i] = true; // <-- Log open state
          openBarrier(i, "ENTRY");
          availableSlots--;
          delay(3000); // Simulate entry time
          closeBarrier(i, "ENTRY");
          gateOpen[i] = false; // Set gate as closed
        } else {
          closeBarrier(i, "ENTRY");
          Serial.println("ENTRY: Car Detected but Parking Full - Access Denied");
        }
      } else if (!carDetected[i] && gateOpen[i]) {
        closeBarrier(i, "ENTRY"); // Ensure gate closes if no car
        gateOpen[i] = false;
      }
    } else {
      // Exit logic
      if (carDetected[i] && !gateOpen[i]) {
        if (getExitPaymentStatus()) { 
          openBarrier(i, "EXIT");
          lastGateOpen[i] = true;
          if (availableSlots < MAX_PARKING_SLOTS) {
            availableSlots++;
          }
          gateOpen[i] = true; // Set gate as open
          delay(3000); // Simulate exit time
          closeBarrier(i, "EXIT");
          gateOpen[i] = false; // Set gate as closed
          resetExitPaymentStatus();
        } else {
          closeBarrier(i, "EXIT");
          Serial.println("EXIT: Payment not completed - Access Denied");
          } 
      }else if (!carDetected[i] && gateOpen[i]) {
        closeBarrier(i, "EXIT"); // gate closes if no car
        gateOpen[i] = false;
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
    Serial.print(lastGateOpen[i] ? 1 : 0); // Use gateOpen state for accuracy
    Serial.println();
    lastGateOpen[i] = false;

    // Status for debugging
    Serial.print("GATE: ");
    Serial.print(i == 0 ? "ENTRY" : "EXIT");
    Serial.print(" | Distance: ");
    Serial.print(distance[i]);
    Serial.print(" cm | Motion: ");
    Serial.print(motion);
    Serial.print(" | Detected: ");
    Serial.print(carDetected[i]);
    Serial.print(" | Gate Open: ");
    Serial.print(gateOpen[i]);
    Serial.print(" | Available Slots: ");
    Serial.println(availableSlots);
  }

  delay(5000); // Loop delay
}

// For Common Anode RGB LED (corrected logic for common anode)
void openBarrier(int index, const char* gateName) {
  servos[index].write(90);  
  digitalWrite(LED_RED_PINS[index], HIGH);    // Red OFF
  digitalWrite(LED_GREEN_PINS[index], LOW);   // Green ON
  Serial.print(gateName);
  Serial.println(" Barrier Opened");
}

void closeBarrier(int index, const char* gateName) {
  servos[index].write(0);
  digitalWrite(LED_RED_PINS[index], LOW);     // Red ON
  digitalWrite(LED_GREEN_PINS[index], HIGH);  // Green OFF
  Serial.print(gateName);
  Serial.println(" Barrier Closed");
}


bool getExitPaymentStatus() {
  return true;  // Change this to false to simulate not paid
}

void resetExitPaymentStatus() {
  Serial.println("EXIT payment status reset to false (mock)");
}
