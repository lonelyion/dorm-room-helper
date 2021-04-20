#ifndef _IR_REMOTE_H
#define _IR_REMOTE_H

#include <Arduino.h>
#include <inttypes.h>

#include <vector>

struct ac_status {
  bool power;
  bool swing;
  uint8_t temperature;
  uint8_t mode;
  uint8_t wind_speed;
};

class ir_remote {
 public:
  /**
     * Default constructor
    */
  ir_remote();
  /**
     * Constructor
     * @param pin The IO pin of infrared LED
     * @param f The frequency of the IR signal in Hz
     * @param dc The duty cycle of the IR signal. 0.5 means for every cycle, the LED will turn on for half the cycle time, and off the other half
     * @param op The duration of a pulse in microseconds when sending a logical 1
     * @param os The duration of the gap/space in microseconds when sending a logical 1
     * @param zp The duration of a pulse in microseconds when sending a logical 0
     * @param zs The duration of the gap/space in microseconds when sending a logical 0
    */
  ir_remote(uint8_t pin,
            uint16_t f = 38000, double dc = 0.5,
            uint16_t op = 562, uint16_t os = 1688,
            uint16_t zp = 562, uint16_t zs = 562);
  /**
     * Set the values of air-conditioner's status
     * @param power The Power status of air-conditioner
     * @param swing Determins whether the swing mode is on
     * @param temperature The target temperature of air-conditioner
     * @param mode The work mode of air-conditioner where 0 for cool, 1 for warm and 2 for dry
     * @param wind_speed The wind speed gear of air-conditioner where 0 for auto, and allows 1/2/3 level
     * @return The result of set operation
    */
  void set_value(bool power, bool swing, uint8_t temperature, uint8_t mode, uint8_t wind_speed);
  /**
     * Get a reference of air-conditioner's status
    */
  ac_status& get_ref();
  /**
     * Send Infrared Signals to the air-conditioner
    */
  void send_signal();

 protected:
  /**
     * Convert current status into infrared pulse/space sequence
     * @return The time sequence to be sent
    */
  std::vector<uint16_t> convert();

 private:
  // The IO pin used for IR LED
  uint8_t ir_pin = A0;
  // The frequency of the IR signal in Hz
  const uint16_t frequency = 38000;
  /**
     * The duty cycle of the IR signal.
     * 0.5 means for every cycle, the LED will turn on for half the cycle time, and off the other half
    */
  const double duty_cycle = 0.5;

  const uint16_t one_pulse = 562;
  const uint16_t one_space = 1688;
  const uint16_t zero_pulse = 562;
  const uint16_t zero_space = 562;

  //
  uint32_t period_time_us = 0;

  ac_status status;

  void pulse(uint16_t us);
  void space(uint16_t us);
};

#endif