#include "gui.h"
#include <windows.h>
#include <stdio.h>

static AppState *g_app_state = NULL;
static HWND g_hwnd_main = NULL;

/**
 * @brief Window Procedure (Tratador de Mensagens da Janela).
 * Gerencia a renderização e o encerramento do programa de forma básica.
 */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // 1. Preenche o fundo com um cinza claro neutro
            HBRUSH bgBrush = CreateSolidBrush(RGB(240, 242, 245));
            FillRect(hdc, &ps.rcPaint, bgBrush);
            DeleteObject(bgBrush);
            
            // 2. Desenha o texto de boas-vindas centralizado
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(60, 64, 75));
            
            RECT rect;
            GetClientRect(hwnd, &rect);
            
            // Texto demonstrando que este é o esqueleto pronto do projeto
            DrawTextA(hdc, 
                      "Projeto C - Interface Grafica Modular (Win32)\n\n"
                      "Este e o seu esqueleto basico limpo.\n"
                      "Edite o arquivo src/gui.c e include/gui.h para desenhar seus componentes.",
                      -1, &rect, 
                      DT_CENTER | DT_VCENTER | DT_WORDBREAK);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

bool gui_init(AppState *app_state) {
    if (app_state == NULL) return false;
    g_app_state = app_state;
    
    HINSTANCE hInstance = GetModuleHandle(NULL);
    
    // Registrando a classe de janela
    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "CModularGUIClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hbrBackground = NULL; // Evita cintilações redesenhando no WM_PAINT
    
    if (!RegisterClassExA(&wc)) {
        return false;
    }
    
    // Dimensões padrão 800x600 centralizado
    int largura = 800;
    int altura = 600;
    int x = (GetSystemMetrics(SM_CXSCREEN) - largura) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - altura) / 2;
    
    g_hwnd_main = CreateWindowExA(
        0,
        "CModularGUIClass",
        "Projeto C - Interface Grafica Modular",
        WS_OVERLAPPEDWINDOW,
        x, y, largura, altura,
        NULL, NULL,
        hInstance, NULL
    );
    
    if (g_hwnd_main == NULL) {
        return false;
    }
    
    ShowWindow(g_hwnd_main, SW_SHOW);
    UpdateWindow(g_hwnd_main);
    
    return true;
}

void gui_run(void) {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
