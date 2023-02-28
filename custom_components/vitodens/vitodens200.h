#pragma once

#include <sstream>
#include "esphome.h"
#include "VitoWiFi.h"

using namespace std;
using namespace std::placeholders;

#define RX2 16
#define TX2 17

class PresetSwitch : public Component, public Switch {
private:
    IDatapoint& _dp;
    uint _is_hot = 0;
public:
    PresetSwitch(IDatapoint *dp) : _dp(*dp) {};
    void setup() override {
        ESP_LOGI("Vitodens200", "PresetSwitch: setup()");
    }
    void write_state(bool state) override;
    void publish_state(bool state) {
        // avoid reverting to old status right away
        ESP_LOGI("Vitodens200", "PresetSwitch: _is_hot = %d", _is_hot);
        if (_is_hot > 0) {
            _is_hot--;
        } else {
            Switch::publish_state(state);
        }
    }
};

class Vitodens200 : public Component, public Climate {
  private:
    // Device data
    DPRaw _dp_get_device_type;

    // Operating data HC1
    DPMode _dp_set_operating_mode;
    DPTempS _dp_set_room_standard_temp;
    DPTempS _dp_set_room_reduced_temp;
    DPTemp _dp_set_dhw_temp;
    DPTempS _dp_set_heating_curve_slope;
    DPTempS _dp_set_heating_curve_level;
    DPStat _dp_economy_mode;
    DPStat _dp_party_mode;

    // Boiler
    DPTemp _dp_outside_temp;

    // Heating circuit HC1
    DPMode _dp_current_operating_mode;
    DPTemp _dp_flow_temp;
    DPCountS _dp_heating_pump_status;
    DPTemp _dp_room_temp;

    // DHW
    DPTemp _dp_dhw_cts1_temp;
    DPTemp _dp_dhw_cts2_temp;
    DPStat _dp_dhw_primary_pump_status;
    DPStat _dp_dhw_circulation_pump_status;

    // Burner
    DPMode _dp_burner_status;

    // Test
//    DPRaw _dp_test;


    // Callbacks for optolink data points
    void _float_cb(Sensor *sensor, const IDatapoint& dp, DPValue value);
    void _short_temp_cb(Sensor *sensor, const IDatapoint& dp, DPValue value);
    void _binary_cb(BinarySensor *sensor, const IDatapoint& dp, DPValue value);
    void _binary_u16_cb(BinarySensor *sensor, const IDatapoint& dp, DPValue value);
//    void _counts_cb(Sensor *sensor, const IDatapoint& dp, DPValue value);
    void _current_operating_mode_cb(TextSensor *sensor, const IDatapoint& dp, DPValue value);
    void _set_operating_mode_cb(TextSensor *sensor, const IDatapoint& dp, DPValue value);
    void _raw_cb(TextSensor *sensor, const IDatapoint& dp, DPValue value);
    void _climate_preset_cb(BinarySensor *sensor, const IDatapoint &dp, DPValue value, ClimatePreset preset, PresetSwitch*);

    // Climate callbacks
    void _room_temp_cb(const IDatapoint& dp, DPValue value);
    void _set_room_standard_temp_cb(const IDatapoint& dp, DPValue value);


    // Scheduled call for initiating communication via optolink
    void _comm(const char*);

  public:
    // Device data
    TextSensor *sensor_get_device_type = new TextSensor();

    // Operating data HC1
    TextSensor *sensor_set_operating_mode = new TextSensor();
    Sensor *sensor_set_room_standard_temp = new Sensor();
    Sensor *sensor_set_room_reduced_temp = new Sensor();
    Sensor *sensor_set_dhw_temp = new Sensor();
    Sensor *sensor_set_heating_curve_slope = new Sensor();
    Sensor *sensor_set_heating_curve_level = new Sensor();
    BinarySensor *sensor_economy_mode = new BinarySensor();
    BinarySensor *sensor_party_mode = new BinarySensor();

    // Boiler
    Sensor *sensor_outside_temp = new Sensor();

    // Heating circuit HC1
    TextSensor *sensor_current_operating_mode = new TextSensor();
    Sensor *sensor_flow_temp = new Sensor();
    BinarySensor *sensor_heating_pump_status = new BinarySensor();
    Sensor *sensor_room_temp = new Sensor();

    // DHW
    Sensor *sensor_dhw_cts1_temp = new Sensor();
    Sensor *sensor_dhw_cts2_temp = new Sensor();
    BinarySensor *sensor_dhw_primary_pump_status = new BinarySensor();
    BinarySensor *sensor_dhw_circulation_pump_status = new BinarySensor();

