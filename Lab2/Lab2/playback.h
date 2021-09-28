#pragma once
#include <string>
#include <dshow.h>

enum PlaybackState
{
    STATE_NO_GRAPH,
    STATE_RUNNING,
    STATE_PAUSED,
};

class DShowPlayer
{
public:
    DShowPlayer(std::string filePath);
    ~DShowPlayer();

    PlaybackState State() const { return m_state; }

    HRESULT Play();
    HRESULT Pause();
    HRESULT FastForward();
    HRESULT Back();

private:
    std::wstring StrToWStr(const std::string& s);

    PlaybackState   m_state;

    IGraphBuilder* m_pGraph;
    IMediaControl* m_pControl;
    IMediaEventEx* m_pEvent;
    IMediaPosition* m_pPosition;
};
