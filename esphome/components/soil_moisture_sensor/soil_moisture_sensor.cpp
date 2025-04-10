#include "esphome/core/log.h"
#include "soil_moisture_sensor.h"
#include <Arduino.h>
#include <TickTwo.h>
#include <vector>

namespace esphome {
namespace soil_moisture_sensor {

// Define pins for various functions
const int capacitancePin = GPIO_NUM_6;

// Capacitance measurement
std::vector<int>
    rawValues;            // Vector to store raw capacitance measurement values
hw_timer_t *timer = NULL; // Hardware timer pointer
int median = 0;
int soilMoisture = 0;

static const char *TAG =
    "soil_moisture_sensor.sensor"; // Define a tag for logging

void SoilMoistureSensor::setup() {
  // Configure pin modes
  pinMode(capacitancePin, INPUT);

  // Set up timers for capacitance measurement
  timer = timerBegin(
      0, 2, true); // Initialize hardware timer with prescaler 2 and auto-reload

  // Set up ticker for measurement
  measurementTicker =
      new TickTwo([this]() { this->startMeasurement(); }, 10, 0, MILLIS);

  measurementTicker->start();

  // Set up ticker for calculating moisture
  moistureCalculationTicker =
      new TickTwo([this]() { this->calculateMoisture(); }, 600, 0, MILLIS);

  moistureCalculationTicker->start();
}

void SoilMoistureSensor::loop() {
  measurementTicker->update();
  moistureCalculationTicker->update();
}

void SoilMoistureSensor::update() { publish_state(soilMoisture); }

void SoilMoistureSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Soil Moisture Sensor");
}

int SoilMoistureSensor::getMedian(const std::vector<int> &values) {
  std::vector<int> temp = values; // Create a copy of the input vector
  std::sort(temp.begin(),
            temp.end());        // Sort the copied vector in ascending order
  return temp[temp.size() / 2]; // Return the middle element (median)
}

// Start charge up time measurement
void SoilMoistureSensor::startMeasurement() {
  pinMode(capacitancePin, OUTPUT);
  digitalWrite(capacitancePin,
               LOW); // Set the pin low to discharge the capacitor
  delayMicroseconds(50);
  noInterrupts(); // Disable interrupts to ensure accurate timing
  pinMode(capacitancePin,
          INPUT);      // Set the pin back to input to measure charge time
  timerRestart(timer); // Restart the hardware timer
  while (digitalRead(capacitancePin) ==
         LOW) // Wait until the pin goes high (capacitor is charged)
    ;
  int rawValue = timerRead(timer); // Read the timer value (charge time)
  interrupts();                    // Enable interrupts

  rawValues.push_back(rawValue); // Add the raw value to the vector
  if (rawValues.size() > 50) {   // If the vector has more than 50 elements
    rawValues.erase(rawValues.begin()); // Remove the oldest element
  }
};

// Calculate soil moisture from measurement
void SoilMoistureSensor::calculateMoisture() {
  if (rawValues.size() == 0) {
    soilMoisture = 0; // If no raw values are available
    return;           // Return a default value
  }

  median = getMedian(rawValues); // Calculate the median of the raw values
  soilMoisture = map(median, 800, 18000, 0,
                     100); // Map the median value to a moisture percentage
                           // (0-100) using linear interpolation
  soilMoisture =
      constrain(soilMoisture, 0, 100); // Ensure value stays within 0-100
}

} // namespace soil_moisture_sensor
} // namespace esphome