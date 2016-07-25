#include "Interface\IClient.h"
#include "ClientStage.h"

// {D38249A9-4565-4336-9F70-1374F1E158E7}
static const GUID IID_IESClient = 
{ 0xd38249a9, 0x4565, 0x4336, { 0x9f, 0x70, 0x13, 0x74, 0xf1, 0xe1, 0x58, 0xe7 } };


STDAPI QueryClientInterface
		(
			REFIID	riid,
			void	**ppv
		)
{
	HRESULT hr = E_NOINTERFACE;

	if ( IID_IESClient == riid )
	{
		CConnectManager *pObject = new CConnectManager( MAX_BUFFER_KEEPINPOOL, EXTEND_BUFFER_SIZE );

		*ppv = reinterpret_cast< void * > ( dynamic_cast< IClient * >( pObject ) );

		if ( *ppv )
		{
			reinterpret_cast< IUnknown * >( *ppv )->AddRef();

			hr = S_OK;
		}
	}

	if ( FAILED( hr ) )
	{
		*ppv = NULL;
	}

	return ( HRESULT )( hr );
}