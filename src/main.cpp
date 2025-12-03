#include <iostream>
#include <vector>
#include <windows.h>
#include <string>

#include "modules/audio_listener/WasapiListener.h"
#include "modules/audio_analyzer/AudioAnalyzer.h"

// Función auxiliar solo para dibujar (UI)
void DrawBar(std::string label, float value, int maxChars = 30)
{
    // Factor de ganancia visual (Ajustar según volumen del PC)
    const float GAIN = 5000.0f;
    int bars = (int)(value * GAIN);
    if (bars > maxChars)
        bars = maxChars;

    std::cout << label << " [";
    for (int i = 0; i < maxChars; ++i)
    {
        std::cout << (i < bars ? "=" : " ");
    }
    std::cout << "] ";
}

int main()
{
    std::cout << "--- AUDIO MTE: MODULAR ARCHITECTURE ---" << std::endl;

    // 1. Iniciar Listener (Capa de Hardware)
    WasapiListener listener;
    if (!listener.Initialize())
    {
        std::cerr << "Error iniciando WASAPI." << std::endl;
        return 1;
    }
    listener.Start();

    // 2. Iniciar Analyzer (Capa Lógica)
    // Le pasamos el Sample Rate real del hardware para que calcule bien los Hz
    AudioAnalyzer analyzer(listener.GetSampleRate());

    std::cout << "Engine Running at " << listener.GetSampleRate() << "Hz" << std::endl;

    // Variables temporales
    std::vector<float> rawAudio;
    AudioBands currentBands;

    // 3. Loop Principal
    while (true)
    {
        // A. Obtener datos del hardware
        if (listener.GetAudioData(rawAudio))
        {

            // B. Alimentar al analizador (No nos importa el buffering)
            analyzer.FeedAudio(rawAudio);

            // C. Intentar obtener un análisis procesado
            // El analizador retornará true solo cuando haya completado una FFT
            while (analyzer.ProcessSpectrum(currentBands))
            {

                // D. Visualización (Capa de UI)
                std::cout << "\r"; // Retorno de carro
                DrawBar("BASS", currentBands.bass);
                DrawBar("MID ", currentBands.mid);
                DrawBar("HIGH", currentBands.treble);
            }
        }

        Sleep(1); // Ceder CPU
    }

    listener.Stop();
    return 0;
}