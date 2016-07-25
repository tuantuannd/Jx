#include "KCore.h"
#include "KNpc.h"
#include "KMissle.h"
#include "KItem.h"
#include "KBuySell.h"
#include "KPlayer.h"
#include "../../Represent/iRepresent/iRepresentshell.h"
#include "KSubWorldSet.h"
#include "scene/KScenePlaceC.h"
#include "ImgRef.h"
#include "GameDataDef.h"
#include "KObjSet.h"

#define  PHYSICSSKILLICON "\\spr\\Ui\\技能图标\\icon_sk_ty_ap.spr"

#define SHOW_SPACE_HEIGHT 5

void	CoreDrawGameObj(unsigned int uObjGenre, unsigned int uId, int x, int y, int Width, int Height, int nParam)
{
//	static int nSpeed = 0;
	switch(uObjGenre)
	{
	case CGOG_NPC:
		if (uId > 0)
		{
			if ((nParam & IPOT_RL_INFRONTOF_ALL) == IPOT_RL_INFRONTOF_ALL)
			{
				int nHeight = Npc[uId].GetNpcPate();

				Npc[uId].PaintBlood(nHeight / 2);	//冒血

				int nnHeight = nHeight;
				nHeight = Npc[uId].PaintChat(nnHeight);	//聊天信息中有名字

				if (nHeight == nnHeight)	//没有聊天信息时绘制人物信息
				{
					if (NpcSet.CheckShowLife())
					{
						nHeight = Npc[uId].PaintLife(nnHeight, false);
					}

					if (NpcSet.CheckShowName())
					{
						if (nnHeight != nHeight)	//有内力显示时
						{
							nHeight += SHOW_SPACE_HEIGHT;//好看
						}

						if (Player[CLIENT_PLAYER_INDEX].GetTargetNpc() && Player[CLIENT_PLAYER_INDEX].GetTargetNpc() == uId)
							nHeight = Npc[uId].PaintInfo(nHeight, false, 14, 0XFF000000);	//被选中的人名放大显示
						else
							nHeight = Npc[uId].PaintInfo(nHeight, false);
						nHeight += 0;
					}
				}
			}
			else if ((nParam & IPOT_RL_OBJECT) == IPOT_RL_OBJECT)
			{
				Npc[uId].Paint();
				if ((int)uId == Player[CLIENT_PLAYER_INDEX].GetTargetNpc())
				{
					Npc[uId].DrawBorder();
				}
			}			
		}
		break;
	case CGOG_MISSLE:
		if (uId > 0)
			Missle[uId].Paint();
		break;
	case CGOG_ITEM:
		if (uId == 0)
			break;

		if (Width == 0 && Height == 0)
		{
#define	ITEM_CELL_WIDTH		26
#define	ITEM_CELL_HEIGHT	26
			Width = Item[uId].GetWidth() * ITEM_CELL_WIDTH;
			Height = Item[uId].GetHeight() * ITEM_CELL_HEIGHT;
			x -= Width / 2;
			y -= Height / 2;
		}
		else
		{
			x += (Width - Item[uId].GetWidth() * ITEM_CELL_WIDTH) / 2;
			y += (Height - Item[uId].GetHeight() * ITEM_CELL_HEIGHT) / 2;
		}
		Item[uId].Paint(x, y);
		break;
	case CGOG_MENU_NPC:
		if (nParam)
		{
			((KNpcRes *)nParam)->SetPos(0, x + Width / 2, y + Height / 2 + 28, 0, FALSE, TRUE);
#define		STAND_TOTAL_FRAME	15
			int nFrame = g_SubWorldSet.m_nLoopRate % STAND_TOTAL_FRAME;
			((KNpcRes *)nParam)->Draw(0, 0, STAND_TOTAL_FRAME, nFrame, true);
		}
		break;
	
	case CGOG_NPC_BLUR_DETAIL(1):
		Npc[uId].GetNpcRes()->m_cNpcBlur.Draw(1);
		break;
	case CGOG_NPC_BLUR_DETAIL(2):
		Npc[uId].GetNpcRes()->m_cNpcBlur.Draw(2);
		break;
	case CGOG_NPC_BLUR_DETAIL(3):
		Npc[uId].GetNpcRes()->m_cNpcBlur.Draw(3);
		break;
	case CGOG_NPC_BLUR_DETAIL(4):
		Npc[uId].GetNpcRes()->m_cNpcBlur.Draw(4);
		break;
	case CGOG_NPC_BLUR_DETAIL(5):
		Npc[uId].GetNpcRes()->m_cNpcBlur.Draw(5);
		break;
	case CGOG_NPC_BLUR_DETAIL(6):
		Npc[uId].GetNpcRes()->m_cNpcBlur.Draw(6);
		break;
	case CGOG_NPC_BLUR_DETAIL(7):
		Npc[uId].GetNpcRes()->m_cNpcBlur.Draw(7);
		break;
	case CGOG_SKILL:
	case CGOG_SKILL_FIGHT:
	case CGOG_SKILL_LIVE:
	case CGOG_SKILL_SHORTCUT:
		{
			int nSkillId = (int)uId;
		if (nSkillId > 0)
		{
			ISkill *pSkill = g_SkillManager.GetSkill(nSkillId,1);
			if (pSkill)
            {
				pSkill->DrawSkillIcon(x, y, Width, Height);
            }
		}
		else
		{
			if(uId == -1) 
			{
				KRUImage RUIconImage;
				RUIconImage.nType = ISI_T_SPR;
				RUIconImage.Color.Color_b.a = 255;
				RUIconImage.bRenderStyle = IMAGE_RENDER_STYLE_ALPHA;
				RUIconImage.uImage = 0;
				RUIconImage.nISPosition = IMAGE_IS_POSITION_INIT;
				RUIconImage.bRenderFlag = 0;
				strcpy(RUIconImage.szImage, PHYSICSSKILLICON);
				RUIconImage.oPosition.nX = x;
				RUIconImage.oPosition.nY = y;
				RUIconImage.oPosition.nZ = 0;
				RUIconImage.nFrame = 0;
				g_pRepresent->DrawPrimitives(1, &RUIconImage, RU_T_IMAGE, 1);
			}
		}
		}
		break;
	case CGOG_OBJECT:
		if (uId)
		{
			if ((nParam & IPOT_RL_INFRONTOF_ALL) == IPOT_RL_INFRONTOF_ALL)
			{
				if (ObjSet.CheckShowName())
					Object[uId].DrawInfo();
			}
			else //if ((nParam & IPOT_RL_COVER_GROUND) == IPOT_RL_COVER_GROUND)
			{
				Object[uId].Draw();
				if ((int)uId == Player[CLIENT_PLAYER_INDEX].GetTargetObj())
					Object[uId].DrawBorder();
			}
		}
		break;
	case CGOG_NPCSELLITEM:
		if (uId < 0)
			break;
		BuySell.PaintItem(uId, x, y);
		break;
	default:
		break;
	}
}

