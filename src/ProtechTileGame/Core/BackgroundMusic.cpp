#include "../GameLocal.h"
#pragma hdrstop


BackgroundMusic::BackgroundMusic()
: m_loop(false)
{
	// nothing
}

void BackgroundMusic::Start(const char *streamFileName, bool loop)
{
	m_streamFileName = streamFileName;
	m_soundSource.Stream(streamFileName, loop);
	m_loop = loop;
}

void BackgroundMusic::EnsureIsStarted(const char *streamFileName, bool loopInCaseNotYetStarted)
{
	if(Str_PathCompare(m_streamFileName.c_str(), streamFileName) != 0 || !IsPlaying())
	{
		Start(streamFileName, loopInCaseNotYetStarted);
	}
}

void BackgroundMusic::ReStart()
{
	if(m_streamFileName != "")
	{
		m_soundSource.Stream(m_streamFileName.c_str(), m_loop);
	}
}

void BackgroundMusic::Stop()
{
	m_soundSource.Stop();
}

bool BackgroundMusic::IsPlaying() const
{
	return m_soundSource.IsValid();
}

float BackgroundMusic::GetPlaybackFraction() const
{
	return m_soundSource.GetPlaybackFraction();
}
