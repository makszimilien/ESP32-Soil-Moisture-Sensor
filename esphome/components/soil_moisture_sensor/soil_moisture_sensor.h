#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include <TickTwo.h>

namespace esphome {
namespace soil_moisture_sensor {

class SoilMoistureSensor : public sensor::Sensor, public PollingComponent {
public:
  void setup() override;
  void loop() override;
  void update() override;
  void dump_config() override;

private:
  int getMedian(const std::vector<int> &values);
  void startMeasurement();
  void calculateMoisture();
  TickTwo *measurementTicker;
  TickTwo *moistureCalculationTicker;
};

} // namespace soil_moisture_sensor
} // namespace esphome