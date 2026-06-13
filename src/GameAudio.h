#ifndef GAME_AUDIO_H
#define GAME_AUDIO_H
#include <cstdint>
#include <cstddef>
#include <string>
#include <Arduino.h>

#include "FS.h"

#include "AudioFileSourceFS.h"
#include "AudioGenerator.h"
#include "AudioOutputI2S.h"
#include "AudioOutputMixer.h"
namespace GameAudio{
enum SoundType{
    BGM,
    SFX,
    SOUND_TYPE_MAX,
};
enum FileType{
    FILE_TYPE_NULL=0,
    FILE_TYPE_WAV,
    FILE_TYPE_MP3,
    FILE_TYPE_FLAC,
    FILE_TYPE_MAX
};
typedef enum {
    MUSIC_SINGLE_PLAY=0,  // 单曲播放：播放一次当前歌曲后停止
    MUSIC_SINGLE_LOOP,    // 单曲循环：循环播放当前歌曲
    MUSIC_SEQUENTIAL,     // 顺序播放：按列表顺序播放，最后一首后停止
    MUSIC_LIST_LOOP,      // 列表循环/循环播放：顺序播放，最后一首后回到第一首循环
    MUSIC_PLAY_RANDOM     // 随机播放：从列表中随机选择歌曲播放
} MusicPlayMode_t;

// 根据咋们的板子，重写I2S函数
class GameI2S : public AudioOutputI2S{
    uint8_t _volume = 5; // 0 - 100
    int8_t dinPin;
    bool _isOutput = true;  // true=OUT, false=IN
    bool _auto_clear = true;
    i2s_chan_handle_t _rx_handle = nullptr;
    i2s_std_slot_config_t _slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
  public:
    void SetAutoClear(bool auto_clear=true){if(i2sOn) return; _auto_clear = auto_clear;}
    void SetStdSlotCfg(i2s_std_slot_config_t slot_cfg){if(i2sOn) return; _slot_cfg = slot_cfg;}
    bool SetPins(int bclk, int wclk, int dout, int din = -1, int mclk = -1);
    void SetOutput(bool isOutput = true){if(i2sOn) return; _isOutput = isOutput;}
    void SetVolume(int8_t val);
    uint8_t GetVolume(void){return _volume;}
    size_t pushMono(int16_t* buf, size_t len);
    size_t pushStereo(int16_t* buf, size_t len);
    size_t pushBatch(int16_t* buf, size_t len, bool is_mono=false);
    bool stop() override;
    bool begin() override;
};
class GameAudio{
  public:
    GameAudio(GameI2S* out, FS& fs): _out(out), _fs(fs){};
    ~GameAudio();
    virtual bool init(){return true;}
    void set_play_mode(MusicPlayMode_t mode){ _play_mode=mode; }
    uint8_t get_play_mode(){ return _play_mode; }
    virtual void play(const char* path, int idx=0){}
    // virtual void replay(){}
    virtual void stop(){}
    virtual void loop(){}
    virtual void end(){}

    inline void set_volume(int8_t val){_out->SetVolume(val);}//_out->SetGain( float(_volume)/100.0 ); }
    inline uint8_t get_volume(){return _out->GetVolume();}

    inline void set_freq(int16_t val){ _freq = val; }
    inline void set_channel(int8_t val){ _channel = val; }
    inline const char* get_file_name(int idx){ return _file_name[idx]; }
    void set_file_name(const char* name, int idx);
    FileType detect_file_type(const char* path);
    // size_t push_mono(int16_t* buf, size_t len);
    // size_t push_stereo(int16_t* buf, size_t len);
    // size_t push_batch(int16_t* buf, size_t len);
    void audio_task_esp8266audio(void *buffer, int length);

  protected: 
    bool _auto=false; // 自动根据文件类型播放
    AudioGenerator *dec[2]={nullptr,nullptr};
    // AudioGeneratorWAV *wav[2];
    AudioFileSource *file[2]={nullptr,nullptr};
    AudioOutputMixer *mixer = nullptr;
    AudioOutputMixerStub *stub[2]={nullptr,nullptr};

    int16_t _freq=-1;
    int8_t _channel=-1;

    char _file_name[2][32]={"",""};
    FileType    _file_type[2]={FILE_TYPE_MP3,FILE_TYPE_WAV};

    GameI2S* _out=nullptr;
    FS &_fs;
    // uint8_t _volume = 2;
    MusicPlayMode_t _play_mode = MUSIC_SINGLE_LOOP;
};

class GameAudioOnly : public GameAudio{
  public:
    GameAudioOnly(FileType type, GameI2S* out, FS& fs);
    GameAudioOnly(GameI2S* out, FS& fs);
    ~GameAudioOnly();
    void play(const char* path,int idx=0) override;
    void stop() override;
    void loop() override;
    void end() override;
  private:
    // AudioGenerator *dec[2]={nullptr,nullptr};
    // // AudioGeneratorWAV *wav[2];
    // AudioFileSource *file[2]={nullptr,nullptr};

    // std::string _file_name[2]={"",""};
    // FileType    _file_type[2]={FILE_TYPE_MP3,FILE_TYPE_WAV};
    void _init(uint8_t idx, int freq=-1, int8_t channel=-1);
    void _init_dec(uint8_t idx, FileType type);
    void _replay(uint8_t idx);
    void _stop(uint8_t idx);
    void _delDec(uint8_t idx);

};
class GameAudioMixer : public GameAudio{
  public:
    // 带混声
    // 设置音效与背景音响度比例
    GameAudioMixer(GameI2S* out,FS& fs,size_t buf_size=256);
    GameAudioMixer(FileType type1, FileType type2, GameI2S* out, FS& fs, size_t buf_size=256);
    ~GameAudioMixer();
    void play(const char* path,int idx=0) override;
    void stop() override;
    void loop() override;
    void end() override;
  private:
    size_t _buff_size=0;

    void _init(uint8_t idx);
    void _init_dec(uint8_t idx, FileType type);
    void _replay(uint8_t idx);
    void _stop(uint8_t idx);
    void _delDec(uint8_t idx);
    void _end(uint8_t idx);
};

} // namespace GameAudio
#endif