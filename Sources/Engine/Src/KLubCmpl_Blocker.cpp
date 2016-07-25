#include "KWin32.h"
#include "string.h"
#include "KLubCmpl_Blocker.h"
#ifndef __linux
#include "conio.h"
#endif

#define d 0
void KLubCmpl_Blocker::ScanIf(KLineNode * pFirstNode)
{
	KLineNode * pNode = pFirstNode;
	KLineNode * pFirstIfNode = NULL;

	while(pNode)
	{
		char * pLine;
		pLine = (char *)pNode->m_pLineMem->GetMemPtr();
		int i = 0;
   		while(pLine[i] != '\0')
		{
			if (pLine[i] == ' '|| pLine[i] == '\t')
			{
				i++;
				continue;
			}
			
			char * pIf;
			char * pElseIf;
			char * pElse;
			char * pEnd;
			
			if ((pIf = strstr(pLine, "if") )== pLine + i)
			{
				//为if语句的头
				if (*(pIf + 2) == '(' || *(pIf + 2) == ' ' || *(pIf+2) == '\t')
				{ 
				  char * pIfEnd;
				
				  //是否为单一连续一行的if 语句
				  if (pIfEnd = strstr(pLine, "end"))
				  {
					//是的话，结束该行扫描
					  if (*(pIfEnd - 1) == ')' ||  *(pIfEnd - 1) == ' '||*(pIfEnd - 1) == '\t') 
						break;
				  }
				  else
				  {
						KStackNode * pStackNode = new KStackNode;
						pStackNode->m_pLine = pNode;
						pStackNode->nKey = KEYIF;
						PushKey(pStackNode);
						break;
				  }
				}
				else
					break;
			}
			else if((pElseIf = strstr(pLine,"elseif")) == pLine + i)
			{

				//为elseif语句的头
				if (*(pElseIf + 6) == '(' || *(pElseIf + 6) == ' ' || *(pElseIf + 6) == '\t')
				{ 
				  
						KStackNode * pStackNode = new KStackNode;
						pStackNode->m_pLine = pNode;
						pStackNode->nKey = KEYELSEIF;
						PushKey(pStackNode);
						break;
				}
				else
					break;

			}
			else if ((pElse = strstr(pLine, "else") )== pLine + i )
			{
				//为elseif语句的头
				if (*(pElse + 4) == '(' || *(pElse + 4) == ' ' || *(pElse + 4) == '\n'|| *(pElse + 4) == '\t')
				{ 
				  
						KStackNode * pStackNode = new KStackNode;
						pStackNode->m_pLine = pNode;
						pStackNode->nKey = KEYELSE;
						PushKey(pStackNode);
						break;
				}
				else
					break;
			}
			else if ((pEnd = strstr(pLine, "end")) == pLine + i)
			{
				if (*(pEnd + 3) == ';' || *(pEnd + 3) == ' ' || *(pEnd + 3) == 10 || *(pEnd + 3) == '\t')
				{ 
					KStackNode * pStackNode = NULL;
					KStackNode * pEndStack = new KStackNode;
					pEndStack->m_pLine = pNode;
					pEndStack->nKey = KEYEND;
					PushKey(pEndStack);

					do
				  {
						pStackNode = PopKey();
						UnitsList.AddHead((KNode *)pStackNode);
				  }
					while(pStackNode->nKey != KEYIF);
				}

				//该为最先if语句，首先转换！
				if (m_StackList.GetNodeCount() == 0)
				{
					KLineNode * pReturnNode = ((KStackNode*)UnitsList.GetHead())->m_pLine;
					this->ExchangeCurLines();
					if (pReturnNode)
	 			ScanIf(pReturnNode);
					return;
				}
				else
				{
					int num = UnitsList.GetNodeCount();
					for(int i = 0 ; i < num;i ++)
					UnitsList.RemoveHead();
				}
			}
			else
			{
				break;
			}
		break;		
}
		pNode = (KLineNode *)pNode->GetNext();
	}
}



