This is preliminary draft version of VEML7700 component for EspHome.

Please better use version from official PR(): `-source: github://pr#6067`

Example config:
```
esphome:
  name: veml7700-test

esp32:
  board: esp32dev
  framework:
    type: arduino

# esp8266:
#   board: nodemcuv2

logger:
  level: DEBUG

external_components:
  - source: github://latonita/esphome-veml7700
    components: [ veml7700 ]

i2c:
  sda: GPIO25
  scl: GPIO32
  scan: true

sensor:
  - platform: veml7700
    address: 0x10
    update_interval: 30s
    auto_mode: true
    #gain: 1/8x
    #integration_time: 100ms
    # lux_compensation: false
    # glass_attenuation_factor: 1.10
    ambient_light:
      name: "Ambient light"
    # ambient_light_counts:
    #   name: "Ambient light counts"
    full_spectrum:
      name: "Full spectrum"
    # full_spectrum_counts:
    #   name: "Full spectrum counts"
    infrared: 
      name: "Near-IR"
    actual_integration_time:
      name: "Actual integration time"
    actual_gain:
      name: "Actual gain"
```
