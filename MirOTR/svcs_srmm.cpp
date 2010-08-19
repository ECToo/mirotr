#include "stdafx.h"
HANDLE hEventIconPressed;
HICON hIconNotSecure, hIconFinished, hIconPrivate, hIconUnverified;

int WindowEvent(WPARAM wParam, LPARAM lParam) {
	MessageWindowEventData *mwd = (MessageWindowEventData *)lParam;
	
	if(mwd->uType == MSG_WINDOW_EVT_CLOSE && options.end_window_close) {
		// FinishSession(mwd->hContact);
		return 0;
	}	
	if(mwd->uType != MSG_WINDOW_EVT_OPEN) return 0;
	if(!options.bHaveSRMMIcons) return 0;
	
	HANDLE hContact = mwd->hContact, hTemp;
	if(options.bHaveMetaContacts && (hTemp = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0)) != 0)
		hContact = hTemp;
	
	if(!CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)MODULENAME))
		return 0;
	
	lib_cs_lock();
	ConnContext *context = otrl_context_find_miranda(otr_user_state, hContact);
	lib_cs_unlock();
	
	SetEncryptionStatus(hContact, otr_context_get_trust(context));
	
	return 0;
}

int SVC_IconPressed(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;
	if(sicd->cbSize < (int)sizeof(StatusIconClickData))
		return 0;
	
	if(strcmp(sicd->szModule, MODULENAME) == 0) {
		char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if(proto && DBGetContactSettingByte(hContact, proto, "ChatRoom", 0))
			return 0;
		HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXT));
		CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM)hMenu, 0);
		TrustLevel level = (TrustLevel) db_dword_get(hContact, MODULENAME, "TrustLevel", TRUST_NOT_PRIVATE);
		MENUITEMINFO minfo = {0};
		minfo.cbSize = sizeof(MENUITEMINFO);
		minfo.fMask = MIIM_STRING|MIIM_STATE|MIIM_BITMAP;
		minfo.fState = MF_DISABLED;

		HWND wnd = GetFocus();
		HDC dc = GetDC(wnd), hdc = CreateCompatibleDC(dc);
		HBITMAP bmpIconPrivate = CreateCompatibleBitmap(dc, 16, 16);
		HGDIOBJ old = SelectObject(hdc, bmpIconPrivate);
		DrawIconEx(hdc, 0, 0, hIconPrivate,16, 16, 0, GetSysColorBrush(COLOR_MENU), DI_NORMAL);

		HBITMAP bmpIconUnverified = CreateCompatibleBitmap(dc, 16, 16);
		SelectObject(hdc, bmpIconUnverified);
		DrawIconEx(hdc, 0, 0, hIconUnverified,16, 16, 0, GetSysColorBrush(COLOR_MENU), DI_NORMAL);

		HBITMAP bmpIconFinished = CreateCompatibleBitmap(dc, 16, 16);
		SelectObject(hdc, bmpIconFinished);
		DrawIconEx(hdc, 0, 0, hIconFinished,16, 16, 0, GetSysColorBrush(COLOR_MENU), DI_NORMAL);

		HBITMAP bmpIconNotSecure = CreateCompatibleBitmap(dc, 16, 16);
		SelectObject(hdc, bmpIconNotSecure);
		DrawIconEx(hdc, 0, 0, hIconNotSecure,16, 16, 0, GetSysColorBrush(COLOR_MENU), DI_NORMAL);

		switch (level) {
			case TRUST_PRIVATE:
				minfo.hbmpItem = bmpIconPrivate;
				minfo.dwTypeData = TranslateT(LANG_STATUS_PRIVATE);
				break;
			case TRUST_UNVERIFIED:
				minfo.hbmpItem = bmpIconUnverified;
				minfo.dwTypeData = TranslateT(LANG_STATUS_UNVERIFIED);
				break;
			case TRUST_FINISHED:
				minfo.hbmpItem = bmpIconFinished;
				minfo.dwTypeData = TranslateT(LANG_STATUS_FINISHED);
				break;
			default:
				minfo.hbmpItem = bmpIconNotSecure;
				minfo.dwTypeData = TranslateT(LANG_STATUS_DISABLED);
		}
		
		SelectObject(hdc, old);
		DeleteDC(hdc);
		ReleaseDC(wnd, dc);
		
		SetMenuItemInfo(hMenu, IDM_OTR_STATUS, FALSE, &minfo);

			minfo.fMask = MIIM_STATE|MIIM_BITMAP;
			minfo.fState = (level==TRUST_NOT_PRIVATE)?MF_ENABLED:MF_GRAYED;
			minfo.hbmpItem = bmpIconUnverified;
			SetMenuItemInfo(hMenu, IDM_OTR_START, FALSE, &minfo);

			minfo.fState = (level==TRUST_NOT_PRIVATE)?MF_GRAYED:MF_ENABLED;
			minfo.hbmpItem = bmpIconNotSecure;
			SetMenuItemInfo(hMenu, IDM_OTR_STOP, FALSE, &minfo);

			minfo.fState = (level==TRUST_NOT_PRIVATE)?MF_GRAYED:MF_ENABLED;
			minfo.hbmpItem = bmpIconFinished;
			SetMenuItemInfo(hMenu, IDM_OTR_REFRESH, FALSE, &minfo);

			minfo.fState = (level==TRUST_NOT_PRIVATE||level==TRUST_FINISHED)?MF_GRAYED:MF_ENABLED;
			minfo.hbmpItem = bmpIconPrivate;
			SetMenuItemInfo(hMenu, IDM_OTR_VERIFY, FALSE, &minfo);

		switch ( (DWORD) TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_NONOTIFY|TPM_RETURNCMD, sicd->clickLocation.x, sicd->clickLocation.y, 0, GetFocus(), 0) )
		{
			case IDM_OTR_START:
				SVC_StartOTR(wParam, 0);
				break;
			case IDM_OTR_REFRESH:
				SVC_RefreshOTR(wParam, 0);
				break;
			case IDM_OTR_STOP:
				SVC_StopOTR(wParam, 0);
				break;
			case IDM_OTR_VERIFY:
				ConnContext *context = otrl_context_find_miranda(otr_user_state, (HANDLE)wParam);
				if (context) VerifyContextDialog(context);
				break;
		}
		DeleteObject(bmpIconPrivate);
		DeleteObject(bmpIconFinished);
		DeleteObject(bmpIconNotSecure);
		DeleteObject(bmpIconUnverified);
	}
	
	return 0;
}

