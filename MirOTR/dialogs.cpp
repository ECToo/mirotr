#include "stdafx.h"
#include "dialogs.h"
#include <Prsht.h>
#include <commctrl.h>
#include <process.h>


INT_PTR CALLBACK DlgProcSMPInitProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg ) {
	case WM_INITDIALOG: 
		{
			if (!lParam) {
				EndDialog(hwndDlg, IDCANCEL);
				return FALSE;
			}
			TranslateDialogDefault( hwndDlg );

			ConnContext *context = (ConnContext*)lParam;
			TCHAR title[512];
			mir_sntprintf(title, 512, TranslateT(LANG_SMP_VERIFY_TITLE), contact_get_nameT((HANDLE)context->app_data));
			SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)title);
			SetDlgItemText(hwndDlg, IDC_STC_SMP_HEAD, title);
			SetWindowLongPtr(hwndDlg, GWL_USERDATA, lParam);

			// Move window to screen center
			// Get the owner window and dialog box rectangles. 
			HWND hwndOwner; RECT rcOwner, rcDlg, rc;
			if ((hwndOwner = GetParent(hwndDlg)) == NULL) 
			{
				hwndOwner = GetDesktopWindow(); 
			}

			GetWindowRect(hwndOwner, &rcOwner); 
			GetWindowRect(hwndDlg, &rcDlg); 
			CopyRect(&rc, &rcOwner); 

			// Offset the owner and dialog box rectangles so that right and bottom 
			// values represent the width and height, and then offset the owner again 
			// to discard space taken up by the dialog box. 

			OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top); 
			OffsetRect(&rc, -rc.left, -rc.top); 
			OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom); 

			// The new position is the sum of half the remaining space and the owner's 
			// original position. 

			SetWindowPos(hwndDlg, 
				HWND_TOP, 
				rcOwner.left + (rc.right / 2), 
				rcOwner.top + (rc.bottom / 2), 
				0, 0,          // Ignores size arguments. 
				SWP_NOSIZE); 

			// end center dialog


			HWND cmb = GetDlgItem(hwndDlg, IDC_CBO_SMP_CHOOSE);
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateT(LANG_SMPTYPE_QUESTION));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateT(LANG_SMPTYPE_PASSWORD));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateT(LANG_SMPTYPE_FINGERPRINT));
			SendMessage(cmb, CB_SELECTSTRING, -1, (WPARAM)TranslateT(LANG_SMPTYPE_QUESTION));
			EnableWindow(GetDlgItem(hwndDlg, IDC_CBO_SMP_CHOOSE), TRUE);

			
			Fingerprint *fp = context->active_fingerprint;
			if (!fp) {
				EndDialog(hwndDlg, IDCANCEL);
				return FALSE;
			}
			TCHAR buff[1024];
			if (!fp->trust || fp->trust[0] == '\0')
				mir_sntprintf(buff, 512, TranslateT(LANG_OTR_SMPQUESTION_VERIFY_DESC), contact_get_nameT((HANDLE)context->app_data));
			else
				mir_sntprintf(buff, 512, TranslateT(LANG_OTR_SMPQUESTION_VERIFIED_DESC), contact_get_nameT((HANDLE)context->app_data));

			SetDlgItemText(hwndDlg, IDC_STC_SMP_INFO, buff);

			SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD1, _T(""));
			SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD1, EM_SETREADONLY, FALSE, 0);
			SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD1, TranslateT(LANG_SMP_QUESTION));
			
			SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD2, _T(""));
			SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD2, EM_SETREADONLY, FALSE, 0);
			SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD2, TranslateT(LANG_SMP_ANSWER));

			
			ShowWindow(GetDlgItem(hwndDlg, IDOK), SW_SHOWNA);
			ShowWindow(GetDlgItem(hwndDlg, IDYES), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDNO), SW_HIDE);
			SetFocus(GetDlgItem(hwndDlg, IDC_CBO_SMP_CHOOSE));
			
			return FALSE;
		}

	case WM_COMMAND: 
		switch ( HIWORD( wParam )) {
			case BN_CLICKED: 
				switch ( LOWORD( wParam )) {	
					case IDYES:
					case IDNO:
					case IDCANCEL:
					case IDOK:
						EndDialog(hwndDlg, LOWORD( wParam ));
						break;
				}
			case CBN_SELCHANGE:
				switch ( LOWORD( wParam )) {	
					case IDC_CBO_SMP_CHOOSE:
						{ 
							ConnContext* context = (ConnContext*)GetWindowLongPtr(hwndDlg, GWL_USERDATA);
							Fingerprint *fp = context->active_fingerprint;
							if (!fp) {
								EndDialog(hwndDlg, IDCANCEL);
								return TRUE;
							}
							BOOL trusted = false;
							if (fp->trust && fp->trust[0] != '\0') trusted = true;

							TCHAR buff[512];
							GetDlgItemText(hwndDlg, IDC_CBO_SMP_CHOOSE, buff, 255);
							if (_tcsncmp(buff, TranslateT(LANG_SMPTYPE_QUESTION), 255)) {
								if (trusted)
									mir_sntprintf(buff, 512, TranslateT(LANG_OTR_SMPQUESTION_VERIFY_DESC), contact_get_nameT((HANDLE)context->app_data));
								else
									mir_sntprintf(buff, 512, TranslateT(LANG_OTR_SMPQUESTION_VERIFIED_DESC), contact_get_nameT((HANDLE)context->app_data));

								SetDlgItemText(hwndDlg, IDC_STC_SMP_INFO, buff);

								SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD1, _T(""));
								SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD1, EM_SETREADONLY, FALSE, 0);
								SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD1, TranslateT(LANG_SMP_QUESTION));

								SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD2, _T(""));
								SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD2, EM_SETREADONLY, FALSE, 0);
								SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD2, TranslateT(LANG_SMP_ANSWER));


								ShowWindow(GetDlgItem(hwndDlg, IDOK), SW_SHOWNA);
								ShowWindow(GetDlgItem(hwndDlg, IDYES), SW_HIDE);
								ShowWindow(GetDlgItem(hwndDlg, IDNO), SW_HIDE);
							} else if (_tcsncmp(buff, TranslateT(LANG_SMPTYPE_PASSWORD), 255)) {
								if (trusted)
									mir_sntprintf(buff, 512, TranslateT(LANG_OTR_SMPPASSWORD_VERIFY_DESC), contact_get_nameT((HANDLE)context->app_data));
								else
									mir_sntprintf(buff, 512, TranslateT(LANG_OTR_SMPPASSWORD_VERIFIED_DESC), contact_get_nameT((HANDLE)context->app_data));

								SetDlgItemText(hwndDlg, IDC_STC_SMP_INFO, buff);

								SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD1, _T(""));
								SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD1, EM_SETREADONLY, FALSE, 0);
								SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD1, TranslateT(LANG_SMP_PASSWORD));

								SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD2, _T(""));
								SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD2, EM_SETREADONLY, TRUE, 0);
								SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD2, _T(""));


								ShowWindow(GetDlgItem(hwndDlg, IDOK), SW_SHOWNA);
								ShowWindow(GetDlgItem(hwndDlg, IDYES), SW_HIDE);
								ShowWindow(GetDlgItem(hwndDlg, IDNO), SW_HIDE);
							} else if (_tcsncmp(buff, TranslateT(LANG_SMPTYPE_FINGERPRINT), 255)) {
								if (trusted)
									mir_sntprintf(buff, 512, TranslateT(LANG_OTR_FPVERIFY_DESC), contact_get_nameT((HANDLE)context->app_data));
								else
									mir_sntprintf(buff, 512, TranslateT(LANG_OTR_FPVERIFIED_DESC), contact_get_nameT((HANDLE)context->app_data));

								SetDlgItemText(hwndDlg, IDC_STC_SMP_INFO, buff);

								unsigned char hash[20];
								lib_cs_lock();
								if (!otrl_privkey_fingerprint_raw(otr_user_state, hash, context->accountname, context->protocol)) {
									lib_cs_unlock();
									EndDialog(hwndDlg, IDCANCEL);
									return FALSE;
								}
								otrl_privkey_hash_to_humanT(buff, hash);
								lib_cs_unlock();
								SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD1, buff);
								SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD1, EM_SETREADONLY, TRUE, 0);
								SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD1, TranslateT(LANG_YOUR_PRIVKEY));

								otrl_privkey_hash_to_humanT(buff, fp->fingerprint);
								SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD2, buff);
								SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD2, EM_SETREADONLY, TRUE, 0);
								SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD2, TranslateT(LANG_CONTACT_FINGERPRINT));

								EnableWindow(GetDlgItem(hwndDlg, IDC_CBO_SMP_CHOOSE), FALSE);

								ShowWindow(GetDlgItem(hwndDlg, IDOK), SW_HIDE);
								ShowWindow(GetDlgItem(hwndDlg, IDYES), SW_SHOWNA);
								ShowWindow(GetDlgItem(hwndDlg, IDNO), SW_SHOWNA);

							}
						}break;
				}
		}
		break;

	}

	return FALSE;
}
void SMPInitDialog(ConnContext* context) {
	if (!context) return;
	CreateDialogParamW(hInst, MAKEINTRESOURCE(IDD_SMP_INPUT), 0, DlgProcSMPInitProc, (LPARAM) context);
}

