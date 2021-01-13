#include "ir-remote.h"

ir_remote::ir_remote() {
    pinMode(ir_pin, OUTPUT);
}

ir_remote::ir_remote(uint8_t pin, uint16_t f, double dc, uint16_t op, uint16_t os, uint16_t zp, uint16_t zs) 
    :   ir_pin(pin), frequency(f), duty_cycle(dc), one_pulse(op), one_space(os), zero_pulse(zs), zero_space(zs)
{
    uint16_t khz = frequency / 1000;
    period_time_us = (1000U + khz / 2) / khz; //= 1000/khz + 1/2 = round(1000.0/khz)
    pinMode(ir_pin, OUTPUT);
}

void ir_remote::set_value(bool power, bool swing, uint8_t temperature, uint8_t mode, uint8_t wind_speed) {
    status.power = power;
    status.swing = swing;
    status.temperature = temperature;
    status.mode = mode;
    status.wind_speed = wind_speed;
}

ac_status& ir_remote::get_ref() {
    return status;
}

std::vector<uint16_t> ir_remote::convert() {
    // PART 1: 构造状态对应的字节，这部分可根据不同的空调型号进行调整
    const uint8_t len = 14;
    uint8_t construct[len];
    memset(construct, 0, sizeof(construct));
    // 固定头部
    construct[0] = 0b10100110;
    // 温度和风向，各四位
    construct[1] = ((status.temperature - 16) << 4) + (status.swing == 0 ? 0b0 : 0b1100);
    // 风速: 0自动，后面代表风速级别
    switch (status.wind_speed) {
        case 1:
            construct[5] = 0b01100000;
            break;
        case 2:
            construct[5] = 0b01000000;
            break;
        case 3:
            construct[5] = 0b00100000;
            break;
        default:
            construct[5] = 0b10100000;
            break;
    }
    // 模式：制冷0加热1除湿2
    switch (status.mode) {
        case 1:
            construct[7] = 0b00100000;
            construct[4] |= 0b10000000;
            break; //制热默认开启辅热
        case 2:
            construct[7] = 0b01000000;
            break;
        default:
            construct[7] = 0b00100000;
            break;
    }
    // 开关：关0开1
    switch (status.power) {
        case 1:
            construct[4] |= 0b01000000;
            construct[12] = 0b10000101;
            break;
        default:
            construct[4] |= 0b00000000;
            construct[12] = 0b10000101;
            break;
    }
    // 校验和
    for (int i = 0; i < 13; ++i) {
        construct[13] += construct[i];
    }
    construct[13] &= 0b11111111;

    // END OF PART 1

    // PART 2: 生成pulse/space时间序列，单位为microseconds/us
    std::vector<uint16_t> time_seq;
    time_seq.reserve(256);   //buffer
    // 查到的固定头部，可根据不同型号修改
    time_seq.push_back(3178);
    time_seq.push_back(2972);
    time_seq.push_back(3097);
    time_seq.push_back(4418);
    // 本质进制转换
    for(uint8_t i = 0; i < len; ++i) {    // every byte in construct[]
        /* LSB 在左边的写法
        for(uint8_t j = 0; j < 8; ++j) {    //every bit in construct[i]
            
            if(construct[i] % 2) {
                time_seq.push_back(one_pulse);
                time_seq.push_back(one_space);
            } else {
                time_seq.push_back(zero_pulse);
                time_seq.push_back(zero_space);
            }
            Serial.print(construct[i] % 2);
            construct[i] /= 2;
        }
        */

        /* HSB 写法 */
        for(uint8_t mask = 0b10000000; mask != 0; mask = (mask >> 1)) {
            if((construct[i] & mask) == 0) {
                time_seq.push_back(zero_pulse);
                time_seq.push_back(zero_space);
                //Serial.printf("0");
            } else {
                time_seq.push_back(one_pulse);
                time_seq.push_back(one_space);
                //Serial.printf("1");
            }
        }
        //Serial.print("\n");
    }
    time_seq.push_back(zero_pulse); //Tailing zero
    //for(int i = 0; i < time_seq.size(); i++) {
        //Serial.print(time_seq[i]);
        //Serial.print(" ");
    //}
    //Serial.print("\n===========\n");
    return time_seq;
}

void ir_remote::send_signal() {
    std::vector<uint16_t> ts = convert();
    size_t len = ts.size();
    for(size_t i = 0; i < len; i++) {
        if(i % 2 == 0) {
            pulse(ts[i]);
        } else {
            space(ts[i]);
        }
    }
}

void ir_remote::pulse(uint16_t us) {
    if(us == 0) return;
    auto now = micros();
    while(micros() - now < us) {
        digitalWrite(ir_pin, HIGH);
        delayMicroseconds(period_time_us*duty_cycle-6);
        digitalWrite(ir_pin, LOW);
        delayMicroseconds(period_time_us*(1-duty_cycle)-7);
    }
}

void ir_remote::space(uint16_t us) {
    if (us==0) return;
    while (us>16383) {
        delayMicroseconds(16383);
        us -= 16383;
    }
    delayMicroseconds(us);
}