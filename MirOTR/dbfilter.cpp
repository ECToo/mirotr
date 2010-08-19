#include "stdafx.h"
#include "dbfilter.h"
static HANDLE hDBEventPreAdd, hDBEventAdded;
static CRITICAL_SECTION RemoveChainCS={0}, *lpRemoveChainCS = &RemoveChainCS;
static UINT_PTR timerId = 0;

struct DeleteEventNode {
	DeleteEventNode		*next;
	time_t				 timestamp;
	HANDLE				 hContact;
	HANDLE				 hDbEvent;
};
struct DeleteEventHead {
	DeleteEventNode *first;
	DeleteEventNode *last;
};
static DeleteEventHead DeleteEvents = {0,0};

VOID CALLBACK DeleteTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	if (!DeleteEvents.first) return;
	EnterCriticalSection(lpRemoveChainCS);
	DeleteEventNode *prev =0, *current, *next;
	DBEVENTINFO info = {0};
	info.cbSize = sizeof(info);
	next = DeleteEvents.first;
	while (current = next) {
		if (difftime(time(0), current->timestamp) < 1) break;
		if(!CallService(MS_DB_EVENT_GET, (WPARAM)current->hDbEvent, (LPARAM)&info)) // && info.flags&DBEF_READ)
		{
			CallService(MS_DB_EVENT_DELETE, (WPARAM)current->hContact, (LPARAM)current->hDbEvent);
			next = current->next;
			if (prev) prev->next = next;
			else if (DeleteEvents.first == current) DeleteEvents.first = next;
			delete current;
		} else {
			prev = current;
			next = current->next;
		}
	}
	if (!DeleteEvents.first) DeleteEvents.last = 0;
	LeaveCriticalSection(lpRemoveChainCS);
}


// add prefix to sent messages
INT_PTR OnDatabaseEventPreAdd(WPARAM wParam, LPARAM lParam) {
	if(!options.prefix_messages || !lParam) return 0;
	HANDLE hContact = (HANDLE)wParam;
	DBEVENTINFO *dbei = (DBEVENTINFO *)lParam;
	if ((dbei->eventType != EVENTTYPE_MESSAGE) || !(dbei->flags & DBEF_SENT) || (dbei->flags & DBEF_OTR_PREFIXED))
		return 0;
	if(dbei->cbBlob == 0 || dbei->pBlob == 0)
		return 0; // just to be safe

	const char *proto = contact_get_proto(hContact);
	if(!proto )	return 0;
	if (DBGetContactSettingByte(hContact, proto, "ChatRoom", 0) == 1)
		return 0;
	
	if(g_metaproto && strcmp(proto, g_metaproto) == 0) {
		hContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0);
		if(!hContact) return 0;
		proto = contact_get_proto(hContact);
		if(!proto )	return 0;
	}
	

	ConnContext *context = otrl_context_find_miranda(otr_user_state, hContact);	
	bool encrypted = otr_context_get_trust(context) != TRUST_NOT_PRIVATE;
	if(!encrypted) return 0;
	
	DBEVENTINFO my_dbei = *dbei; // copy the other event

	char *msg = (char *)dbei->pBlob;
	char *newmsg = 0;
	DWORD alloclen = 0;
	DWORD msglen = strlen(msg);
	if (dbei->flags & DBEF_UTF) {
		int prefixlen = strnlen(options.prefix, 64);
		if (strncmp(msg, options.prefix, prefixlen) == 0) return 0;
		alloclen = (msglen+prefixlen+1)*sizeof(char);

		// get additional data
		DWORD len2 = alloclen- prefixlen*sizeof(char);
		int datalen = dbei->cbBlob - len2;
		if (datalen < 0) datalen = 0;

		newmsg = (char*)mir_alloc(alloclen);
		memset(newmsg, 0, alloclen+datalen);
		strncpy(newmsg, options.prefix, prefixlen);
		strncat(newmsg, msg, msglen);
		// append additional data
		if (datalen) {
				memcpy(newmsg+alloclen, msg+len2, datalen);
				alloclen += datalen;
		}
	} else {
		bool dz = false;
		if(msglen < dbei->cbBlob) {
			for(unsigned int i = msglen; i < dbei->cbBlob; i++) {
				if(msg[i] == 0 && msg[i - 1] == 0) {
					dz = true;
					break;
				}
			}
		}
		if(dz) {
			// yes, unicode
			wchar_t* msgw = (wchar_t *)&msg[msglen];
			wchar_t *prefixw = mir_utf8decodeW(options.prefix);
			int prefixlenw = wcslen(prefixw);
			if (wcsncmp(msgw, prefixw, prefixlenw) == 0) {
				mir_free(prefixw);
				return 0;
			}
			int msglenw = wcslen(msgw);

			char *prefix = mir_utf8decodeA(options.prefix);
			int prefixlen = strlen(prefix);

			alloclen = (msglen+prefixlen+1)* sizeof(char) + (msglenw + prefixlenw +1) * sizeof(wchar_t);
			// get additional data
			DWORD len2 = (msglen+1)* sizeof(char) + (msglenw+1) * sizeof(wchar_t);
			int datalen = dbei->cbBlob - len2;
			if (datalen < 0) datalen = 0;
			
			newmsg = (char*)mir_alloc(alloclen+datalen);
			memset(newmsg, 0, alloclen+datalen);
			strncpy(newmsg, prefix, prefixlen);
			strncat(newmsg, msg, msglen);
			wchar_t *p = (wchar_t*) newmsg + (msglen + prefixlen + 1) * sizeof(char);
			wcsncpy(p, prefixw, prefixlenw);
			wcsncat(p, msgw, msglenw);
			mir_free(prefix);
			mir_free(prefixw);
			// append additional data
			if (datalen) {
				memcpy(newmsg+alloclen, msg+len2, datalen);
				alloclen += datalen;
			}

		} else {
			char *prefix = mir_utf8decodeA(options.prefix);
			int prefixlen = strlen(prefix);
			if (strncmp(msg, prefix, prefixlen) == 0) {
				mir_free(prefix);
				return 0;
			}
			alloclen = msglen+prefixlen+sizeof(char);
			// get additional data
			DWORD len2 = alloclen-prefixlen;
			int datalen = dbei->cbBlob - len2;
			if (datalen < 0) datalen = 0;

			newmsg = (char*)mir_alloc(alloclen+datalen);
			memset(newmsg, 0, alloclen+datalen);
			strncpy(newmsg, prefix, prefixlen);
			strncat(newmsg, msg, msglen);
			mir_free(prefix);
			// append additional data
			if (datalen) {
				memcpy(newmsg+alloclen, msg+len2, datalen);
				alloclen += datalen;
			}
		}

	}
	
	my_dbei.pBlob = (BYTE *)newmsg;
	my_dbei.cbBlob = alloclen;
	my_dbei.flags |= DBEF_OTR_PREFIXED;

	CallService(MS_DB_EVENT_ADD, wParam, (LPARAM)&my_dbei);
	if (newmsg) mir_free(newmsg);
	
	// stop original event from being added
	return 1;
}

