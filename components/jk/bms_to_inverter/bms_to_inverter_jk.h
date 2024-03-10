#pragma once

#include "esphome/core/defines.h"

#include "esphome/components/jk_bms/jk_bms.h"
#include "esphome/components/bms_to_inverter/bms_to_inverter.h"

namespace esphome {
namespace jk {

class BmsToInverterJK : public bms_to_inverter::BmsToInverter {
 public:
  explicit BmsToInverterJK(jk_bms::JkBms *bms) : bms_(bms){};

  void setup() override;
  void update() override;

 protected:
  jk_bms::JkBms *bms_;
  void update_bms_data(bms_to_inverter::BmsData *bms_data) override;
};

}  // namespace jk
}  // namespace esphome
