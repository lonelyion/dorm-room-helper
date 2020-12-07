//Servo.h
//Code by Yutao Gu
#ifndef _LONELYION_SERVO_H
#define _LONELYION_SERVO_H

#include <Arduino.h>
#include <esp32-hal-ledc.h>         //用ESP32的HAL库来做PWM输出
                                    //纯软件的方式会导致时钟周期不一定每次一致

#define TIMER_WIDTH 16
#define TIMER_WIDTH_TICKS 65536

class Servo {
public:
    Servo(uint8_t p);
    void attach();
    void write_angle(uint8_t angle);
    void open_the_door();

private:
    const static int MIN = 500;     //脉冲时间最小值
    const static int MAX = 2500;    //脉冲时间最大值
    const static int FREQ = 50;     //PWM频率
    const static int PERIOD_US = (int)(1000000/FREQ);    //PWM周期，单位us

    uint8_t pin = 26;               //Servo连接到的GPIO端口
    uint8_t channel = 0;            //HAL库用的channel，默认为0
    uint16_t map(uint8_t angle);    //将角度转换为PWM微秒值
};

#endif