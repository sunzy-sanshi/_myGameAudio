#include "GameAudio.h"
#include "AudioGeneratorMP3.h"
#include "AudioGeneratorWAV.h"
#include "AudioGeneratorFLAC.h"
#include "AudioFileSourceID3.h"

namespace GameAudio{
GameAudioOnly::GameAudioOnly(FileType type, GameI2S* out, FS& fs) : GameAudio(out, fs){
    _init_dec(BGM, type);
    _auto = false;
}
GameAudioOnly::GameAudioOnly(GameI2S* out, FS& fs) : GameAudio(out, fs){
    _init_dec(BGM, FILE_TYPE_WAV);
    _auto = true;
}
GameAudioOnly::~GameAudioOnly(){
    end();
}
void GameAudioOnly::play(const char* path,int idx){
    if(_auto){
        auto type = detect_file_type(path);
        if(!dec[BGM] || _file_type[BGM] != type){
            _delDec(BGM);
            _init_dec(BGM, type);
        }else{
            _stop(BGM);
        }
    }else{
        _stop(BGM);
    }
    file[BGM] = new AudioFileSourceFS( _fs,path );
    // Serial.println("[play]开始播放音频 1");
    if(dec[BGM]->begin(file[BGM], _out)){
        set_file_name(path, BGM);
    }
    // Serial.println("[play]开始播放音频 2");
}
void  GameAudioOnly::loop(){
    if(dec[BGM]&&dec[BGM]->isRunning()){
        if(!dec[BGM]->loop()){
            if(_play_mode == MUSIC_SINGLE_PLAY) _stop(BGM);
            else if(_play_mode == MUSIC_SINGLE_LOOP) _replay(BGM);
        }
    }
}
void GameAudioOnly::stop(){
    _stop(BGM);
}
void GameAudioOnly::end(){
    _delDec(BGM);
}
void GameAudioOnly::_init(uint8_t idx, int freq, int8_t channel){
    
}
void GameAudioOnly::_init_dec(uint8_t idx, FileType type){
    if(!dec[BGM]){
        _file_type[BGM] = type;
        if(_file_type[BGM] == FILE_TYPE_MP3){
            dec[BGM] = new AudioGeneratorMP3();
        }else if(_file_type[BGM] == FILE_TYPE_WAV){
            dec[BGM] = new AudioGeneratorWAV();
        }else if(_file_type[BGM] == FILE_TYPE_FLAC){
            dec[BGM] = new AudioGeneratorFLAC();
        }else{
            dec[BGM] = new AudioGeneratorMP3();
            _file_type[BGM] = FILE_TYPE_MP3;
        }
    }
}
void GameAudioOnly::_replay(uint8_t idx){
    _stop(BGM);
    if(_file_name[BGM]!=""){
        Serial.println("继续播放");
        file[BGM]= new AudioFileSourceFS(_fs,_file_name[BGM]);
        if(!dec[BGM]->begin(file[BGM], _out)){
            set_file_name("", BGM);
        }
    }
}
void GameAudioOnly::_stop(uint8_t idx){
    if(dec[BGM]){
        if(dec[BGM]->isRunning()){
            dec[BGM]->stop();
        }
    }
    if(file[BGM]){
        delete file[BGM];
        file[BGM] = nullptr;
    }
}
void GameAudioOnly::_delDec(uint8_t idx){
    if( dec[BGM] ){
        if(dec[BGM]->isRunning()){
            dec[BGM]->stop();
            delete dec[BGM];
            dec[BGM] = nullptr;
        }
    }
    if(file[BGM]){
        delete file[BGM];
        file[BGM] = nullptr;
    }
    set_file_name("", idx);
}

} // namespace GameAudioOnly