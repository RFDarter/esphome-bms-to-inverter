#pragma once

#include "esphome/core/defines.h"

// #define USE_ESP32  // just so fix intelisens
// #include "esphome/components/mqtt/custom_mqtt_device.h"
#include "can.h"
#include "esphome/components/canbus/canbus.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text/text.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/application.h"
#include "esphome/core/base_automation.h"

#include "charge_logic_rebulk_voltage.h"

#ifdef USE_MQTT
#include "esphome/components/mqtt/mqtt_const.h"
// NOLINTNEXTLINE
#include "esphome/components/mqtt/mqtt_client.h"
#endif

namespace esphome {
#ifdef USE_MQTT
namespace mqtt {
extern MQTTClientComponent *global_mqtt_client;  // NOLINT
}
#endif
namespace bms_to_inverter {

/**
 * --- A Bms implementation would need to provide these components as a minimum ---
 * sensor:voltage
 * sensor:current
 * sensor:soc
 * number:cell_count
 */

#define BATTERY_MAX_CYCLES 6000.0
#define SOFTWARE_VERSION 1.0
#define HARDWARE_VERSION 1.0

enum CanProtocol {
  CAN_PROTOCOL_PYLON_1_2,
  CAN_PROTOCOL_PYLON_PLUS,
  CAN_PROTOCOL_SMA,
  CAN_PROTOCOL_VICTRON,
};

enum InverterType {
  INVERTER_TYPE_GENERIC,
  INVERTER_TYPE_VICTRON,
};

enum InverterConnectionType {
  INVERTER_CONNECTION_TYPE_SERIAL,
  INVERTER_CONNECTION_TYPE_CAN,
  INVERTER_CONNECTION_TYPE_MQTT,
};

enum MasterConnectionType {
  MASTER_CONNECTION_TYPE_SERIAL,
  MASTER_CONNECTION_TYPE_CAN,
  MASTER_CONNECTION_TYPE_WIFI,
};

enum DeviceRole {
  DEVICE_ROLE_MASTER,
  DEVICE_ROLE_SLAVE,
};

// contains the data ffrom the bms wich eatch implementation needs to update
struct BmsData {
  bool allow_charge;
  bool allow_discharge;

  float total_capacity;
  float state_of_charge;
  float state_of_health;
  float voltage;
  float current;
  float max_charge_current;
  float max_discharge_current;
  float temperatur;
  float min_temperatur;
  float max_temperatur;
  float mosfet_temperatur;
  float min_cell_voltage;
  float max_cell_voltage;
  float min_voltage_cell;
  float max_voltage_cell;
  float cell_voltages[32];
  float temperatures[4];
  bool charge_request;
  float charge_cycles;
  bool is_bms_connected;
  float cell_count;
  /* Sma style
    Bit 0
    00000100 high voltage
    00010000 low voltage
    01000000 high temp
    Bit 1
    00000001 low temp
    00000100 high charge temp
    00010000 low charge temp
    01000000 high discharge current
    Bit 2
    00000001 high charge current
    00000100 contactor
    00010000 short circute
    01000000 bms internal
    */
  uint32_t errors_bitmask;
};

// Conatins the values the user can provide. the voltages will be set to match the cell count
struct UserControlValues {
  float charge_voltage;
  float float_voltage;
  float rebulk_voltage;
  float max_charge_current;
  float max_discharge_current;
  double absorbtion_time;
  double float_time;
  std::string name;
};

// Contains all the Data we send to the Inverter or Master
// If we are the Master they would need to ge aggregated
struct DataToSendToInverter {
  BmsData bms_data;
  ChargeLogicValues charge_logic_values;
  UserControlValues user_control_values;
};

class BmsToInverter : public PollingComponent {
 public:
  explicit BmsToInverter();
  /*Should be called in every implimentation */
  void setup() override;
  /*Should be called in every implimentation */
  void update() override;
  void dump_config() override;

  void send_mqtt_message_to_inverter();
  void send_can_message_to_inverter();

  void set_device_role(DeviceRole device_role) { device_role_ = device_role; }
  void set_topic(const std::string &mqtt_topic) { this->mqtt_topic_ = mqtt_topic; }
  void set_inverter_type(InverterType inverter_type) { this->inverter_type_ = inverter_type; }
  void set_inverter_connection_type(InverterConnectionType inverter_connection_type) {
    this->inverter_connection_type_ = inverter_connection_type;
  }
  void set_master_connection_type(MasterConnectionType master_connection_type) {
    this->master_connection_type_ = master_connection_type;
  }
  void set_canbus(canbus::Canbus *canbus) { this->canbus_ = canbus; }

