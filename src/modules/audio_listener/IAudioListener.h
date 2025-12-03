#pragma once
#include <vector>

// IAudioListener.h
// Interfaz abstracta para estandarizar la entrada de audio.
// Permite cambiar la implementación (WASAPI, ASIO, Linux ALSA) sin romper el programa principal.

class IAudioListener
{
public:
    virtual ~IAudioListener() {}

    // Prepara los recursos del sistema (Drivers, memoria)
    virtual bool Initialize() = 0;

    // Comienza la captura de audio
    virtual void Start() = 0;

    // Detiene la captura
    virtual void Stop() = 0;

    // Extrae los datos disponibles en el buffer.
    // Retorna true si se obtuvieron datos, false si hubo silencio o error.
    virtual bool GetAudioData(std::vector<float> &outputBuffer) = 0;

    // Getters de configuración
    virtual int GetSampleRate() const = 0;
    virtual int GetChannels() const = 0;
};