unsigned int CALLBACK verify_context_thread(void *param);
void VerifyContextDialog(ConnContext* context) {
	if (!context) return;
	CloseHandle((HANDLE)_beginthreadex(0, 0, verify_context_thread, context, 0, 0));
}

INT_PTR CALLBACK DlgProcVerifyContext(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg ) {
	case WM_INITDIALOG: 
		{
			if (!lParam) {
				EndDialog(hwndDlg, IDCANCEL);
				return FALSE;
			}
			SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)_T(LANG_OTR_FPVERIFY_TITLE));
			SetDlgItemText(hwndDlg, IDC_STC_SMP_HEAD, _T(LANG_OTR_FPVERIFY_TITLE));
			TranslateDialogDefault( hwndDlg );
			SetWindowLongPtr(hwndDlg, GWL_USERDATA, lParam);

			// Move window to screen center
			// Get the owner window and dialog box rectangles. 
			HWND hwndOwner; RECT rcOwner, rcDlg, rc;
			if ((hwndOwner = GetParent(hwndDlg)) == NULL) 
			{
				hwndOwner = GetDesktopWindow(); 
			}

			GetWindowRect(hwndOwner, &rcOwner); 
			GetWindowRect(hwndDlg, &rcDlg); 
			CopyRect(&rc, &rcOwner); 

			// Offset the owner and dialog box rectangles so that right and bottom 
			// values represent the width and height, and then offset the owner again 
			// to discard space taken up by the dialog box. 

			OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top); 
			OffsetRect(&rc, -rc.left, -rc.top); 
			OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom); 

			// The new position is the sum of half the remaining space and the owner's 
			// original position. 

			SetWindowPos(hwndDlg, 
				HWND_TOP, 
				rcOwner.left + (rc.right / 2), 
				rcOwner.top + (rc.bottom / 2), 
				0, 0,          // Ignores size arguments. 
				SWP_NOSIZE); 

			// end center dialog

			ConnContext *context = (ConnContext*)lParam;
			Fingerprint *fp = context->active_fingerprint;
			if (!fp) {
				EndDialog(hwndDlg, IDCANCEL);
				return FALSE;
			}
			TCHAR buff[512];
			if (!fp->trust || fp->trust[0] == '\0')
				mir_sntprintf(buff, 512, TranslateT(LANG_OTR_FPVERIFY_DESC), contact_get_nameT((HANDLE)context->app_data));
			else
				mir_sntprintf(buff, 512, TranslateT(LANG_OTR_FPVERIFIED_DESC), contact_get_nameT((HANDLE)context->app_data));

			SetDlgItemText(hwndDlg, IDC_STC_SMP_INFO, buff);

			unsigned char hash[20];
			lib_cs_lock();
			if (!otrl_privkey_fingerprint_raw(otr_user_state, hash, context->accountname, context->protocol)) {
				lib_cs_unlock();
				EndDialog(hwndDlg, IDCANCEL);
				return FALSE;
			}
			otrl_privkey_hash_to_humanT(buff, hash);
			lib_cs_unlock();
			SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD1, buff);
			SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD1, EM_SETREADONLY, TRUE, 0);
			SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD1, TranslateT(LANG_YOUR_PRIVKEY));
			
			otrl_privkey_hash_to_humanT(buff, fp->fingerprint);
			SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD2, buff);
			SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD2, EM_SETREADONLY, TRUE, 0);
			SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD2, TranslateT(LANG_CONTACT_FINGERPRINT));

			EnableWindow(GetDlgItem(hwndDlg, IDC_CBO_SMP_CHOOSE), FALSE);

			ShowWindow(GetDlgItem(hwndDlg, IDOK), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDYES), SW_SHOWNA);
			ShowWindow(GetDlgItem(hwndDlg, IDNO), SW_SHOWNA);
			SetFocus(GetDlgItem(hwndDlg, IDCANCEL));
			
			return FALSE;
		}

	case WM_COMMAND: 
		switch ( HIWORD( wParam )) {
			case BN_CLICKED: 
				switch ( LOWORD( wParam )) {	
					case IDYES:
					case IDNO:
					case IDCANCEL:
					case IDOK:
						EndDialog(hwndDlg, LOWORD( wParam ));
						break;
				}
		}
		break;

	}

	return FALSE;
}

