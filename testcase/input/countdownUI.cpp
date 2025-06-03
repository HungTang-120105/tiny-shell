#include <windows.h>
#include <string>
#include <thread>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"CountdownWindowClass";

    WNDCLASSW wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Countdown Timer",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    // Tạo font ban đầu
    HFONT hFont = CreateFontW(
        48, 0, 0, 0, FW_BOLD, 
        FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, 
        CLEARTYPE_QUALITY, VARIABLE_PITCH, 
        L"Arial"
    );

    HWND hStatic = CreateWindowW(
        L"STATIC", L"10",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        0, 50, 300, 60,
        hwnd, NULL, hInstance, NULL
    );
    SendMessageW(hStatic, WM_SETFONT, (WPARAM)hFont, TRUE);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Thêm mutable để có thể thay đổi hFont trong lambda
    std::thread countdownThread([hwnd, hStatic, hFont]() mutable {
        for (int i = 20; i >= 0; --i) {
            std::wstring text = std::to_wstring(i);
            SetWindowTextW(hStatic, text.c_str());

            if (i <= 3) {
                HFONT hRedFont = CreateFontW(
                    48, 0, 0, 0, FW_BOLD, 
                    FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                    OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, 
                    CLEARTYPE_QUALITY, VARIABLE_PITCH, 
                    L"Arial"
                );
                SendMessageW(hStatic, WM_SETFONT, (WPARAM)hRedFont, TRUE);
                DeleteObject(hFont); // Xóa font cũ
                hFont = hRedFont;    // Gán font mới (cần mutable)
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        PostMessageW(hwnd, WM_CLOSE, 0, 0);
    });

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    countdownThread.join();
    DeleteObject(hFont);
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}