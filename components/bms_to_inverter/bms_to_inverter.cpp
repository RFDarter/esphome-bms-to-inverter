#include "bms_to_inverter.h"
#include "esphome/core/log.h"
#include "esphome/components/json/json_util.h"

namespace esphome {
namespace bms_to_inverter {

static const char *const TAG = "bms_to_inverter.BmsToInverter";

BmsToInverter::BmsToInverter() {}

void BmsToInverter::dump_config() {}

void BmsToInverter::setup() {
  // this->charge_logics_[0] = new ChargeLogicRebulkVoltage(this);
  // this->charge_logics_[1] = new ChargeLogicRebulkVoltage(this);
  // this->active_charge_logic_ = this->charge_logics_[0];
  // this->active_charge_logic_->change_charge_status(CHARGE_STATUS_WAIT);
  charge_logics_.push_back(new ChargeLogicRebulkVoltage(this));
  charge_logics_.push_back(new ChargeLogicRebulkVoltage(this));
  charge_logics_.push_back(new ChargeLogicRebulkVoltage(this));
  charge_logics_.push_back(new ChargeLogicRebulkVoltage(this));

  this->active_charge_logic_ = charge_logics_.at(0);

  if (this->user_charge_logic_select_ != nullptr) {
    this->user_charge_logic_select_->add_on_state_callback(
        [this](std::string text, size_t length) { this->active_charge_logic_ = this->charge_logics_.at(length); });
    // needed bc state is allready published befor we could add our on state callback
    this->active_charge_logic_ = this->charge_logics_.at(this->user_charge_logic_select_->active_index().value());
  }

  if (this->user_can_protocol_select_ != nullptr) {
    this->user_can_protocol_select_->add_on_state_callback(
        [this](std::string text, size_t length) { this->can_protocol_ = CanProtocol(length); });
    // needed bc state is allready published befor we could add our on state callback
    this->can_protocol_ = CanProtocol(this->user_can_protocol_select_->active_index().value());
  }
}

void BmsToInverter::update_user_control_values_() {
  float cell_count = 0;
  if (std::isnan(this->bms_data_.cell_count)) {
    ESP_LOGW(TAG, "Cell count is not provided by the bms. Can not update user control data");
  } else {
    cell_count = this->bms_data_.cell_count;
  }

  if (this->user_charge_voltage_number_ != nullptr && this->user_charge_voltage_number_->has_state()) {
    this->user_control_values_.charge_voltage = this->user_charge_voltage_number_->state * cell_count;
  } else {
    this->user_control_values_.charge_voltage = NAN;
  }
  if (this->user_float_voltage_number_ != nullptr && this->user_float_voltage_number_->has_state()) {
    this->user_control_values_.float_voltage = this->user_float_voltage_number_->state * cell_count;
  } else {
    this->user_control_values_.float_voltage = NAN;
  }
  if (this->user_rebulk_voltage_number_ != nullptr && this->user_rebulk_voltage_number_->has_state()) {
    this->user_control_values_.rebulk_voltage = this->user_rebulk_voltage_number_->state * cell_count;
  } else {
    this->user_control_values_.rebulk_voltage = NAN;
  }
  if (this->user_max_charge_current_number_ != nullptr && this->user_max_charge_current_number_->has_state()) {
    this->user_control_values_.max_charge_current = this->user_max_charge_current_number_->state;
  } else {
    this->user_control_values_.max_charge_current = NAN;
  }
  if (this->user_max_discharge_current_number_ != nullptr && this->user_max_discharge_current_number_->has_state()) {
    this->user_control_values_.max_discharge_current = this->user_max_discharge_current_number_->state;
  } else {
    this->user_control_values_.max_discharge_current = NAN;
  }
  if (this->user_absorbtion_time_number_ != nullptr && this->user_absorbtion_time_number_->has_state()) {
    this->user_control_values_.absorbtion_time = this->user_absorbtion_time_number_->state;
  } else {
    this->user_control_values_.absorbtion_time = NAN;
  }
  if (this->user_float_time_number_ != nullptr && this->user_float_time_number_->has_state()) {
    this->user_control_values_.float_time = this->user_float_time_number_->state;
  } else {
    this->user_control_values_.float_time = NAN;
  }
  if (this->user_battery_name_text_ != nullptr && this->user_battery_name_text_->has_state()) {
    this->user_control_values_.name = this->user_battery_name_text_->state;
  } else {
    this->user_control_values_.name = "bms_inverter";
  }
}

void BmsToInverter::update_master_() {
  /**===================================
   * Send data to Inverter
  ====================================*/
  switch (this->inverter_connection_type_) {
    case INVERTER_CONNECTION_TYPE_SERIAL:
      /* code */
      break;
    case INVERTER_CONNECTION_TYPE_CAN:
      this->send_can_message_to_inverter();
      break;
    case INVERTER_CONNECTION_TYPE_MQTT:
      this->send_mqtt_message_to_inverter();
      break;
  }
}

void BmsToInverter::update_slave_() {}

void BmsToInverter::update() {
  // Debug
  switch (this->debug_charge_status_select_->active_index().value()) {
    case 0:  // Auto - do nothing
      break;
    case 1:  // Wait
      this->active_charge_logic_->change_charge_status(CHARGE_STATUS_WAIT);
      break;
    case 2:  // Bulk
      this->active_charge_logic_->change_charge_status(CHARGE_STATUS_BULK);
      break;
    case 3:  // Absorbtion
      this->active_charge_logic_->change_charge_status(CHARGE_STATUS_ABSORBTION);
      break;
    case 4:  // Float
      this->active_charge_logic_->change_charge_status(CHARGE_STATUS_FLOAT);
      break;
  }

  this->update_bms_data(&this->bms_data_);
  this->update_user_control_values_();

  if (!this->active_charge_logic_->are_essential_values_present()) {
    ESP_LOGW(TAG, "Not all values are provided to update the charge logic!");
  }
  this->active_charge_logic_->update_charge_logic_values(&this->charge_logic_values_);

  // TODO aggregate values with slaves!!
  this->aggregated_data_.bms_data = this->bms_data_;
  this->aggregated_data_.charge_logic_values = this->charge_logic_values_;
  this->aggregated_data_.user_control_values = this->user_control_values_;

  this->log_bms_data_();
  this->log_user_control_values_();
  this->log_charge_logic_values_();
  this->log_aggregated_values_();

  // Debug Logs
  if (this->user_can_protocol_select_ != nullptr) {
    ESP_LOGD(TAG, "Selected can protocol is: %s(%d)", this->user_can_protocol_select_->state.c_str(),
             this->can_protocol_);
  } else {
    ESP_LOGD(TAG, "Can protocol select == nullptr");
  }

  if (this->user_charge_logic_select_ != nullptr) {
    ESP_LOGD(TAG, "Selected charge logic is: %s", this->user_charge_logic_select_->state.c_str());
  } else {
    ESP_LOGD(TAG, "Charge logic select == nullptr");
  }

  ESP_LOGI(TAG, "%s", this->bms_data_.is_bms_connected ? "Bms is connected" : "Bms is not connected");

  switch (this->device_role_) {
    case DEVICE_ROLE_MASTER:
      update_master_();
      break;

    case DEVICE_ROLE_SLAVE:
      update_slave_();
      break;
  }
}

void BmsToInverter::on_slave_message(std::vector<uint8_t> &message) { ESP_LOGI(TAG, "message from slave"); }
void BmsToInverter::on_inverter_message(std::vector<uint8_t> &message) { ESP_LOGI(TAG, "inverter heartbeat"); }

void BmsToInverter::version_to_2_byte_array_(float ver, uint8_t *ver_as_byte, uint8_t dec_places) {
  std::string version_string = std::to_string(ver);
  size_t decimal_position = version_string.find('.');
  if (decimal_position != std::string::npos) {
    ver_as_byte[0] = std::stoul(version_string.substr(0, decimal_position));
    ver_as_byte[1] = std::stoul(version_string.substr(decimal_position + 1, dec_places));
  }
}

std::string BmsToInverter::inverter_connection_type_to_string_(InverterConnectionType inverter_connection_type) {
  switch (inverter_connection_type) {
    case INVERTER_CONNECTION_TYPE_CAN:
      return "CAN";
      break;
    case INVERTER_CONNECTION_TYPE_MQTT:
      return "MQTT";
      break;
    case INVERTER_CONNECTION_TYPE_SERIAL:
      return "SERIAL";
      break;
    default:
      return "UNDEFINED";
      break;
  }
}

void BmsToInverter::log_bms_data_() {
  ESP_LOGD(TAG, "------------ Bms Data -------------");
  ESP_LOGD(TAG, "Max Charge Current: %f", this->bms_data_.max_charge_current);
  ESP_LOGD(TAG, "Max Discharge Current: %f", this->bms_data_.max_discharge_current);
  ESP_LOGD(TAG, "State Of Charge: %f", this->bms_data_.state_of_charge);
  ESP_LOGD(TAG, "State Of Health: %f", this->bms_data_.state_of_health);
  ESP_LOGD(TAG, "Total Capacity: %f", this->bms_data_.total_capacity);
  ESP_LOGD(TAG, "Voltage: %f", this->bms_data_.voltage);
  ESP_LOGD(TAG, "Current: %f", this->bms_data_.current);
  ESP_LOGD(TAG, "Min Cell Voltage: %f", this->bms_data_.min_cell_voltage);
  ESP_LOGD(TAG, "Max Cell Voltage: %f", this->bms_data_.max_cell_voltage);
  ESP_LOGD(TAG, "Min Temperatur: %f", this->bms_data_.min_temperatur);
  ESP_LOGD(TAG, "Max Temperatur: %f", this->bms_data_.max_temperatur);
  ESP_LOGD(TAG, "Cell Count: %f", this->bms_data_.cell_count);
  ESP_LOGD(TAG, "Charge Request: %s", this->bms_data_.charge_request ? "yes" : "no");
  ESP_LOGD(TAG, "Is Bms Connected: %s", this->bms_data_.is_bms_connected ? "yes" : "no");
}

void BmsToInverter::log_user_control_values_() {
  ESP_LOGD(TAG, "------------ User Control Values -------------");
  ESP_LOGD(TAG, "Charge Voltage: %f", this->user_control_values_.charge_voltage);
  ESP_LOGD(TAG, "Float Voltage: %f", this->user_control_values_.float_voltage);
  ESP_LOGD(TAG, "Absorbtion Voltage: %f", this->user_control_values_.absorbtion_time);
  ESP_LOGD(TAG, "Rebulk Voltage: %f", this->user_control_values_.rebulk_voltage);
  ESP_LOGD(TAG, "Max Charge Current: %f", this->user_control_values_.max_charge_current);
  ESP_LOGD(TAG, "Max Discharge Current: %f", this->user_control_values_.max_discharge_current);
  ESP_LOGD(TAG, "Absobrtion Time: %f", this->user_control_values_.absorbtion_time);
  ESP_LOGD(TAG, "Float Time: %f", this->user_control_values_.float_time);
  ESP_LOGD(TAG, "Battery Name: %s", this->user_control_values_.name.c_str());
}

void BmsToInverter::log_charge_logic_values_() {
  // TODO do it!
}

void BmsToInverter::log_aggregated_values_() {
  // TODO do it!
}

void BmsToInverter::send_mqtt_message_to_inverter() {
#ifdef USE_MQTT
  // NOLINTBEGIN
  std::string mqtt_message = json::build_json([this](JsonObject root) {
    root["Dc"]["Voltage"] = this->aggregated_data_.bms_data.voltage;
    root["Dc"]["Current"] = this->aggregated_data_.bms_data.current;
    root["Dc"]["Power"] = this->aggregated_data_.bms_data.voltage * this->aggregated_data_.bms_data.current;
    root["Dc"]["Temperature"] = this->aggregated_data_.bms_data.temperatur;

    root["InstalledCapacity"] = this->aggregated_data_.bms_data.total_capacity;
    root["Soc"] = this->aggregated_data_.bms_data.state_of_charge;

    root["Alarms"]["LowVoltage"] = this->aggregated_data_.bms_data.errors_bitmask & 0b00010000 ? 2 : 0;  // low voltage
    root["Alarms"]["HighVoltage"] =
        this->aggregated_data_.bms_data.errors_bitmask & 0b00000000000000100 ? 2 : 0;  // high voltage
    root["Alarms"]["HighChargeCurrent"] = this->aggregated_data_.bms_data.errors_bitmask & 0b000000010000000000000000
                                              ? 2
                                              : 0;  // Charge overcurrent protection
    root["Alarms"]["HighChargeTemperature"] =
        this->aggregated_data_.bms_data.errors_bitmask & 0b01000000 ? 2 : 0;  // Charge Overtemperature
    root["Alarms"]["LowChargeTemperature"] =
        this->aggregated_data_.bms_data.errors_bitmask & 0b0000000100000000 ? 2 : 0;  // Charge Undertemperature

    root["Info"]["ChargeRequest"] = this->aggregated_data_.bms_data.charge_request ? "1" : "0";
    if (!std::isnan(this->aggregated_data_.charge_logic_values.charge_voltage)) {
      root["Info"]["MaxChargeVoltage"] = this->aggregated_data_.charge_logic_values.charge_voltage;
    }
    if (!std::isnan(this->aggregated_data_.charge_logic_values.max_charge_current)) {
      root["Info"]["MaxChargeCurrent"] = this->aggregated_data_.charge_logic_values.max_charge_current;
    }
    if (!std::isnan(this->aggregated_data_.charge_logic_values.max_discharge_current)) {
      root["Info"]["MaxDischargeCurrent"] = this->aggregated_data_.charge_logic_values.max_discharge_current;
    }

    if (!std::isnan(this->aggregated_data_.bms_data.min_voltage_cell)) {
      root["System"]["MinVoltageCellId"] = this->aggregated_data_.bms_data.min_voltage_cell;
    }
    if (!std::isnan(this->aggregated_data_.bms_data.min_cell_voltage)) {
      root["System"]["MinCellVoltage"] = this->aggregated_data_.bms_data.min_cell_voltage;
    }
    if (!std::isnan(this->aggregated_data_.bms_data.max_voltage_cell)) {
      root["System"]["MaxVoltageCellId"] = this->aggregated_data_.bms_data.max_voltage_cell;
    }
    if (!std::isnan(this->aggregated_data_.bms_data.max_cell_voltage)) {
      root["System"]["MaxCellVoltage"] = this->aggregated_data_.bms_data.max_cell_voltage;
    }
    if (!std::isnan(this->aggregated_data_.bms_data.min_temperatur)) {
      root["System"]["MinCellTemperature"] = this->aggregated_data_.bms_data.min_temperatur;
    }
    if (!std::isnan(this->aggregated_data_.bms_data.max_temperatur)) {
      root["System"]["MaxCellTemperature"] = this->aggregated_data_.bms_data.max_temperatur;
    }
    if (!std::isnan(this->aggregated_data_.bms_data.mosfet_temperatur)) {
      root["System"]["MOSTemperature"] = this->aggregated_data_.bms_data.mosfet_temperatur;
    }
    // root["System"]["NrOfModulesOnline"];
    // root["System"]["NrOfModulesOffline"];
    // root["System"]["NrOfModulesBlockingCharge"];
    // root["System"]["NrOfModulesBlockingDischarge"];

    for (int i = 0; i < (sizeof(this->aggregated_data_.bms_data.cell_voltages) /
                         sizeof(this->aggregated_data_.bms_data.cell_voltages[0]));
         i++) {
      char buff[10];
      sprintf(buff, "Cell%d", i + 1);
      if (!std::isnan(this->aggregated_data_.bms_data.cell_voltages[i])) {
        root["Voltages"][buff] = this->aggregated_data_.bms_data.cell_voltages[i];
      }
    }
  });

  mqtt::global_mqtt_client->publish(mqtt_topic_, mqtt_message);
  // NOLINTEND
#endif
}

void BmsToInverter::send_can_message_to_inverter() {
  switch (this->can_protocol_) {
    case CAN_PROTOCOL_PYLON_1_2:
      this->send_frame_0x0359_();
      this->send_frame_0x0351_();
      this->send_frame_0x0355_();
      this->send_frame_0x0356_();
      this->send_frame_0x035C_();
      this->send_frame_0x070_();
      this->send_frame_0x0371_();
      this->send_frame_0x035e_();
      break;
    case CAN_PROTOCOL_PYLON_PLUS:
      this->send_frame_0x0359_();
      this->send_frame_0x0351_();
      this->send_frame_0x0355_();
      this->send_frame_0x0356_();
      this->send_frame_0x035C_();
      this->send_frame_0x070_();
      this->send_frame_0x0371_();
      this->send_frame_0x0379_();
      this->send_frame_0x035e_();
      break;
    case CAN_PROTOCOL_SMA:
      this->send_frame_0x035a_();
      this->send_frame_0x0351_();
      this->send_frame_0x0355_();
      this->send_frame_0x0356_();
      this->send_frame_0x035e_();
      this->send_frame_sma_0x035f_();
      this->send_frame_0x035e_();
      break;
    case CAN_PROTOCOL_VICTRON:
      this->send_frame_0x035a_();
      this->send_frame_0x0351_();
      this->send_frame_0x0355_();
      this->send_frame_0x0356_();
      this->send_frame_0x0372_();
      this->send_frame_0x0373_();
      this->send_frame_0x0374_();
      this->send_frame_0x0379_();  // sure?
      this->send_frame_0x0382_();
      this->send_frame_victron_0x035f_();
      this->send_frame_0x035e_();
      break;
  }

  // this->send_frame_0x0351_();
  // this->send_frame_0x0355_();
  // this->send_frame_0x0356_();
  // this->send_frame_0x035a_();
  // this->send_frame_0x035e_();
  // this->send_frame_0x035f_();
  // this->send_frame_0x0373_();
}
// Pylon 1.2 | Pylon + | SMA | Victron
// Chargevoltage / MaxChargeCurrent / MaxDischargeCurrent
void BmsToInverter::send_frame_0x0351_() {
  static CanMessage0x0351 message;

  if (std::isnan(this->aggregated_data_.charge_logic_values.charge_voltage) ||
      std::isnan(this->aggregated_data_.charge_logic_values.max_charge_current) ||
      std::isnan(this->aggregated_data_.charge_logic_values.max_discharge_current)) {
    ESP_LOGW(TAG, "One of the required sensor states is NaN. Unable to populate 0x0351 frame. Skipped");
    return;
  }

  message.ChargeVoltage =
      (this->aggregated_data_.charge_logic_values.charge_voltage * 10.0f);  // 41V * 10 ... 64V * 10 = 410...640
  message.MaxChargingCurrent =
      (this->aggregated_data_.charge_logic_values.max_charge_current * 10.0f);  // 0A * 10 ... 1200A * 10 = 0...12000
  message.MaxDischargingCurrent =
      (this->aggregated_data_.charge_logic_values.max_discharge_current * 10.0f);  // 0A * 10 ... 1200A * 10 = 0...12000
  message.DischargeVoltageLimit = 65535;                                           // 41V * 10 ... 48V * 10 = 410...480

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x0351, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// Pylon 1.2 | Pylon + | SMA | Victron
// soc / soh
void BmsToInverter::send_frame_0x0355_() {
  static CanMessage0x0355 message;

  if (std::isnan(this->aggregated_data_.bms_data.state_of_charge) ||
      std::isnan(this->aggregated_data_.bms_data.state_of_health)) {
    ESP_LOGW(TAG, "One of the required sensor states is NaN. Unable to populate 0x0355 frame. Skipped");
    return;
  }

  message.StateOfCharge = this->aggregated_data_.bms_data.state_of_charge;                    // 0%...100%
  message.StateOfHealth = this->aggregated_data_.bms_data.state_of_health;                    // 0%...100%
  message.StateOfChargeHighRes = (this->aggregated_data_.bms_data.state_of_charge * 100.0f);  // 0.00%...100.00%

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x0355, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// Pylon 1.2 | Pylon + | SMA | Victron
// voltage / curent / temperatur
void BmsToInverter::send_frame_0x0356_() {
  static CanMessage0x0356 message;

  if (std::isnan(this->aggregated_data_.bms_data.voltage) || std::isnan(this->aggregated_data_.bms_data.current) ||
      std::isnan(this->aggregated_data_.bms_data.temperatur)) {
    ESP_LOGW(TAG, "One of the required sensor states is NaN. Unable to populate 0x0356 frame. Skipped");
    return;
  }

  message.BatteryVoltage = (this->aggregated_data_.bms_data.voltage * 100.0f);        // V
  message.BatteryCurrent = (this->aggregated_data_.bms_data.current * 10.0f);         // A
  message.BatteryTemperature = (this->aggregated_data_.bms_data.temperatur * 10.0f);  // °C
  message.ChargeCycles = this->aggregated_data_.bms_data.charge_cycles;

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x0356, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// SMA | Victron
// alarm
void BmsToInverter::send_frame_0x035a_() {
  static CanMessage0x035A message;
  message.AlarmBitmask = this->aggregated_data_.bms_data.errors_bitmask;

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x035A, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// Pylon 1.2 | Pylon +
// Request Bitmap
void BmsToInverter::send_frame_0x035C_() {
  static CanMessage0x035C message;
  if (std::isnan(this->aggregated_data_.bms_data.allow_charge) ||
      std::isnan(this->aggregated_data_.bms_data.allow_discharge)) {
    ESP_LOGW(TAG, "Not all sensors are available. Unable to populate 0x035C frame. Skipped");
    return;
  }
  message.RequestBitmask = 0;
  message.RequestBitmask |= this->aggregated_data_.bms_data.allow_charge ? 0x80 : 0;
  message.RequestBitmask |= this->aggregated_data_.bms_data.allow_discharge ? 0x40 : 0;

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x035C, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// SMA | Pylon 1.2 | Pylonn + | Victron
// model name
void BmsToInverter::send_frame_0x035e_() {
  static CanMessage0x035E message;
  memcpy(message.Model, this->aggregated_data_.user_control_values.name.c_str(), 8);

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x035E, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// SMA
// chemistry / hardware version / software version / capacity
void BmsToInverter::send_frame_sma_0x035f_() {
  static CanMessageSma0x035F message;

  uint16_t cell_chemistry = 0;
  uint8_t hardware_version[] = {0, 0};  // displayed by victron under device as firmware version
  uint8_t software_version[] = {0, 0};

  version_to_2_byte_array_(HARDWARE_VERSION, hardware_version);
  version_to_2_byte_array_(SOFTWARE_VERSION, software_version);

  if (std::isnan(this->bms_data_.total_capacity)) {
    ESP_LOGW(TAG, "One of the required sensor states is NaN. Unable to populate 0x035f frame. Skipped");
    return;
  }

  message.CellChemistry = cell_chemistry;
  message.HardwareVersion[0] = hardware_version[0];
  message.HardwareVersion[1] = hardware_version[1];
  message.SoftwareVersion[0] = software_version[0];
  message.SoftwareVersion[1] = software_version[1];
  message.NominalCapacity = (uint16_t) this->aggregated_data_.bms_data.total_capacity;

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x035F, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// Victron
// chemistry / hardware version / software version / capacity
void BmsToInverter::send_frame_victron_0x035f_() {
  static CanMessageVictron0x035F message;

  uint8_t hardware_version[] = {0, 0};  // displayed by victron under device as firmware version
  uint8_t software_version[] = {0, 0};

  version_to_2_byte_array_(HARDWARE_VERSION, hardware_version);
  version_to_2_byte_array_(SOFTWARE_VERSION, software_version);

  if (std::isnan(this->aggregated_data_.bms_data.total_capacity)) {
    ESP_LOGW(TAG, "One of the required sensor states is NaN. Unable to populate 0x035f frame. Skipped");
    return;
  }
  message.ProductId = 0;
  message.HardwareVersion[0] = hardware_version[0];
  message.HardwareVersion[1] = hardware_version[1];
  message.SoftwareVersion[0] = software_version[0];
  message.SoftwareVersion[1] = software_version[1];
  message.NominalCapacity = (uint16_t) this->aggregated_data_.bms_data.total_capacity;

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x035F, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// Pylon 1.2 | Pylon +
// Alarm
void BmsToInverter::send_frame_0x0359_() {
  static CanMessage0x0359 message;
  message.AlarmBitmask |= this->aggregated_data_.bms_data.errors_bitmask & 0x04 ? 0x02 : 0x00;    // high voltage
  message.AlarmBitmask |= this->aggregated_data_.bms_data.errors_bitmask & 0x10 ? 0x04 : 0x00;    // low voltage
  message.AlarmBitmask |= this->aggregated_data_.bms_data.errors_bitmask & 0x40 ? 0x08 : 0x00;    // high temp
  message.AlarmBitmask |= this->aggregated_data_.bms_data.errors_bitmask & 0x0100 ? 0x10 : 0x00;  // low temp
  message.AlarmBitmask |=
      this->aggregated_data_.bms_data.errors_bitmask & 0x010000 ? 0x40 : 0x00;  // high charge current
  message.AlarmBitmask |=
      this->aggregated_data_.bms_data.errors_bitmask & 0x4000 ? 0x0100 : 0x00;  // high discharge current
  message.AlarmBitmask |=
      this->aggregated_data_.bms_data.errors_bitmask & 0x5400 ? 0x0800 : 0x00;  // etc -> internal bms alarm

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x0359, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// Pylon 1.2 | Pylon +
// min cell voltage / max cell voltage / min temp / max temp
void BmsToInverter::send_frame_0x070_() {
  static CanMessage0x070 message;
  if (std::isnan(this->aggregated_data_.bms_data.min_cell_voltage) ||
      std::isnan(this->aggregated_data_.bms_data.max_cell_voltage) ||
      std::isnan(this->aggregated_data_.bms_data.min_temperatur) ||
      std::isnan(this->aggregated_data_.bms_data.max_temperatur)) {
    ESP_LOGW(TAG, "Not all sensors are available. Unable to populate 0x070 frame. Skipped");
    return;
  }
  message.MinCellVoltage = this->aggregated_data_.bms_data.min_cell_voltage * 100.0f;
  message.MaxCellVoltage = this->aggregated_data_.bms_data.max_cell_voltage * 100.0f;
  message.MinTemperature = this->aggregated_data_.bms_data.min_temperatur * 10.0f;
  message.MaxTemperature = this->aggregated_data_.bms_data.max_temperatur * 10.0f;

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x070, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// Pylon 1.2 | Pylon +
// min voltage cell id / max voltage cell id / min temp sensor id/ max temp sensor id
void BmsToInverter::send_frame_0x0371_() {
  static CanMessage0x0371 message;

  if (std::isnan(this->aggregated_data_.bms_data.min_voltage_cell) ||
      std::isnan(this->aggregated_data_.bms_data.max_voltage_cell)) {
    ESP_LOGW(TAG, "Not all sensors are available. Unable to populate 0x0371 frame. Skipped");
    return;
  }
  message.MinVoltageCellId = this->aggregated_data_.bms_data.min_voltage_cell;
  message.MaxVoltageCellId = this->aggregated_data_.bms_data.max_voltage_cell;
  message.MinTemperaturSensorId = 1;  // set temperature sensor id to 1
  message.MaxTemperaturSensorId = 1;  // set temperature sensor id to 1

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x0371, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// Victron
// battery modules count / modules block charing count / modules block discharging count / modules offline count
void BmsToInverter::send_frame_0x0372_() {
  static CanMessage0x0372 message;

  // TODO take slaves into account
  message.BatteryModulesBlockChargingCount = 0;
  message.BatteryModulesBlockDischargingCount = 0;
  message.BatteryModulesCount = 1;
  message.BatteryModulesOfflineCount = 0;

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x0372, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// Victron
// min cell voltage / max cell voltage / min temp / max temp
void BmsToInverter::send_frame_0x0373_() {
  static CanMessage0x0373 message;

  if (std::isnan(this->aggregated_data_.bms_data.min_cell_voltage) ||
      std::isnan(this->aggregated_data_.bms_data.max_cell_voltage) ||
      std::isnan(this->aggregated_data_.bms_data.min_temperatur) ||
      std::isnan(this->aggregated_data_.bms_data.max_temperatur)) {
    ESP_LOGW(TAG, "Not all sensors are available. Unable to populate 0x0373 frame. Skipped");
    return;
  }
  message.MinCellvoltage = this->aggregated_data_.bms_data.min_cell_voltage * 1000.0f;
  message.MaxCellvoltage = this->aggregated_data_.bms_data.max_cell_voltage * 1000.0f;
  message.MinTemperature = 273 + this->aggregated_data_.bms_data.min_temperatur;
  message.MaxTemperature = 273 + this->aggregated_data_.bms_data.max_temperatur;

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x0373, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// Victron
// min voltage cell id
void BmsToInverter::send_frame_0x0374_() {
  static CanMessage0x0374 message{};

  if (std::isnan(this->aggregated_data_.bms_data.min_voltage_cell)) {
    ESP_LOGW(TAG, "Not all sensors are available. Unable to populate 0x0374 frame. Skipped");
    return;
  }
  int min_cell_id = this->aggregated_data_.bms_data.min_voltage_cell;

  message.MinVoltageCellId[0] = min_cell_id + 48;

  while (min_cell_id >= 10) {
    min_cell_id /= 10;
  }
  // would break and display garbage if 'min_cell_id' would be >= 100
  message.MinVoltageCellId[1] = min_cell_id + 48;

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x0374, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// Victron
// max voltage cell id
void BmsToInverter::send_frame_0x0375_() {
  static CanMessage0x0375 message{};

  if (std::isnan(this->aggregated_data_.bms_data.max_voltage_cell)) {
    ESP_LOGW(TAG, "Not all sensors are available. Unable to populate 0x0375 frame. Skipped");
    return;
  }
  int max_cell_id = this->aggregated_data_.bms_data.max_voltage_cell;

  message.MaxVoltageCellId[0] = max_cell_id + 48;

  while (max_cell_id >= 10) {
    max_cell_id /= 10;
  }
  // would break and display garbage if 'max_cell_id' would be >= 100
  message.MaxVoltageCellId[1] = max_cell_id + 48;

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x0375, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// Victron
// min temperatur sensor id
void BmsToInverter::send_frame_0x0376_() {
  static CanMessage0x0376 message{};
  message.MinTempertaurSensorId[0] = '0';

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x0376, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// Victron
// max temperatur sensor id
void BmsToInverter::send_frame_0x0377_() {
  static CanMessage0x0377 message{};
  message.MaxTempertaurSensorId[0] = '0';

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x0377, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// Pylon + | Victron
// capacity
void BmsToInverter::send_frame_0x0379_() {
  static CanMessage0x0379 message{};

  if (std::isnan(this->aggregated_data_.bms_data.total_capacity)) {
    ESP_LOGW(TAG, "Not all sensors are available. Unable to populate 0x0379 frame. Skipped");
    return;
  }
  message.NominalCapacity = this->aggregated_data_.bms_data.total_capacity;

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x0379, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}
// Vitron
// ProduktIdentification
void BmsToInverter::send_frame_0x0382_() {
  static CanMessage0x0382 message{};
  memcpy(message.ProduktIdentification, this->aggregated_data_.user_control_values.name.c_str(), 8);

  auto *ptr = reinterpret_cast<uint8_t *>(&message);
  this->canbus_->send_data(0x0382, false, false, std::vector<uint8_t>(ptr, ptr + sizeof message));
}

}  // namespace bms_to_inverter
}  // namespace esphome
