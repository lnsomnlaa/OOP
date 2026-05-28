#include "Lab3Dialogs.h"
#include "resource.h"

#include <sstream>

struct TextDlgCtx {
    const char* title = nullptr;
    std::string* text = nullptr;
};

static INT_PTR CALLBACK TextDlgProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_INITDIALOG) {
        auto* ctx = reinterpret_cast<TextDlgCtx*>(lp);
        SetWindowLongPtrA(dlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ctx));
        if (ctx && ctx->title) {
            SetWindowTextA(dlg, ctx->title);
        }
        if (ctx && ctx->text) {
            SetDlgItemTextA(dlg, IDC_PARAMS_EDIT, ctx->text->c_str());
        }
        return TRUE;
    }
    if (msg == WM_COMMAND && LOWORD(wp) == IDOK) {
        auto* ctx = reinterpret_cast<TextDlgCtx*>(GetWindowLongPtrA(dlg, GWLP_USERDATA));
        if (ctx && ctx->text) {
            char buf[8192] = {0};
            GetDlgItemTextA(dlg, IDC_PARAMS_EDIT, buf, static_cast<int>(sizeof(buf) - 1));
            *ctx->text = buf;
        }
        EndDialog(dlg, IDOK);
        return TRUE;
    }
    if (msg == WM_COMMAND && LOWORD(wp) == IDCANCEL) {
        EndDialog(dlg, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

bool Lab3_EditTextDialog(HWND owner, const char* windowTitle, std::string& textInOut) {
    TextDlgCtx ctx;
    ctx.title = windowTitle;
    ctx.text = &textInOut;
    const INT_PTR r =
        DialogBoxParamA(GetModuleHandle(nullptr), MAKEINTRESOURCEA(IDD_PARAMS), owner, TextDlgProc,
                        reinterpret_cast<LPARAM>(&ctx));
    return r == IDOK;
}

bool Lab3_ParseInts(const std::string& text, std::vector<int>& out) {
    out.clear();
    std::istringstream iss(text);
    long v = 0;
    while (iss >> v) {
        out.push_back(static_cast<int>(v));
    }
    return !out.empty();
}
