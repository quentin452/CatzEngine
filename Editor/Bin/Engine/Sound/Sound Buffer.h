#pragma once
/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************/
#if EE_PRIVATE
/******************************************************************************/
enum VIRTUALIZATION_MODE // 2D Sound Virtualization Mode
{
    VIRT_NONE, // none
    VIRT_LOW,  // low
    VIRT_HIGH, // high
};
/******************************************************************************/
#define MAX_SOUND_SPEED 2
inline Flt SoundSpeed(Flt speed) { return Mid(speed, 0.0f, (Flt)MAX_SOUND_SPEED); }

#if HAS_THREADS
#define SOUND_TIMER 25 // 25 ms which is 40 Hz/FPS, recommended value to be between 16.666 ms (60 Hz/FPS) .. 33.333 ms (30 Hz/FPS), also the callback will be triggered at least once per frame (due to 'SoundEvent' being triggered at the end of each frame to immediately process any changes), shorter timers result in smaller memory usage at the cost of additional overhead on the CPU
#define SOUND_TIME (SOUND_TIMER * 2 * 2 * MAX_SOUND_SPEED) // 2 (2 half buffers) * 2 (safety due to sounds being started at different times) * MAX_SOUND_SPEED
#define SOUND_TIME_RECORD (SOUND_TIMER * 2 * 2) // 2 (2 half buffers) * 2 (safety due to sounds being started at different times), this doesn't need MAX_SOUND_SPEED because sounds are always recorded with speed=1
#else
#define SOUND_TIMER 50
#define SOUND_TIME 1200 // when there are no threads available, set a big sound buffer, to allow some tolerance for pauses during loading
#define SOUND_TIME_RECORD 500 // when there are no threads available, set a big sound buffer, to allow some tolerance for pauses during loading
#endif

#define SOUND_SAMPLES(freq) ((freq)*SOUND_TIME / 1000) // total number of samples needed for full buffer (2 halfs) for a sound to be played using SOUND_TIMER
/******************************************************************************/
struct Stereo {
    I16 l, r;
};
struct ResampleBuffer // buffer used for resampling, holds few last samples, and sample offset
{
    Flt offset;  // sample offset carried over from previous buffers
    Int samples; // number of samples in the buffer
    union        // 3 previous samples needed for cubic interpolation, +1 used from current buffer = 4 in total
    {
        I16 mono[3];
        Stereo stereo[3];
    };

    void init() {
        offset = 0;
        samples = 0;
    }
};
const_mem_addr struct AudioBuffer {
    Byte data[SOUND_SAMPLES(48000) / 2 * SIZE(I16)]; // /2 to get size for half buffer (instead of full), "*SIZE(I16)" for 16-bit samples, here only 1-channel mono is used, to use more channels and higher frequency, multiple buffers will need to be used
};
const_mem_addr struct AudioVoice {
    Bool play, remove;
    Byte channels,
        block,
        buffers,     // how many buffers available
        queued,      // how many buffers queued for processing
        buffer_i;    // index of the buffer for processing
    Int samples,     // how many samples in a single buffer
        buffer_size, // size in bytes of a single buffer
        buffer_raw,  // bytes already processed in the current 'buffer_i' buffer for processing
        total_raw;   // bytes already processed for all buffers
    Flt speed,
        volume[2]; // volume for 2 channels
    ResampleBuffer resample_buf;
    AudioBuffer *buffer[2 * 2 * 2]; // 2halfs * 2channels * 2freq (to support 96kHz), because base is 1half * 1channel * 48kHz
    AudioVoice *next;               // next voice in list

    void update();

    ~AudioVoice();
};
/******************************************************************************/
const_mem_addr struct SoundBuffer // can be moved however 'memAddressChanged' needs to be called afterwards
{
    // manage
    void del();
    Bool create(Int frequency, Int bits, Int channels, Int samples, Bool is3D = false);

    // operations
    void memAddressChanged(); // !! can't be called when the SoundBuffer is playing/paused !!
#if OPEN_SL
    Bool emulate3D();
#endif
#if DIRECT_SOUND
    Bool lock(Int pos = 0, Int size = -1);
    void unlock();
#endif

    // get / set
    Bool is() C;
    Bool is3D() C;
    void raw(Int raw);
    Int raw() C;
    void pan(Flt pan);
    void volume(Flt volume);
    void frequency(Int frequency);
    void speed(Flt speed);
    void range(Flt range);
    Flt range() C;
    void pos(C Vec &pos);
    Vec pos() C;
    void vel(C Vec &vel);
    Vec vel() C;

    void set3DParams(C _Sound &sound, Bool pos_range, Bool speed);

    Int buffers() C {
#if CUSTOM_AUDIO
        return _voice ? _voice->buffers : 0;
#else
        return 2; // all other audios use only 2 half buffers
#endif
    }

    // stop / play
    void stop();
    void pause();
    void toggle(Bool loop);
    Bool playing() C;
    void play(Bool loop);

#if EE_PRIVATE
    void zero();
#endif
    ~SoundBuffer() { del(); }
    SoundBuffer();

#if !EE_PRIVATE
  private:
#endif
    SoundStream::Params _par;
#if DIRECT_SOUND
    Ptr _lock_data;
    UInt _lock_size;
    IDirectSoundBuffer *_s;
    IDirectSound3DBuffer *_s3d;
#elif XAUDIO
    IXAudio2SourceVoice *_sv;
    Mems<Byte> _data;
    Bool _3d;
#elif OPEN_AL
    Bool _3d;
    UInt _buffer[2], _source;
#elif OPEN_SL
    Bool _3d;
    Int _processed;
    Flt _volume, _range;
    Vec _pos;
    SLObjectItf player_object;
    SLPlayItf player_play;
    SLVolumeItf player_volume;
    SLPlaybackRateItf player_playback_rate;
    SLAndroidSimpleBufferQueueItf player_buffer_queue;
    SL3DLocationItf player_location;
    SL3DDopplerItf player_doppler;
    SL3DSourceItf player_source;
    Mems<Byte> _data;
#elif CUSTOM_AUDIO
    AudioVoice *_voice;
    Bool _3d;
    Flt _volume; // used only if _3d
#endif

    NO_COPY_CONSTRUCTOR(SoundBuffer);
};
/******************************************************************************/
#if XAUDIO
extern IXAudio2 *XAudio;
#elif CUSTOM_AUDIO
extern Thread AudioThread;
extern Int AudioOutputFreq, AudioOutputFrameSamples, AudioOutputFrameSize;
extern Ptr AudioOutputFrameData;
#endif
/******************************************************************************/
#endif
/******************************************************************************/
