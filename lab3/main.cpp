#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commdlg.h>

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "resource.h"
#include "ShapeBinaryRegistry.h"
#include "ShapeFactory.h"
#include "ShapeList.h"
#include "ShapeRenderer.h"

#include "LineShape.h"
#include "RectangleShape.h"
#include "EllipseShape.h"
#include "CircleShape.h"
#include "TriangleShape.h"
#include "PointShape.h"
#include "Cube3DShape.h"
#include "Sphere3DShape.h"
#include "Tetra3DShape.h"
#include "PolygonShape.h"

constexpr UINT IDM_EXIT            = 1001;
constexpr UINT IDM_CLEAR           = 1002;
constexpr UINT IDM_SAVE_BINARY     = 1003;
constexpr UINT IDM_LOAD_BINARY     = 1004;
constexpr UINT IDM_INPUT_MOUSE     = 1010;
constexpr UINT IDM_INPUT_DIALOG    = 1011;
constexpr UINT IDM_ADD_DIALOG      = 1020;
constexpr UINT IDM_DELETE_SELECTED = 1021;
constexpr UINT IDM_EDIT_SELECTED   = 1022;

constexpr UINT IDC_TOOLBAR_COMBO   = 3001;
constexpr UINT IDC_BTN_COLOR       = 3002;
constexpr UINT IDC_BTN_ADD_DIALOG  = 3003;
constexpr UINT IDC_SHAPE_LISTBOX   = 3004;
constexpr UINT IDC_BTN_DELETE      = 3005;
constexpr UINT IDC_BTN_CLEAR_ALL   = 3006;
constexpr UINT IDC_BTN_EXIT        = 3007;
constexpr UINT IDC_BTN_INPUT_MOUSE = 3008;
constexpr UINT IDC_BTN_INPUT_DLG   = 3009;
constexpr UINT IDC_BTN_EDIT        = 3010;
constexpr UINT IDC_BTN_SAVE        = 3011;
constexpr UINT IDC_BTN_LOAD        = 3012;

constexpr int TOOLBAR_H = 40;
constexpr int PANEL_W   = 175;
constexpr int BTN_H     = 26;
constexpr int BTN_W     = 147;
constexpr int COMBO_W   = 120;
constexpr int COMBO_DH  = 200;

static ShapeList   g_list;
static HWND        g_hwnd      = nullptr;
static HWND        g_hCombo    = nullptr;
static HWND        g_hListBox  = nullptr;
static HWND        g_hBtnMouse = nullptr;
static HWND        g_hBtnDlg   = nullptr;

// Resizable control handles
static HWND g_hSepHorz     = nullptr;
static HWND g_hSepVert     = nullptr;
static HWND g_hLabelShapes = nullptr;
static HWND g_hLabelShape  = nullptr;
static HWND g_hBtnDelete   = nullptr;
static HWND g_hBtnClearAll = nullptr;
static HWND g_hBtnEdit     = nullptr;
static HWND g_hBtnSave     = nullptr;
static HWND g_hBtnLoad     = nullptr;
static HWND g_hLabelInput  = nullptr;
static HWND g_hBtnExit     = nullptr;

static bool        g_useMouse    = true;
static std::string g_currentTool = "line";
static COLORREF    g_currentColor  = RGB(0, 0, 0);
static COLORREF    g_customColors[16] = {};

static int                       g_selectedShape = -1;
static std::vector<COLORREF>     g_shapeColors;
static std::vector<POINT>        g_pendingClicks;

static void redraw() { if (g_hwnd) InvalidateRect(g_hwnd, nullptr, TRUE); }

/** Rebuild the right-panel ListBox from ShapeBinaryRegistry labels */
static void refreshListBox() {
    if (!g_hListBox) return;
    SendMessage(g_hListBox, LB_RESETCONTENT, 0, 0);
    for (size_t i = 0; i < g_list.count(); ++i) {
        const Shape* s = g_list.at(i);
        std::string lab = std::to_string(i + 1) + ". " + ShapeBinaryRegistry::listLabelFor(s);
        SendMessageA(g_hListBox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(lab.c_str()));
    }
}

static int listSelectedIndex() {
    if (!g_hListBox) return LB_ERR;
    return static_cast<int>(SendMessage(g_hListBox, LB_GETCURSEL, 0, 0));
}

