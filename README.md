# Keylogger

**Author:** Byte Reaper  
**License:** MIT 

---

## Description

**Keylogger** (Free Version) is a lightweight, proof-of-concept Windows keylogger written in C, demonstrating low-level keyboard capture and basic anti-debugging techniques. It records all printable Unicode keystrokes to a UTF-8 log file and employs multiple checks to thwart casual analysis.

---

## Features

1. **Global Low-Level Hook**  
   - Uses `WH_KEYBOARD_LL` to intercept all key events.

2. **Unicode & Special-Key Support**  
   - Captures and logs every printable character (via `ToUnicode` + `WideCharToMultiByte`).  
   - Renders named placeholders for Backspace, Enter, Tab, Ctrl, Alt, Shift, CapsLock, Esc, Arrow keys, Delete, etc.  
   - Fallback lookup for unmapped VK codes using `GetKeyNameText`.

3. **UTF-8 Log Output**  
   - All output is written as UTF-8 to `keylogger.txt` in the working directory.

4. **Basic Anti-Debugging**  
   - API check: `CheckRemoteDebuggerPresent(GetCurrentProcess())`  
   - Native call: `NtQueryInformationProcess(ProcessDebugPort)`  
   - Manual PEB flag inspection (C + standalone ASM) reading `BeingDebugged` → `ExitProcess`

5. **Standalone ASM Module**  
   - Includes `pebDebug.asm` for an early PEB-based check before the main C code runs.

---

## Requirements

- Windows 7 or later (x64).  
- GCC (MinGW-w64) or MSVC toolchain.  
- NASM (for the ASM module).  
- No external libraries beyond standard WinAPI.

---

## Build & Installation

1. **Assemble the anti-debug module**  
   
 1 - nasm -f win64 pebDebug.asm -o peb.o 
 2 - gcc -c keylogger.c -o keylogger.o
 3 - gcc keylogger.o peb.o  -o FreeKeylogger
Verify

Ensure FreeKeylogger.exe  reside in the same folder.

Run from an elevated or normal console; it auto-creates/appends keylogger.txt.

Usage
Open a Command Prompt in the build directory.

Run:

.\FreeKeylogger.exe
Leave the console open; it will append keystrokes to keylogger.txt.

Stop logging by closing the console window or pressing Ctrl+C.

Disclaimer & Legal
For educational/research use only.

Do NOT deploy on systems without explicit permission.

Unauthorized use may violate local laws—use responsibly.

Further Reading
Windows Internals by Mark Russinovich et al.

MSDN: SetWindowsHookEx

MSDN: NtQueryInformationProcess

Byte Reaper © 2025