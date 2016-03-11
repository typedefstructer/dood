#include<windows.h>
#include<shellapi.h>
#include<tchar.h>

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"linked_list.cpp"

#define global static

#define WM_TRAYSHOW WM_USER + 1
#define APP_EXIT 101
#define APP_STARTUP 102
#define APP_REFRESH 103

global bool app_running;
global char dir1_path[512];
global char dir2_path[512];
global linked_list dir1_list = {};
global linked_list dir2_list = {};
global node *safe;
global node *unsafe;

global bool startup_state;

void setwallpaper(char *path)
{
	SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, path,
						 SPIF_UPDATEINIFILE | SPIF_SENDCHANGE | SPIF_SENDWININICHANGE);
}

void showtrayicon(HWND window)
{
	NOTIFYICONDATA nid = {};
	nid.cbSize = sizeof(nid);
	nid.hWnd = window;
	nid.uID = 101;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_INFO | NIF_TIP;
	nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(1));
	nid.uCallbackMessage = WM_TRAYSHOW;

	_tcscpy(nid.szTip, _T("dood | nlife"));
	_tcscpy(nid.szInfo, _T("dood - the wallpaper master"));

	nid.dwInfoFlags = NIIF_INFO;
	Shell_NotifyIcon(NIM_ADD, &nid);
}

void hidetrayicon(HWND window)
{
	NOTIFYICONDATA nid = {};
	nid.cbSize = sizeof(nid);
	nid.hWnd = window;
	nid.uID = 101;
	Shell_NotifyIcon(NIM_DELETE, &nid);
}

void showpopupontray(HWND window)
{
	HMENU menu;
	POINT cursor;

	GetCursorPos(&cursor);
	menu = CreatePopupMenu();

	DWORD is_set = MF_CHECKED;

	if(startup_state) is_set = MF_CHECKED;
	else is_set = MF_UNCHECKED;

	AppendMenu(menu, MF_STRING | is_set, APP_STARTUP, "&Auto Start");
	AppendMenu(menu, MF_STRING, APP_REFRESH, "&Refresh");
	AppendMenu(menu, MF_SEPARATOR, 0, 0);
	AppendMenu(menu, MF_STRING, APP_EXIT, "&Exit");

	SetForegroundWindow(window);
	TrackPopupMenu(menu, TPM_LEFTALIGN, cursor.x, cursor.y, 0, window, 0);
	DestroyMenu(menu);
}

void get_exe_directory(char *buffer, int size, int flag=0)
{
	HMODULE app = GetModuleHandle(0);
	GetModuleFileName(app, buffer, size);

	if(flag==0)
	{
		char *rm_slash = strrchr(buffer, '\\');
		*rm_slash = 0;
	}
}

void getappdirs(char *res, char *root, char *dir)
{
	strcpy(res, root);
	strcat(res, dir);
}

void getfiles(linked_list *list, char *path)
{
	HANDLE find;
	WIN32_FIND_DATA fd;

	char search_path[600];
	strcpy(search_path, path);
	strcat(search_path, "\\*");

	find = FindFirstFile(search_path, &fd);
	char file_path[512];

	do
	{
		bool is_dir = fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		if(is_dir || fd.cFileName[0] == '.') continue;

		strcpy(file_path, path);
		strcat(file_path, "\\");
		strcat(file_path, fd.cFileName);

		list_insert_end(list, file_path, sizeof(file_path));
	} while(FindNextFile(find, &fd));
}

void refresh()
{
	node *n = dir1_list;

	while(n)
	{
		node *tmp = n;
		n=n->next;
		list_delete(&dir1_list, tmp);
	}

	n = dir2_list;
	while(n)
	{
		node *tmp = n;
		n=n->next;
		list_delete(&dir2_list, tmp);
	}

	getfiles(&dir1_list, dir1_path);
	getfiles(&dir2_list, dir2_path);

	safe = dir1_list;
	unsafe = dir2_list;
}

void enablestartup()
{
	char exe[512];
	get_exe_directory(exe, sizeof(exe), 1);
	HKEY key = 0;

	RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &key);
	RegSetValueEx(key, "dood_nlife", 0, REG_SZ, (BYTE*)exe, 2*(strlen(exe)+1));
	RegCloseKey(key);
}

