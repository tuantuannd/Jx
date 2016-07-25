#ifndef KOptionH
#define	KOptionH

class KOption
{
private:
	int		m_nSndVolume;
	int		m_nMusicVolume;
	int		m_nGamma;
public:
	int		GetSndVolume() { return m_nSndVolume; }
	void	SetSndVolume(int nSndVolume);
	int		GetMusicVolume() { return m_nMusicVolume; }
	void	SetMusicVolume(int nMusicVolume);
	int		GetGamma() { return m_nGamma; };
	void	SetGamma(int nGamma);
};

extern KOption Option;
#endif