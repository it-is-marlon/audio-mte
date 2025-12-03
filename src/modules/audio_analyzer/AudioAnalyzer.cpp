#include "AudioAnalyzer.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

AudioAnalyzer::AudioAnalyzer(int sampleRate, size_t fftSize)
    : _sampleRate(sampleRate), _fftSize(fftSize)
{
    // Reservar memoria para evitar realocaciones dinámicas constantes
    _internalBuffer.reserve(_fftSize * 2);
    _complexBuffer.resize(_fftSize);
}

AudioAnalyzer::~AudioAnalyzer() {}

void AudioAnalyzer::FeedAudio(const std::vector<float> &rawData)
{
    // Simplemente agregamos al buffer interno
    _internalBuffer.insert(_internalBuffer.end(), rawData.begin(), rawData.end());
}

// Lógica de FFT (Privada, igual que antes)
void AudioAnalyzer::FFT(std::vector<Complex> &buffer)
{
    size_t n = buffer.size();
    if (n <= 1)
        return;

    std::vector<Complex> even(n / 2);
    std::vector<Complex> odd(n / 2);

    for (size_t i = 0; i < n / 2; ++i)
    {
        even[i] = buffer[2 * i];
        odd[i] = buffer[2 * i + 1];
    }

    FFT(even);
    FFT(odd);

    for (size_t i = 0; i < n / 2; ++i)
    {
        Complex t = std::polar(1.0, -2.0 * M_PI * i / n) * odd[i];
        buffer[i] = even[i] + t;
        buffer[i + n / 2] = even[i] - t;
    }
}

void AudioAnalyzer::ApplyHannWindow(std::vector<float> &audioData)
{
    size_t size = audioData.size();
    for (size_t i = 0; i < size; ++i)
    {
        double multiplier = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (size - 1)));
        audioData[i] = audioData[i] * (float)multiplier;
    }
}

float AudioAnalyzer::CalculateBandEnergy(const std::vector<float> &spectrum, int minHz, int maxHz)
{
    // Conversión: Hz -> Indice del Array FFT
    // Indice = Frecuencia * FFT_Size / SampleRate
    int binStart = (int)((double)minHz * _fftSize / _sampleRate);
    int binEnd = (int)((double)maxHz * _fftSize / _sampleRate);

    // Protección de límites
    if (binStart < 0)
        binStart = 0;
    if (binEnd >= (int)spectrum.size())
        binEnd = (int)spectrum.size() - 1;
    if (binStart > binEnd)
        return 0.0f;

    float energySum = 0.0f;
    for (int i = binStart; i <= binEnd; ++i)
    {
        energySum += spectrum[i];
    }

    // Retornamos el promedio
    return energySum / (binEnd - binStart + 1);
}

bool AudioAnalyzer::ProcessSpectrum(AudioBands &outputBands)
{
    // 1. Verificación: ¿Tenemos suficientes datos para una FFT?
    if (_internalBuffer.size() < _fftSize)
    {
        outputBands.hasNewData = false;
        return false;
    }

    // 2. Extracción de un bloque exacto (Frame)
    std::vector<float> frame(_internalBuffer.begin(), _internalBuffer.begin() + _fftSize);

    // Descartamos los datos usados del buffer (Estrategia sin solapamiento por simplicidad)
    _internalBuffer.erase(_internalBuffer.begin(), _internalBuffer.begin() + _fftSize);

    // 3. Ventaneo
    ApplyHannWindow(frame);

    // 4. Preparación para FFT
    for (size_t i = 0; i < _fftSize; ++i)
    {
        _complexBuffer[i] = Complex(frame[i], 0.0);
    }

    // 5. Ejecución FFT
    FFT(_complexBuffer);

    // 6. Cálculo de Magnitudes (Solo mitad del espectro)
    std::vector<float> spectrum(_fftSize / 2);
    for (size_t i = 0; i < _fftSize / 2; ++i)
    {
        spectrum[i] = (float)std::abs(_complexBuffer[i]);
    }

    // 7. Interpretación Musical (Mapping)
    outputBands.bass = CalculateBandEnergy(spectrum, 20, 150);       // Bajos profundos
    outputBands.mid = CalculateBandEnergy(spectrum, 150, 4000);      // Voces e instrumentos
    outputBands.treble = CalculateBandEnergy(spectrum, 4000, 20000); // Brillo y platillos
    outputBands.hasNewData = true;

    return true;
}