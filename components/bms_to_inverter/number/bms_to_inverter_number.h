#pragma once

#include "esphome/core/component.h"
#include "esphome/components/number/number.h"
#include "esphome/core/preferences.h"

namespace esphome {
namespace bms_to_inverter {

class BmsToInverterNumber : public number::Number, public Component {
 public:
  void dump_config() override{};
  void set_initial_value(float initial_value) { initial_value_ = initial_value; }
  void set_restore_value(bool restore_value) { this->restore_value_ = restore_value; }

  void setup() override;

 protected:
  void control(float value) override;
  ESPPreferenceObject pref_;
  float initial_value_;
  bool restore_value_{false};
};

}  // namespace bms_to_inverter
}  // namespace esphome
