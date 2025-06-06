/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// battery.cpp
//
// implementation of CHudBattery class
//
#include <string.h>
#include <stdio.h>

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "battery.h"

ConVar hud_battery_width_pos("hud_battery_width_pos", "5", FCVAR_BHL_ARCHIVE, "Armor weight position");
ConVar hud_battery_top("hud_battery_top", "0", FCVAR_BHL_ARCHIVE, "Armor on top");
ConVar hud_battery_hide("hud_battery_hide", "0", FCVAR_BHL_ARCHIVE, "Hide armor HUD if you don't have any armor");

DEFINE_HUD_ELEM(CHudBattery);

void CHudBattery::Init(void)
{
	BaseHudClass::Init();
	m_iBat = 0;
	m_fFade = 0;
	m_iFlags = 0;

	HookMessage<&CHudBattery::MsgFunc_Battery>("Battery");
};

void CHudBattery::VidInit()
{
	int HUD_suit_empty = gHUD.GetSpriteIndex("suit_empty");
	int HUD_suit_full = gHUD.GetSpriteIndex("suit_full");

	m_hSprite1 = m_hSprite2 = 0; // delaying get sprite handles until we know the sprites are loaded
	m_rc1 = gHUD.GetSpriteRect(HUD_suit_empty);
	m_rc2 = gHUD.GetSpriteRect(HUD_suit_full);
	m_iHeight = m_rc2.bottom - m_rc1.top;
	m_fFade = 0;
};

int CHudBattery::MsgFunc_Battery(const char *pszName, int iSize, void *pbuf)
{
	m_iFlags |= HUD_ACTIVE;

	BEGIN_READ(pbuf, iSize);
	int battery = READ_SHORT();
	battery = clamp(battery, 0, 999);

	if (battery != m_iBat)
	{
		m_fFade = FADE_TIME;
		m_iBat = battery;
	}

	return 1;
}

void CHudBattery::Draw(float flTime)
{
	if (gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH)
		return;

	int r, g, b, x, y;
	float a;
	wrect_t rc;

	rc = m_rc2;
	rc.top += m_iHeight * ((float)(100 - (min(100, m_iBat))) * 0.01f); // battery can go from 0 to 100 so * 0.01 goes from 0 to 1

	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return;

	if (hud_battery_hide.GetBool() && m_iBat <= 0)
		return;

	if (!hud_dim.GetBool())
		a = MIN_ALPHA + ALPHA_POINTS_MAX;
	else if (m_fFade > 0)
	{
		// Fade the armor number back to dim
		m_fFade -= (gHUD.m_flTimeDelta * 20);
		if (m_fFade <= 0)
			m_fFade = 0;
		a = MIN_ALPHA + (m_fFade / FADE_TIME) * ALPHA_POINTS_FLASH;
	}
	else
		a = MIN_ALPHA;

	a *= gHUD.GetHudTransparency();
	gHUD.GetHudColor(HudPart::Armor, m_iBat, r, g, b);
	ScaleColors(r, g, b, a);

	int iOffset = (m_rc1.bottom - m_rc1.top) / 6;

	if (hud_battery_top.GetBool())
	{
		y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight * 2.0;
		x = gHUD.m_iFontHeight * 1.2 - gHUD.m_iFontHeight;
	}
	else
	{
		y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;
		x = ScreenWidth / hud_battery_width_pos.GetInt();
	}

	// make sure we have the right sprite handles
	if (!m_hSprite1)
		m_hSprite1 = gHUD.GetSprite(gHUD.GetSpriteIndex("suit_empty"));
	if (!m_hSprite2)
		m_hSprite2 = gHUD.GetSprite(gHUD.GetSpriteIndex("suit_full"));

	SPR_Set(m_hSprite1, r, g, b);
	SPR_DrawAdditive(0, x, y - iOffset, &m_rc1);

	if (rc.bottom > rc.top)
	{
		SPR_Set(m_hSprite2, r, g, b);
		SPR_DrawAdditive(0, x, y - iOffset + (rc.top - m_rc2.top), &rc);
	}

	x += (m_rc1.right - m_rc1.left);
	x = gHUD.DrawHudNumber(x, y, DHN_3DIGITS | DHN_DRAWZERO, m_iBat, r, g, b);
}
