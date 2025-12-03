#include "WasapiListener.h"

// Macros auxiliares para manejo de errores COM
#define EXIT_ON_ERROR(hres) \
    if (FAILED(hres))       \
    {                       \
        goto Exit;          \
    }
#define SAFE_RELEASE(punk) \
    if ((punk) != NULL)    \
    {                      \
        (punk)->Release(); \
        (punk) = NULL;     \
    }

// Constantes de tiempo (100 nanosegundos por unidad)
const REFERENCE_TIME REFTIMES_PER_SEC = 10000000;

WasapiListener::WasapiListener()
{
    // Inicializamos COM en el hilo actual si no se ha hecho
    CoInitialize(NULL);
}

WasapiListener::~WasapiListener()
{
    Stop();
    CleanUp();
    CoUninitialize();
}

void WasapiListener::CleanUp()
{
    if (_pWaveFormat)
    {
        CoTaskMemFree(_pWaveFormat);
        _pWaveFormat = nullptr;
    }
    SAFE_RELEASE(_pEnumerator);
    SAFE_RELEASE(_pDevice);
    SAFE_RELEASE(_pAudioClient);
    SAFE_RELEASE(_pCaptureClient);
    _initialized = false;
}

bool WasapiListener::Initialize()
{
    HRESULT hr;

    // 1. Obtener Enumerador de Dispositivos
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator), (void **)&_pEnumerator);
    if (FAILED(hr))
        return false;

    // 2. Obtener el dispositivo por defecto (Render/Speakers)
    hr = _pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &_pDevice);
    if (FAILED(hr))
        return false;

    // 3. Activar el Cliente de Audio
    hr = _pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void **)&_pAudioClient);
    if (FAILED(hr))
        return false;

    // 4. Obtener el formato de mezcla (MixFormat) del sistema
    hr = _pAudioClient->GetMixFormat(&_pWaveFormat);
    if (FAILED(hr))
        return false;

    // 5. Inicializar en modo LOOPBACK
    hr = _pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK, // <--- Importante: Loopback
        REFTIMES_PER_SEC,
        0,
        _pWaveFormat,
        NULL);
    if (FAILED(hr))
        return false;

    // 6. Obtener el servicio de captura
    hr = _pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void **)&_pCaptureClient);
    if (FAILED(hr))
        return false;

    _initialized = true;
    return true;
}

void WasapiListener::Start()
{
    if (_initialized && !_recording)
    {
        HRESULT hr = _pAudioClient->Start();
        if (SUCCEEDED(hr))
        {
            _recording = true;
        }
    }
}

void WasapiListener::Stop()
{
    if (_recording)
    {
        _pAudioClient->Stop();
        _recording = false;
    }
}

bool WasapiListener::GetAudioData(std::vector<float> &outputBuffer)
{
    if (!_recording || !_pCaptureClient)
        return false;

    HRESULT hr;
    UINT32 packetLength = 0;
    UINT32 numFramesAvailable;
    BYTE *pData;
    DWORD flags;

    // Revisar si hay paquetes disponibles
    hr = _pCaptureClient->GetNextPacketSize(&packetLength);
    if (FAILED(hr) || packetLength == 0)
        return false;

    // Obtener el buffer
    hr = _pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
    if (FAILED(hr))
        return false;

    // Procesar datos
    outputBuffer.clear();

    if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
    {
        // Si hay silencio, llenamos con ceros
        outputBuffer.resize(numFramesAvailable * _pWaveFormat->nChannels, 0.0f);
    }
    else
    {
        // Convertimos los bytes crudos a float
        // Asumimos formato IEEE Float 32-bit (estándar en Windows moderno)
        float *pFloatData = (float *)pData;
        int totalSamples = numFramesAvailable * _pWaveFormat->nChannels;

        outputBuffer.assign(pFloatData, pFloatData + totalSamples);
    }

    // Liberar buffer para que Windows siga escribiendo
    _pCaptureClient->ReleaseBuffer(numFramesAvailable);
    return true;
}

int WasapiListener::GetSampleRate() const
{
    return _pWaveFormat ? _pWaveFormat->nSamplesPerSec : 0;
}

int WasapiListener::GetChannels() const
{
    return _pWaveFormat ? _pWaveFormat->nChannels : 0;
}