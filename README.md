This is preliminary draft version of VEML7700 component for EspHome.

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
  frequency: 5kHz

sensor:
  - platform: veml7700
    address: 0x10
    update_interval: 30s
    gain: X1
    integration_time: 100ms
    glass_attenuation_factor: 1.0
    ambient_light:
      name: "Ambient light"
    white:
      name: "White"
```