// set SRMM icon status, if applicable
void SetEncryptionStatus(HANDLE hContact, TrustLevel level) {
	//char dbg_msg[2048];
	//dbg_msg[0] = 0;

	//strcat(dbg_msg, "Set encyption status: ");
	//strcat(dbg_msg, (encrypted ? "true" : "false"));

	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	bool chat_room = (proto && DBGetContactSettingByte(hContact, proto, "ChatRoom", 0));

	// if(!chat_room) DBWriteContactSettingByte(hContact, MODULENAME, "Encrypted", (encrypted ? 1 : 0));

	if(options.bHaveSRMMIcons) {
		//strcat(dbg_msg, "\nchanging icon");
		StatusIconData sid = {0}, sid2={0};
		sid.cbSize = sizeof(sid);
		sid.szModule = MODULENAME;
		sid.dwId = 0;
		sid.flags = MBF_HIDDEN;

		sid2.cbSize = sid.cbSize;
		sid2.szModule = MODULENAME;
		sid2.dwId = 1;
		sid2.flags = MBF_HIDDEN;
		if (!chat_room) {
			switch (level) {
				case TRUST_FINISHED:
					sid.flags = 0;
					break;
				case TRUST_UNVERIFIED:
					sid2.flags = MBF_DISABLED;
					break;
				case TRUST_PRIVATE:
					sid2.flags = 0;
					break;
				default:
					sid.flags = MBF_DISABLED;
					break;
			}
		}
		CallService(MS_MSG_MODIFYICON, (WPARAM)hContact, (LPARAM)&sid);
		CallService(MS_MSG_MODIFYICON, (WPARAM)hContact, (LPARAM)&sid2);
		db_dword_set(hContact, MODULENAME, "TrustLevel", level);

		if(!chat_room && options.bHaveMetaContacts) {
			HANDLE hMeta = (HANDLE)CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0);
			if(hMeta && hContact == (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hMeta, 0)) {
				//strcat(dbg_msg, "\nrecursing for meta");
				SetEncryptionStatus(hMeta, level);
			}
		}
	}
	//PUShowMessage(dbg_msg, SM_NOTIFY);
}

void InitSRMM() {
	// add icon to srmm status icons
	if(options.bHaveSRMMIcons) {
		hIconNotSecure = LoadIcon(ICON_NOT_PRIVATE, 0);
		hIconFinished = LoadIcon(ICON_FINISHED, 0);
		hIconPrivate = LoadIcon(ICON_PRIVATE, 0);
		hIconUnverified = LoadIcon(ICON_UNVERIFIED, 0);

		StatusIconData sid = {0};
		sid.cbSize = sizeof(sid);
		sid.szModule = MODULENAME;
		sid.dwId = 0;
		sid.hIcon = hIconFinished;
		sid.hIconDisabled = hIconNotSecure;
		sid.flags = MBF_DISABLED | MBF_HIDDEN;
		sid.szTooltip = Translate(LANG_OTR_TOOLTIP);
		CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);

		sid.dwId = 1;
		sid.hIcon = hIconPrivate;
		sid.hIconDisabled = hIconUnverified;
		CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);
		
		// hook the window events so that we can can change the status of the icon
		
		hEventIconPressed = HookEvent(ME_MSG_ICONPRESSED, SVC_IconPressed);
	}
}
void DeinitSRMM() {
	UnhookEvent(hEventIconPressed);
	ReleaseIcon(ICON_NOT_PRIVATE, 0);
	ReleaseIcon(ICON_FINISHED, 0);
	ReleaseIcon(ICON_PRIVATE, 0);
	ReleaseIcon(ICON_UNVERIFIED, 0);
	hIconNotSecure = hIconFinished = hIconPrivate = hIconUnverified =0;
}