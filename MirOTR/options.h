#ifndef _OPTIONS_INC
#define _OPTIONS_INC

//#include "stdafx.h"

extern char* g_metaproto;

#define PRIVATE_KEY_FILENAME "otr.private_key"
#define FINGERPRINT_STORE_FILENAME "otr.fingerprints"

extern char g_fingerprint_store_filename[MAX_PATH];
extern char g_private_key_filename[MAX_PATH];

#define CONTACT_DEFAULT_POLICY 	0xFFFF

typedef enum {ED_POP, ED_BAL, ED_MB} ErrorDisplay;

#define OPTIONS_PREFIXLEN 64

typedef struct {
	OtrlPolicy default_policy;
	ErrorDisplay err_method;
	bool prefix_messages;
	bool msg_inline;
	bool msg_popup;
	char prefix[OPTIONS_PREFIXLEN];
	
	bool timeout_finished;

	bool no_history;
	bool autoshow_verify;
	
	bool end_offline, end_window_close;

	// temporary options
	bool bHaveMetaContacts, bHavePopups, bHaveSRMMIcons, bHaveSecureIM;

} Options;

extern Options options;

#define WMU_REFRESHLIST			(WM_USER + 241)
#define WMU_REFRESHPROTOLIST	(WM_USER + 242)
#define CONTACT_DEFAULT_POLICY 	0xFFFF

#include <map>
typedef std::map<HANDLE, OtrlPolicy> ContactPolicyMap;
typedef enum {FPM_VERIFY, FPM_NOTRUST, FPM_DELETE } FPModify;
typedef std::map<Fingerprint*, FPModify> FPModifyMap;

int OptInit(WPARAM wParam, LPARAM lParam);

void LoadOptions();
void SaveOptions();
extern "C" INT_PTR OpenOptions(WPARAM wParam, LPARAM lParam);
void InitOptions();
void DeinitOptions();

void ReadPrivkeyFiles();

static INT_PTR CALLBACK DlgProcMirOTROpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcMirOTROptsProto(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcMirOTROptsContacts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcMirOTROptsFinger(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
