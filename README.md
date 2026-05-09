ESPHome components
---

**p1reader** is exposing values form a Dutch Smart Meter Requirements (DSMR) port

An small example of YAML configuration:
```yaml
logger:
  baud_rate: 0

external_components:
  - source: ./components/
    components: [p1reader]

uart:
  id: uart_bus
  rx_pin: GPIO3
  baud_rate: 115200
  rx_buffer_size: 2048

p1reader:
  - id: p1reader_esp
    uart_id: uart_bus
    buffer_size: 4096

#text_sensor:
#  - platform: p1reader
#    name: "Üzenet időbélyeg"
#    p1reader_id: p1reader_esp
#    id: p1reader_timestamp
#    obis_code: "0-0:1.0.0"
#  - platform: p1reader
#    name: "Üzenet"
#    p1reader_id: p1reader_esp
#    id: p1reader_raw
#    obis_code: "raw"

sensor:
  - platform: p1reader
    name: "Pillanatnyi teljesítmény vételezés"
    p1reader_id: p1reader_esp
    id: p1reader_electricity_currently_delivered
    obis_code: "1-0:1.7.0"
    unit_of_measurement: kW
    accuracy_decimals: 3
    state_class: measurement
    device_class: power
  - platform: p1reader
    name: "Alacsony tarifán vételezve"
    p1reader_id: p1reader_esp
    id: p1reader_electricity_delivered_1
    obis_code: "1-0:1.8.2"
    unit_of_measurement: kWh
    accuracy_decimals: 3
    state_class: total_increasing
    device_class: energy
```

**wifibox** is a replacement for Centrometals's Wifi Box product

An small example of YAML configuration:
```yaml
external_components:
  - source: ./components/
    components: [wifibox]

uart:
  id: uart_bus
  rx_pin: GPIO16
  tx_pin: GPIO17
  baud_rate: 57600
  rx_buffer_size: 2048

wifibox:
  id: wifibox_esp
  uart_id: uart_bus
  buffer_size: 2048
  mqtt_id: "XYXZ1234"

sensor:
  - platform: wifibox
    wifibox_id: wifibox_esp
    name: "Boiler Temperature"
    key: "B_Tk1"
    id: cm_pelet_set_boiler_temperature
    icon: "mdi:thermometer"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
  - platform: wifibox
    wifibox_id: wifibox_esp
    name: "Fire Sensor"
    key: "B_FotV"
    id: cm_pelet_set_fire_sensor
    icon: "mdi:fire"
    unit_of_measurement: "kOhm"    
```