#include "GameAudio.h"
#include "AudioGeneratorMP3.h"
#include "AudioGeneratorWAV.h"
#include "AudioGeneratorFLAC.h"
namespace GameAudio{
GameAudioMixer::GameAudioMixer(FileType type1, FileType type2, GameI2S* out, FS& fs, size_t buf_size) : GameAudio(out,fs),_buff_size(buf_size){
    _init(0);
    _init(1);
    _init_dec(0,type1);
    _init_dec(1,type2);
    _auto = false;
    // set_volume(5);
}
GameAudioMixer::GameAudioMixer(GameI2S* out,FS &fs,size_t buf_size) : GameAudio(out,fs),_buff_size(buf_size){
    _init(0);
    _init(1);
    _auto = true;
    // set_volume(5);
}
GameAudioMixer::~GameAudioMixer(){
    for(int i=0; i<SOUND_TYPE_MAX; i++){
        _end(i);
    }
    if(mixer){
        delete mixer;
        mixer = nullptr;
    }
}

void GameAudioMixer::play(const char* path, int idx){
    if(_auto){
        auto type = detect_file_type(path);
        if(!dec[idx] || _file_type[idx] != type){
            _delDec(idx);
            _init_dec(idx, type);
        }else{
            _stop(idx);
        }
    }else{
        _stop(idx);
    }
    file[idx] = new AudioFileSourceFS(_fs, path);
    
    // UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.printf("栈剩余最小值: %u 字节\n", watermark);

    if(dec[idx]->begin(file[idx], stub[idx])){
        set_file_name(path, idx);
        // Serial.printf("播放%s\n", _file_name[idx]);

        // UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);
        // Serial.printf("栈剩余最小值: %u 字节\n", watermark);
    }
    // Serial.printf("free heap: %d\n", ESP.getFreeHeap());
    // if(file[idx]->isOpen()){
    //     Serial.printf("[play] mixer=%p, stub=%p, dec=%p, i2s=%p\n", (void*)mixer, (void*)stub[idx], (void*)dec[idx], (void*)_out);
    //     if(dec[idx]->begin(file[idx], stub[idx])){
    //         set_file_name(path, idx);
    //         Serial.printf("播放%s\n", _file_name[idx]);
    //     }
    // }else{
    //     Serial.printf("文件打开失败: %s\n", path);
    //     set_file_name("", idx);
    // }
}
void GameAudioMixer::_replay(uint8_t idx){
    // Serial.printf("[_replay] mixer=%p, stub=%p, dec=%p\n", (void*)mixer, (void*)stub[idx], (void*)dec[idx]);
    _stop(idx);
    if(_file_name[idx] != ""){
        // Serial.printf("重播%s\n",_file_name[idx]);
        file[idx] = new AudioFileSourceFS(_fs,_file_name[idx]);
        if(!dec[idx]->begin(file[idx], stub[idx])){
            set_file_name("", idx);
        }
    }
}
void  GameAudioMixer::loop(){
    // Serial.println("00");
    if(!mixer) return;
    bool loop=false;
    // Serial.println("10");
    if(dec[BGM]&&dec[BGM]->isRunning()){
        loop = true;
        // Serial.println("11");
        if(!dec[BGM]->loop()){
            if(_play_mode == MUSIC_SINGLE_PLAY) _stop(BGM);
            else if(_play_mode == MUSIC_SINGLE_LOOP) _replay(BGM);
        }
        // Serial.println("12");
    }
    // Serial.println("20");
    if(dec[SFX]&&dec[SFX]->isRunning()){
        loop = true;
        // Serial.println("21");
        if(!dec[SFX]->loop()) _stop(SFX);
        // Serial.println("22");
    }
    // Serial.println("30");
    if(!loop) mixer->loop();
    // Serial.println("40");
}
void GameAudioMixer::stop(){
    // Serial.printf("[stop] mixer=%p\n", (void*)mixer);
    // if(mixer) mixer->stop();
    for(int i=0; i<SOUND_TYPE_MAX; i++){
        _stop(i);
    }
}
void GameAudioMixer::end(){
    // Serial.printf("[end] mixer=%p\n", (void*)mixer);
    _out->stop();
    for(int i=0; i<SOUND_TYPE_MAX; i++){
        _end(i);
    }
    if(mixer){
        mixer->stop();
        mixer->flush();
        delete mixer;
        mixer = nullptr;
    }
}

void GameAudioMixer::_init(uint8_t idx){
    if(!mixer){
        mixer = new AudioOutputMixer(_buff_size,_out);
        if(_freq>0) mixer->SetRate(_freq);
        if(_channel>0) mixer->SetChannels(_channel);
    }
    if(!stub[idx]){
        stub[idx] = mixer->NewInput();
        stub[idx]->SetGain(0.5/(2*idx+1));
        if(_freq>0) stub[idx]->SetRate(_freq);
        if(_channel>0) stub[idx]->SetChannels(_channel);
    }
}
void GameAudioMixer::_init_dec(uint8_t idx, FileType type){
    if(!dec[idx]){
        _file_type[idx] = type;
        if(type == FILE_TYPE_MP3){
            dec[idx] = new AudioGeneratorMP3();
        }else if(type == FILE_TYPE_WAV){
            dec[idx] = new AudioGeneratorWAV();
        }else if(type == FILE_TYPE_FLAC){
            dec[idx] = new AudioGeneratorFLAC();
        }else{
            dec[idx] = new AudioGeneratorMP3();
        }
    }
}
void GameAudioMixer::_stop(uint8_t idx){
    // Serial.printf("[_stop] mixer=%p, stub=%p, dec=%p\n", (void*)mixer, (void*)stub[idx], (void*)dec[idx]);
    if(dec[idx] && dec[idx]->isRunning()){
        if(dec[idx]->isRunning()) dec[idx]->stop();
    }
    if(stub[idx]){
        stub[idx]->stop();
        stub[idx]->flush();
    }
    if(_out){
    }
    if(file[idx]){
        delete file[idx];
        file[idx] = nullptr;
    }
}
void GameAudioMixer::_delDec(uint8_t idx){
    // Serial.printf("[_delDec] mixer=%p, stub=%p, dec=%p\n", (void*)mixer, (void*)stub[idx], (void*)dec[idx]);
    if(stub[idx]){
        stub[idx]->stop();
        stub[idx]->flush();
    }
    if( dec[idx] ){
        if(dec[idx]->isRunning()) dec[idx]->stop();
        delete dec[idx];
        dec[idx] = nullptr;
    }
    if(_out){
    }
    if(file[idx]){
        delete file[idx];
        file[idx] = nullptr;
    }
}
void GameAudioMixer::_end(uint8_t idx){
    // Serial.printf("[_end] mixer=%p, stub=%p, dec=%p\n", (void*)mixer, (void*)stub[idx], (void*)dec[idx]);
    if(stub[idx]){
        stub[idx]->stop();
        stub[idx]->flush();
        delete stub[idx];
        stub[idx] = nullptr;
    }
    if( dec[idx] ){
        if(dec[idx]->isRunning()) dec[idx]->stop();
        delete dec[idx];
        dec[idx] = nullptr;
    }
    if(file[idx]){
        delete file[idx];
        file[idx] = nullptr;
    }
    set_file_name("", idx);
}
} // namespace GameAudioMixer