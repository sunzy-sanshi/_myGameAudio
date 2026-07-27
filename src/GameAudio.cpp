#include "GameAudio.h"
namespace GameAudio{


// bool GameI2S::SetPins(int bclk, int wclk, int dout, int din, int mclk){
//     if (i2sOn) { return false; }
//     bclkPin = bclk;
//     wclkPin = wclk;
//     doutPin = dout;
//     dinPin = din;
//     mclkPin = mclk;
//     return true;
// }

// void GameI2S::SetVolume(int8_t val){
//     _volume = val<0 ? 0: (val>100?100:val);
//     if(_volume>70){
//         SetGain(float(_volume-70)/15.2 + 2.0);
//     }else if(_volume>40){
//         SetGain(float(_volume-40)/20.0 + 0.5);
//     }else if(_volume>10){
//         SetGain(float(_volume-10)/75.0 + 0.1);
//     }else{
//         SetGain(float(_volume)/100.0);
//     }
//     // SetGain(float(_volume)/100.0);
// }
// size_t GameI2S::pushMono(int16_t* buf, size_t len){
//     if(!i2sOn || !buf || len == 0) return 0;
//     size_t pushed = 0;
//     for(size_t i = 0; i < len; i++) {
//         int16_t samples_array[] = {buf[i], buf[i]};
//         if(ConsumeSample(samples_array))  break;  // 缓冲区满
//         pushed++;
//     }
//     return pushed;
// }
// size_t GameI2S::pushStereo(int16_t* buf, size_t len){
//     uint16_t pushed = ConsumeSamples(buf, len/2);
//     // if(!i2sOn || !buf || len == 0) return 0;
//     // size_t pushed = 0;
//     // for(size_t i = 0; i < len; i += 2) {
//     //     int16_t samples_array[] = {buf[i], buf[i + 1]};
//     //     if(!ConsumeSample(samples_array)) break;  // 缓冲区满
//     //     pushed += 2;  // 推送了两个样本
//     // }
//     return 2*pushed;
// }
// size_t GameI2S::pushBatch(int16_t* buf, size_t len, bool is_mono){
//     // 双声道len=samples*2, 单声道len=samples
//     return is_mono ? pushMono(buf, len) : pushStereo(buf, len);
// }

// bool GameI2S::stop(){
//     if (!i2sOn) { return false; }
//     #ifdef ESP32
//         if (_tx_handle) {
//             i2s_channel_disable(_tx_handle);
//             i2s_del_channel(_tx_handle);
//             _tx_handle = nullptr;
//         }
//         if (_rx_handle) {
//             i2s_channel_disable(_rx_handle);
//             i2s_del_channel(_rx_handle);
//             _rx_handle = nullptr;
//         }
//     #elif defined(ESP8266)
//         i2s_end();
//     #elif defined(ARDUINO_ARCH_RP2040)
//         i2s.end();
//     #endif
//     i2sOn = false;
//     return true;
// }
// bool GameI2S::begin(){ 
//     #ifdef ESP32
//         i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
//         chan_cfg.dma_desc_num = _buffers;
//         chan_cfg.dma_frame_num = _bufferWords;
//         chan_cfg.auto_clear = _auto_clear;
//         if (_isOutput) {
//             assert(ESP_OK == i2s_new_channel(&chan_cfg, &_tx_handle, nullptr));  // TX
//         } else {
//             assert(ESP_OK == i2s_new_channel(&chan_cfg, nullptr, &_rx_handle));  // RX ← 这里配置
//         }
//         i2s_std_config_t std_cfg = {
//             .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(hertz),
//             .slot_cfg = _slot_cfg,
//             .gpio_cfg = {
//                 .mclk = mclkPin < 0 ? I2S_GPIO_UNUSED : (gpio_num_t)mclkPin,
//                 .bclk = bclkPin < 0 ? I2S_GPIO_UNUSED : (gpio_num_t)bclkPin,
//                 .ws = wclkPin < 0 ? I2S_GPIO_UNUSED : (gpio_num_t)wclkPin,
//                 .dout = doutPin < 0 ? I2S_GPIO_UNUSED : (gpio_num_t)doutPin,
//                 .din = dinPin < 0 ? I2S_GPIO_UNUSED : (gpio_num_t)dinPin,
//                 .invert_flags = {
//                     .mclk_inv = false,
//                     .bclk_inv = false,
//                     .ws_inv = false,
//                 },
//             },
//         };
//         #if SOC_CLK_APLL_SUPPORTED
//             std_cfg.clk_cfg.clk_src = _useAPLL ? i2s_clock_src_t::I2S_CLK_SRC_APLL : i2s_clock_src_t::I2S_CLK_SRC_DEFAULT;
//         #endif
//         std_cfg.slot_cfg.bit_shift = !lsb_justified; // I2S = shift, LSBJ = no shift
//         if (_isOutput) {
//             assert(ESP_OK == i2s_channel_init_std_mode(_tx_handle, &std_cfg));
//             // Fill w/0s to start off
//             int16_t a[2] = {0, 0};
//             size_t written = 0;
//             do {
//                 i2s_channel_preload_data(_tx_handle, (void*)a, sizeof(a), &written);
//             } while (written);

//             i2sOn = (ESP_OK == i2s_channel_enable(_tx_handle));
//         } else {
//             assert(ESP_OK == i2s_channel_init_std_mode(_rx_handle, &std_cfg));
//             i2sOn = (ESP_OK == i2s_channel_enable(_rx_handle));
//         }
//     #elif defined(ESP8266)
//     #elif defined(ARDUINO_ARCH_RP2040)
//     #endif
//     Serial.printf("I2S config: hertz=%u, _slot_cfg.bit_shift=%d, _auto_clear=%d\n", hertz, _slot_cfg.bit_shift, _auto_clear);
//     Serial.printf("GPIO: bclk=%d, ws=%d, dout=%d, mclk=%d\n",
//     bclkPin, wclkPin, doutPin, mclkPin);
//     SetRate(hertz ? hertz : 44100); // Default
//     SetVolume(_volume);
//     return true;
// }


GameAudio::~GameAudio(){
    for(int i=0;i<SOUND_TYPE_MAX;i++){
        if(stub[i]){
            delete stub[i];
            stub[i]=nullptr;
        }
        if(dec[i]){
            delete dec[i];
            dec[i]=nullptr;
        }
        if(file[i]){
            delete file[i];
            file[i]=nullptr;
        }
    }
    if(mixer){
        delete mixer;
        mixer=nullptr;
    }
}
void GameAudio::set_file_name(const char* name, int idx){
    if(name){
        if(_file_name[idx]){
            free(_file_name[idx]);
        }
        _file_name[idx]=strdup(name);
        //snprintf(_file_name[idx], 32, "%s", name);
    }else{
        if(_file_name[idx]){
            free(_file_name[idx]);
            _file_name[idx]=nullptr;
        }

        // _file_name[idx][0]='\0';
    }
}
FileType GameAudio::detect_file_type(const char* path){
    const char* dot = strrchr(path, '.');
    if(!dot) return FILE_TYPE_MP3;  // 默认
    else if(strcasecmp(dot, ".wav")==0) return FILE_TYPE_WAV;
    else if(strcasecmp(dot, ".flac")==0) return FILE_TYPE_FLAC;
    return FILE_TYPE_MP3;  // .mp3及其他默认
}
// size_t GameAudio::push_mono(int16_t* buf, size_t len){
//     if(!_out || !buf || len == 0) return 0;
//     size_t pushed = 0;
//     for(size_t i = 0; i < len; i++) {
//         int16_t samples_array[] = {buf[i], buf[i]};
//         if(!_out->ConsumeSample(samples_array))  break;  // 缓冲区满
//         pushed++;
//     }
//     return pushed;
// }
// size_t GameAudio::push_stereo(int16_t* buf, size_t len) {
//     if(!_out || !buf || len == 0) return 0;
//     size_t pushed = 0;
//     for(size_t i = 0; i < len; i += 2) {
//         int16_t samples_array[] = {buf[i], buf[i + 1]};
//         if(!_out->ConsumeSample(samples_array)) break;  // 缓冲区满
//         pushed += 2;  // 推送了两个样本
//     }
//     return pushed;
// }
// size_t GameAudio::push_batch(int16_t* buf, size_t samples) {
//     uint16_t pushed_pairs = _out->ConsumeSamples(buf, samples);
//     return pushed_pairs * 2;
// }
void GameAudio::audio_task_esp8266audio(void *buffer, int length) 
{
    // static GameI2S* audio = nullptr;
    // static float volume = 0.156f;  // 40/255 ≈ 0.156
    // int16_t* input = (int16_t*)buffer;
    // int samples = length / sizeof(int16_t);
    // for (int i = 0; i < samples; i += 2) {
    //     int16_t samples_array[2];
    //     samples_array[0] = input[i+1];  // 左声道
    //     samples_array[1] = input[i];    // 右声道
    //     if(!audio->ConsumeSample(samples_array)) {
    //         // 缓冲区满
    //         break;
    //     }
    // }
}

}  // namespace