BOOL KLubCmpl_Blocker::ExchangeCurLines()
{
	static int nLabelNum = 0;
	static int nReturnLabelNum = 0;
	int nCount = 0;
	char szNewLine[100];
	nCount = UnitsList.GetNodeCount() ;
	if (nCount== 0)
		return FALSE;
	int i = 1;
	char szNewLabel[30];
	char szNewReturnLabel[30];

	//		if (s1) then 
	//		.....
	//		....
	//		end;
	
	KStackNode * pIfEndNode = (KStackNode *)UnitsList.GetTail();
	int nEndLabelNum = nCount + nLabelNum ;
	
	//当前的组合应为if end;
	if (nCount == 2)
	{
			KStackNode * pNode = (KStackNode*)UnitsList.RemoveHead();
			KStackNode * pEndNode = (KStackNode*)UnitsList.RemoveHead();
			
			KLineNode * pNewLine = new KLineNode;
			KLineNode * pNewReturnLine = new KLineNode;
			pNewLine->m_pLineMem = new KMemClass1;
			pNewReturnLine->m_pLineMem = new KMemClass1;
			sprintf(szNewReturnLabel, "\nLabel(\"ReturnLabel%d\");\n", ++ nReturnLabelNum  );
			sprintf(szNewLabel, "\nLabel(\"AutoLabel%d\");\n", ++ nLabelNum );
			char * Buf = (char *)pNewLine->m_pLineMem->Alloc(strlen(szNewLabel)+d);
			strcpy(Buf,szNewLabel);
			Buf = (char *)pNewReturnLine->m_pLineMem->Alloc(strlen(szNewReturnLabel)+d);
			strcpy(Buf,szNewReturnLabel);

			pEndNode->m_pLine->InsertAfter((KNode*) pNewReturnLine);
			m_Lines.AddTail((KNode*)pNewLine);
			
			sprintf(szNewLine, "%s Goto(\"AutoLabel%d\") end;\n", (char *)pNode->m_pLine->m_pLineMem->GetMemPtr(), nLabelNum );
			szNewLine[pNode->m_pLine->m_pLineMem->GetMemLen() - 1] = ' ' ;
			
			KMemClass1 * pMem = new KMemClass1;
			pMem->Alloc(strlen(szNewLine) +d);
			strcpy((char *)pMem->GetMemPtr(), szNewLine);
		//	delete pNode->m_pLine->m_pLineMem;
			pNode->m_pLine->m_pLineMem = pMem;

			KStackNode * pNextNode = pEndNode;
			pNewLine->m_pNext = pNode->m_pLine->GetNext();
			pNode->m_pLine->GetNext()->m_pPrev = pNewLine;

			//Set Tail to NextNode prev
			pNextNode->m_pLine->GetPrev()->m_pNext = &m_Lines.m_ListTail;
			m_Lines.m_ListTail.m_pPrev = pNextNode->m_pLine->m_pPrev;

			pNode->m_pLine->m_pNext = pNextNode->m_pLine;
			pNextNode->m_pLine->m_pPrev = pNode->m_pLine;

			KLineNode * pGotoReturnNode = new KLineNode;
			char szGotoReturn[100];
			sprintf(szGotoReturn, "Goto(\"ReturnLabel%d\");\n", nReturnLabelNum  );
			pGotoReturnNode->m_pLineMem =	 new KMemClass1;
			char * Buff = (char * )pGotoReturnNode->m_pLineMem->Alloc(strlen(szGotoReturn)+d);
			strcpy(Buff, szGotoReturn);
			m_Lines.AddTail(pGotoReturnNode);

			pEndNode->m_pLine->Remove();
			return TRUE;
	}

	nReturnLabelNum ++ ;
	while(1)
	{ 
		if (i != nCount - 2)
		{
			KStackNode * pNode = (KStackNode*)UnitsList.GetHead();
			
			KLineNode * pNewLine = new KLineNode;
			pNewLine->m_pLineMem = new KMemClass1;
						
			sprintf(szNewLabel, "\nLabel(\"AutoLabel%d\");\n", nLabelNum );
			
			char * Buf = (char *)pNewLine->m_pLineMem->Alloc(strlen(szNewLabel)+d);
			strcpy(Buf,szNewLabel);
			m_Lines.AddTail((KNode*)pNewLine);
			
			sprintf(szNewLine, "%s Goto(\"AutoLabel%d\") end;\n", (char *)pNode->m_pLine->m_pLineMem->GetMemPtr(), nLabelNum );
			szNewLine[pNode->m_pLine->m_pLineMem->GetMemLen() - 1] = ' ' ;
			g_StrRep(szNewLine,"elseif", "if");
			
			KMemClass1 * pMem = new KMemClass1;
			pMem->Alloc(strlen(szNewLine)+1);
			strcpy((char *)pMem->GetMemPtr(), szNewLine);
			
			// pNode->m_pLine->m_pLineMem->Free();
			pNode->m_pLine->m_pLineMem = pMem;

			KStackNode * pNextNode = (KStackNode* )pNode->GetNext();
			pNewLine->m_pNext = pNode->m_pLine->GetNext();
			pNode->m_pLine->GetNext()->m_pPrev = pNewLine;

			//Set Tail to NextNode prev
			pNextNode->m_pLine->GetPrev()->m_pNext = &m_Lines.m_ListTail;
			m_Lines.m_ListTail.m_pPrev = pNextNode->m_pLine->m_pPrev;

			pNode->m_pLine->m_pNext = pNextNode->m_pLine;
			pNextNode->m_pLine->m_pPrev = pNode->m_pLine;

			KLineNode * pGotoReturnNode = new KLineNode;
			char szGotoReturn[100];
			sprintf(szGotoReturn, "Goto(\"ReturnLabel%d\");\n", nReturnLabelNum );
			pGotoReturnNode->m_pLineMem = new KMemClass1;
			char * Buff = (char * )pGotoReturnNode->m_pLineMem->Alloc(strlen(szGotoReturn)+d);
			strcpy(Buff, szGotoReturn);
			m_Lines.AddTail(pGotoReturnNode);

		//	return TRUE;
			UnitsList.RemoveHead();
			i++;
			nLabelNum ++;
//			Print();
//			getch();
			continue;
		}

		else
		{
			KStackNode * pElseIfNode	= (KStackNode*)UnitsList.GetHead();
			KStackNode * pElseNode		= (KStackNode*)UnitsList.GetHead()->GetNext();
			KStackNode * pEndNode		= (KStackNode*)UnitsList.GetHead()->GetNext()->GetNext();

		 	if ((pElseIfNode->nKey != KEYELSEIF && pElseIfNode->nKey != KEYIF) ||pElseNode->nKey != KEYELSE || pEndNode->nKey != KEYEND)
				return FALSE;
			
			KLineNode * pNewLine =			new KLineNode;
			KLineNode * pNewElseLine =		new KLineNode;
			KLineNode * pNewReturnLine =	new KLineNode;
			pNewLine->m_pLineMem =			new KMemClass1;
			pNewReturnLine->m_pLineMem =	new KMemClass1;
			pNewElseLine->m_pLineMem =		new KMemClass1;
			char							szNewElseLine[50];

			sprintf(szNewReturnLabel, "\nLabel(\"ReturnLabel%d\");\n", nReturnLabelNum  );
			sprintf(szNewLabel, "\nLabel(\"AutoLabel%d\");\n", nLabelNum );
			sprintf(szNewElseLine, "\nLabel(\"AutoLabel%d\");\n", ++nLabelNum);
			
			char * Buf = (char *)pNewLine->m_pLineMem->Alloc(strlen(szNewLabel)+d);
			strcpy(Buf,szNewLabel);

			Buf = (char *)pNewReturnLine->m_pLineMem->Alloc(strlen(szNewReturnLabel)+d);
			strcpy(Buf,szNewReturnLabel);

			Buf = (char *)pNewElseLine->m_pLineMem->Alloc(strlen(szNewElseLine)+d);
			strcpy(Buf,szNewElseLine);

			pEndNode->m_pLine->InsertAfter((KNode*) pNewReturnLine);

			m_Lines.AddTail((KNode*)pNewLine);
			m_Lines.AddTail((KNode*)pNewElseLine);
			

			sprintf(szNewLine, "%s Goto(\"AutoLabel%d\") else Goto(\"AutoLabel%d\") end;\n", (char *)pElseIfNode->m_pLine->m_pLineMem->GetMemPtr(), nLabelNum - 1, nLabelNum );
			szNewLine[pElseIfNode->m_pLine->m_pLineMem->GetMemLen() - 1] = ' ' ;
			g_StrRep(szNewLine,"elseif", "if");

			
			KMemClass1 * pMem = new KMemClass1;
			pMem->Alloc(strlen(szNewLine)+d);
			strcpy((char *)pMem->GetMemPtr(), szNewLine);
			
		//	delete pElseIfNode->m_pLine->m_pLineMem;
			pElseIfNode->m_pLine->m_pLineMem = pMem;
			//printf("1\n");
			//Print();
			//getch();
			pNewLine->m_pNext = pElseIfNode->m_pLine->GetNext();
			pElseIfNode->m_pLine->GetNext()->m_pPrev = pNewLine;
			
			pElseNode->m_pLine->GetPrev()->m_pNext = pNewElseLine;
			pNewElseLine->m_pPrev = pElseNode->m_pLine->GetPrev();
			
			pNewElseLine->m_pNext = pElseNode->m_pLine->GetNext();
			pElseNode->m_pLine->GetNext()->m_pPrev = pNewElseLine;

			pEndNode->m_pLine->GetPrev()->m_pNext = &m_Lines.m_ListTail;
			m_Lines.m_ListTail.m_pPrev = pEndNode->m_pLine->GetPrev();

			pElseIfNode->m_pLine->m_pNext = pEndNode->m_pLine->GetNext();
			pEndNode->m_pLine->GetNext()->m_pPrev  = pElseIfNode->m_pLine;

	//printf("3\n");
	//Print();
	//getch();
			KLineNode * pGotoReturnNode1 = new KLineNode;
			KLineNode * pGotoReturnNode2 = new KLineNode;
			
			char szGotoReturn[100];
			sprintf(szGotoReturn, "Goto(\"ReturnLabel%d\");\n",  nReturnLabelNum  );
			
			pGotoReturnNode1->m_pLineMem = new KMemClass1;
			pGotoReturnNode2->m_pLineMem = new KMemClass1;

			char * Buff = (char * )pGotoReturnNode1->m_pLineMem->Alloc(strlen(szGotoReturn)+d);
			strcpy(Buff, szGotoReturn);
			
			Buff = (char * )pGotoReturnNode2->m_pLineMem->Alloc(strlen(szGotoReturn)+d);
			strcpy(Buff, szGotoReturn);
			
			
			pNewElseLine->InsertBefore((KNode*) pGotoReturnNode1);
			m_Lines.AddTail(pGotoReturnNode2);
			
			//pEndNode->m_pLine->Remove();
			//pElseNode->m_pLine->Remove();
			//pEndNode->m_pLine->Remove();
//			Print();
//			getch();
			UnitsList.RemoveHead();
			UnitsList.RemoveHead();
			UnitsList.RemoveHead();
			return TRUE;
		}
	}
}

