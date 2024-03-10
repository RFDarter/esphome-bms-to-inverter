#include "charge_logic_rebulk_voltage.h"
#include "bms_to_inverter.h"

namespace esphome {
namespace bms_to_inverter {

static const char *const TAG = "bms_to_inverter.ChargeLogicRebulkVoltage";

void ChargeLogicRebulkVoltage::change_charge_status(ChargeStatus charge_status) {
  this->charge_status_ = charge_status;
  std::string charge_status_string;
  switch (charge_status_) {
    case CHARGE_STATUS_NO_BMS_CONNECTED:
      charge_status_string = "NO_BMS_CONNECTED";
      break;
    case CHARGE_STATUS_WAIT:
      charge_status_string = "WAIT";
      break;
    case CHARGE_STATUS_BULK:
      charge_status_string = "BULK";
      break;
    case CHARGE_STATUS_ABSORBTION:
      this->timestamp_started_absorbtion_ = millis();
      charge_status_string = "ABSORBTION";
      break;
    case CHARGE_STATUS_FLOAT:
      charge_status_string = "FLOAT";
      break;
    default:
      charge_status_string = "UNDEF";
      break;
  }
  if (this->parent_->charge_status_text_sensor_ != nullptr) {
    this->parent_->charge_status_text_sensor_->publish_state(charge_status_string);
  }
}

void ChargeLogicRebulkVoltage::update_charge_logic_values(ChargeLogicValues *charge_logic_values) {
  if (!this->parent_->bms_data_.is_bms_connected) {
    this->change_charge_status(CHARGE_STATUS_NO_BMS_CONNECTED);
  }
  // TODO take errors into account!!

  float battery_voltage = this->parent_->bms_data_.voltage;

  switch (this->charge_status_) {
    case CHARGE_STATUS_NO_BMS_CONNECTED:
      // Bms connected
      if (this->parent_->bms_data_.is_bms_connected) {
        this->change_charge_status(CHARGE_STATUS_BULK);
      }
      [[fallthrough]];
    case CHARGE_STATUS_WAIT:
      charge_logic_values->charge_voltage = 0.1;
      charge_logic_values->max_charge_current = 0;
      charge_logic_values->max_discharge_current = 0;
      break;
    case CHARGE_STATUS_BULK:
      charge_logic_values->charge_voltage = this->parent_->user_control_values_.charge_voltage;
      charge_logic_values->max_charge_current = this->parent_->user_control_values_.max_charge_current;
      charge_logic_values->max_discharge_current = this->parent_->user_control_values_.max_discharge_current;

      // We hit target voltage, start absobtion time and switch status to absorbtion
      if (battery_voltage >= this->parent_->user_control_values_.charge_voltage) {
        this->change_charge_status(CHARGE_STATUS_ABSORBTION);
      }
      break;
    case CHARGE_STATUS_ABSORBTION:
      charge_logic_values->charge_voltage = this->parent_->user_control_values_.charge_voltage;
      charge_logic_values->max_charge_current = this->parent_->user_control_values_.max_charge_current;
      charge_logic_values->max_discharge_current = this->parent_->user_control_values_.max_discharge_current;

      // We hit target absorbtion time,switch to float
      if ((millis() - this->timestamp_started_absorbtion_) >=
          (this->parent_->user_control_values_.absorbtion_time * 3600 * 1000)) {
        this->change_charge_status(CHARGE_STATUS_FLOAT);
      }
      break;
    case CHARGE_STATUS_FLOAT:
      charge_logic_values->charge_voltage = this->parent_->user_control_values_.float_voltage;
      charge_logic_values->max_charge_current = this->parent_->user_control_values_.max_charge_current;
      charge_logic_values->max_discharge_current = this->parent_->user_control_values_.max_discharge_current;

      // We hit the rebulk foltage, switch status to bulk
      if (battery_voltage <= this->parent_->user_control_values_.rebulk_voltage) {
        this->change_charge_status(CHARGE_STATUS_BULK);
      }
      break;
  }
}

bool ChargeLogicRebulkVoltage::are_essential_values_present() {
  return (std::isnan(this->parent_->bms_data_.voltage) || std::isnan(this->parent_->bms_data_.cell_count));
}

}  // namespace bms_to_inverter
}  // namespace esphome