  void set_user_charge_voltage_number(number::Number *user_charge_voltage_number) {
    this->user_charge_voltage_number_ = user_charge_voltage_number;
  }
  void set_user_float_voltage_number(number::Number *user_float_voltage_number) {
    this->user_float_voltage_number_ = user_float_voltage_number;
  }
  void set_user_absorbtion_time_number(number::Number *user_absorbtion_time_number) {
    this->user_absorbtion_time_number_ = user_absorbtion_time_number;
  }
  void set_user_rebulk_voltage_number(number::Number *user_rebulk_voltage_number) {
    this->user_rebulk_voltage_number_ = user_rebulk_voltage_number;
  }
  void set_user_float_time_number(number::Number *user_float_time_number) {
    this->user_float_time_number_ = user_float_time_number;
  }
  void set_user_max_charge_current_number(number::Number *user_max_charge_current_number) {
    this->user_max_charge_current_number_ = user_max_charge_current_number;
  }
  void set_user_max_discharge_current_number(number::Number *user_max_discharge_current_number) {
    this->user_max_discharge_current_number_ = user_max_discharge_current_number;
  }
  void set_user_battery_name_text(text::Text *user_battery_name_text) {
    this->user_battery_name_text_ = user_battery_name_text;
  }
  void set_user_charge_logic_select(select::Select *user_charge_logic_select) {
    this->user_charge_logic_select_ = user_charge_logic_select;
  }
  void set_user_can_protocol_select(select::Select *user_can_protocol_select) {
    this->user_can_protocol_select_ = user_can_protocol_select;
  }

  void set_charge_status_text_sensor(text_sensor::TextSensor *charge_status_text_sensor) {
    this->charge_status_text_sensor_ = charge_status_text_sensor;
  }

  void set_debug_charge_status_select(select::Select *debug_charge_status_select) {
    this->debug_charge_status_select_ = debug_charge_status_select;
  }
  void set_test_switch_switch(switch_::Switch *test_switch) {}
  void on_slave_message(std::vector<uint8_t> &message);
  void on_inverter_message(std::vector<uint8_t> &message);

 protected:
  friend class ChargeLogic;
  friend class ChargeLogicRebulkVoltage;

  DeviceRole device_role_{DEVICE_ROLE_MASTER};
  InverterType inverter_type_{INVERTER_TYPE_GENERIC};
  InverterConnectionType inverter_connection_type_{INVERTER_CONNECTION_TYPE_SERIAL};
  MasterConnectionType master_connection_type_{MASTER_CONNECTION_TYPE_SERIAL};
  std::string mqtt_topic_{};
  canbus::Canbus *canbus_;

  /**==============================
   * User Config
   ==============================*/
  number::Number *user_charge_voltage_number_{nullptr};
  number::Number *user_float_voltage_number_{nullptr};
  number::Number *user_rebulk_voltage_number_{nullptr};
  number::Number *user_absorbtion_time_number_{nullptr};
  number::Number *user_float_time_number_{nullptr};
  number::Number *user_max_charge_current_number_{nullptr};
  number::Number *user_max_discharge_current_number_{nullptr};
  text::Text *user_battery_name_text_{nullptr};
  select::Select *user_charge_logic_select_{nullptr};
  select::Select *user_can_protocol_select_{nullptr};
  text_sensor::TextSensor *charge_status_text_sensor_{nullptr};

  /**==============================
   * Debug
   ==============================*/
  select::Select *debug_charge_status_select_{nullptr};

  /**==============================
   * Charge logic
   ================================*/
  // ChargeLogic *charge_logics_[2]{nullptr};
  std::vector<ChargeLogic *> charge_logics_;
  ChargeLogic *active_charge_logic_{nullptr};

  BmsData bms_data_{};
  ChargeLogicValues charge_logic_values_{};
  DataToSendToInverter aggregated_data_;
  UserControlValues user_control_values_{};
  CanProtocol can_protocol_{CAN_PROTOCOL_PYLON_1_2};

  /**Each implementation would need to gather bms data and update 'this->bms_data'
   * unused or values that are not privided by the bms would ndeed to the set to 'NAN'
   */
  virtual void update_bms_data(BmsData *bms_data) = 0;
  void update_user_control_values_();

  void update_master_();
  void update_slave_();

  void send_frame_0x0351_();
  void send_frame_0x0355_();
  void send_frame_0x0356_();
  void send_frame_0x035a_();
  void send_frame_0x035C_();
  void send_frame_0x035e_();
  void send_frame_sma_0x035f_();
  void send_frame_victron_0x035f_();
  void send_frame_0x0359_();
  void send_frame_0x070_();
  void send_frame_0x0371_();
  void send_frame_0x0372_();
  void send_frame_0x0373_();
  void send_frame_0x0374_();
  void send_frame_0x0375_();
  void send_frame_0x0376_();
  void send_frame_0x0377_();
  void send_frame_0x0379_();
  void send_frame_0x0382_();

  void version_to_2_byte_array_(float ver, uint8_t *ver_as_byte, uint8_t dec_places = 1);

  std::string inverter_connection_type_to_string_(InverterConnectionType inverter_connection_type);

  void log_bms_data_();
  void log_user_control_values_();
  void log_charge_logic_values_();
  void log_aggregated_values_();
};

}  // namespace bms_to_inverter
}  // namespace esphome
