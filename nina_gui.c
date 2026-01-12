#include <windows.h>
#include <commdlg.h>   // Datei-Dialoge
#include <shellapi.h>  // ShellExecute
#include <stdio.h>
#include <stdlib.h>

HWND hwndInput, hwndOutput;

// ------------------------
// Datei-Dialoge
// ------------------------
void BrowseInput(HWND hwnd) {
    OPENFILENAME ofn;
    char szFile[260] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Waldbrand Files\0*.w\0All Files\0*.*\0";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        SetWindowText(hwndInput, szFile);
    }
}

void BrowseOutput(HWND hwnd) {
    OPENFILENAME ofn;
    char szFile[260] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "C File\0*.c\0Executable\0*.exe\0All Files\0*.*\0";
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn)) {
        SetWindowText(hwndOutput, szFile);
    }
}

// ------------------------
// Compile mit ShellExecute (kein Terminal)
// ------------------------
void Compile(HWND hwnd) {
    char inputPath[260], outputPath[260];
    GetWindowText(hwndInput, inputPath, 260);
    GetWindowText(hwndOutput, outputPath, 260);

    if (strlen(inputPath) == 0 || strlen(outputPath) == 0) {
        MessageBox(hwnd, "Bitte Eingabe- und Ausgabedatei auswählen!", "Fehler", MB_OK | MB_ICONERROR);
        return;
    }

    // ShellExecute ruft nina.exe auf, SW_HIDE = kein Terminal
    char args[512];
    snprintf(args, sizeof(args), "\"%s\" -o \"%s\"", inputPath, outputPath);

    HINSTANCE result = ShellExecute(hwnd, "open", "nina.exe", args, NULL, SW_HIDE);

    if ((int)result <= 32) {
        MessageBox(hwnd, "Fehler beim Compilieren!", "Fehler", MB_OK | MB_ICONERROR);
    } else {
        MessageBox(hwnd, "Transpilation erfolgreich!", "Erfolg", MB_OK | MB_ICONINFORMATION);
        // GUI bleibt offen, keine Reset nötig
    }
}

// -----------------------------
// Win32 GUI
// -----------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            CreateWindow("STATIC", "Waldbrand Datei:", WS_VISIBLE | WS_CHILD, 10, 10, 100, 20, hwnd, NULL, NULL, NULL);
            hwndInput = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 10, 300, 20, hwnd, NULL, NULL, NULL);
            CreateWindow("BUTTON", "Durchsuchen...", WS_VISIBLE | WS_CHILD, 430, 10, 100, 20, hwnd, (HMENU)1, NULL, NULL);

            CreateWindow("STATIC", "Zielort:", WS_VISIBLE | WS_CHILD, 10, 50, 100, 20, hwnd, NULL, NULL, NULL);
            hwndOutput = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 50, 300, 20, hwnd, NULL, NULL, NULL);
            CreateWindow("BUTTON", "Durchsuchen...", WS_VISIBLE | WS_CHILD, 430, 50, 100, 20, hwnd, (HMENU)2, NULL, NULL);

            CreateWindow("BUTTON", "Compile", WS_VISIBLE | WS_CHILD, 200, 90, 150, 30, hwnd, (HMENU)3, NULL, NULL);
        } break;

        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case 1: BrowseInput(hwnd); break;
                case 2: BrowseOutput(hwnd); break;
                case 3: Compile(hwnd); break;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "NinaGUI";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindow("NinaGUI", "Nina", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
                             CW_USEDEFAULT, CW_USEDEFAULT, 550, 170,
                             NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