KLineNode * KLubCmpl_Blocker::Load(LPSTR FileName)
{
	KPakFile	File;
	DWORD		dwSize;
	PVOID		Buffer;
	KLineNode * pFirstLine = NULL;
	int			nState = 0;
	
	// check file name
	if (FileName[0] == 0)		return NULL;
	
	if (!File.Open(FileName))
	{
		g_DebugLog("Can't open tab file : %s", FileName);
		return NULL;
	}

	dwSize = File.Size();
	KMemClass1 Memory;

	Buffer = Memory.Alloc(dwSize);
	File.Read(Buffer, dwSize);

	DWORD nCurPos = 0;
	while(nCurPos < dwSize)
	{
	long i = 0;
	 char szLine[100];
	   while(nCurPos <= dwSize)
	   {
		if (((char*)Buffer)[nCurPos] == '\n')
			break;
	 	szLine[i++] = ((char*)Buffer)[nCurPos ++];
	   }
	
	 szLine[i - 1]	= '\n';
	 szLine[i]		= '\0';
	 nCurPos		= nCurPos + 1;
	 
	 KLineNode * pLineNode = new KLineNode; 
	 KMemClass1 * pMem = new KMemClass1;
	 pMem->Alloc(i);
	 pLineNode->m_pLineMem = pMem;
	 strcpy((char *)pLineNode->m_pLineMem->GetMemPtr(), szLine);
	 if (strstr(szLine,"function main()"))  pFirstLine = pLineNode;
	 
	 if (strstr(szLine,"end;--main" ))  nState = 1;
	 
	 if (nState == 0)
		 m_Lines.AddTail(pLineNode);
	 else
		 m_RestLines.AddTail(pLineNode);
	}

	return pFirstLine;
}