    // Burner
    Sensor *sensor_burner_status = new Sensor();

    // Switches
    PresetSwitch *economy_switch = new PresetSwitch(&_dp_economy_mode);
    PresetSwitch *party_switch = new PresetSwitch(&_dp_party_mode);

    // Test
//    TextSensor *sensor_test = new TextSensor();

    // Climate
    ClimateTraits traits() override;
    void control(const ClimateCall &call) override;


    Vitodens200();
    void setup() override;
    void loop() override;

};

VitoWiFi_setProtocol(P300);


void PresetSwitch::write_state(bool state) {
    ESP_LOGI("Vitodens200", "PresetSwitch write_state(): %d", state);
    // don't allow the next update to revert the state
    DPValue value(state);
    VitoWiFi.writeDatapoint(_dp, value);
    publish_state(state);
    _is_hot = 2;
}

// optolink class definitions
Vitodens200::Vitodens200() :
  // Device data
  _dp_get_device_type("getDeviceType", "slow", 0x00F8),

  // Operating data HC1
  _dp_set_operating_mode("setOperatingMode", "fast", 0x2323, true),
  _dp_set_room_standard_temp("roomStandardTemp", "fast", 0x2306),
  _dp_set_room_reduced_temp("roomReducedTemp", "slow", 0x2307),
  _dp_set_dhw_temp("setDHWTemp", "fast", 0x6300),
  _dp_set_heating_curve_slope("setHeatingCurveSlope", "slow", 0x27D3),
  _dp_set_heating_curve_level("setHeatingCurveLevel", "slow", 0x27D4),
  _dp_economy_mode("economyMode", "fast", 0x2302, true),
  _dp_party_mode("partymode", "fast", 0x2303, true),

  // Boiler
  _dp_outside_temp("outsideTemp", "slow", 0x5525),

  // Heating circuit HC1
  _dp_current_operating_mode("currentOperatingMode", "slow", 0x2500),
  _dp_flow_temp("flowTemp", "fast", 0x0810),
  _dp_heating_pump_status("heatingPumpStatus", "fast", 0x7663),
  _dp_room_temp("roomTemp", "slow", 0x0896),

  // DHW
  _dp_dhw_cts1_temp("DHW CTS1 Temp", "slow", 0x0812),
  _dp_dhw_cts2_temp("DHW CTS2 Temp", "slow", 0x0814),
  _dp_dhw_primary_pump_status("DHW_PrimaryPumpStatus", "fast", 0x6513),
  _dp_dhw_circulation_pump_status("DHW_CirculationPumpStatus", "fast", 0x6515),

  // Burner
  _dp_burner_status("burnerStatus", "fast", 0x55D3)

  // Test
//  _dp_test("getTest", "fast", 0x2303)
{
}

void Vitodens200::_comm(const char* group) {
  VitoWiFi.readGroup(group);
}

void Vitodens200::_float_cb(Sensor* sensor, const IDatapoint& dp, DPValue value) {
  ESP_LOGD("optolink", "Datapoint %s - %s: %f", dp.getGroup(), dp.getName(), value.getFloat());
  sensor->publish_state(value.getFloat());
}

void Vitodens200::_short_temp_cb(Sensor* sensor, const IDatapoint& dp, DPValue value) {
  ESP_LOGD("optolink", "Datapoint %s - %s: %d", dp.getGroup(), dp.getName(), value.getU8());
  sensor->publish_state(value.getU8());
}

void Vitodens200::_binary_cb(BinarySensor* sensor, const IDatapoint& dp, DPValue value) {
  ESP_LOGD("optolink", "Datapoint %s - %s: %d", dp.getGroup(), dp.getName(), value.getBool());
  sensor->publish_state(value.getBool());
}

void Vitodens200::_binary_u16_cb(BinarySensor* sensor, const IDatapoint& dp, DPValue value) {
  ESP_LOGD("optolink", "Datapoint %s - %s: %d", dp.getGroup(), dp.getName(), value.getU16());
  sensor->publish_state(value.getU16() & 0xFF != 0);
}

//void Vitodens200::_counts_cb(Sensor* sensor, const IDatapoint& dp, DPValue value) {
//  ESP_LOGD("optolink", "Datapoint %s - %s: %d", dp.getGroup(), dp.getName(), value.getU16());
//  sensor->publish_state(value.getU16());
//}

