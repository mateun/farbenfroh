
#include "sound.h"
#include <cstdint>
#include <vector>
#include <dsound.h>

static LPDIRECTSOUND8 dsound8 = nullptr;


bool strequal(char* a, const std::string& b, int len) {
    for (int i = 0; i < len; i++) {
        if (a[i] != b[i]) return false;
    }

    return true;
}

Sound* loadSoundFileExt(const std::string& fileName, HWND hwnd) {
    int error;
    FILE *filePtr;
    error = fopen_s(&filePtr, fileName.c_str(), "rb");
    if (error != 0) {
        exit(   1231);
    }

    std::vector<WaveChunkFormat> chunkFormats;
    uint8_t riffHeader[4];
    uint32_t waveSize;
    char formType[4];


    fread(riffHeader, 1, 4, filePtr);
    fread(&waveSize, 4, 1, filePtr);
    fread(formType, 1, 4, filePtr);

    if (!strequal(formType, "WAVE", 4)) {
        exit(1);
    }

    char potentialJunk[4];
    fread(potentialJunk, 1, 4, filePtr);
    if (strequal(potentialJunk, "JUNK", 4)) {
        uint32_t junkSize;
        fread(&junkSize, 1, 4, filePtr);
        fseek(filePtr, junkSize+4, SEEK_CUR);
    } else {
        if (!strequal(potentialJunk, "fmt ", 4)) {
            exit(2);
        }
    }

    uint32_t fmtSize;
    fread(&fmtSize, 1, 4, filePtr);

    uint16_t wFormatTag;
    fread(&wFormatTag, 1, 2, filePtr);
    if (wFormatTag != WAVE_FORMAT_PCM) {
        exit(1);
    }

    uint16_t channels;
    fread(&channels, 1, 2, filePtr);

    uint32_t samplesPerSec;
    fread(&samplesPerSec, 1, 4, filePtr);

    uint32_t avgBytesPerSec;
    fread(&avgBytesPerSec, 1, 4, filePtr);

    uint16_t blockAlign;
    fread(&blockAlign, 1, 2, filePtr);

    uint16_t  bitsPerSample;
    fread(&bitsPerSample, 1, 2, filePtr);

    char dataId[4];
    fread(dataId, 1, 4, filePtr);
    if (!strequal(dataId, "data", 4)) {
        // no data found, but should be here
        if (!strequal(dataId, "bext", 4)) {
            exit(2);
        } else {
            // skip over bext metadata
            uint32_t bextSize;
            fread(&bextSize, 1, 4, filePtr);
            fseek(filePtr, bextSize, SEEK_CUR);
            fread(&dataId, 1, 4, filePtr);
            if (!strequal(dataId, "data", 4)) {
                exit(4);
            }
        }
    }
    uint32_t dataSize;
    fread(&dataSize, 1, 4, filePtr);

    // We are finally at the actual sound data. Now we take care for the DirectSound interface:
    // Set the wave format of secondary buffer that this wave file will be loaded onto.
    if (!dsound8) {
        dsound8  = initAudio(hwnd);
    }
    WAVEFORMATEX waveFormat;
    DSBUFFERDESC bufferDesc;
    IDirectSoundBuffer *tempBuffer;
    unsigned char *waveData;
    unsigned char *bufferPtr;
    unsigned long bufferSize;
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nSamplesPerSec = samplesPerSec;
    waveFormat.wBitsPerSample = bitsPerSample;
    waveFormat.nChannels = channels;
    waveFormat.nBlockAlign = blockAlign;
    waveFormat.nAvgBytesPerSec = avgBytesPerSec;
    waveFormat.cbSize = 0;

    // Set the buffer description of the secondary sound buffer that the wave file will be loaded onto.
    bufferDesc.dwSize = sizeof(DSBUFFERDESC);
    bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
    bufferDesc.dwBufferBytes = dataSize;
    bufferDesc.dwReserved = 0;
    bufferDesc.lpwfxFormat = &waveFormat;
    bufferDesc.guid3DAlgorithm = GUID_NULL;

    // Create a temporary sound buffer with the specific buffer settings.
    HRESULT result = dsound8->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
    if (FAILED(result)) {
        exit(1);
    }

    LPDIRECTSOUNDBUFFER8 secondaryBuffer;
    // Test the buffer format against the direct sound 8 interface and create the secondary buffer.
    result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void **) &secondaryBuffer);
    if (FAILED(result)) {
        exit(1);
    }

    // Release the temporary buffer.
    tempBuffer->Release();
    tempBuffer = 0;


    // Lock the secondary buffer to write wave data into it.
    result = (secondaryBuffer)->Lock(0, dataSize, (void **) &bufferPtr, (DWORD *) &bufferSize, NULL, 0,0);

    if (FAILED(result)) {
        exit(3);
    }

    // Read the actual sound data and copy it to the DSOUND buffer:
    waveData = new uint8_t [dataSize];
    fread(waveData, 1, dataSize, filePtr);
    memcpy(bufferPtr, waveData, dataSize);

    // Unlock the secondary buffer after the data has been written to it.
    result = (secondaryBuffer)->Unlock((void *) bufferPtr, bufferSize, NULL, 0);
    if (FAILED(result)) {
        exit(1);
    }

    // Release the wave data since it was copied into the secondary buffer.
    delete[] waveData;
    waveData = 0;

    // Set position at the beginning of the sound buffer.
    result = secondaryBuffer->SetCurrentPosition(0);
    if (FAILED(result)) {
        exit(1);
    }

    // Set volume of the buffer to 100%.
    result = secondaryBuffer->SetVolume(DSBVOLUME_MAX);
    if (FAILED(result)) {
        exit(1);
    }

    return new Sound{secondaryBuffer, fileName};

}

