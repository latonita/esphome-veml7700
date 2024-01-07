#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "esphome/core/optional.h"

namespace esphome {
namespace veml7700 {

// https://www.vishay.com/docs/84286/veml7700.pdf

enum VEML7700CommandRegisters {
  VEML7700_CR_ALS_CONF_0 = 0x00,  // W: ALS gain, integration time, interrupt, and shutdown
  VEML7700_CR_ALS_WH = 0x01,      // W: ALS high threshold window setting
  VEML7700_CR_ALS_WL = 0x02,      // W: ALS low threshold window setting
  VEML7700_CR_PWR_SAVING = 0x03,  // W: Set (15 : 3) 0000 0000 0000 0b
  VEML7700_CR_ALS = 0x04,         // R: MSB, LSB data of whole ALS 16 bits
  VEML7700_CR_WHITE = 0x05,       // R: MSB, LSB data of whole WHITE 16 bits
  VEML7700_CR_ALS_INT = 0x06      // R: ALS INT trigger event
};

#pragma pack(1)
//
// VEML7700_CR_ALS_CONF_0 Register (0x00)
//
union VEML7700ConfigurationRegister {
  uint16_t raw;
  uint8_t raw_bytes[2];
  struct {
    bool ALS_SD : 1;       // ALS shut down setting: 0 = ALS power on, 1 = ALS shut down
    bool ALS_INT_EN : 1;   // ALS interrupt enable setting: 0 = ALS INT disable, 1 = ALS INT enable
    bool reserved_2 : 1;   // 0
    bool reserved_3 : 1;   // 0
    uint8_t ALS_PERS : 2;  // 00 - 1, 01- 2, 10 - 4, 11 - 8
    uint8_t ALS_IT : 4;    // ALS integration time setting
    bool reserved_10 : 1;  // 0
    uint8_t ALS_GAIN : 2;  // Gain selection
    bool reserved_13 : 1;  // 0
    bool reserved_14 : 1;  // 0
    bool reserved_15 : 1;  // 0
  };
};

//
// Power Saving Mode: PSM Register (0x03)
//
union VEML7700PSMRegister {
  uint16_t raw;
  struct {
    bool PSM_EN : 1;
    uint8_t PSM : 2;
    uint16_t reserved : 13;
  };
};

#pragma pack(0)

// Sensor gain levels
enum VEML7700Gain {
  VEML7700_GAIN_1 = 0,  // default
  VEML7700_GAIN_2 = 1,
  VEML7700_GAIN_1_8 = 2,
  VEML7700_GAIN_1_4 = 3,
};
const uint8_t VEML7700_GAIN_COUNT = 4;

enum VEML7700IntegrationTime {
  VEML7700_INTEGRATION_TIME_25MS = 0b1100,  // 12
  VEML7700_INTEGRATION_TIME_50MS = 0b1000,  // 8
  VEML7700_INTEGRATION_TIME_100MS = 0b0000,
  VEML7700_INTEGRATION_TIME_200MS = 0b0001,
  VEML7700_INTEGRATION_TIME_400MS = 0b0010,
  VEML7700_INTEGRATION_TIME_800MS = 0b0011,
};
const uint8_t VEML7700_IT_TIME_COUNT = 6;

enum VEML7700Persistence {
  VEML7700_PERSISTENCE_1 = 0,
  VEML7700_PERSISTENCE_2 = 1,
  VEML7700_PERSISTENCE_4 = 2,
  VEML7700_PERSISTENCE_8 = 3,
};

enum VEML7700PSM {
  VEML7700_PSM_MODE_1 = 0,
  VEML7700_PSM_MODE_2 = 1,
  VEML7700_PSM_MODE_3 = 2,
  VEML7700_PSM_MODE_4 = 3,
};

class VEML7700Component : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void dump_config() override;
  void update() override;

  void set_gain(VEML7700Gain gain) { this->gain_ = gain; }
  void set_integration_time(VEML7700IntegrationTime time) { this->integration_time_ = time; }
  void set_attenuation_factor(float factor) { this->attenuation_factor_ = factor; }

  void set_ambient_light_sensor(sensor::Sensor *sensor) { this->ambient_light_sensor_ = sensor; }
  void set_white_sensor(sensor::Sensor *sensor) { this->white_sensor_ = sensor; }

 protected:
  esphome::i2c::ErrorCode configure_();
  bool read_sensor_data_();

  uint8_t map_time_to_index_(VEML7700IntegrationTime it);
  float calculate_lux_(uint16_t counts, VEML7700Gain gain, VEML7700IntegrationTime time);

  bool reading_{false};

  float als_lux_{0};
  float white_lux_{0};

  VEML7700Gain gain_{VEML7700_GAIN_1};
  VEML7700IntegrationTime integration_time_{VEML7700_INTEGRATION_TIME_100MS};
  float attenuation_factor_{1.0};

  sensor::Sensor *ambient_light_sensor_{nullptr};  // ALS high resolution channel - visible light
  sensor::Sensor *white_sensor_{nullptr};          // White channel - wide range
};

}  // namespace veml7700
}  // namespace esphome