void disablestartup()
{
	HKEY key = 0;
	RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &key);
	RegDeleteValue(key, "dood_nlife");
	RegCloseKey(key);
}

bool startupstatus()
{
	HKEY key = 0;
	RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_QUERY_VALUE, &key);

	DWORD size = 513*2;

	char exe[512];
	char actual_path[512];
	LONG err = RegGetValue(key, 0, "dood_nlife", RRF_RT_ANY, 0, exe, &size);

	get_exe_directory(actual_path, sizeof(actual_path), 1);

	bool same_path = (strcmp(actual_path, exe) == 0);

	if(err != ERROR_SUCCESS || (!same_path))
	{
		return false;
	}
	return true;
}

LRESULT CALLBACK
windowprocedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;
	switch(message)
	{
		case WM_CREATE:
		{
			showtrayicon(window);
		} break;

		case WM_TRAYSHOW:
	    {
			switch(lparam)
			{
				case WM_RBUTTONUP:
				{
					showpopupontray(window);
				}
			}
		} break;

		case WM_COMMAND:
		{
			switch(LOWORD(wparam))
			{
				case APP_EXIT:
				{
					app_running = false;
					PostQuitMessage(0);
					hidetrayicon(window);
				} break;

				case APP_STARTUP:
				{
					if(!startup_state)
					{
						enablestartup();
					}
					else
					{
						disablestartup();
					}

					startup_state = !startup_state;
				} break;

				case APP_REFRESH:
				{
					refresh();
				} break;
			}
		} break;

		default:
		{
			result = DefWindowProc(window, message, wparam, lparam);
		} break;
	}
	return result;
}


int CALLBACK
WinMain(HINSTANCE instance,
		HINSTANCE previnstance,
		LPSTR cmdline,
		int cmdshow)
{
	char exe_directory[512];
	char *dir1 = "\\safe";
	char *dir2 = "\\unsafe";
	char *path = 0;

	startup_state = startupstatus();

	get_exe_directory(exe_directory, 512);
	getappdirs(dir1_path, exe_directory, dir1);
	getappdirs(dir2_path, exe_directory, dir2);
	refresh();

	WNDCLASSEX window_class = {};

	window_class.cbSize = sizeof(window_class);
	window_class.lpfnWndProc = windowprocedure;
	window_class.hInstance = instance;
	window_class.hCursor = LoadCursor(0, IDC_ARROW);
	window_class.lpszClassName = "nlife_dood";

	RegisterClassEx(&window_class);

	HWND window = CreateWindowEx(0, window_class.lpszClassName,
								 "nlife_dood", WS_POPUP,
								 CW_USEDEFAULT, CW_USEDEFAULT,
								 CW_USEDEFAULT, CW_USEDEFAULT,
								 0, 0, instance, 0);

	RegisterHotKey(window, 1, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'W');
	RegisterHotKey(window, 2, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'S');

	MSG message;
	app_running = true;
	while(app_running)
	{
		while(GetMessage(&message, 0, 0, 0) > 0)
		{
			switch(message.message)
			{
				case WM_HOTKEY:
			    {
					if(message.wParam == 1)
					{
						path = 0;
						if(unsafe)
						{
							path = (char *)unsafe->data;
							unsafe = unsafe->next;
						}
						else
						{
							unsafe = dir2_list;
							if(unsafe)
							{
								path = (char *)unsafe->data;
								unsafe = unsafe->next;
							}
						}
					}
					else if(message.wParam == 2)
					{
						path = 0;
						if(safe)
						{
							path = (char *)safe->data;
							safe = safe->next;
						}
						else
						{
							safe = dir1_list;
							if(safe)
							{
								path = (char *)safe->data;
								safe = safe->next;
							}
						}
					}

					if(path)
					{
						setwallpaper(path);
					}

				} break;

				default:
				{
					TranslateMessage(&message);
					DispatchMessage(&message);
				} break;
			}
		}
	}

	return 0;
}