static bool parseInts(const std::string& text, std::vector<int>& out) {
    out.clear();
    std::istringstream iss(text);
    long v = 0;
    while (iss >> v) out.push_back(static_cast<int>(v));
    return !out.empty();
}

static void tryAddShape(const std::string& tool, const std::vector<int>& params) {
    Shape* s = ShapeFactory::create(tool, params);
    if (!s) return;
    g_list.add(s);
    g_shapeColors.push_back(g_currentColor);
    redraw();
    refreshListBox();
}

static void onCanvasClick(LONG x, LONG y) {
    if (!g_useMouse) return;
    const IMouseTool* tool = MouseToolRegistry::find(g_currentTool);
    if (!tool) return;
    POINT p = {x, y};
    g_pendingClicks.push_back(p);
    if (g_pendingClicks.size() >= tool->pointCount()) {
        tryAddShape(tool->commandId(), tool->toParams(g_pendingClicks));
        g_pendingClicks.clear();
    }
}

static void syncInputButtons() {
    if (!g_hBtnMouse || !g_hBtnDlg) return;
    SendMessage(g_hBtnMouse, BM_SETSTATE, g_useMouse  ? TRUE : FALSE, 0);
    SendMessage(g_hBtnDlg,   BM_SETSTATE, !g_useMouse ? TRUE : FALSE, 0);
}

//File dialogs

static bool pickSavePath(HWND owner, char* buf, int bufChars) {
    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner   = owner;
    ofn.lpstrFile   = buf;
    ofn.nMaxFile    = static_cast<DWORD>(bufChars);
    ofn.lpstrFilter = "Binary shapes (*.bin)\0*.bin\0All files\0*.*\0\0";
    ofn.nFilterIndex = 1;
    ofn.Flags       = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = "bin";
    return GetSaveFileNameA(&ofn) != 0;
}

static bool pickOpenPath(HWND owner, char* buf, int bufChars) {
    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner   = owner;
    ofn.lpstrFile   = buf;
    ofn.nMaxFile    = static_cast<DWORD>(bufChars);
    ofn.lpstrFilter = "Binary shapes (*.bin)\0*.bin\0All files\0*.*\0\0";
    ofn.nFilterIndex = 1;
    ofn.Flags       = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    return GetOpenFileNameA(&ofn) != 0;
}

//Params dialog

