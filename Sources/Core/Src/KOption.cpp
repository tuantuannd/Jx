#include "KCore.h"
#include "KSubWorldSet.h"
#include "KOption.h"
#include "../../Represent/iRepresent/iRepresentshell.h"

extern struct iRepresentShell*	g_pRepresent;

#ifndef _SERVER
KOption	Option;

void	KOption::SetGamma(int nGamma)
{
	m_nGamma = nGamma;
	g_pRepresent->SetGamma(nGamma);
}

void KOption::SetSndVolume(int nSndVolume)
{
	if (nSndVolume <= 3)
		m_nSndVolume = 0;
	else if (nSndVolume >= 100)
		m_nSndVolume = 100;
	else if (nSndVolume >= 40)
		m_nSndVolume = 80 + (nSndVolume - 50) * 2 / 5;
	else
		m_nSndVolume = 40 + nSndVolume;
}

//KMapMusic::SetGameVolume modify by wooy
/*响度是听觉的基础。正常人听觉的强度范围为0dB-140dB(也有人认为是 -5dB－130dB).固然，超出人耳的
可听频率范围（即频域）的声音，即使响度再大，人耳也听不出来（即响度为零）。但在人耳的可听频域内，
若声音弱到或强到一定程度，人耳同样是听不到的。当声音减弱到人耳刚刚可以听见时，此时的声音强度
称为“闻阈”。一般以1kHz纯音为准进行测量，人耳刚能听到的声压为0dB(一般大于0.3dB即有感受),
此时的主观响度级定为零方。而当声音增强到使人耳感到疼痛时，这个阈值称为“痛阈”。仍以1kHz纯音为准
来进行测量，使人耳感到疼痛时声强级约达到140dB左右。实验表明，闻阈和痛阈是随声压、频率变化的。
闻阈和痛阈随频率变化的等响度曲线（弗莱彻－门逊曲线）之间的区域就是人耳的听觉范围，通常认为，
对于1kHz纯音，0dB-20dB为宁静声，30dB-40dB为微弱声，50dB-70dB为正常声，80dB-100dB为响音声，
110dB-130dB为极响声。面对于1kHz以外可听声，在同一级等响度曲线上有无数个等效的声压-频率值,
例如，200Hz的30dB的声音和1kHz的10dB的声音在人耳听起来却具有相同的响度。小于0dB闻阈和大于140痛阈时
为不可听声，即使是人耳最敏感频率范围的声音，人耳也觉察不到。*/
void KOption::SetMusicVolume(int nMusicVolume)
{
	if (nMusicVolume <= 3)
		m_nMusicVolume = 0;
	else if (nMusicVolume >= 100)
		m_nMusicVolume = 100;
	else if (nMusicVolume >= 40)
		m_nMusicVolume = 80 + (nMusicVolume - 50) * 2 / 5;
	else
		m_nMusicVolume = 40 + nMusicVolume;

	g_SubWorldSet.m_cMusic.SetGameVolume(m_nMusicVolume);
}

#endif