void KLubCmpl_Blocker::PushKey(KStackNode * pStackNode)  
{
	g_DebugLog("Push (%d) %s", pStackNode->nKey, pStackNode->m_pLine->m_pLineMem->GetMemPtr());

	m_StackList.AddHead((KNode*)pStackNode);
}

KStackNode* KLubCmpl_Blocker::PopKey()
{
	g_DebugLog("Pop  (%d) %s", ((KStackNode*)(m_StackList.GetHead()))->nKey, ((KStackNode*)(m_StackList.GetHead()))->m_pLine->m_pLineMem->GetMemPtr());
	return (KStackNode*) m_StackList.RemoveHead();
}


int  KLubCmpl_Blocker::GetBuffer(KMemClass1 * &pMem)
{
	LPSTR		DataBuf;
	DWORD		dwLen = 0;

	KLineNode * pNode = (KLineNode *)m_Lines.GetHead();
	
	while(pNode)
	{
		dwLen += pNode->m_pLineMem->GetMemLen();
		pNode = (KLineNode*)pNode->GetNext();
	}
	
	pNode = (KLineNode *)m_RestLines.GetHead();
	while(pNode)
	{
		dwLen += pNode->m_pLineMem->GetMemLen();
		pNode = (KLineNode*)pNode->GetNext();
	}
	
	if (dwLen == 0 ) return 0;
	KMemClass1 *pFileMem = new KMemClass1;
	DataBuf = (LPSTR)pFileMem->Alloc(dwLen*2);
	pNode = (KLineNode*)m_Lines.GetHead();
	int nANum = 0;
	while(pNode)
	{
		strcpy(DataBuf, (LPSTR)pNode->m_pLineMem->GetMemPtr() ); 
		
		DataBuf += strlen(DataBuf)  ;
		pNode = (KLineNode*)pNode->GetNext();
	}
	
	pNode = (KLineNode *)m_RestLines.GetHead();
	while(pNode)
	{
		strcpy(DataBuf, (LPSTR)pNode->m_pLineMem->GetMemPtr() ); 
		DataBuf += strlen(DataBuf)  ;
		pNode = (KLineNode*)pNode->GetNext();
	}
	
	nANum = DataBuf - (char*)pFileMem->GetMemPtr() + strlen(DataBuf);

	pMem = pFileMem;

	return nANum;
}

BOOL	KLubCmpl_Blocker::Write(LPSTR szFileName)
{

	KFile		File;
	
	if (szFileName[0] == 0)
		return FALSE;

	KMemClass1 * pMem = NULL;
	int len = GetBuffer(pMem);
	if (len == 0 ) return FALSE;

	// create ini file
	if (!File.Create(szFileName))
		return FALSE;

	// write ini file
	
	File.Write(pMem->GetMemPtr(), len);
	File.Close();
	delete pMem;
	return TRUE;
}

void KLubCmpl_Blocker::Print()
{
	KLineNode*	pNode = (KLineNode*)m_Lines.GetHead();
	printf("\n---------------BEGIN--------------\n");
	while(pNode)
	{
		printf("%s",(LPSTR)pNode->m_pLineMem->GetMemPtr());
		pNode = (KLineNode*)pNode->GetNext();
	}
	printf("\n---------------END--------------\n");
}
