#pragma once
#include "IAudioListener.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <vector>

class WasapiListener : public IAudioListener
{
private:
    // --- Propiedades Privadas (Estado Interno) ---
    // Punteros COM de Windows (La parte "fea" queda oculta aquí)
    IMMDeviceEnumerator *_pEnumerator = nullptr;
    IMMDevice *_pDevice = nullptr;
    IAudioClient *_pAudioClient = nullptr;
    IAudioCaptureClient *_pCaptureClient = nullptr;
    WAVEFORMATEX *_pWaveFormat = nullptr;

    bool _initialized = false;
    bool _isRecording = false;

    // Métodos auxiliares privados (Helpers)
    void CleanUp();

public:
    // Constructor / Destructor
    WasapiListener();
    ~WasapiListener();

    // --- Implementación del Contrato Público ---
    bool Initialize() override;
    void Start() override;
    void Stop() override;
    bool GetAudioData(std::vector<float> &outputBuffer) override;

    // Getter para saber el formato (útil para quien reciba los datos)
    int GetSampleRate() const;
    int GetChannels() const;
};