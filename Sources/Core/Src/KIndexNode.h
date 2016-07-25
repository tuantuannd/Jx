#ifndef KIndexNodeH
#define	KIndexNodeH

class KIndexNode : public KNode
{
public:
	int		m_nIndex;
	int		m_Ref;
public:
	KIndexNode() { m_nIndex = NULL; m_Ref = 0; };
	void	AddRef() { m_Ref++; };
	void	Release() { _ASSERT(m_Ref > 0); m_Ref--; };
};

#endif
