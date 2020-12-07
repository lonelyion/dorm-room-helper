//nfc.h
//Code by Yutao Gu
#ifndef _LONELYION_NFC_H
#define _LONELYION_NFC_H

#include <Arduino.h>
#include <PN532_HSU.h>
#include <PN532.h>
#include <vector>
#include <secrets.h>

//这个Class会用Serial2（RX = 16, TX = 17）
//作为与PN532模块的通信接口
//协议采用High Speed UART(HSU)
class NfcReader {
public:
    NfcReader(bool d = false);              //构造函数，bool值为是否输出调试信息
    bool initialize();                      //初始化，true则连接成功
    void print_version_data();              //输出NFC模组信息
    bool read_and_check_match();            //需要循环执行的函数，返回读取到的NFC卡UID是否为allow_list上面的

private:
    static bool is_equal(const std::vector<uint8_t> &mv, const uint8_t ma[], const uint8_t &len);

    bool show_debug_info = false;
    PN532_HSU *hsu;
    PN532 *nfc;
    uint32_t versiondata;

    std::vector<std::vector<uint8_t> > allow_list;
    uint8_t allow_list_length;
};

#endif