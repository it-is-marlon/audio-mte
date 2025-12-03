#pragma once
#include <vector>
#include <complex>
#include <cmath>

// Estructura de datos para entregar resultados limpios al Main
struct AudioBands
{
    float bass;      // Energía en bajos (20-150Hz)
    float mid;       // Energía en medios (150-4000Hz)
    float treble;    // Energía en agudos (4000-20000Hz)
    bool hasNewData; // Bandera para saber si se procesó un frame nuevo
};

class AudioAnalyzer
{
private:
    using Complex = std::complex<double>;

    // Configuración interna
    int _sampleRate;
    size_t _fftSize;

    // Memoria interna (Estado del objeto)
    std::vector<float> _internalBuffer;
    std::vector<Complex> _complexBuffer;

    // Métodos privados matemáticos (Caja negra)
    void FFT(std::vector<Complex> &buffer);
    void ApplyHannWindow(std::vector<float> &audioData);
    float CalculateBandEnergy(const std::vector<float> &spectrum, int minHz, int maxHz);

public:
    AudioAnalyzer(int sampleRate, size_t fftSize = 2048);
    ~AudioAnalyzer();

    // Método principal: Ingiere datos crudos y gestiona su propio buffer
    void FeedAudio(const std::vector<float> &rawData);

    // Método de consulta: ¿Tienes resultados listos?
    // Si hay suficientes datos acumulados, procesa y retorna true.
    bool ProcessSpectrum(AudioBands &outputBands);
};