#pragma once
#include "IAudioListener.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <vector>
#include <iostream>

// WasapiListener.h
// Implementación concreta usando la API nativa de Windows (WASAPI) en modo Loopback.

class WasapiListener : public IAudioListener
{
private:
    // Punteros COM para interactuar con Windows
    IMMDeviceEnumerator *_pEnumerator = nullptr;
    IMMDevice *_pDevice = nullptr;
    IAudioClient *_pAudioClient = nullptr;
    IAudioCaptureClient *_pCaptureClient = nullptr;
    WAVEFORMATEX *_pWaveFormat = nullptr;

    // Estado interno
    bool _initialized = false;
    bool _recording = false;

    // Métodos privados para limpieza interna
    void CleanUp();

public:
    WasapiListener();
    ~WasapiListener(); // El destructor se encarga de liberar memoria COM

    bool Initialize() override;
    void Start() override;
    void Stop() override;
    bool GetAudioData(std::vector<float> &outputBuffer) override;

    int GetSampleRate() const override;
    int GetChannels() const override;
};