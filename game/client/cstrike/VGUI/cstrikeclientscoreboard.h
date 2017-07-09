//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CSTRIKECLIENTSCOREBOARDDIALOG_H
#define CSTRIKECLIENTSCOREBOARDDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <clientscoreboarddialog.h>
#include "cs_shareddefs.h"

//-----------------------------------------------------------------------------
// Purpose: Game ScoreBoard
//-----------------------------------------------------------------------------
class CCSClientScoreBoardDialog : public CClientScoreBoardDialog
{
private:
	DECLARE_CLASS_SIMPLE( CCSClientScoreBoardDialog, CClientScoreBoardDialog );
	
public:
	CCSClientScoreBoardDialog( IViewPort *pViewPort );
	~CCSClientScoreBoardDialog();

#ifdef _CLIENT_FIXES
	virtual void FireGameEvent( IGameEvent *event );
#endif

protected:
	virtual void InitScoreboardSections();
	virtual void UpdateTeamInfo();
	virtual bool GetPlayerScoreInfo(int playerIndex, KeyValues *outPlayerInfo);
	virtual void UpdatePlayerInfo();

	// vgui overrides for rounded corner background
	virtual void PaintBackground();
	virtual void PaintBorder();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	virtual void AddHeader(); // add the start header of the scoreboard
	virtual void AddSection(int teamType, int teamNumber); // add a new section header for a team

	static bool CSPlayerSortFunc( KeyValues *it1, KeyValues *it2 );
	void CSPlayerSortFunc();

	int GetSectionFromTeamNumber( int teamNumber );

	enum
	{
		CSTRIKE_NAME_WIDTH = 320,
		CSTRIKE_CLASS_WIDTH = 56,
		CSTRIKE_SCORE_WIDTH = 40,
		CSTRIKE_DEATH_WIDTH = 46,
		CSTRIKE_PING_WIDTH = 46,
	};

	// rounded corners
	Color					 m_bgColor;
	Color					 m_borderColor;

	CUtlVector<KeyValues*> m_teamPlayers[TEAM_MAXCOUNT];
};


#endif // CSTRIKECLIENTSCOREBOARDDIALOG_H
