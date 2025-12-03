#pragma once
#include <vector>

// Clase abstracta pura (Interfaz)
class IAudioListener
{
public:
    virtual ~IAudioListener() {}

    // Inicializa los recursos (drivers, memoria)
    virtual bool Initialize() = 0;

    // Comienza la escucha
    virtual void Start() = 0;

    // Detiene la escucha
    virtual void Stop() = 0;

    // Obtiene los datos crudos del buffer actual
    // Retorna true si hay datos nuevos
    virtual bool GetAudioData(std::vector<float> &outputBuffer) = 0;
};