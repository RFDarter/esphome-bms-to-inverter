#pragma once

#include "esphome/components/text/text.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"

namespace esphome {
namespace bms_to_inverter {

// We keep this separate so we don't have to template and duplicate
// the text input for each different size flash allocation.
class TemplateTextSaverBase {
 public:
  virtual bool save(const std::string &value) { return true; }

  virtual void setup(uint32_t id, std::string &value) {}

 protected:
  ESPPreferenceObject pref_;
  std::string prev_;
};

template<uint8_t SZ> class TextSaver : public TemplateTextSaverBase {
 public:
  bool save(const std::string &value) override {
    int diff = value.compare(this->prev_);
    if (diff != 0) {
      // If string is bigger than the allocation, do not save it.
      // We don't need to waste ram setting prev_value either.
      int size = value.size();
      if (size <= SZ) {
        // Make it into a length prefixed thing
        unsigned char temp[SZ + 1];
        memcpy(temp + 1, value.c_str(), size);
        // SZ should be pre checked at the schema level, it can't go past the char range.
        temp[0] = ((unsigned char) size);
        this->pref_.save(&temp);
        this->prev_.assign(value);
        return true;
      }
    }
    return false;
  }

  // Make the preference object.  Fill the provided location with the saved data
  // If it is available, else leave it alone
  void setup(uint32_t id, std::string &value) override {
    this->pref_ = global_preferences->make_preference<uint8_t[SZ + 1]>(id);

    char temp[SZ + 1];
    bool hasdata = this->pref_.load(&temp);

    if (hasdata) {
      value.assign(temp + 1, (size_t) temp[0]);
    }

    this->prev_.assign(value);
  }
};

class BmsToInverterText : public text::Text, public PollingComponent {
 public:
  void set_template(std::function<optional<std::string>()> &&f) { this->f_ = f; }

  void setup() override {
    std::string value;
    value = this->initial_value_;
    if (this->pref_) {
      uint32_t key = this->get_object_id_hash();
      key += this->traits.get_min_length() << 2;
      key += this->traits.get_max_length() << 4;
      key += fnv1_hash(this->traits.get_pattern()) << 6;
      this->pref_->setup(key, value);
    }
    if (!value.empty())
      this->publish_state(value);
  }
  void update() override {}
  void dump_config() override{};
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  void set_initial_value(const std::string &initial_value) { this->initial_value_ = initial_value; }
  void set_value_saver(TemplateTextSaverBase *restore_value_saver) { this->pref_ = restore_value_saver; }

 protected:
  void control(const std::string &value) override {
    this->publish_state(value);

    if (this->pref_) {
      if (!this->pref_->save(value)) {
      }
    }
  }
  bool optimistic_ = false;
  std::string initial_value_;
  Trigger<std::string> *set_trigger_ = new Trigger<std::string>();
  optional<std::function<optional<std::string>()>> f_{nullptr};

  TemplateTextSaverBase *pref_ = nullptr;
};

}  // namespace bms_to_inverter
}  // namespace esphome
