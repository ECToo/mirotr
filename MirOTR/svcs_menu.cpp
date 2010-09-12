#include "stdafx.h"
#include "otr.h"

HANDLE hMenuBuildEvent, hStopItem, hStartItem;

////////////////////////////////
///////// Menu Services
///////////////////////
int StartOTR(HANDLE hContact) {
	
	const char *proto = contact_get_proto(hContact);
	if(!proto) return 1; // error
	char *uname = contact_get_id(hContact);
	if(!uname) return 1; // error
	WORD pol = DBGetContactSettingWord(hContact, MODULENAME, "Policy", CONTACT_DEFAULT_POLICY);
	if(pol == CONTACT_DEFAULT_POLICY) pol = options.default_policy;
	
	lib_cs_lock();
    char *msg = otrl_proto_default_query_msg(MODULENAME, pol);
    otr_gui_inject_message(hContact, proto, proto, uname, msg ? msg : "?OTRv2?");
	lib_cs_unlock();
    otrl_message_free(msg);
	mir_free(uname);
	return 0;
}
int SVC_StartOTR(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam, hSub;
	if(options.bHaveMetaContacts && (hSub = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0)) != 0) {
		hContact = hSub;
	}

	if ( options.bHaveSecureIM && CallService("SecureIM/IsContactSecured", (WPARAM)hContact, 0) != 0 ) {
		TCHAR msg[512];
		mir_sntprintf(msg, 512, TranslateT(LANG_OTR_SECUREIM_STARTED), contact_get_nameT(hContact));
		ShowError(msg);
		return 0;
	}
	
	TCHAR buff[512];
	mir_sntprintf(buff, 512, TranslateT(LANG_SESSION_REQUEST_OTR), contact_get_nameT(hContact));
	ShowMessage(hContact, buff);

	int res = StartOTR(hContact);
	if (res) return res;

	return 0;
}
int SVC_RefreshOTR(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam, hSub;
	if(options.bHaveMetaContacts && (hSub = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0)) != 0) {
		hContact = hSub;
	}

	if ( options.bHaveSecureIM && CallService("SecureIM/IsContactSecured", (WPARAM)hContact, 0) != 0 ) {
		TCHAR msg[512];
		mir_sntprintf(msg, 512, TranslateT(LANG_OTR_SECUREIM_STARTED), contact_get_nameT(hContact));
		ShowError(msg);
		return 0;
	}
	
	TCHAR buff[512];
	mir_sntprintf(buff, 512, TranslateT(LANG_SESSION_TRY_CONTINUE_OTR), contact_get_nameT(hContact));
	ShowMessage(hContact, buff);

	int res = StartOTR(hContact);
	if (res) return res;

	return 0;
}
int otr_disconnect_contact(HANDLE hContact) {
	HANDLE hSub;
	if(ServiceExists(MS_MC_GETMOSTONLINECONTACT) && (hSub = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0)) != 0) {
		hContact = hSub;
	}
	
	const char *proto = contact_get_proto(hContact);
	if(!proto) return 1; // error
	char *uname = contact_get_id(hContact);
	if(!uname) return 1; // error

	lib_cs_lock();
	otrl_message_disconnect(otr_user_state, &ops, hContact, proto, proto, uname);
	lib_cs_unlock();
	mir_free(uname);
	return 0;
}

int SVC_StopOTR(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	
	// prevent this filter from acting on injeceted messages for metas, when they are passed though the subcontact's proto send chain
	if (otr_disconnect_contact(hContact)) return 0;

	SetEncryptionStatus(hContact, TRUST_NOT_PRIVATE);

	TCHAR buff[512];
	mir_sntprintf(buff, 512, TranslateT(LANG_SESSION_TERMINATED_OTR), contact_get_nameT(hContact));
	ShowMessage(hContact, buff);
	
	return 0;
}

int SVC_VerifyOTR(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	ConnContext *context = otrl_context_find_miranda(otr_user_state, (HANDLE)wParam);
	if (!context) return 1;
	//VerifyContextDialog(context);	
	SMPInitDialog(context);
	return 0;
}

void InitMenu() {
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.flags = CMIF_NOTOFFLINE | CMIF_TCHAR | CMIF_ICONFROMICOLIB;
	mi.position = -400000;

	mi.ptszName = _T(LANG_STOP_OTR);
	mi.pszService = MS_OTR_MENUSTOP;
	mi.icolibItem = GetIconHandle(ICON_PRIVATE);

	hStopItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	
	mi.ptszName = _T(LANG_START_OTR);
	mi.pszService = MS_OTR_MENUSTART;
	mi.icolibItem = GetIconHandle(ICON_NOT_PRIVATE);

	hStartItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	hMenuBuildEvent = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, SVC_PrebuildContactMenu);

}

void DeinitMenu() {
	UnhookEvent(hMenuBuildEvent);
}

int SVC_PrebuildContactMenu(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | CMIF_NOTOFFLINE | CMIF_TCHAR | CMIF_ICONFROMICOLIB;
	
	const char *proto = contact_get_proto(hContact);
	WORD pol = CONTACT_DEFAULT_POLICY;
	
	if(!proto || DBGetContactSettingByte(hContact, proto, "ChatRoom", 0) == 1) {
		goto hide_all;
	}
	
	if(proto && g_metaproto && strcmp(proto, g_metaproto) == 0) {
		// make menu act as per most online subcontact
		hContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0);
		if(!hContact)
			goto hide_all;
		proto = contact_get_proto(hContact);
	}

	pol = DBGetContactSettingWord(hContact, MODULENAME, "Policy", CONTACT_DEFAULT_POLICY);
	if(pol == CONTACT_DEFAULT_POLICY) pol = options.default_policy;
	
	if(pol == OTRL_POLICY_NEVER || pol == OTRL_POLICY_ALWAYS) {
		goto hide_all;
	}

	{
		ConnContext *context = otrl_context_find_miranda(otr_user_state, hContact);	
		TrustLevel encrypted = otr_context_get_trust(context);
		if(encrypted != TRUST_NOT_PRIVATE) {
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hStopItem, (LPARAM)&mi);
			mi.flags |= CMIF_HIDDEN;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hStartItem, (LPARAM)&mi);
		} else {
			//if(pol == OTRL_POLICY_MANUAL_MOD) {
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hStartItem, (LPARAM)&mi);
				mi.flags |= CMIF_HIDDEN;
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hStopItem, (LPARAM)&mi);
			//} else { // should only be 'opportunistic'
			//	goto hide_all;
			//}
		}	
	}
	
	return 0;

hide_all:
	mi.flags |= CMIF_HIDDEN;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hStopItem, (LPARAM)&mi);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hStartItem, (LPARAM)&mi);
	return 0;
}