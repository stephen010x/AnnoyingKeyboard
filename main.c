#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>

#include "extra_keydefs.h"

#define __FORCE_INLINE__ __attribute__((always_inline))
#define ABS(n) ({typeof(n) x = (n); (x > 0) ? x : -x;})
#define SIGN(n) ({typeof(n) x = (n); (x == 0) ? 0 : ((x > 0) ? 1 : -1);})
#define LENGTHOF(n) (sizeof(n)/sizeof(*(n)))

HHOOK g_hHook = NULL;

const DWORD keyrows[][13] = {
	{VK_1,       VK_2, VK_3, VK_4, VK_5, VK_6, VK_7, VK_8, VK_9, VK_0,                  VK_OEM_MINUS, VK_OEM_PLUS, VK_BACK},
	{VK_TAB,     VK_Q, VK_W, VK_E, VK_R, VK_T, VK_Y, VK_U, VK_I, VK_O,                  VK_P,         VK_OEM_4,    VK_OEM_6},
	{VK_CAPITAL, VK_A, VK_S, VK_D, VK_F, VK_G, VK_H, VK_J, VK_K, VK_L,                  VK_OEM_1,     VK_OEM_7,    VK_RETURN},
	{VK_LSHIFT,  VK_Z, VK_X, VK_C, VK_V, VK_B, VK_N, VK_M, VK_OEM_COMMA, VK_OEM_PERIOD, VK_OEM_2,     VK_RSHIFT,   VK_RETURN},
};

const char filetext[] = 
	"-- Welcome to AnnoyingKeyboard.exe! --\n\n"
	"The AnnoyingKeyboard.exe enhances your\n"
	"typing experience by increasing your\n"
	"number of typos by over 240 percent!\n\n"
	"This program includes a random chance to:\n"
	"1) Randomly swap two letters!\n"
	"2) Replacing a letter with a nearby\n"
	"   one on the keyboard!\n"
	"3) Outright ignoring your keypress!\n\n"
	"I hope you enjoy!\n\n\n"
	"-- HOW TO TURN OFF ANNOYINGKEYBOARD.EXE --\n\n"
	"Step 1) Press CTRL + Q\n"
	"Step 2) Breathe a sigh of relief.\n\n\n"
	"-- HOW TO CHANGE THE RATE OF TYPOS --\n\n"
	"Keybinds:\n"
	"  CTRL + 0    (VERY SUBTLE)\n"
	"  CTRL + 1    (SUBTLE)\n"
	"  CTRL + 2    (FREQUENT)\n"
	"  CTRL + 3    (VERY FREQUENT)\n"
	"  CTRL + 4    (UNINTELLIGIBLE)\n\n\n"
	"AnnoyingKeyboard.exe was made by Stephen Harris\n\n"
    "   |\\      _,,,---,,_\n"
    "   /,`.-'`'    -.  ;-;;,_\n"
    "  |,4-  ) )-,_. ,\\ (  `'-'\n" /* ASCII Cat art by Felix Lee  */
    " '---''(_/--'  `-'\\_)\n";
	

// inclusive, probably
__FORCE_INLINE__ int randint(int min, int max) {
    return min + rand() % (max - min + 1);
}

void popstruct_keypress(INPUT* pinput, DWORD keycode) {
	pinput[0].type = INPUT_KEYBOARD;
	pinput[0].ki.wVk = keycode;
	pinput[1].type = INPUT_KEYBOARD;
	pinput[1].ki.wVk = keycode;
	pinput[1].ki.dwFlags = KEYEVENTF_KEYUP;
}

void simpleMB(char* text) {
	MessageBox(NULL, text, "Annoying Keyboard", MB_OK | MB_ICONINFORMATION);
}

