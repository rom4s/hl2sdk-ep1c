//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/Label.h"
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "text_message.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudHintDisplay : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudHintDisplay, vgui::Panel );

public:
	CHudHintDisplay( const char *pElementName );

	void Init();
	void Reset();
	void MsgFunc_HintText(bf_read &msg);

	bool SetHintText(wchar_t *text);
	void LocalizeAndDisplay(const char *pszHudTxtMsg, const char *szRawString);

	virtual void PerformLayout();

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnThink();

protected:
	vgui::HFont m_hFont;
	Color		m_bgColor;
	vgui::Label *m_pLabel;
	CUtlVector<vgui::Label *> m_Labels;
	CPanelAnimationVarAliasType(int, m_iTextX, "text_xpos", "8", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iTextY, "text_ypos", "8", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iCenterX, "center_x", "0", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iCenterY, "center_y", "0", "proportional_int");
};

DECLARE_HUDELEMENT( CHudHintDisplay );
DECLARE_HUD_MESSAGE( CHudHintDisplay, HintText );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHintDisplay::CHudHintDisplay( const char *pElementName ) : BaseClass(NULL, "HudHintDisplay"), CHudElement( pElementName )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	SetVisible( false );
	m_pLabel = new vgui::Label( this, "HudHintDisplayLabel", "" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintDisplay::Init()
{
	HOOK_HUD_MESSAGE( CHudHintDisplay, HintText );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintDisplay::Reset()
{
	SetHintText( NULL );
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HintMessageHide" ); 
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintDisplay::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetFgColor( GetSchemeColor("HintMessageFg", pScheme) );
	m_hFont = pScheme->GetFont( "HudHintText", true );
	m_pLabel->SetBgColor( GetSchemeColor("HintMessageBg", pScheme) );
	m_pLabel->SetPaintBackgroundType( 2 );
	m_pLabel->SetSize( 0, GetTall() );		// Start tiny, it'll grow.
}

//-----------------------------------------------------------------------------
// Purpose: Sets the hint text, replacing variables as necessary
//-----------------------------------------------------------------------------
bool CHudHintDisplay::SetHintText( wchar_t *text )
{
	if ( text == NULL || text[0] == L'\0' )
	{
		return false;
	}

	// clear the existing text
	for (int i = 0; i < m_Labels.Count(); i++)
	{
		m_Labels[i]->MarkForDeletion();
	}
	m_Labels.RemoveAll();

	wchar_t *p = text;

	while ( p )
	{
		wchar_t *line = p;
		wchar_t *end = wcschr( p, L'\n' );
		int linelengthbytes = 0;
		if ( end )
		{
			//*end = 0;	//eek
			p = end+1;
			linelengthbytes = ( end - line ) * 2;
		}
		else
		{
			p = NULL;
		}

		wchar_t buf[512] = L"";
		memcpy( buf, line, linelengthbytes > 0 ? linelengthbytes : wcslen(line) * 2 );
		
		// put it in a label
		vgui::Label *label = vgui::SETUP_PANEL(new vgui::Label(this, NULL, buf));
		label->SetFont( m_hFont );
		label->SetPaintBackgroundEnabled( false );
		label->SetPaintBorderEnabled( false );
		label->SizeToContents();
		label->SetContentAlignment( vgui::Label::a_west );
		label->SetFgColor( GetFgColor() );
		m_Labels.AddToTail( vgui::SETUP_PANEL(label) );
	}

	InvalidateLayout( true );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Resizes the label
//-----------------------------------------------------------------------------
void CHudHintDisplay::PerformLayout()
{
	BaseClass::PerformLayout();
	int i;

	int wide, tall;
	GetSize( wide, tall );

	// find the widest line
	int iDesiredLabelWide = 0;
	for ( i=0; i < m_Labels.Count(); ++i )
	{
		iDesiredLabelWide = max( iDesiredLabelWide, m_Labels[i]->GetWide() );
	}

	// find the total height
	int fontTall = vgui::surface()->GetFontTall( m_hFont );
	int labelTall = fontTall * m_Labels.Count();

	iDesiredLabelWide += m_iTextX*2;
	labelTall += m_iTextY*2;

	int x, y;
	if ( m_iCenterX < 0 )
	{
		x = 0;
	}
	else if ( m_iCenterX > 0 )
	{
		x = wide - iDesiredLabelWide;
	}
	else
	{
		x = (wide - iDesiredLabelWide) / 2;
	}

	if ( m_iCenterY > 0 )
	{
		y = 0;
	}
	else if ( m_iCenterY < 0 )
	{
		y = tall - labelTall;
	}
	else
	{
		y = (tall - labelTall) / 2;
	}

	x = max(x,0);
	y = max(y,0);

	iDesiredLabelWide = min(iDesiredLabelWide,wide);
	m_pLabel->SetBounds( x, y, iDesiredLabelWide, labelTall );

	// now lay out the sub-labels
	for ( i=0; i<m_Labels.Count(); ++i )
	{
		int xOffset = (wide - m_Labels[i]->GetWide()) * 0.5;
		m_Labels[i]->SetPos( xOffset, y + m_iTextY + i*fontTall );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates the label color each frame
//-----------------------------------------------------------------------------
void CHudHintDisplay::OnThink()
{
	m_pLabel->SetFgColor(GetFgColor());
	for (int i = 0; i < m_Labels.Count(); i++)
	{
		m_Labels[i]->SetFgColor(GetFgColor());
	}
}

//-----------------------------------------------------------------------------
// Purpose: Activates the hint display
//-----------------------------------------------------------------------------
void CHudHintDisplay::MsgFunc_HintText( bf_read &msg )
{
	// how many strings do we receive ?
	msg.ReadByte();
	
	// Read the string(s)
	char szString[255]; // 2048
	msg.ReadString( szString, sizeof(szString) );

	char *tmpStr = hudtextmessage->LookupString( szString, NULL );
	LocalizeAndDisplay( tmpStr, szString );
}

//-----------------------------------------------------------------------------
// Purpose: Localize, display, and animate the hud element
//-----------------------------------------------------------------------------
void CHudHintDisplay::LocalizeAndDisplay( const char *pszHudTxtMsg, const char *szRawString )
{
	static wchar_t szBuf[128];
	wchar_t *pszBuf;

	// init buffers & pointers
	szBuf[0] = 0;
	pszBuf = szBuf;

	// try to localize
	if ( pszHudTxtMsg )
	{
		pszBuf = vgui::localize()->Find( pszHudTxtMsg );
	}
	else
	{
		pszBuf = vgui::localize()->Find( szRawString );
	}

	if ( !pszBuf )
	{
		// use plain ASCII string 
		vgui::localize()->ConvertANSIToUnicode( szRawString, szBuf, sizeof(szBuf) );
		pszBuf = szBuf;
	}

	// make it visible
	if ( SetHintText( pszBuf ) )
	{
		SetVisible( true );
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HintMessageShow" ); 
	}
	else
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HintMessageHide" ); 
	}
}