Sound *createSoundFromWaveFile(const char *fileName, HWND hwnd) {
    if (!dsound8) {
        dsound8  = initAudio(hwnd);
    }
    int error;
    FILE *filePtr;
    unsigned int count;
    WaveHeaderType waveFileHeader;
    WAVEFORMATEX waveFormat;
    DSBUFFERDESC bufferDesc;
    HRESULT result;
    IDirectSoundBuffer *tempBuffer;
    unsigned char *waveData;
    unsigned char *bufferPtr;
    unsigned long bufferSize;
    bool isBext = false;

    error = fopen_s(&filePtr, fileName, "rb");
    if (error != 0) {
        exit(1);
    }

    // Read in the wave file header.
    count = fread(&waveFileHeader, sizeof(waveFileHeader), 1, filePtr);
    if (count != 1) {
        exit(1);
    }

    // Check that the chunk ID is the RIFF format.
    if ((waveFileHeader.chunkId[0] != 'R') || (waveFileHeader.chunkId[1] != 'I') ||
        (waveFileHeader.chunkId[2] != 'F') || (waveFileHeader.chunkId[3] != 'F')) {
        exit(1);
    }

    // Check that the file format is the WAVE format.
    if ((waveFileHeader.format[0] != 'W') || (waveFileHeader.format[1] != 'A') ||
        (waveFileHeader.format[2] != 'V') || (waveFileHeader.format[3] != 'E')) {
        exit(1);
    }

    // Check that the sub chunk ID is the fmt format.
    if ((waveFileHeader.subChunkId[0] != 'f') || (waveFileHeader.subChunkId[1] != 'm') ||
        (waveFileHeader.subChunkId[2] != 't') || (waveFileHeader.subChunkId[3] != ' ')) {
        exit(1);
    }

    // Check that the audio format is WAVE_FORMAT_PCM.
    if (waveFileHeader.audioFormat != WAVE_FORMAT_PCM) {
        exit(1);
    }

    // Check that the wave file was recorded in stereo format.
    if (waveFileHeader.numChannels != 2) {
        exit(1);
    }

    // Check that the wave file was recorded at a sample rate of 44.1 KHz.
    /*if (waveFileHeader.sampleRate != 44100)
    {
        exit(1);
    }*/

    // Ensure that the wave file was recorded in 16 bit format.
    if (waveFileHeader.bitsPerSample != 16) {
        exit(1);
    }

    // Check for the data chunk header.
    if ((waveFileHeader.dataChunkId[0] != 'd') || (waveFileHeader.dataChunkId[1] != 'a') ||
        (waveFileHeader.dataChunkId[2] != 't') || (waveFileHeader.dataChunkId[3] != 'a')) {

        if ((waveFileHeader.dataChunkId[0] == 'b') && waveFileHeader.dataChunkId[1] == 'e'
            && waveFileHeader.dataChunkId[2] == 'x' && waveFileHeader.dataChunkId[3] == 't') {
            isBext = true;
        }
    }



    // Move to the beginning of the wave data which starts at the end of the data(or bext) chunk header.
    fseek(filePtr, sizeof(WaveHeaderType), SEEK_SET);

    if (!isBext) {
        // Create a temporary buffer to hold the wave file data.
        waveData = new unsigned char[waveFileHeader.dataSize];
        if (!waveData) {
            exit(1);
        }

        // Read in the wave file data into the newly created buffer.
        count = fread(waveData, 1, waveFileHeader.dataSize, filePtr);
        if (count != waveFileHeader.dataSize) {
            exit(1);
        }
    } else {
        // First we read bext chunk, then the data chunk (if we find it right after bext).
        auto bextData = new uint8_t[waveFileHeader.dataSize];
        auto bextCount = fread(bextData, 1, waveFileHeader.dataSize, filePtr);
        if (bextCount != waveFileHeader.dataSize) {
            exit(2);
        }
        uint8_t dataCkId[4];
        fread(dataCkId, 1, 4, filePtr);
        if (dataCkId[0] != 'd') {
            // Where is the data chunk??
            exit(3);
        }
        fread(&waveFileHeader.dataSize, 1, 4, filePtr);
        printf("actual data chunk size: %u\n", waveFileHeader.dataSize);
        waveData = new unsigned char [waveFileHeader.dataSize];
        if (!waveData) {
            exit(1);
        }

        // Read in the wave file data into the newly created buffer.
        count = fread(waveData, 1, waveFileHeader.dataSize, filePtr);
        if (count != waveFileHeader.dataSize) {
            exit(1);
        }
    }





    // Close the file once done reading.
    error = fclose(filePtr);
    if (error != 0) {
        exit(1);
    }

    // Set the wave format of secondary buffer that this wave file will be loaded onto.
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nSamplesPerSec = waveFileHeader.sampleRate;
    waveFormat.wBitsPerSample = 16;
    waveFormat.nChannels = 2;
    waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize = 0;

    // Set the buffer description of the secondary sound buffer that the wave file will be loaded onto.
    bufferDesc.dwSize = sizeof(DSBUFFERDESC);
    bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
    bufferDesc.dwBufferBytes = waveFileHeader.dataSize;
    bufferDesc.dwReserved = 0;
    bufferDesc.lpwfxFormat = &waveFormat;
    bufferDesc.guid3DAlgorithm = GUID_NULL;

    // Create a temporary sound buffer with the specific buffer settings.
    result = dsound8->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
    if (FAILED(result)) {
        exit(1);
    }

    LPDIRECTSOUNDBUFFER8 secondaryBuffer;
    // Test the buffer format against the direct sound 8 interface and create the secondary buffer.
    result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void **) &secondaryBuffer);
    if (FAILED(result)) {
        exit(1);
    }

    // Release the temporary buffer.
    tempBuffer->Release();
    tempBuffer = 0;


    // Lock the secondary buffer to write wave data into it.