unsigned int CALLBACK verify_context_thread(void *param) {
	CallService(MS_SYSTEM_THREAD_PUSH, 0, 0);

	if (param) {
		ConnContext *context = (ConnContext *)param;
		TCHAR msg[1024];
		switch ( DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SMP_INPUT), 0, DlgProcVerifyContext, (LPARAM)param) ) {
			case IDOK:
			case IDYES:
				lib_cs_lock();
				otrl_context_set_trust(context->active_fingerprint, "verified");
				otrl_privkey_write_fingerprints(otr_user_state, g_fingerprint_store_filename);
				lib_cs_unlock();
				mir_sntprintf(msg, 1024, TranslateT(LANG_FINGERPRINT_VERIFIED), contact_get_nameT((HANDLE)context->app_data));
				msg[1023] = '\0';
				ShowMessage((HANDLE)context->app_data, msg);
				SetEncryptionStatus(context->app_data, otr_context_get_trust(context));
				break;
			case IDNO:
				lib_cs_lock();
				otrl_context_set_trust(context->active_fingerprint, NULL);
				otrl_privkey_write_fingerprints(otr_user_state, g_fingerprint_store_filename);
				lib_cs_unlock();
				mir_sntprintf(msg, 1024, TranslateT(LANG_FINGERPRINT_NOT_VERIFIED), contact_get_nameT((HANDLE)context->app_data));
				msg[1023] = '\0';
				ShowMessage((HANDLE)context->app_data, msg);
				SetEncryptionStatus(context->app_data, otr_context_get_trust(context));
				break;
		}
	}

	CallService(MS_SYSTEM_THREAD_POP, 0, 0);
	return 0; 
}