void Vitodens200::_current_operating_mode_cb(TextSensor* sensor, const IDatapoint& dp, DPValue value) {
  ESP_LOGD("optolink", "Datapoint %s - %s: %d", dp.getGroup(), dp.getName(), value.getU8());
  int code = value.getU8();
  stringstream t;
  switch(code) {
    case 0:
      t << "Standby mode (permanently)";
      break;
    case 1:
      t << "Red. Operation (timer)";
      break;
    case 2:
      t << "Normal Operation (timer)";
      break;
    case 3:
      t << "Normal Operation (permanently)";
      break;
    default:
      t << "Unknown (" <<code <<")";
      break;
  }
  sensor->publish_state(t.str());
}

void Vitodens200::_set_operating_mode_cb(TextSensor* sensor, const IDatapoint& dp, DPValue value) {
  ESP_LOGD("optolink", "Datapoint %s - %s: %d", dp.getGroup(), dp.getName(), value.getU8());
  int code = value.getU8();
  stringstream t;
  switch(code) {
    case 0:
      t << "Standby mode";
      this->mode = climate::CLIMATE_MODE_OFF;
      break;
    case 1:
      t << "DHW only";
      this->mode = climate::CLIMATE_MODE_OFF;
      break;
    case 2:
      t << "Central heating and DHW";
      this->mode = climate::CLIMATE_MODE_AUTO;
      break;
    case 3:
      t << "Permanently red. Operation";
      break;
    case 4  :
      t << "Permanently normal Operation";
      break;
    default:
      t << "Unknown (" <<code <<")";
      break;
  }
  // publish sensor
  sensor->publish_state(t.str());
  // publish climate
  publish_state();
}

void Vitodens200::_raw_cb(TextSensor* sensor, const IDatapoint& dp, DPValue value) {
  char str[10];
  value.getString(str, sizeof(str));
  ESP_LOGD("optolink", "Datapoint %s - %s: %s", dp.getGroup(), dp.getName(), str);
  sensor->publish_state(str);
}

void Vitodens200::_room_temp_cb(const IDatapoint &dp, DPValue value) {
    ESP_LOGD("optolink", "Datapoint %s - %s: %d", dp.getGroup(), dp.getName(), value.getFloat());
    sensor_room_temp->publish_state(value.getFloat());
    if (current_temperature != value.getFloat()) {
        current_temperature = value.getFloat();
        publish_state();
    }
}

void Vitodens200::_set_room_standard_temp_cb(const IDatapoint &dp, DPValue value) {
    ESP_LOGD("optolink", "Datapoint %s - %s: %d", dp.getGroup(), dp.getName(), value.getU8());
    sensor_set_room_standard_temp->publish_state(value.getU8());
    if (target_temperature != value.getU8()) {
        target_temperature = value.getU8();
        publish_state();
    }
}

void Vitodens200::_climate_preset_cb(BinarySensor *sensor, const IDatapoint &dp, DPValue value, ClimatePreset preset, PresetSwitch *preset_switch) {
    ESP_LOGD("optolink", "Datapoint %s - %s: %d", dp.getGroup(), dp.getName(), value.getBool());
    sensor->publish_state(value.getBool());
    if (value.getBool()) {
        this->preset = preset;
        publish_state();
    } else if (this->preset == preset) {
        this->preset = climate::CLIMATE_PRESET_NONE;
        publish_state();
    }

    preset_switch->publish_state(value.getBool());
}