//    result = (secondaryBuffer)->Lock(0, waveFileHeader.dataSize, (void **) &bufferPtr, (DWORD *) &bufferSize, NULL, 0,
//                                     0);
    unsigned char *bufferPtr2;
    unsigned long bufferSize2;
    result = (secondaryBuffer)->Lock(0, waveFileHeader.dataSize, (void **) &bufferPtr, &bufferSize, (void **) &bufferPtr2, &bufferSize2,
                                     DSBLOCK_ENTIREBUFFER);
    if (FAILED(result)) {
        exit(3);
    }

    // Copy the wave data into the buffer.
    memcpy(bufferPtr, waveData, waveFileHeader.dataSize);

    // Unlock the secondary buffer after the data has been written to it.
    result = (secondaryBuffer)->Unlock((void *) bufferPtr, bufferSize, NULL, 0);
    if (FAILED(result)) {
        exit(1);
    }

    // Release the wave data since it was copied into the secondary buffer.
    delete[] waveData;
    waveData = 0;

    // Set position at the beginning of the sound buffer.
    result = secondaryBuffer->SetCurrentPosition(0);
    if (FAILED(result)) {
        exit(1);
    }

    // Set volume of the buffer to 100%.
    result = secondaryBuffer->SetVolume(DSBVOLUME_MAX);
    if (FAILED(result)) {
        exit(1);
    }

    Sound *s = new Sound{secondaryBuffer, fileName};

    return s;
}

