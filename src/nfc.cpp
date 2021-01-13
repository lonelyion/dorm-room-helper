//nfc.cpp
//Code by Yutao Gu
#include <nfc.h>

bool NfcReader::is_equal(const std::vector<uint8_t> &mv, const uint8_t ma[], const uint8_t &len) {
    for(int i = 0; i < len; i++) {
        if(mv[i] != ma[i]) return false;
    }
    return true;
}

NfcReader::NfcReader(bool debug, uint8_t mrt) : show_debug_info(debug), max_retry_time(mrt), allow_list(_ALLOW_LIST), allow_list_length(allow_list.size()) {
    hsu = new PN532_HSU(Serial2);
    nfc = new PN532(*hsu);

    if(show_debug_info) {
        Serial.println("=====ALLOW LIST=====");
        Serial.printf("Length: %hhu\n", allow_list_length);
        for(std::vector<std::vector<uint8_t>>::iterator it = allow_list.begin(); it != allow_list.end(); it++) {
            for(auto ele = it->begin(); ele != it->end(); ele++) {
                Serial.print(" 0x");
                Serial.print(*ele, HEX);
            }
            Serial.print("\n");
        }
        Serial.println("=====   END   =====");
    }
}

bool NfcReader::initialize() {
    int retry_time = 0;
    retry:
    nfc->begin();
    uint32_t vd = nfc->getFirmwareVersion();
    if(!vd) {
        Serial.println("Didn't find PN53x board, retry in 10s");
        delay(10000);
        //delete hsu;
        //delete nfc;
        if(retry_time <= max_retry_time) {
            ++retry_time;
            goto retry;
        } else {
            Serial.println("Exceeded max retry times...");
            return false;
        }
    }
    this->versiondata = vd;
    
    //一直等待重新读取，如果参数值小于0xFF则是最大次数
    nfc->setPassiveActivationRetries(0xFF);
    nfc->SAMConfig();
    return true;
}

void NfcReader::print_version_data() {
    Serial.print("Found chip PN5");
    Serial.println((versiondata >> 24) & 0xFF, HEX);
    Serial.print("Firmware ver. ");
    Serial.print((versiondata >> 16) & 0xFF, DEC);
    Serial.print('.');
    Serial.println((versiondata >> 8) & 0xFF, DEC);
}

bool NfcReader::read_and_check_match() {
    bool success = false;
    static uint8_t uid_buf[] = {0, 0, 0, 0, 0, 0, 0};   //存储返回UID的Buffer
    static uint8_t uid_length;                          //UID的长度

    //等待 ISO14443A 类型的卡片 (Mifare等等).
    //读取到信息的时候，uid数组会更新，相应的uid_length也会更新
    //Mifare Classic卡片是4 bytes长， Mifare Ultralight是7 bytes长
    success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid_buf[0], &uid_length);

    if(success) {
        //输出调试信息
        if(show_debug_info) {
            Serial.println("Found a card!");
            Serial.print("UID Length: ");
            Serial.print(uid_length, DEC);
            Serial.println(" bytes");
            Serial.print("UID Value: ");
            for (uint8_t i = 0; i < uid_length; i++) {
                Serial.print(" 0x");
                Serial.print(uid_buf[i], HEX);
            }
            Serial.print("\n");
        }
        //判断
        for (uint8_t i = 0; i < allow_list_length; i++) {
            if (is_equal(allow_list[i], uid_buf, uid_length)) {
                if(show_debug_info) Serial.println("Card is in Allow List.");
                return true;
            }
        }
        if(show_debug_info) Serial.println("Card is NOT in Allow List.");
    } else {    //if(success)
        //if(show_debug_info) Serial.println("Timed out waiting for a card.");
    }
    return false;
}