void Vitodens200::setup() {
    // enable logging
//    Serial1.begin(115200);
    VitoWiFi.setLogger(&Serial);
    VitoWiFi.enableLogger();
  // Device data
  _dp_get_device_type.setLength(2).setCallback(bind(&Vitodens200::_raw_cb, this, sensor_get_device_type, _1, _2));

  // Operating data HC1
  _dp_set_operating_mode.setCallback(bind(&Vitodens200::_set_operating_mode_cb, this, sensor_set_operating_mode, _1, _2));
  _dp_set_room_reduced_temp.setCallback(bind(&Vitodens200::_short_temp_cb, this, sensor_set_room_reduced_temp, _1, _2));
  _dp_set_dhw_temp.setCallback(bind(&Vitodens200::_float_cb, this, sensor_set_dhw_temp, _1, _2));
  _dp_set_heating_curve_slope.setCallback(bind(&Vitodens200::_short_temp_cb, this, sensor_set_heating_curve_slope, _1, _2));
  _dp_set_heating_curve_level.setCallback(bind(&Vitodens200::_short_temp_cb, this, sensor_set_heating_curve_level, _1, _2));
  _dp_economy_mode.setCallback(bind(&Vitodens200::_climate_preset_cb, this, sensor_economy_mode, _1, _2, climate::CLIMATE_PRESET_ECO, economy_switch));
  _dp_party_mode.setCallback(bind(&Vitodens200::_climate_preset_cb, this, sensor_party_mode, _1, _2, climate::CLIMATE_PRESET_COMFORT, party_switch));

  // Boiler
  _dp_outside_temp.setCallback(bind(&Vitodens200::_float_cb, this, sensor_outside_temp, _1, _2));

  // Heating circuit HC1
  _dp_current_operating_mode.setCallback(bind(&Vitodens200::_current_operating_mode_cb, this, sensor_current_operating_mode, _1, _2));
  _dp_flow_temp.setCallback(bind(&Vitodens200::_float_cb, this, sensor_flow_temp, _1, _2));
  _dp_heating_pump_status.setCallback(bind(&Vitodens200::_binary_u16_cb, this, sensor_heating_pump_status, _1, _2));
//  _dp_room_temp.setCallback(bind(&Vitodens200::_float_cb, this, sensor_room_temp, _1, _2));

  // DHW
  _dp_dhw_cts1_temp.setCallback(bind(&Vitodens200::_float_cb, this, sensor_dhw_cts1_temp, _1, _2));
  _dp_dhw_cts2_temp.setCallback(bind(&Vitodens200::_float_cb, this, sensor_dhw_cts2_temp, _1, _2));
  _dp_dhw_primary_pump_status.setCallback(bind(&Vitodens200::_binary_cb, this, sensor_dhw_primary_pump_status, _1, _2));
  _dp_dhw_circulation_pump_status.setCallback(bind(&Vitodens200::_binary_cb, this, sensor_dhw_circulation_pump_status, _1, _2));

  // Burner
  _dp_burner_status.setCallback(bind(&Vitodens200::_short_temp_cb, this, sensor_burner_status, _1, _2));

  // Test
//  _dp_test.setLength(1).setCallback(bind(&Vitodens200::_raw_cb, this, sensor_test, _1, _2));

  // Climate callbacks
  _dp_room_temp.setCallback(bind(&Vitodens200::_room_temp_cb, this, _1, _2));
  _dp_set_room_standard_temp.setWriteable(true).setCallback(bind(&Vitodens200::_set_room_standard_temp_cb, this, _1, _2));

  // this callback will be used for all DPs without specific callback
  // must be set after adding at least 1 datapoint
  //  VitoWiFi.setGlobalCallback(&Vitodens200::_global_cb);

  VitoWiFi.setup(&Serial2, RX2, TX2);
  // slow group
  set_interval("vitodens200_comm_slow", 60000, bind(&Vitodens200::_comm, this, "slow"));
  // fast group
  set_interval("vitodens200_comm_fast", 10000, bind(&Vitodens200::_comm, this, "fast"));
  ESP_LOGI("Vitodens200", "Component Initialized.");
}

void Vitodens200::loop() {
  VitoWiFi.loop();
}

void Vitodens200::control(const ClimateCall &call) {
    if (call.get_mode().has_value()) {
      // User requested mode change
      ClimateMode mode = *call.get_mode();
      // Send mode to hardware
      // ...
        ESP_LOGI("Vitodens200", "New mode requested.");
      // Publish updated state
      if (mode == climate::CLIMATE_MODE_OFF) {
          // set to stand-by
          DPValue value((uint8_t)0);
          VitoWiFi.writeDatapoint(_dp_set_operating_mode, value);
      } else if (mode == climate::CLIMATE_MODE_AUTO) {
          // set to heat + dhw
          DPValue value((uint8_t)2);
          VitoWiFi.writeDatapoint(_dp_set_operating_mode, value);
      }
    }
    if (call.get_target_temperature().has_value()) {
      // User requested target temperature change
      uint8_t temp = *call.get_target_temperature() + 0.5;
      ESP_LOGI("Vitodens200", "New temperature requested: %d", temp);
      // Send target temp to climate
      DPValue value(temp);
      VitoWiFi.writeDatapoint(_dp_set_room_standard_temp, value);
    }
}

ClimateTraits Vitodens200::traits() {
    // The capabilities of the climate device
    auto traits = climate::ClimateTraits();
    traits.set_supports_current_temperature(true);
    traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_AUTO});
    traits.set_supported_presets({climate::CLIMATE_PRESET_HOME, climate::CLIMATE_PRESET_SLEEP, CLIMATE_PRESET_COMFORT, CLIMATE_PRESET_ECO});
    return traits;
}


Vitodens200* boiler = new Vitodens200();