void	CoreGetGameObjLightInfo(unsigned int uObjGenre, unsigned int uId, KLightInfo *pLightInfo)
{
	switch (uObjGenre)
	{
	case CGOG_NPC:
		if (uId > 0 && uId < MAX_NPC)
		{
			Npc[uId].GetMpsPos(&pLightInfo->oPosition.nX, &pLightInfo->oPosition.nY);
			pLightInfo->oPosition.nZ = Npc[uId].m_Height;
			pLightInfo->dwColor = 0;
			if (Npc[uId].m_RedLum > 255)
			{
				Npc[uId].m_RedLum = 255;
			}
			if (Npc[uId].m_GreenLum > 255)
			{
				Npc[uId].m_GreenLum = 255;
			}
			if (Npc[uId].m_BlueLum > 255)
			{
				Npc[uId].m_BlueLum = 255;
			}
			pLightInfo->dwColor = 0xff000000 | Npc[uId].m_RedLum << 16 | Npc[uId].m_GreenLum << 8 | Npc[uId].m_BlueLum;
			pLightInfo->nRadius = Npc[uId].m_CurrentVisionRadius;
		}
		break;
	case CGOG_OBJECT:
		break;
	case CGOG_MISSLE:
		if (uId > 0 && uId < MAX_MISSLE)
		{
			if (Missle[uId].m_nMissleId > 0)
			{
				Missle[uId].GetLightInfo(pLightInfo);
			}
		}
		break;
	default:
		break;
	}
}