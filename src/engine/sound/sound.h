#pragma once
#include <dsound.h>
#include <cinttypes>
#include <string>

/**
 * This is a simplified header type I took from a book from Andre LaMothe, I believe.
 * It assumes the data chunkId where there can be a "bext" chunk in fact.
 * The implementation takes care of this, but
 * I also found already other non supported chunk types for now - e.g. "junk".
 *
 */
struct WaveHeaderType
{
    char chunkId[4];
    unsigned long chunkSize;
    char format[4];
    char subChunkId[4];
    unsigned long subChunkSize;
    unsigned short audioFormat;
    unsigned short numChannels;
    unsigned long sampleRate;
    unsigned long bytesPerSecond;
    unsigned short blockAlign;
    unsigned short bitsPerSample;
    char dataChunkId[4];
    unsigned long dataSize;
};

struct FmtDetails {
    uint16_t wChannels;
    uint32_t dwSamplesPerSec;
    uint32_t dwAvgBytesPerSec;
    uint16_t wBlockAlign;
    uint16_t wBitsPerSample;
};

struct WaveChunkFormat {
    char chunkId[4];
    unsigned long chunkSize;
};

struct Sound {
    LPDIRECTSOUNDBUFFER8 secondaryBuffer;
    std::string name;
};

LPDIRECTSOUND8 initAudio(HWND hwnd);
Sound* createSoundFromWaveFile(const char* fileName);
void playSound(Sound*, bool loop, long volume = -1000);
void stopSound(Sound* s);
// The sound files can be imported into Audacity and then exported as signed 16-bit PCM 44.1khz.
// I don't know why but this format just works.
Sound* loadSoundFileExt(const std::string& fileName, HWND hwnd);

