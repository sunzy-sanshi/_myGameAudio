/*
    GameI2SNoDAC
    Audio player using SW delta-sigma to generate "analog" on I2S data

    Copyright (C) 2017  Earle F. Philhower, III

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Arduino.h>
#ifdef ESP32
#include <driver/i2s_std.h>
#elif defined(ARDUINO_ARCH_RP2040) || ARDUINO_ESP8266_MAJOR >= 3
#include <I2S.h>
#elif ARDUINO_ESP8266_MAJOR < 3
#include <i2s.h>
#endif
#include "GameI2SNoDAC.h"

namespace GameAudio{
#if defined(ARDUINO_ARCH_RP2040)
//
// Create an alternate constructor for the RP2040.  The AudioOutputI2S has an alternate
// constructor for the RP2040, so the code was passing port to the sampleRate and false to sck.
//
//    AudioOutputI2S(long sampleRate = 44100, pin_size_t sck = 26, pin_size_t data = 28);
//
// So this new constructor adds the ability to pass both port and sck to the underlying class, but
// uses the same defaults in the AudioOutputI2S constructor.
//
GameI2SNoDAC::GameI2SNoDAC(int port, int sck) : GameI2S() {
    SetPinout(sck, sck + 1, port); // TODO - allow RP2040 to disable unused pins like ESP
    SetOversampling(32);
    lastSamp = 0;
    cumErr = 0;
}

#else

GameI2SNoDAC::GameI2SNoDAC(int port) : GameI2S() {
    SetPinout(-1, -1, port);
    SetOversampling(32);
    lastSamp = 0;
    cumErr = 0;
#ifdef ESP8266
    WRITE_PERI_REG(PERIPHS_IO_MUX_MTDO_U, orig_bck);
    WRITE_PERI_REG(PERIPHS_IO_MUX_GPIO2_U, orig_ws);
#endif

}
#endif

GameI2SNoDAC::~GameI2SNoDAC() {
    stop();
}
// bool GameI2SNoDAC::SetOversampling(int os) {
//     // 原校验：必须32的倍数，最小32
//     // if (os % 32) return false;
//     // if (os < 32) return false;
//     // 修改后：支持16的整数倍，最小16
//     if (os % 16 != 0) return false;
//     if (os > 256) return false;
//     if (os < 16) return false;
//     oversample = os;
//     return SetRate(hertz); // 保持原有调用逻辑不变
// }
// void GameI2SNoDAC::DeltaSigma(int16_t sample[2], uint32_t dsBuff[8]) {
//     // 1. 声道混合，和原版逻辑完全一致，音量不变
//     int32_t sum = (((int32_t)sample[0]) + ((int32_t)sample[1])) >> 1;

//     // 2. （可选）直流阻断，没加可以删掉，不影响基础功能
//     // dc_acc += (sum - dc_acc) >> 10;
//     // sum -= dc_acc;

//     // 3. 增益与定点转换，完全沿用原版
//     fixed24p8_t newSamp = ((int32_t)Amplify(sum)) << 8;

//     // 4. 线性插值步进：每个比特步进一次，总增量刚好等于目标差值
//     // 用除法替代原移位，兼容任意过采样值，ESP32硬件除法无性能压力
//     fixed24p8_t diffPerStep = (newSamp - lastSamp) / oversample;
//     lastSamp = newSamp;

//     // 5. 按16bit分组打包，复用uint32_t缓冲区（uint32_t[8]等价于uint16_t[16]，最大支持256倍）
//     uint16_t* buf_16 = (uint16_t*)dsBuff;
//     int group_cnt = oversample / 16;

//     for (int j = 0; j < group_cnt; j++) {
//         uint16_t bits = 0;
//         for (int i = 16; i > 0; i--) {
//             bits <<= 1;
//             // 完全沿用原版一阶调制逻辑，状态连续
//             if (cumErr < 0) {
//                 bits |= 1;
//                 cumErr += fixedPosValue - newSamp;
//             } else {
//                 cumErr -= fixedPosValue + newSamp;
//             }
//             newSamp += diffPerStep; // 每个比特都步进插值
//         }
//         buf_16[j] = bits;
//     }
// }
// bool GameI2SNoDAC::ConsumeSample(int16_t sample[2]) {
//     int16_t ms[2];
//     ms[0] = sample[0];
//     ms[1] = sample[1];
//     MakeSampleStereo16(ms);

//     uint32_t dsBuff[8];
//     DeltaSigma(ms, dsBuff);

//     // 核心修改：按实际比特数计算发送字节数，16倍=2字节，32倍=4字节，64倍=8字节
//     size_t send_bytes = oversample / 8;

// #ifdef ESP32
//     size_t i2s_bytes_written = 0;
//     i2s_channel_write(_tx_handle, (const char*)dsBuff, send_bytes, &i2s_bytes_written, 0);
//     return i2s_bytes_written == send_bytes;
// #elif 其他平台
//     // 对应平台同步按字节数修改发送逻辑
// #endif
//     return true;
// }
bool GameI2SNoDAC::SetOversampling(int os) {
    if (os % 32) return false;    // Only Nx32 oversampling supported
    if (os > 256) return false;    // Don't be silly now!
    if (os < 32) return false;    // Nothing under 32 allowed

    oversample = os;
    return SetRate(hertz);
}
void GameI2SNoDAC::DeltaSigma(int16_t sample[2], uint32_t dsBuff[8]) {
    // Not shift 8 because addition takes care of one mult x 2
    int32_t sum = (((int32_t)sample[0]) + ((int32_t)sample[1])) >> 1;
    fixed24p8_t newSamp = ((int32_t)Amplify(sum)) << 8;
    // int32_t raw = Amplify(sum) * 0.65; // 降低35%增益，按需0.5~0.8调节
    // fixed24p8_t newSamp = raw << 8;

    int oversample32 = oversample / 32;
    // How much the comparison signal changes each oversample step
    fixed24p8_t diffPerStep = (newSamp - lastSamp) >> (4 + oversample32);

    // Don't need lastSamp anymore, store this one for next round
    lastSamp = newSamp;

    for (int j = 0; j < oversample32; j++) {
        uint32_t bits = 0; // The bits we convert the sample into, MSB to go on the wire first

        for (int i = 32; i > 0; i--) {
            bits = bits << 1;
            if (cumErr < 0) {
                bits |= 1;
                cumErr += fixedPosValue - newSamp;
            } else {
                // Bits[0] = 0 handled already by left shift
                cumErr -= fixedPosValue + newSamp;
            }
            newSamp += diffPerStep; // Move the reference signal towards destination
        }
        dsBuff[j] = bits;
    }
}

bool GameI2SNoDAC::ConsumeSample(int16_t sample[2]) {
    int16_t ms[2];
    ms[0] = sample[0];
    ms[1] = sample[1];
    MakeSampleStereo16(ms);

    // Make delta-sigma filled buffer
    uint32_t dsBuff[8];
    DeltaSigma(ms, dsBuff);

    // Either send complete pulse stream or nothing
#ifdef ESP32
    size_t i2s_bytes_written = sizeof(uint32_t);
    i2s_channel_write(_tx_handle, (const char *)dsBuff, sizeof(uint32_t) * (oversample / 32), &i2s_bytes_written, 0);
    return i2s_bytes_written ? true : false;
#elif defined(ESP8266)
    if (!i2s_write_sample_nb(dsBuff[0])) {
        return false;    // No room at the inn
    }
    // At this point we've sent in first of possibly 8 32-bits, need to send
    // remaining ones even if they block.
    for (int i = 32; i < oversample; i += 32) {
        i2s_write_sample(dsBuff[i / 32]);
    }
#elif defined(ARDUINO_ARCH_RP2040)
    for (int i = 0; i < oversample / 32; i++) {
        i2s.write((int32_t)dsBuff[i], true);
    }
#endif
    return true;
}

}