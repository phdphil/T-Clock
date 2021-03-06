// Created by Stoic Joker: Tuesday, 03/30/2010 @ 10:27:12pm
#include "tclock.h"

hotkey_t* tchk = NULL;

static BOOL bReset;
static BOOL bHaveKey;
static BOOL bFirstTime;
static UINT uLastKey;
char szHotKeySubKey[] = "HotKeys";
WNDPROC OldEditClassProc; // Default Procedure for Edit Controls
#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0)
//================================================================================================
//-----------------------------------------------+++--> Save the New HotKey Configuration Settings:
void OnApply(HWND hDlg)   //----------------------------------------------------------------+++-->
{
	char subkey[TNY_BUFF] = {0};
	int i;
	
	if(tchk[0].bValid) { // Add/Edit Active Timers
		RegisterHotKey(g_hwndTClockMain, HOT_TIMER, tchk[0].fsMod, tchk[0].vk);
	} else {
		tchk[0].vk = 0;
		tchk[0].fsMod = 0;
		strcpy(tchk[0].szText, "None");
		UnregisterHotKey(g_hwndTClockMain, HOT_TIMER);
	}
	
	if(tchk[1].bValid) { // Display StopWatch
		RegisterHotKey(g_hwndTClockMain, HOT_STOPW, tchk[1].fsMod, tchk[1].vk);
	} else {
		tchk[1].vk = 0;
		tchk[1].fsMod = 0;
		strcpy(tchk[1].szText, "None");
		UnregisterHotKey(g_hwndTClockMain, HOT_STOPW);
	}
	
	if(tchk[2].bValid) { // Display Watched (and active) Timers
		RegisterHotKey(g_hwndTClockMain, HOT_WATCH, tchk[2].fsMod, tchk[2].vk);
	} else {
		tchk[2].vk = 0;
		tchk[2].fsMod = 0;
		strcpy(tchk[2].szText, "None");
		UnregisterHotKey(g_hwndTClockMain, HOT_WATCH);
	}
	
	if(tchk[3].bValid) { // Display Watched (and active) Timers
		RegisterHotKey(g_hwndTClockMain, HOT_PROPR, tchk[3].fsMod, tchk[3].vk);
	} else {
		tchk[3].vk = 0;
		tchk[3].fsMod = 0;
		strcpy(tchk[3].szText, "None");
		UnregisterHotKey(g_hwndTClockMain, HOT_PROPR);
	}
	
	if(tchk[4].bValid) { // Display Watched (and active) Timers
		RegisterHotKey(g_hwndTClockMain, HOT_CALEN, tchk[4].fsMod, tchk[4].vk);
	} else {
		tchk[4].vk = 0;
		tchk[4].fsMod = 0;
		strcpy(tchk[4].szText, "None");
		UnregisterHotKey(g_hwndTClockMain, HOT_CALEN);
	}
	///////////////////////////////////////////////////////////////////////////////////
	for(i = 0; i <= 4; i++) {
		wsprintf(subkey, "%s\\HK%d", szHotKeySubKey, i);
		api.SetInt(subkey, "bValid", tchk[i].bValid);
		api.SetInt(subkey, "fsMod",  tchk[i].fsMod);
		api.SetStr(subkey, "szText", tchk[i].szText);
		api.SetInt(subkey, "vk",  tchk[i].vk);
	}
	//////////////////////////////////////////////////////////////////////////////////
	bFirstTime = FALSE; // DO NOT Let Property Sheet Manager Fire OnApply(...) Twice!
	EnableDlgItem(hDlg, IDCE_HK_ADD,  FALSE);
	EnableDlgItem(hDlg, IDCE_HK_STOP, FALSE);
	EnableDlgItem(hDlg, IDCE_HK_TIME, FALSE);
	EnableDlgItem(hDlg, IDCE_HK_PROP, FALSE);
	EnableDlgItem(hDlg, IDCE_HK_CALN, FALSE);
	EnableDlgItem(hDlg, IDCB_HK_ADD,  TRUE);
	EnableDlgItem(hDlg, IDCB_HK_STOP, TRUE);
	EnableDlgItem(hDlg, IDCB_HK_TIME, TRUE);
	EnableDlgItem(hDlg, IDCB_HK_PROP, TRUE);
	EnableDlgItem(hDlg, IDCB_HK_CALN, TRUE);
}
//================================================================================================
//-------------------------------------------+++--> Display the HotKey Combination Pressed by User:
BOOL ShowHotKey(HWND hWnd, int iCtrlID, char* szKey, BOOL bMod, BOOL bFnKey, BOOL bEnd)
{
	static char szHotKey[TNY_BUFF] = {0};
	char szTmp[TNY_BUFF];
	
	(void)bMod;
	
	Edit_GetText(hWnd, szTmp, TNY_BUFF);
	if(_stricmp("None", szTmp)) { // None = Zero Input - Clear Display.
		wnsprintf(szHotKey,sizeof(szHotKey)-1, "%s + %s", szTmp, szKey); // HotKey = X + Y
		Edit_SetText(hWnd, szHotKey); //-+> Display the Key(s) Pressed.
		bHaveKey = bEnd; // Beginning & End Confirmed, Combo Save-able
		bFnKey = FALSE; //---------+++--> This IS NOT Just an F'n Key!
	} else {
		Edit_SetText(hWnd, szKey); // Have the First (begin) Key, or,
		bHaveKey = bFnKey; // This Allows the F'n Keys to Work Alone.
	}
	if(bHaveKey && bFnKey && bEnd) { //-+> This IS Just an F'n Key...
		strncpy_s(tchk[iCtrlID].szText, sizeof(tchk[iCtrlID].szText), szKey, _TRUNCATE);
	} else { //--+++--> This is a HotKey Combo...
		strncpy_s(tchk[iCtrlID].szText, sizeof(tchk[iCtrlID].szText), szHotKey, _TRUNCATE);
	}
	
	return TRUE;
}
//===============================================================================================
//--------------------------+++--> Edit Control Subclass Procedure to Catch/Record HotKey Strokes:
LRESULT APIENTRY SubClassEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)   //-+++-->
{
	char szKeyTmp[TNY_BUFF];
	int iCtrlID;
	
	switch(uMsg) {
	case WM_GETDLGCODE:
		return DLGC_WANTMESSAGE;
		
	case WM_CHAR: // This is Required to Block (Direct Access)
		return TRUE; //-+> Keyboard Input to the Edit Control.
		
	case WM_KEYDOWN: //-------------+++--> Looping is Forbidden!
		if((UINT)wParam == uLastKey) return TRUE; //-+> By This.
		if((UINT)wParam == VK_LWIN || (UINT)wParam == VK_RWIN) {
			return TRUE; // Block Acceptance of the Windows Keys
		} //-+> as Their Behavior is (reserved) Unpredictable...
		iCtrlID = (GetDlgCtrlID(hWnd) - 1026); // Edit Control=X
		if((tchk[iCtrlID].fsMod == MOD_SHIFT) && // Translation:
		   (((UINT)wParam <= VK_DIVIDE) || // Prevent User From
			((UINT)wParam >= VK_NUMLOCK))) { // Selecting any of
			bReset = TRUE; // the Dangerous Key Combinations
		} //----------//------//------------//-----------+++-->
		if(bReset) { //------//------------//-----------+++-->
			tchk[iCtrlID].bValid = FALSE; //-----------+++-->
			Edit_SetText(hWnd, "None"); ///-----------+++-->
			bReset = FALSE;
		}
		if((wParam >= VK_SHIFT) && (wParam <= VK_MENU)) {
			if(wParam == VK_CONTROL) {
				tchk[iCtrlID].fsMod = MOD_CONTROL;
			}
			
			else if(wParam == VK_SHIFT) {
				if(tchk[iCtrlID].fsMod)
					tchk[iCtrlID].fsMod |= MOD_SHIFT;
				else
					tchk[iCtrlID].fsMod = MOD_SHIFT;
			}
			
			else if(wParam == VK_MENU) {
				if(tchk[iCtrlID].fsMod)
					tchk[iCtrlID].fsMod |= MOD_ALT;
				else
					tchk[iCtrlID].fsMod = MOD_ALT;
			}
			
			if(!GetKeyNameText((LONG)lParam, szKeyTmp, sizeof(szKeyTmp)))
				szKeyTmp[0] = '\0';
			ShowHotKey(hWnd, iCtrlID, szKeyTmp, TRUE, FALSE, FALSE);
		}
		//-++-> VK_F1 = 0x70 (112) - VK_F24 = 0x87 (135)
		else if((wParam >= VK_F1) && (wParam <= VK_F24)) {
			tchk[iCtrlID].vk = (UINT)wParam;
			if(!GetKeyNameText((LONG)lParam, szKeyTmp, sizeof(szKeyTmp)))
				szKeyTmp[0] = '\0';
			ShowHotKey(hWnd, iCtrlID, szKeyTmp, FALSE, TRUE, TRUE);
		}
		
		else { //-+> Oh Shit! ...Sombody Hit the AnyKey!!!
			tchk[iCtrlID].vk = (UINT)wParam;
			if(!GetKeyNameText((LONG)lParam, szKeyTmp, sizeof(szKeyTmp)))
				szKeyTmp[0] = '\0';
			ShowHotKey(hWnd, iCtrlID, szKeyTmp, FALSE, FALSE, TRUE);
		}
		uLastKey = (UINT)wParam; //-+> Check for This on Next Loop, To Break Loop!
		return TRUE; //-------------------------------+++--> End of Case WM_KEYDOWN:
//==================================================================================
	case WM_KEYUP:
		uLastKey = 0; //-//-------+++--> Clear the Slate.
		if(!bHaveKey) { // IF Key Record Failed, Go Dark.
			iCtrlID = (GetDlgCtrlID(hWnd) - 1026);
			strcpy(tchk[iCtrlID].szText, "None");
			tchk[iCtrlID].bValid = FALSE;
			Edit_SetText(hWnd, "None");
			tchk[iCtrlID].fsMod = 0;
			tchk[iCtrlID].vk = 0;
		} else { // ELSE Offer to Save New Key Combination.
			bReset = TRUE;
			iCtrlID = (GetDlgCtrlID(hWnd) - 1026);
			tchk[iCtrlID].bValid = TRUE;
			SendPSChanged(GetParent(hWnd));
		} return TRUE; //-------------------------------+++--> End of Case WM_KEYUP:
//==================================================================================
	}
	//-------------------+++--> Let Windows Handle What We Forget to...
	return CallWindowProc(OldEditClassProc, hWnd, uMsg, wParam, lParam);
}
//================================================================================================
//-------------------------------//---------------------------+++--> Initialize the HotKeys Dialog:
static void OnInit(HWND hDlg)   //----------------------------------------------------------+++-->
{
	char subkey[TNY_BUFF] = {0};
	int i;
	
	bFirstTime = TRUE;
	tchk = (hotkey_t*)malloc(sizeof(hotkey_t) * 5);
	for(i=0; i <= 4; i++) {
		wsprintf(subkey, "%s\\HK%d", szHotKeySubKey, i);
		tchk[i].bValid = api.GetInt(subkey, "bValid", 0);
//		api.GetStrEx(subkey, "szText", tchk[i].szText, GEN_BUFF, "None");
		api.GetStrEx(subkey, "szText", tchk[i].szText, TNY_BUFF, "None");
		tchk[i].fsMod = api.GetInt(subkey, "fsMod", 0);
		tchk[i].vk = api.GetInt(subkey, "vk", 0);
	}
	
	SetDlgItemText(hDlg, IDCE_HK_ADD, tchk[0].szText);
	SetDlgItemText(hDlg, IDCE_HK_STOP, tchk[1].szText);
	SetDlgItemText(hDlg, IDCE_HK_TIME, tchk[2].szText);
	SetDlgItemText(hDlg, IDCE_HK_PROP, tchk[3].szText);
	SetDlgItemText(hDlg, IDCE_HK_CALN, tchk[4].szText);
	
	// Subclass the Edit Controls
	OldEditClassProc  = SubclassWindow(GetDlgItem(hDlg,IDCE_HK_ADD), SubClassEditProc);
	SubclassWindow(GetDlgItem(hDlg,IDCE_HK_STOP), SubClassEditProc);
	SubclassWindow(GetDlgItem(hDlg,IDCE_HK_TIME), SubClassEditProc);
	SubclassWindow(GetDlgItem(hDlg,IDCE_HK_PROP), SubClassEditProc);
	SubclassWindow(GetDlgItem(hDlg,IDCE_HK_CALN), SubClassEditProc);
//==================================================================================
}
//================================================================================================
//----------------------------------//---------------+++--> Free TCHOTKEY Structure Memory on Exit:
static void OnDestroy()   //-------------------------------------------------------+++-->
{
	if(tchk) {
		free(tchk);   // Free, and...? (Crash Unless You Include the Next Line)
		tchk = NULL; //<--+++--> Thank You Don Beusee for reminding me to do this.
	}
}
//================================================================================================
//---------------------------------+++--> Dialog Precedure For Configurable HotKeys Properties Tab:
INT_PTR CALLBACK PageHotKeyProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)   //----+++-->
{
	switch(message) {
	case WM_INITDIALOG:
		OnInit(hDlg);
		return TRUE;
	case WM_DESTROY:
		OnDestroy();
		break;
		
	case WM_NOTIFY:
		switch(((NMHDR*)lParam)->code) {
		case PSN_APPLY: // Because the Second Time You Try to Register the
			if(bFirstTime) OnApply(hDlg); //-+> Same HotKey is an Error...
			break;
			
		} return TRUE;
		
	case WM_COMMAND: {
		WORD id, code;
		id = LOWORD(wParam);
		code = HIWORD(wParam);
		
		if(code == EN_KILLFOCUS) { //--++-> Add This After Bugg Found By:
			if(!tchk[(id - 1026)].bValid) { // ewemoa - DonationCoder.com
				SetDlgItemText(hDlg, id, "None");  // Burn the HotKet on
				tchk[(id - 1026)].bValid = FALSE; // Input Focus Failure
				bReset = TRUE;
			} return TRUE;
		}
		
		switch(id) {
		case IDCB_HK_ADD:
			EnableDlgItem(hDlg, IDCE_HK_ADD, TRUE);
			EnableDlgItem(hDlg, IDCB_HK_ADD, FALSE);
			SetDlgItemText(hDlg, IDCE_HK_ADD, "None");
			SetFocus(GetDlgItem(hDlg, IDCE_HK_ADD));
			tchk[0].bValid = FALSE; // Required to Clear/Remove HotKey
			SendPSChanged(hDlg);
			tchk[0].fsMod = 0;
			tchk[0].vk = 0;
			bFirstTime = TRUE; // This Will Allow (boo-boo) Re-Do's
			break;
			
		case IDCB_HK_STOP:
			EnableDlgItem(hDlg, IDCE_HK_STOP, TRUE);
			EnableDlgItem(hDlg, IDCB_HK_STOP, FALSE);
			SetDlgItemText(hDlg, IDCE_HK_STOP, "None");
			SetFocus(GetDlgItem(hDlg, IDCE_HK_STOP));
			tchk[1].bValid = FALSE; // Required to Clear/Remove HotKey
			SendPSChanged(hDlg);
			tchk[1].fsMod = 0;
			tchk[1].vk = 0;
			bFirstTime = TRUE; // This Will Allow (boo-boo) Re-Do's
			break;
			
		case IDCB_HK_TIME:
			EnableDlgItem(hDlg, IDCE_HK_TIME, TRUE);
			EnableDlgItem(hDlg, IDCB_HK_TIME, FALSE);
			SetDlgItemText(hDlg, IDCE_HK_TIME, "None");
			SetFocus(GetDlgItem(hDlg, IDCE_HK_TIME));
			tchk[2].bValid = FALSE; // Required to Clear/Remove HotKey
			SendPSChanged(hDlg);
			tchk[2].fsMod = 0;
			tchk[2].vk = 0;
			bFirstTime = TRUE; // This Will Allow (boo-boo) Re-Do's
			break;
			
		case IDCB_HK_PROP:
			EnableDlgItem(hDlg, IDCE_HK_PROP, TRUE);
			EnableDlgItem(hDlg, IDCB_HK_PROP, FALSE);
			SetDlgItemText(hDlg, IDCE_HK_PROP, "None");
			SetFocus(GetDlgItem(hDlg, IDCE_HK_PROP));
			tchk[3].bValid = FALSE; // Required to Clear/Remove HotKey
			SendPSChanged(hDlg);
			tchk[3].fsMod = 0;
			tchk[3].vk = 0;
			bFirstTime = TRUE; // This Will Allow (boo-boo) Re-Do's
			break;
			
		case IDCB_HK_CALN:
			EnableDlgItem(hDlg, IDCE_HK_CALN, TRUE);
			EnableDlgItem(hDlg, IDCB_HK_CALN, FALSE);
			SetDlgItemText(hDlg, IDCE_HK_CALN, "None");
			SetFocus(GetDlgItem(hDlg, IDCE_HK_CALN));
			tchk[4].bValid = FALSE; // Required to Clear/Remove HotKey
			SendPSChanged(hDlg);
			tchk[4].fsMod = 0;
			tchk[4].vk = 0;
			bFirstTime = TRUE; // This Will Allow (boo-boo) Re-Do's
			break;
		}
		return TRUE;}
	}
	return FALSE;
}
//================================================================================================
//----------------------------------+++--> Retrieve HotKey Configuration Information From Registry:
void GetHotKeyInfo(HWND hWnd)
{
	char subkey[TNY_BUFF] = {0};
	int i;
	
	hotkey_t* hk = (hotkey_t*)malloc(sizeof(hotkey_t) * 6);
	if(!hk) return;
	
	for(i = 0; i <= 5; i++) {
		wsprintf(subkey, "%s\\HK%d", szHotKeySubKey, i);
		hk[i].bValid = api.GetIntEx(subkey, "bValid", 0);
		hk[i].fsMod = api.GetIntEx(subkey, "fsMod", 0);
		hk[i].vk = api.GetIntEx(subkey, "vk", 0);
	}
	
	if(hk[0].bValid) RegisterHotKey(hWnd, HOT_TIMER, hk[0].fsMod, hk[0].vk); // Add/Edit Timer
	if(hk[1].bValid) RegisterHotKey(hWnd, HOT_STOPW, hk[1].fsMod, hk[1].vk); // StopWatch Dialog
	if(hk[2].bValid) RegisterHotKey(hWnd, HOT_WATCH, hk[2].fsMod, hk[2].vk); // Timer Watch List
	if(hk[3].bValid) RegisterHotKey(hWnd, HOT_PROPR, hk[3].fsMod, hk[3].vk); // T-Clock Properties
	if(hk[4].bValid) RegisterHotKey(hWnd, HOT_CALEN, hk[4].fsMod, hk[4].vk); // T-Clock Calendar
	if(hk[5].bValid) RegisterHotKey(hWnd, HOT_TSYNC, hk[5].fsMod, hk[5].vk); // Synchronize Time
	free(hk);
}
