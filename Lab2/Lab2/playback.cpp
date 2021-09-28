#include "playback.h"

DShowPlayer::DShowPlayer(std::string filePath) :
    m_state(STATE_NO_GRAPH),
    m_pGraph(NULL),
    m_pControl(NULL),
    m_pEvent(NULL),
    m_pPosition(NULL)
{
    // Initialize the COM library.
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        printf("ERROR - Could not initialize COM library");
        return;
    }

    // Create the filter graph manager and query for interfaces.
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
        IID_IGraphBuilder, (void**)&m_pGraph);
    if (FAILED(hr))
    {
        printf("ERROR - Could not create the Filter Graph Manager.");
        return;
    }

    hr = m_pGraph->QueryInterface(IID_IMediaControl, (void**)&m_pControl);
    hr = m_pGraph->QueryInterface(IID_IMediaEvent, (void**)&m_pEvent);
    hr = m_pGraph->QueryInterface(IID_IMediaPosition, (void**)&m_pPosition);
    // Build the graph. IMPORTANT: Change this string to a file on your system.
    hr = m_pGraph->RenderFile(StrToWStr(filePath).c_str(), NULL);
    hr = m_pControl->Run();
    m_state = STATE_RUNNING;
}

DShowPlayer::~DShowPlayer()
{
    m_pControl->Release();
    m_pEvent->Release();
    m_pGraph->Release();
    m_pPosition->Release();
    CoUninitialize();
}

HRESULT DShowPlayer::Play()
{
    if (m_state != STATE_PAUSED)
    {
        return VFW_E_WRONG_STATE;
    }

    HRESULT hr = m_pControl->Run();
    if (SUCCEEDED(hr))
    {
        m_state = STATE_RUNNING;
    }
    return hr;
}

HRESULT DShowPlayer::Pause()
{
    if (m_state != STATE_RUNNING)
    {
        return VFW_E_WRONG_STATE;
    }

    HRESULT hr = m_pControl->Pause();
    if (SUCCEEDED(hr))
    {
        m_state = STATE_PAUSED;
    }
    return hr;
}

HRESULT DShowPlayer::FastForward()
{
    double rate;
    m_pPosition->get_Rate(&rate);

    rate = rate == 1 ? 1.5 : rate == 1.5 ? 2 : 1;

    HRESULT hr = m_pPosition->put_Rate(rate);
    return hr;
}

HRESULT DShowPlayer::Back()
{
    HRESULT hr = m_pPosition->put_CurrentPosition(0);
    return hr;
}

std::wstring DShowPlayer::StrToWStr(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}
