#include "bms_to_inverter_number.h"
#include "esphome/core/log.h"

namespace esphome {
namespace bms_to_inverter {

static const char *const TAG = "bms_to_inverter.number";

void BmsToInverterNumber::setup() {
  float value;
  if (!this->restore_value_) {
    value = this->initial_value_;
  } else {
    this->pref_ = global_preferences->make_preference<float>(this->get_object_id_hash());
    if (!this->pref_.load(&value)) {
      if (!std::isnan(this->initial_value_)) {
        value = this->initial_value_;
      } else {
        value = this->traits.get_min_value();
      }
    }
  }
  this->publish_state(value);
}

void BmsToInverterNumber::control(float value) {
  if (this->restore_value_) {
    this->pref_.save(&value);
  }
  this->publish_state(value);
}

}  // namespace bms_to_inverter
}  // namespace esphome