char VkCodeToChar(DWORD vkCode) {
    BYTE keyboardState[256];
    WORD charCode[2];
    if (!GetKeyboardState(keyboardState))
        return 0;
    int result = ToAscii(vkCode, MapVirtualKey(vkCode, MAPVK_VK_TO_VSC), keyboardState, charCode, 0);
    if (result == 1) {
        return (char)charCode[0];
    } else {
        return ' ';
    }
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	/*if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *pKeyboard = (KBDLLHOOKSTRUCT *)lParam;
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
                if (pKeyboard->vkCode == 'Q') {
                    PostQuitMessage(0);
                }
            }
        }
    }*/
	static int trate = 39;
	if (nCode >= 0 && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
		KBDLLHOOKSTRUCT* pk = (KBDLLHOOKSTRUCT*)lParam;
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			switch (pk->vkCode) {
				case 'Q':
					PostQuitMessage(0);
					break;
				case '0':
					trate = 60;
					simpleMB("Typo rate set to \"VERY SUBTLE\"");
					break;
				case '1':
					trate = 40;
					simpleMB("Typo rate set to \"SUBTLE\"");
					break;
				case '2':
					trate = 20;
					simpleMB("Typo rate set to \"FREQUENT\"");
					break;
				case '3':
					trate = 10;
					simpleMB("Typo rate set to \"VERY FREQUENT\"");
					break;
				case '4':
					trate = 5;
					simpleMB("Typo rate set to \"UNINTELLIGIBLE\"");
					break;
		}
    }
	if (nCode >= 0 && wParam == WM_KEYDOWN) {
        PKBDLLHOOKSTRUCT pKey = (PKBDLLHOOKSTRUCT)lParam;
		DWORD keycode = pKey->vkCode;
		static DWORD keybuff;
		printf("Key %c pressed (%u)\n", VkCodeToChar(keycode), keycode);
		// second stage of swap
		if (keybuff != 0) {
			printf("TYPO: swap complete\n");
			INPUT input[4] = {0};
			popstruct_keypress(&input[0], keycode);
			popstruct_keypress(&input[2], keybuff);
			keybuff = 0;
			SendInput(4, input, sizeof(INPUT));
			return 1;
		}
		// chance typo generator
		/*int sbk = keycode == VK_RSHIFT || 
		          keycode == VK_LSHIFT;
				  //keycode == VK_RETURN || 
				  //keycode == VK_TAB;
				  //keycode == VK_SPACE;*/
		int sbk = keycode == VK_RSHIFT ||
				  keycode == VK_LSHIFT ||
				  keycode == VK_SPACE;
		int nrk = keycode == VK_SPACE ||
		          keycode == VK_RSHIFT ||
				  keycode == VK_LSHIFT ||
				  keycode == VK_RETURN;
		if (keycode == VK_BACK || keycode == VK_RETURN)
			goto end;
		int rint = randint(1, trate);
		if (rint == 1 || sbk && rint == 2 && randint(0, 1)) {
			switch (randint(2*nrk, 4)) {
			case 0: // replace
			case 1:
				printf("TYPO: replace\n");
				for (int j = 0; j < LENGTHOF(keyrows); j++) 
				for (int i = 0; i < LENGTHOF(keyrows[0]); i++) {
					if (keycode != keyrows[j][i]) continue;
					int k = i + randint(0, 1) * 2 - 1;
					if (k < 0)
						k += 2;
					if (k >= LENGTHOF(keyrows[0]))
						k -= 2;
					INPUT input[2] = {0};
					popstruct_keypress(&input[0], keyrows[j][k]);
					SendInput(2, input, sizeof(INPUT));
					return 1;
				}
				break;
			case 2: // swap
			case 3:
				printf("TYPO: swap start\n");
				keybuff = keycode;
				return 1;
			case 4: // miss
				printf("TYPO: miss\n");
				return 1;
			}
			return 1; // return 1 to block original key press
		}
    }
    end: return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

int errorbox(int code) {
	char errormsg[64];
	snprintf(errormsg, sizeof(errormsg), "Failed to start AnnoyingKeyboard.exe\nError Code %d", code);
	MessageBox(NULL, errormsg, "ERROR: Annoying Keyboard", MB_OK | MB_ICONINFORMATION);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MSG msg;
	srand(time(NULL));
	MessageBox(NULL, "Annoying Keyboard Started!", "Annoying Keyboard", MB_OK | MB_ICONINFORMATION);
	MessageBox(NULL, "Typo rate set to \"SUBTLE\"", "Annoying Keyboard", MB_OK | MB_ICONINFORMATION);
	FILE *file = fopen("README_AnnoyingKeyboard.txt", "w");
    if (file == NULL) {
        perror("Failed to write file");
		errorbox(1);
        return 1;
    }
	printf("\n\t--ASCII Cat art by Felix Lee--\n\n");
    fprintf(file, filetext);
    fclose(file);
	HINSTANCE hInstance2 = ShellExecute(NULL, "open", "notepad.exe", "./README_AnnoyingKeyboard.txt", NULL, SW_SHOWNORMAL);
    if ((int)hInstance2 <= 32) {
		perror("Failed to open file with notepad");
        errorbox(2);
        return 2;
    }
    g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(g_hHook);
	MessageBox(NULL, "Annoying Keyboard Stopped!\n\nI hope you had fun! :)", "Annoying Keyboard", MB_OK | MB_ICONINFORMATION);
    return 0;
}