static INT_PTR CALLBACK ParamsDlgProc(HWND dlg, UINT msg, WPARAM wp, LPARAM) {
    if (msg == WM_INITDIALOG) {
        const char* hint = "Params: space-separated integers";
        if      (g_currentTool == "line")      hint = "line: x1 y1 x2 y2";
        else if (g_currentTool == "rectangle") hint = "rectangle: left top right bottom";
        else if (g_currentTool == "ellipse")   hint = "ellipse: left top right bottom";
        else if (g_currentTool == "circle")    hint = "circle: cx cy radius";
        else if (g_currentTool == "triangle")  hint = "triangle: x1 y1 x2 y2 x3 y3";
        else if (g_currentTool == "point")     hint = "point: x y";
        else if (g_currentTool == "cube3d")    hint = "cube3d: left top right bottom  OR  cx cy cz edge";
        else if (g_currentTool == "sphere3d")  hint = "sphere3d: cx cy r  OR  cx cy cz r";
        else if (g_currentTool == "tetra3d")   hint = "tetra3d: x1 y1 x2 y2 x3 y3  OR  cx cy cz edge";
        else if (g_currentTool == "polygon")   hint = "polygon: N  (then click)  OR  x1 y1 x2 y2 ...";
        SetWindowTextA(dlg, hint);
        return TRUE;
    }
    if (msg == WM_COMMAND && LOWORD(wp) == IDOK) {
        char line[1024] = {0};
        GetDlgItemTextA(dlg, IDC_PARAMS_EDIT, line, static_cast<int>(sizeof(line) - 1));
        std::vector<int> params;
        if (parseInts(line, params)) {
            if (g_currentTool == "polygon" && params.size() == 1) {
                const int n = params[0];
                if (n >= 3 && n <= 200) {
                    MouseToolRegistry::setPointCountOverride("polygon", static_cast<size_t>(n));
                    g_pendingClicks.clear();
                }
            } else {
                tryAddShape(g_currentTool, params);
            }
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

static void showParamsDialog(HWND owner) {
    DialogBoxParamA(GetModuleHandle(nullptr), MAKEINTRESOURCEA(IDD_PARAMS), owner, ParamsDlgProc, 0);
}

//Menu (File only)

static void setupMenu(HWND wnd) {
    HMENU bar  = CreateMenu();
    HMENU file = CreateMenu();
    AppendMenuA(file, MF_STRING, IDM_CLEAR,       "Clear");
    AppendMenuA(file, MF_STRING, IDM_SAVE_BINARY, "Save list (binary)...");
    AppendMenuA(file, MF_STRING, IDM_LOAD_BINARY, "Load list (binary)...");
    AppendMenuA(file, MF_SEPARATOR, 0, nullptr);
    AppendMenuA(file, MF_STRING, IDM_EXIT, "Exit");
    AppendMenuA(bar, MF_POPUP, reinterpret_cast<UINT_PTR>(file), "&File");
    SetMenu(wnd, bar);
}

static HWND makeBtn(HWND wnd, HINSTANCE inst, const char* label, UINT id, int x, int& y, int gap = 4) {
    HWND h = CreateWindowExA(0, "BUTTON", label, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x, y, BTN_W, BTN_H, wnd, (HMENU)(UINT_PTR)(id), inst, nullptr);
    y += BTN_H + gap;
    return h;
}

static HWND makeLabel(HWND wnd, HINSTANCE inst, const char* text, int x, int& y, int w, int h = 18, int gap = 22) {
    HWND hw = CreateWindowExA(0, "STATIC", text, WS_CHILD | WS_VISIBLE, x, y, w, h, wnd, nullptr, inst, nullptr);
    y += gap;
    return hw;
}

//Toolbar

static void createToolbar(HWND wnd, HINSTANCE inst) {
    g_hLabelShape = CreateWindowExA(0, "STATIC", "Shape:", WS_CHILD | WS_VISIBLE | SS_LEFT, 8, 12, 48, 18, wnd, nullptr, inst, nullptr);
    g_hCombo = CreateWindowExA(0, "COMBOBOX", nullptr, WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST, 58, 8, COMBO_W, COMBO_DH, wnd, (HMENU)(UINT_PTR)(IDC_TOOLBAR_COMBO), inst, nullptr);
    for (const std::string& id : ShapeFactory::shapeIds())
        SendMessageA(g_hCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(id.c_str()));
    SendMessage(g_hCombo, CB_SETCURSEL, 0, 0);
    CreateWindowExA(0, "BUTTON", "Color", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 186, 8, 64, BTN_H, wnd, (HMENU)(UINT_PTR)(IDC_BTN_COLOR), inst, nullptr);
    CreateWindowExA(0, "BUTTON", "Add (Dialog)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 256, 8, 96, BTN_H, wnd, (HMENU)(UINT_PTR)(IDC_BTN_ADD_DIALOG), inst, nullptr);
    RECT rc; GetClientRect(wnd, &rc);
    g_hSepHorz = CreateWindowExA(0, "STATIC", nullptr, WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, 0, TOOLBAR_H - 1, rc.right, 2, wnd, nullptr, inst, nullptr);
}

//Right panel

static void createRightPanel(HWND wnd, HINSTANCE inst) {
    RECT rc; GetClientRect(wnd, &rc);
    const int H  = rc.bottom;
    const int px = rc.right - PANEL_W;
    int y = TOOLBAR_H + 8;

    g_hSepVert = CreateWindowExA(0, "STATIC", nullptr, WS_CHILD | WS_VISIBLE | SS_ETCHEDVERT, px , TOOLBAR_H, 2, H - TOOLBAR_H, wnd, nullptr, inst, nullptr);
    g_hLabelShapes = makeLabel(wnd, inst, "Shapes:", px + 8, y, PANEL_W - 16);
    const int buttonsH = BTN_H * 8 + 16 + 20 + 4 * 5 + 12 * 3;
    const int listH    = H - y - buttonsH - 8;
    const int safeH    = listH > 20 ? listH : 20;
    g_hListBox = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", nullptr, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, px + 6, y, PANEL_W - 14, safeH, wnd, (HMENU)(UINT_PTR)(IDC_SHAPE_LISTBOX), inst, nullptr);
    y += safeH + 8;
    g_hBtnEdit     = makeBtn(wnd, inst, "Edit Selected",   IDC_BTN_EDIT,     px + 12, y);
    g_hBtnDelete   = makeBtn(wnd, inst, "Delete Selected", IDC_BTN_DELETE,   px + 12, y);
    g_hBtnClearAll = makeBtn(wnd, inst, "Clear All",       IDC_BTN_CLEAR_ALL, px + 12, y, 12);
    g_hBtnSave = makeBtn(wnd, inst, "Save (binary)", IDC_BTN_SAVE, px + 12, y);
    g_hBtnLoad = makeBtn(wnd, inst, "Load (binary)", IDC_BTN_LOAD, px + 12, y, 12);
    g_hLabelInput = makeLabel(wnd, inst, "Input mode:", px + 8, y, PANEL_W - 16, 16, 20);
    g_hBtnMouse   = makeBtn(wnd, inst, "Mouse",       IDC_BTN_INPUT_MOUSE, px + 12, y);
    g_hBtnDlg     = makeBtn(wnd, inst, "Dialog only", IDC_BTN_INPUT_DLG,   px + 12, y, 12);
    g_hBtnExit = makeBtn(wnd, inst, "Exit", IDC_BTN_EXIT, px + 12, y);
}

//Adaptive layout

static void deferCtrl(HDWP& hdwp, HWND h, int x, int y, int w, int ht) {
    if (h) hdwp = DeferWindowPos(hdwp, h, nullptr, x, y, w, ht, SWP_NOZORDER | SWP_NOACTIVATE);
}

static void deferBtn(HDWP& hdwp, HWND h, int x, int& y, int gap = 4) {
    deferCtrl(hdwp, h, x, y, BTN_W, BTN_H);
    y += BTN_H + gap;
}

static void deferLabel(HDWP& hdwp, HWND h, int x, int& y, int w, int ht = 18, int gap = 22) {
    deferCtrl(hdwp, h, x, y, w, ht);
    y += gap;
}

static void repositionControls(HWND wnd) {
    RECT rc; GetClientRect(wnd, &rc);
    const int W  = rc.right;
    const int H  = rc.bottom;
    const int px = W - PANEL_W;
    int y = TOOLBAR_H + 8;

    HDWP hdwp = BeginDeferWindowPos(16);

    deferCtrl(hdwp, g_hSepHorz, 0,  TOOLBAR_H - 1, W,          2);
    deferCtrl(hdwp, g_hSepVert, px, TOOLBAR_H,     2, H - TOOLBAR_H);
    deferLabel(hdwp, g_hLabelShapes, px + 8, y, PANEL_W - 16);
    const int buttonsH = BTN_H * 8 + 16 + 20 + 4 * 5 + 12 * 3;
    const int listH    = H - y - buttonsH - 8;
    const int safeH    = listH > 20 ? listH : 20;
    deferCtrl(hdwp, g_hListBox, px + 6, y, PANEL_W - 14, safeH);
    y += safeH + 8;
    deferBtn(hdwp, g_hBtnEdit,     px + 12, y);
    deferBtn(hdwp, g_hBtnDelete,   px + 12, y);
    deferBtn(hdwp, g_hBtnClearAll, px + 12, y, 12);
    deferBtn(hdwp, g_hBtnSave, px + 12, y);
    deferBtn(hdwp, g_hBtnLoad, px + 12, y, 12);
    deferLabel(hdwp, g_hLabelInput, px + 8, y, PANEL_W - 16, 16, 20);
    deferBtn(hdwp, g_hBtnMouse, px + 12, y);
    deferBtn(hdwp, g_hBtnDlg,   px + 12, y, 12);
    deferBtn(hdwp, g_hBtnExit, px + 12, y);
    EndDeferWindowPos(hdwp);
}

//Painting

static void paintCanvas(HWND wnd) {
    RECT rc; GetClientRect(wnd, &rc);
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(wnd, &ps);

    // Canvas area: below toolbar, left of panel
    RECT canvas = { 0, TOOLBAR_H, rc.right - PANEL_W, rc.bottom };
    IntersectClipRect(hdc, canvas.left, canvas.top, canvas.right, canvas.bottom);
    FillRect(hdc, &canvas, reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));

    // Draw each shape with its stored color; highlight selected in blue
    const size_t count = g_list.count();
    for (size_t i = 0; i < count; ++i) {
        const Shape* s = g_list.at(i);
        COLORREF col = (i < g_shapeColors.size()) ? g_shapeColors[i] : RGB(0, 0, 0);
        const bool sel = (static_cast<int>(i) == g_selectedShape);

        HPEN pen = sel ? CreatePen(PS_SOLID, 3, RGB(0, 100, 255))
                       : CreatePen(PS_SOLID, 1, col);
        HPEN   oldPen = static_cast<HPEN>(SelectObject(hdc, pen));
        HBRUSH oldBr  = static_cast<HBRUSH>(SelectObject(hdc, GetStockObject(NULL_BRUSH)));

        ShapeRenderer::draw(hdc, s);

        // Second pass with dot pen for selected shape outline
        if (sel) {
            HPEN dotPen = CreatePen(PS_DOT, 1, RGB(0, 100, 255));
            SelectObject(hdc, dotPen);
            ShapeRenderer::draw(hdc, s);
            SelectObject(hdc, oldPen);
            DeleteObject(dotPen);
        } else {
            SelectObject(hdc, oldPen);
        }
        SelectObject(hdc, oldBr);
        DeleteObject(pen);
    }

    EndPaint(wnd, &ps);
}

//WndProс

static LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {

    case WM_CREATE: {
        g_hwnd = wnd;
        HINSTANCE inst = reinterpret_cast<CREATESTRUCT*>(lp)->hInstance;
        setupMenu(wnd);
        createToolbar(wnd, inst);
        createRightPanel(wnd, inst);
        syncInputButtons();
        repositionControls(wnd);
        return 0;
    }

    case WM_SIZE:
        repositionControls(wnd);
        InvalidateRect(wnd, nullptr, TRUE);
        return 0;

    case WM_DESTROY:
        g_hwnd = nullptr;
        PostQuitMessage(0);
        return 0;

    case WM_LBUTTONDOWN: {
        RECT rc; GetClientRect(wnd, &rc);
        SHORT cx = static_cast<SHORT>(LOWORD(lp));
        SHORT cy = static_cast<SHORT>(HIWORD(lp));
        if (g_useMouse && cy > TOOLBAR_H && cx < rc.right - PANEL_W)
            onCanvasClick(cx, cy);
        return 0;
    }

    case WM_COMMAND: {
        const UINT cmd   = LOWORD(wp);
        const UINT notif = HIWORD(wp);

        // ListBox selection — highlight shape on canvas
        if (cmd == IDC_SHAPE_LISTBOX && notif == LBN_SELCHANGE) {
            int sel = static_cast<int>(SendMessage(g_hListBox, LB_GETCURSEL, 0, 0));
            g_selectedShape = (sel == LB_ERR) ? -1 : sel;
            redraw();
            return 0;
        }

        // ComboBox — update current tool
        if (cmd == IDC_TOOLBAR_COMBO && notif == CBN_SELCHANGE) {
            char buf[64] = {0};
            int sel = static_cast<int>(SendMessage(g_hCombo, CB_GETCURSEL, 0, 0));
            if (sel != CB_ERR) {
                SendMessageA(g_hCombo, CB_GETLBTEXT, sel, reinterpret_cast<LPARAM>(buf));
                g_currentTool = buf;
                g_pendingClicks.clear();
            }
            return 0;
        }

        // Color picker
        if (cmd == IDC_BTN_COLOR) {
            CHOOSECOLORA cc = {};
            cc.lStructSize  = sizeof(cc);
            cc.hwndOwner    = wnd;
            cc.rgbResult    = g_currentColor;
            cc.lpCustColors = g_customColors;
            cc.Flags        = CC_FULLOPEN | CC_RGBINIT;
            if (ChooseColorA(&cc)) g_currentColor = cc.rgbResult;
            return 0;
        }

        // Add shape via params dialog
        if (cmd == IDC_BTN_ADD_DIALOG) {
            showParamsDialog(wnd);
            return 0;
        }

        // Edit selected shape via its registered codec dialog
        if (cmd == IDC_BTN_EDIT || cmd == IDM_EDIT_SELECTED) {
            const int sel = listSelectedIndex();
            if (sel == LB_ERR || static_cast<size_t>(sel) >= g_list.count()) {
                MessageBoxA(wnd, "Select an object in the list.", "Lab3",
                            MB_OK | MB_ICONWARNING);
                return 0;
            }
            Shape* s = g_list.at(static_cast<size_t>(sel));
            const IShapeBinaryCodec* codec =
                ShapeBinaryRegistry::find(s->binaryTypeTag());
            if (!codec) {
                MessageBoxA(wnd, "No editor for this type.", "Lab3",
                            MB_OK | MB_ICONERROR);
                return 0;
            }
            if (codec->editProperties(wnd, s)) {
                redraw();
                refreshListBox();
            }
            return 0;
        }

        // Delete selected
        if (cmd == IDC_BTN_DELETE || cmd == IDM_DELETE_SELECTED) {
            const int sel = listSelectedIndex();
            if (sel != LB_ERR && static_cast<size_t>(sel) < g_list.count()) {
                g_list.removeAt(static_cast<size_t>(sel));
                if (sel < static_cast<int>(g_shapeColors.size()))
                    g_shapeColors.erase(g_shapeColors.begin() + sel);
                g_selectedShape = -1;
                redraw();
                refreshListBox();
            }
            return 0;
        }

        // Clear all
        if (cmd == IDC_BTN_CLEAR_ALL || cmd == IDM_CLEAR) {
            g_list.clear();
            g_shapeColors.clear();
            g_pendingClicks.clear();
            g_selectedShape = -1;
            redraw();
            refreshListBox();
            return 0;
        }

        // Save to binary file
        if (cmd == IDC_BTN_SAVE || cmd == IDM_SAVE_BINARY) {
            char path[MAX_PATH] = {0};
            if (pickSavePath(wnd, path, static_cast<int>(sizeof(path)))) {
                if (ShapeBinaryRegistry::saveListToFile(g_list, path))
                    MessageBoxA(wnd, "List saved.", "Lab3", MB_OK | MB_ICONINFORMATION);
                else
                    MessageBoxA(wnd, "Save failed.", "Lab3", MB_OK | MB_ICONERROR);
            }
            return 0;
        }

        // Load from binary file
        if (cmd == IDC_BTN_LOAD || cmd == IDM_LOAD_BINARY) {
            char path[MAX_PATH] = {0};
            if (pickOpenPath(wnd, path, static_cast<int>(sizeof(path)))) {
                if (ShapeBinaryRegistry::loadListFromFile(g_list, path)) {
                    // Colors aren't stored in binary — fill with black
                    g_shapeColors.assign(g_list.count(), RGB(0, 0, 0));
                    g_pendingClicks.clear();
                    g_selectedShape = -1;
                    redraw();
                    refreshListBox();
                    MessageBoxA(wnd, "List loaded.", "Lab3", MB_OK | MB_ICONINFORMATION);
                } else {
                    MessageBoxA(wnd, "Load failed.", "Lab3", MB_OK | MB_ICONERROR);
                }
            }
            return 0;
        }

        // Input mode
        if (cmd == IDC_BTN_INPUT_MOUSE) {
            g_useMouse = true;  syncInputButtons(); return 0;
        }
        if (cmd == IDC_BTN_INPUT_DLG) {
            g_useMouse = false; syncInputButtons(); return 0;
        }

        // Exit
        if (cmd == IDC_BTN_EXIT || cmd == IDM_EXIT) {
            DestroyWindow(wnd); return 0;
        }

        break;
    }

    case WM_PAINT:
        paintCanvas(wnd);
        return 0;
    }
    return DefWindowProc(wnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE, LPSTR, int show) {
    const char* cls = "SimpleShapeEditorLab3";
    WNDCLASSA wc    = {};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = inst;
    wc.lpszClassName = cls;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassA(&wc);

    HWND w = CreateWindowExA(0, cls, "Graphical Editor — Lab3 (binary I/O)", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 980, 640, nullptr, nullptr, inst, nullptr);
    if (!w) return 0;
    ShowWindow(w, show);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}