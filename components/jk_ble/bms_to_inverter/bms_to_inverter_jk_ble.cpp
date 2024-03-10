#include "bms_to_inverter_jk_ble.h"

namespace esphome {
namespace jk_ble {

static const char *const TAG = "bms_to_inverter.BmsToInverterJKBle";

void BmsToInverterJKBle::setup() { bms_to_inverter::BmsToInverter::setup(); }
void BmsToInverterJKBle::update() { bms_to_inverter::BmsToInverter::update(); }

void BmsToInverterJKBle::update_bms_data(bms_to_inverter::BmsData *bms_data) {
  // NOLINTBEGIN
  if (this->bms_->charging_switch_ != nullptr) {  // no has_state() default should be off/false
    bms_data->allow_charge = this->bms_->charging_switch_->state;
  } else {
    bms_data->allow_charge = true;
  }

  if (this->bms_->discharging_switch_ != nullptr) {
    bms_data->allow_charge = this->bms_->discharging_switch_->state;
  } else {  // if no sensor is provided we assume discharging is allowed
    bms_data->allow_discharge = true;
  }

  if (this->bms_->online_status_binary_sensor_ != nullptr) {
    if (this->bms_->online_status_binary_sensor_->has_state()) {
      bms_data->is_bms_connected = this->bms_->online_status_binary_sensor_->state;
    } else {
      bms_data->is_bms_connected = false;
    }
  } else {
    bms_data->is_bms_connected = false;
  }

  if (this->bms_->total_battery_capacity_setting_sensor_ != nullptr &&
      this->bms_->total_battery_capacity_setting_sensor_->has_state()) {
    bms_data->total_capacity = this->bms_->total_battery_capacity_setting_sensor_->get_state();
  } else {
    bms_data->total_capacity = NAN;
  }
  if (this->bms_->charging_cycles_sensor_ != nullptr && this->bms_->charging_cycles_sensor_->has_state()) {
    bms_data->charge_cycles = this->bms_->charging_cycles_sensor_->get_state();
    bms_data->state_of_health = (float) (uint16_t) (1 - (double) (bms_data->charge_cycles / BATTERY_MAX_CYCLES)) * 100;
  } else {  // if no sensor to calculate soh are provided we just set it to 100%
    bms_data->state_of_health = 100;
  }
  if (this->bms_->state_of_charge_sensor_ != nullptr && this->bms_->state_of_charge_sensor_->has_state()) {
    bms_data->state_of_charge = this->bms_->state_of_charge_sensor_->get_state();
  } else {
    bms_data->state_of_charge = NAN;
  }
  if (this->bms_->total_voltage_sensor_ != nullptr && this->bms_->total_voltage_sensor_->has_own_name()) {
    bms_data->voltage = this->bms_->total_voltage_sensor_->get_state();
  } else {
    bms_data->voltage = NAN;
  }
  if (this->bms_->current_sensor_ != nullptr && this->bms_->current_sensor_->has_state()) {
    bms_data->current = this->bms_->current_sensor_->get_state();
  } else {
    bms_data->current = NAN;
  }
  if (this->bms_->max_charge_current_number_ != nullptr && this->bms_->max_charge_current_number_->has_state()) {
    bms_data->max_charge_current = this->bms_->max_charge_current_number_->state;
  } else {
    bms_data->max_charge_current = NAN;
  }
  if (this->bms_->max_discharge_current_number_ != nullptr && this->bms_->max_discharge_current_number_->has_state()) {
    bms_data->max_discharge_current = this->bms_->max_discharge_current_number_->state;
  } else {
    bms_data->max_discharge_current = NAN;
  }

  if (this->bms_->min_cell_voltage_sensor_ != nullptr && this->bms_->min_cell_voltage_sensor_->has_state()) {
    bms_data->min_cell_voltage = this->bms_->min_cell_voltage_sensor_->get_state();
  } else {
    bms_data->min_cell_voltage = NAN;
  }
  if (this->bms_->max_cell_voltage_sensor_ != nullptr && this->bms_->max_cell_voltage_sensor_->has_state()) {
    bms_data->max_cell_voltage = this->bms_->max_cell_voltage_sensor_->get_state();
  } else {
    bms_data->max_cell_voltage = NAN;
  }
  if (this->bms_->max_voltage_cell_sensor_ != nullptr && this->bms_->max_voltage_cell_sensor_->has_state()) {
    bms_data->max_voltage_cell = this->bms_->max_voltage_cell_sensor_->get_state();
  } else {
    bms_data->max_voltage_cell = NAN;
  }
  if (this->bms_->min_voltage_cell_sensor_ != nullptr && this->bms_->min_voltage_cell_sensor_->has_state()) {
    bms_data->min_voltage_cell = this->bms_->min_voltage_cell_sensor_->get_state();
  } else {
    bms_data->min_voltage_cell = NAN;
  }
  if (this->bms_->temperatures_[0].temperature_sensor_ != nullptr &&
      this->bms_->temperatures_[0].temperature_sensor_->has_state()) {
    bms_data->temperatur = this->bms_->temperatures_[0].temperature_sensor_->get_state();
  } else {
    bms_data->temperatur = NAN;
  }
  if (this->bms_->power_tube_temperature_sensor_ != nullptr &&
      this->bms_->power_tube_temperature_sensor_->has_state()) {
    bms_data->mosfet_temperatur = this->bms_->power_tube_temperature_sensor_->get_state();
  } else {
    bms_data->mosfet_temperatur = NAN;
  }

  float min_temp = NAN;
  for (auto &temp_sensor : this->bms_->temperatures_) {
    if (temp_sensor.temperature_sensor_ != nullptr &&
        (temp_sensor.temperature_sensor_->get_state() < min_temp || std::isnan(min_temp))) {
      min_temp = temp_sensor.temperature_sensor_->get_state();
    }
  }
  bms_data->min_temperatur = min_temp;

  float max_temp = NAN;
  for (auto &temp_sensor : this->bms_->temperatures_) {
    if (temp_sensor.temperature_sensor_ != nullptr &&
        (temp_sensor.temperature_sensor_->state > max_temp || std::isnan(max_temp))) {
      max_temp = temp_sensor.temperature_sensor_->get_state();
    }
  }
  bms_data->max_temperatur = max_temp;

  bms_data->errors_bitmask = 0;  // reset errors every update
  uint32_t error_bitmask = 0;
  if (this->bms_->errors_bitmask_sensor_ != nullptr && this->bms_->errors_bitmask_sensor_->has_state()) {
    error_bitmask = this->bms_->errors_bitmask_sensor_->get_state();
  }
  if (error_bitmask & 0b0001) {  // over temp
    bms_data->errors_bitmask |= 0b01000000;
  }
  if (error_bitmask & 0b0010) {  // under temp
    bms_data->errors_bitmask |= 0b0000000100000000;
  }
  if (error_bitmask & 0b1000) {  // cell undervoltage
    bms_data->errors_bitmask |= 0b00010000;
  }
  if (error_bitmask & 0100000000000000) {  // over current
    bms_data->errors_bitmask |= 0b000000010000000000000000;
  }
  if (error_bitmask & 0b0001000000000000) {  // cell overvoltage
    bms_data->errors_bitmask |= 0b00000100;
  }

  for (int i = 0; i < sizeof(this->bms_->temperatures_) / sizeof(this->bms_->temperatures_[0]) - 1; i++) {
    if (this->bms_->temperatures_[i].temperature_sensor_ != nullptr &&
        this->bms_->temperatures_[i].temperature_sensor_->has_state()) {
      bms_data->temperatures[i] = this->bms_->temperatures_[i].temperature_sensor_->get_state();
    } else {
      bms_data->temperatures[i] = NAN;
    }
  }
  for (int i = 0; i < sizeof(this->bms_->cells_) / sizeof(this->bms_->cells_[0]) - 1; i++) {
    if (this->bms_->cells_[i].cell_voltage_sensor_ != nullptr &&
        this->bms_->cells_[i].cell_voltage_sensor_->has_state()) {
      bms_data->cell_voltages[i] = this->bms_->cells_[i].cell_voltage_sensor_->get_state();
    } else {
      bms_data->cell_voltages[i] = NAN;
    }
  }
  if (this->bms_->cell_count_number_ != nullptr && this->bms_->cell_count_number_->has_state()) {
    bms_data->cell_count = this->bms_->cell_count_number_->state;
  } else {
    bms_data->cell_count = NAN;
  }
  // NOLINTEND
}

}  // namespace jk_ble
}  // namespace esphome
