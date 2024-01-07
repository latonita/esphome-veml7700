#include "veml7700.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"

namespace esphome {
namespace veml7700 {

static const char *const TAG = "veml7700";

static const float RESOLUTION_MAP[VEML7700_GAIN_COUNT][VEML7700_IT_TIME_COUNT] = {
    // 100ms   200ms   400ms   800ms   25ms    50ms
    {0.0576, 0.0288, 0.0144, 0.0072, 0.2304, 0.1152},  // X1
    {0.0288, 0.0144, 0.0072, 0.0036, 0.1152, 0.0576},  // X2
    {0.4608, 0.2304, 0.1152, 0.0576, 1.8432, 0.9216},  // X/8
    {0.2304, 0.1152, 0.0576, 0.0288, 0.9216, 0.4608}   // X/4
};

static const char *VEML7700_GAIN_STR[VEML7700_GAIN_COUNT] = {"x1", "x2", "x1/8", "x1/4"};
static const uint16_t VEML7700_IT_TIME[VEML7700_IT_TIME_COUNT] = {100, 200, 400, 800, 25, 50};

uint8_t VEML7700Component::map_time_to_index_(VEML7700IntegrationTime it) {
  uint8_t it_index = 0;
  if (it <= VEML7700_INTEGRATION_TIME_800MS) {
    it_index = it;
  } else if (it == VEML7700_INTEGRATION_TIME_25MS) {
    it_index = 4;
  } else if (it == VEML7700_INTEGRATION_TIME_50MS) {
    it_index = 5;
  } else
    it_index = 0;
  return it_index < VEML7700_IT_TIME_COUNT ? it_index : 0;
}

bool VEML7700Component::read_sensor_data_() {
  ESP_LOGD(TAG, "Reading sensor data");
  esphome::i2c::ErrorCode err;
  uint16_t als_couts{0};
  uint16_t white_couts{0};

  err = this->read_register(VEML7700CommandRegisters::VEML7700_CR_ALS, (uint8_t *) &als_couts, 2, false);
  if (err)
    ESP_LOGW(TAG, "Error reading ALS register");

  err = this->read_register(VEML7700CommandRegisters::VEML7700_CR_WHITE, (uint8_t *) &white_couts, 2, false);
  if (err)
    ESP_LOGW(TAG, "Error reading WHITE register");

  float lux_resolution = RESOLUTION_MAP[this->gain_ & 0b11][this->map_time_to_index_(this->integration_time_)];
  ESP_LOGD(TAG, "Resolution lx/counts = %.4f", lux_resolution);

  this->als_lux_ = lux_resolution * (float) als_couts;
  this->white_lux_ = lux_resolution * (float) white_couts;

  ESP_LOGD(TAG, "ALS counts = %d, lux = %.1f", als_couts, this->als_lux_);
  ESP_LOGD(TAG, "WHITE counts = %d, lux = %.1f ", white_couts, this->white_lux_);

  uint32_t als_lux = 0;

  if (this->ambient_light_sensor_ != nullptr) {
    this->ambient_light_sensor_->publish_state(this->als_lux_);
  }

  if (this->white_sensor_ != nullptr) {
    this->white_sensor_->publish_state(this->white_lux_);
  }

  return true;
}

esphome::i2c::ErrorCode VEML7700Component::configure_() {
  ESP_LOGD(TAG, "Configure");
  esphome::i2c::ErrorCode err;

  VEML7700ConfigurationRegister als_conf{0};
  als_conf.raw = 0;
  als_conf.ALS_SD = false;
  als_conf.ALS_INT_EN = false;
  als_conf.ALS_PERS = VEML7700Persistence::VEML7700_PERSISTENCE_1;
  als_conf.ALS_IT = this->integration_time_;
  als_conf.ALS_GAIN = this->gain_;
  ESP_LOGD(TAG, "Setting ALS_CONF_0 to 0x%04X", als_conf.raw);
  err = this->write_register(VEML7700CommandRegisters::VEML7700_CR_ALS_CONF_0, als_conf.raw_bytes, 2);
  if (!err)
    return err;

  uint16_t threshold_low = 0x0000;  // set low threshold to min
  err = this->write_register(VEML7700CommandRegisters::VEML7700_CR_ALS_WH, (uint8_t *) &threshold_low, 2);
  if (!err)
    return err;

  uint16_t threshold_high = 0xffff;  // set high threshold to max
  err = this->write_register(VEML7700CommandRegisters::VEML7700_CR_ALS_WL, (uint8_t *) &threshold_high, 2);
  if (!err)
    return err;

  VEML7700PSMRegister psm{0};
  psm.PSM = VEML7700PSM::VEML7700_PSM_MODE_1;
  psm.PSM_EN = false;
  ESP_LOGD(TAG, "Setting PSM to 0x%04X", psm.raw);
  err = this->write_register(VEML7700CommandRegisters::VEML7700_CR_PWR_SAVING, (uint8_t *) &(psm.raw), 2);
  if (!err)
    return err;

  delay(3);  // 2.5 ms before the first measurement is needed, allowing for the correct start of the signal processor
             // and oscillator.

  err = this->read_register(VEML7700CommandRegisters::VEML7700_CR_ALS_CONF_0, als_conf.raw_bytes, 2);
  ESP_LOGD(TAG, "Read ALS_CONF_0 0x%04X, err =  %d", als_conf.raw, err);
  return err;
}

void VEML7700Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up VEML7700");

  auto err = this->configure_();
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Sensor configuration failed");
    this->mark_failed();
  }
}

void VEML7700Component::dump_config() {
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  Gain: %s", VEML7700_GAIN_STR[this->gain_]);
  ESP_LOGCONFIG(TAG, "  Integration time: %d ms", VEML7700_IT_TIME[this->map_time_to_index_(this->integration_time_)]);
  ESP_LOGCONFIG(TAG, "  Attenuation factor: %f", this->attenuation_factor_);
  LOG_UPDATE_INTERVAL(this);

  if (this->ambient_light_sensor_ != nullptr)
    LOG_SENSOR("  ", "ALS full spectrum channel", this->ambient_light_sensor_);
  if (this->white_sensor_ != nullptr)
    LOG_SENSOR("  ", "White channel", this->white_sensor_);

  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with I2C VEML-7700 failed!");
  }
}

void VEML7700Component::update() {
  ESP_LOGD(TAG, "Updating");

  if (this->is_ready() && !this->reading_) {
    this->reading_ = true;
    this->read_sensor_data_();
    this->reading_ = false;
  }
}

}  // namespace veml7700
}  // namespace esphome