INT_PTR OnDatabaseEventAdded(WPARAM wParam, LPARAM lParam) {
	if(!options.no_history) return 0;

	DBEVENTINFO info = {0};
	info.cbSize = sizeof(info);

	static char* prefixutf = mir_utf8encodeT(TranslateT(LANG_INLINE_PREFIX));
	static char* prefix = Translate(LANG_INLINE_PREFIX);
	static DWORD lenutf = strlen(prefixutf);
	static DWORD len = strlen(prefix);
	info.cbBlob = lenutf*2;
	info.pBlob = (PBYTE)mir_alloc(info.cbBlob);
	if(!CallService(MS_DB_EVENT_GET, (WPARAM)lParam, (LPARAM)&info)) {
		if(info.eventType == EVENTTYPE_MESSAGE) {
			HANDLE hContact = (HANDLE)wParam;
			ConnContext *context = otrl_context_find_miranda(otr_user_state, hContact);
			if (context && otr_context_get_trust(context) != TRUST_NOT_PRIVATE 
				&& ( (info.flags&DBEF_UTF && !(info.cbBlob >lenutf && 0==strncmp((char*)info.pBlob, prefixutf, lenutf)))
					|| (!(info.flags&DBEF_UTF) && !(info.cbBlob >len && 0==strncmp((char*)info.pBlob, prefix, len)))
					)
				) // only delete encrypted messages that are no OTR system messages
			{
				DeleteEventNode *node = new DeleteEventNode();
				node->hContact = hContact;
				node->hDbEvent = (HANDLE) lParam;
				node->timestamp = time(0);
				node->next = 0;
				EnterCriticalSection(lpRemoveChainCS);
				if (DeleteEvents.last)
					DeleteEvents.last->next = node;
				else 
					DeleteEvents.first = node;
				DeleteEvents.last = node;
				LeaveCriticalSection(lpRemoveChainCS);
			}
		}
	}
	mir_free(prefixutf);
	mir_free(info.pBlob);
	return 0;
}

void InitDBFilter() {
	InitializeCriticalSectionAndSpinCount(lpRemoveChainCS, 500);
	hDBEventPreAdd = HookEvent(ME_DB_EVENT_FILTER_ADD, OnDatabaseEventPreAdd);
	hDBEventAdded = HookEvent(ME_DB_EVENT_ADDED, OnDatabaseEventAdded);
	timerId = SetTimer(0, 0, 1000, DeleteTimerProc);
}
void DeinitDBFilter() {
	UnhookEvent(hDBEventPreAdd);
	hDBEventPreAdd = 0;
	UnhookEvent(hDBEventAdded);
	hDBEventAdded = 0;
	if (timerId) KillTimer(0, timerId);
	DeleteTimerProc(0,0,0,0);
	DeleteCriticalSection(lpRemoveChainCS);
}