void playSound(Sound *sound, bool loop, long volume) {
    DWORD flags = loop ? DSBPLAY_LOOPING : 0;
    sound->secondaryBuffer->SetCurrentPosition(0);
    sound->secondaryBuffer->SetVolume(volume);
    sound->secondaryBuffer->Play(0, 0, flags);


}

void stopSound(Sound *s) {
    s->secondaryBuffer->Stop();
}

LPDIRECTSOUND8 initAudio(HWND hwnd) {
    LPDIRECTSOUNDBUFFER sbPrimary;
    LPDIRECTSOUND8 dsound8;

    HRESULT hr = DirectSoundCreate8(NULL, &dsound8, NULL);
    if (FAILED(hr)) {
        printf("sound error: %d", hr);
        exit(99);
    }

    dsound8->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
    DSBUFFERDESC dd;
    memset(&dd, 0, sizeof(DSBUFFERDESC));
    dd.dwSize = sizeof(DSBUFFERDESC);
    dd.dwFlags = DSBCAPS_PRIMARYBUFFER;
    dd.dwBufferBytes = 0;
    dd.lpwfxFormat = NULL;

    hr = dsound8->CreateSoundBuffer(&dd, &sbPrimary, NULL);
    if (FAILED(hr)) {
        printf("sound buffer error %d", hr);
        exit(1);
    }


    WAVEFORMATEX wfx;
    memset(&wfx, 0, sizeof(WAVEFORMATEX));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2;
    wfx.nSamplesPerSec = 44100;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (wfx.wBitsPerSample * wfx.nChannels) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    hr = sbPrimary->SetFormat(&wfx);
    if (FAILED(hr)) {
        printf("error while setting format for primary soundbuffer\n");
        exit(1);
    }

    // Secondary buffer
    LPDIRECTSOUNDBUFFER buf;
    memset(&dd, 0, sizeof(DSBUFFERDESC));
    dd.dwSize = sizeof(DSBUFFERDESC);
    dd.dwFlags = DSBCAPS_STATIC;
    dd.dwBufferBytes = wfx.nSamplesPerSec * 2 * 16;        // 1 second for 2 bytes per sample (16bit)
    dd.lpwfxFormat = &wfx;

    hr = dsound8->CreateSoundBuffer(&dd, &buf, NULL);
    if (FAILED(hr)) {
        printf("secondary sound buffer error %d", hr);
        exit(110);
    }

    uint16_t *audio_ptr_1;
    uint16_t *audio_ptr_2;
    DWORD audio_l_1;
    DWORD audio_l_2;

    hr = buf->Lock(0, dd.dwBufferBytes, (void **) &audio_ptr_1, &audio_l_1, (void **) &audio_ptr_2, &audio_l_2,
                   DSBLOCK_ENTIREBUFFER);
    if (FAILED(hr)) {
        printf("secondary sound buffer lock error %d", hr);
        exit(111);
    }

    // Play a little square-wave test sound here.
    // Current sample per "note"
    int currentSample = 0;
    // The overall position in the soundbuffer.
    int writePosition = 0;
    int freqs[12] = {26, 13, 55, 26, 52, 96,
                     10, 20, 55, 26, 52, 75};
    float durations[12] = {1, 0.5, 0.5, 0.5, 0.5, 2,
                           1, 0.5, 0.5, 0.5, 0.5, 2};
    for (int f = 0; f < 12; f++) {
        int samplesPerCycle = wfx.nSamplesPerSec / freqs[f];
        float duration = durations[f];
        currentSample = 0;
        while (currentSample < wfx.nSamplesPerSec * duration) {
            for (int i = 0; i < samplesPerCycle / 2; i++) {
                if (currentSample < audio_l_1 / 2) {
                    audio_ptr_1[writePosition] = 2000;
                }
                currentSample++;
                writePosition++;
            }
            for (int i = 0; i < samplesPerCycle / 2; i++) {
                if (currentSample < audio_l_1 / 2) {
                    audio_ptr_1[writePosition] = -2000;
                }
                currentSample++;
                writePosition++;
            }
        }
    }

    buf->Play(0, 0, 0);

    return dsound8;


}


