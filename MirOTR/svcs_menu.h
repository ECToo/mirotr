#pragma once
int SVC_StartOTR(WPARAM wParam, LPARAM lParam);
int SVC_RefreshOTR(WPARAM wParam, LPARAM lParam);
int SVC_StopOTR(WPARAM wParam, LPARAM lParam);
int SVC_VerifyOTR(WPARAM wParam, LPARAM lParam);
int SVC_PrebuildContactMenu(WPARAM wParam, LPARAM lParam);
void InitMenu();
void DeinitMenu();