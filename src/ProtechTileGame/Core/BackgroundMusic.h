#ifndef __BACKGROUNDMUSIC_H
#define __BACKGROUNDMUSIC_H


class BackgroundMusic
{
private:
	SoundSource	m_soundSource;
	PTString	m_streamFileName;
	bool		m_loop;

public:
				BackgroundMusic();
	void		Start(const char *streamFileName, bool loop);
	void		EnsureIsStarted(const char *streamFileName, bool loopInCaseNotYetStarted);
	void		ReStart();
	void		Stop();
	bool		IsPlaying() const;
	float		GetPlaybackFraction() const;
};


#endif	// __BACKGROUNDMUSIC_H
