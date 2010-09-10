#pragma once
void VerifyContextDialog(ConnContext* context);
void SMPInitDialog(ConnContext* context);
INT_PTR CALLBACK DlgProcVerifyContext(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void SMPDialogUpdate(ConnContext *context, int percent);
void SMPDialogReply(ConnContext *context, const char* question);