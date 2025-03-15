import esphome.codegen as cg  # Import ESPHome code generation utilities
import esphome.config_validation as cv  # Import ESPHome configuration validation utilities
from esphome.components import sensor  # Import ESPHome sensor component definitions
from esphome.const import ICON_EMPTY  # Import ESPHome constant for an empty icon

soil_moisture_sensor_ns = cg.esphome_ns.namespace("soil_moisture_sensor")  # Define a namespace for the custom component

SoilMoistureSensor = soil_moisture_sensor_ns.class_("SoilMoistureSensor", sensor.Sensor, cg.PollingComponent)  # Define the SoilMoistureSensor class, inheriting from sensor.Sensor and cg.PollingComponent

CONFIG_SCHEMA = sensor.sensor_schema(  # Define the configuration schema for the sensor
    SoilMoistureSensor, unit_of_measurement="%", icon=ICON_EMPTY, accuracy_decimals=0  # Specify unit, icon, and accuracy
).extend(cv.polling_component_schema("2s"))  # Extend the schema with polling component options, polling every 2 seconds

async def to_code(config):  # Define the function to generate C++ code from the configuration
    var = await sensor.new_sensor(config)  # Create a new sensor object from the configuration
    await cg.register_component(var, config)  # Register the sensor component with ESPHome

    cg.add(var.update()) # Add code to update the sensor's value during polling.