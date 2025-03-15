#include "esphome/core/log.h" // Include ESPHome logging functionality
#include "soil_moisture_sensor.h" // Include the header file for this custom component
#include <Arduino.h>              // Include Arduino core functions
#include <TickTwo.h> // Include TickTwo library for non-blocking timers
#include <vector>    // Include vector library for dynamic arrays

namespace esphome {
namespace soil_moisture_sensor {

// Define pins for various functions
const int capacitancePin =
    GPIO_NUM_6; // Define the GPIO pin used for capacitance measurement

// Capacitance measurement
std::vector<int>
    rawValues;            // Vector to store raw capacitance measurement values
hw_timer_t *timer = NULL; // Hardware timer pointer
int median = 0;           // Variable to store the median of raw values
int soilMoisture = 200;   // Variable to store the calculated soil moisture
                          // percentage, initialized to a default value.

static const char *TAG =
    "soil_moisture_sensor.sensor"; // Define a tag for logging

void SoilMoistureSensor::setup() {
  // Configure pin modes
  pinMode(capacitancePin, INPUT); // Configure the capacitance pin as input

  // Set up timers for capacitance measurement
  timer = timerBegin(
      0, 2, true); // Initialize hardware timer with prescaler 2 and auto-reload

  // Set up ticker for measurement
  measurementTicker =
      new TickTwo([this]() { this->startMeasurement(); }, 20, 0,
                  MILLIS); // Create a TickTwo timer to trigger measurements
                           // every 100 milliseconds

  measurementTicker->start(); // Start the TickTwo timer
}

void SoilMoistureSensor::loop() {
  measurementTicker->update();
} // Update the TickTwo timer in the main loop

void SoilMoistureSensor::update() {
  publish_state(calculateMoisture());
} // Update the sensor state with the calculated moisture value

void SoilMoistureSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Soil Moisture Sensor"); // Log the component configuration
}

int SoilMoistureSensor::getMedian(const std::vector<int> &values) {
  std::vector<int> temp = values; // Create a copy of the input vector
  std::sort(temp.begin(),
            temp.end());        // Sort the copied vector in ascending order
  return temp[temp.size() / 2]; // Return the middle element (median)
}

// Start charge up time measurement
void SoilMoistureSensor::startMeasurement() {
  pinMode(capacitancePin, OUTPUT); // Set the capacitance pin as output
  digitalWrite(capacitancePin,
               LOW);     // Set the pin low to discharge the capacitor
  delayMicroseconds(50); // Delay for a short time
  noInterrupts();        // Disable interrupts to ensure accurate timing
  pinMode(capacitancePin,
          INPUT);      // Set the pin back to input to measure charge time
  timerRestart(timer); // Restart the hardware timer
  while (digitalRead(capacitancePin) ==
         LOW) // Wait until the pin goes high (capacitor is charged)
    ;
  int rawValue = timerRead(timer); // Read the timer value (charge time)
  interrupts();                    // Enable interrupts

  rawValues.push_back(rawValue); // Add the raw value to the vector
  if (rawValues.size() > 100) {  // If the vector has more than 20 elements
    rawValues.erase(rawValues.begin()); // Remove the oldest element to keep the
                                        // vector size limited
  }
};

// Calculate water level from measurement
int SoilMoistureSensor::calculateMoisture() {
  if (rawValues.size() == 0) { // If no raw values are available
    return 200;                // Return a default value
  }

  median = getMedian(rawValues); // Calculate the median of the raw values
  soilMoisture = map(median, 800, 18000, 0,
                     100); // Map the median value to a moisture percentage
                           // (0-100) using linear interpolation
  soilMoisture =
      constrain(soilMoisture, 0, 100); // Ensure value stays within 0-100

  return soilMoisture; // Return the calculated moisture percentage
}

} // namespace soil_moisture_sensor
} // namespace esphome