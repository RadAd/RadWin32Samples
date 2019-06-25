#include <Rad\GUI\Window.h>
#include <Rad\GUI\MDIFrame.h>
#include <Rad\GUI\WindowCreate.h>
#include <Rad\GUI\RegClass.h>
#include <Rad\GUI\MessageLoop.h>
#include <Rad\GUI\CommCtrl.h>

#include <WinUser.h>

#include "resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace rad;

class MyMDIFrame : public MDIFrame
{
public:
    static MyMDIFrame* Create(HINSTANCE hInstance)
    {
        MyMDIFrame* w = new MyMDIFrame();
        w->CreateWnd(hInstance, _T("MDI App"));
        return w;
    }

    RegClass GetMDIFrameReg(HINSTANCE _hInstance) override
    {
        RegClass rc = MDIFrame::GetMDIFrameReg(_hInstance);
        rc.SetMenu(IDR_MENU1);
        return rc;
    }

protected:
    virtual LRESULT OnCreate(LPCREATESTRUCT CreateStruct) override
    {
        LRESULT r = MDIFrame::OnCreate(CreateStruct);

        const int ImageListID = 0;
        const DWORD buttonStyles = BTNS_AUTOSIZE;
        TBBUTTON tbButtons[] =
        {
            { MAKELONG(STD_FILENEW,  ImageListID), ID_WINDOW_NEW,               TBSTATE_ENABLED, buttonStyles, { 0 }, 0, (INT_PTR) L"New" },
            { MAKELONG(STD_FILEOPEN, ImageListID), 0,                           0,               buttonStyles, { 0 }, 0, (INT_PTR) L"Open" },
            { MAKELONG(STD_FILESAVE, ImageListID), 0,                           0,               buttonStyles, { 0 }, 0, (INT_PTR) L"Save" },
            { MAKELONG(STD_HELP,     ImageListID), ID_HELP_ABOUT,               TBSTATE_ENABLED, buttonStyles, { 0 }, 0, (INT_PTR) L"Help" },
        };

        m_toolbar.Create256(*this, WS_CHILD | TBSTYLE_WRAPABLE | TBSTYLE_TOOLTIPS | TBSTYLE_LIST, 0, HINST_COMMCTRL, IDB_STD_SMALL_COLOR, tbButtons, ARRAYSIZE(tbButtons), 16, 16);
        m_toolbar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
        m_toolbar.ShowWindow();

        return r;
    }

    virtual LRESULT OnCommand(WORD NotifyCode, WORD ID, HWND hWnd) override
    {
        switch (ID)
        {
        case ID_FILE_EXIT:
            PostQuitMessage(0);
            break;

        case ID_HELP_ABOUT:
            MessageBox(*this, _T("RadWin32 MDI Sample App"), _T("MDI App"), MB_OK | MB_ICONINFORMATION);
            break;

        case ID_WINDOW_NEW:
            //(new Window())->CreateMDIChildWnd(_T("MDI Child New"), this);
            CreateChild(new Window(), _T("MDI Child New"));
            break;

        case ID_WINDOW_CASCADE:
            CascadeWindows();
            break;

        case ID_WINDOW_TILEHORIZONTALLY:
            TileWindows(MDITILE_SKIPDISABLED | MDITILE_HORIZONTAL);
            break;

        case ID_WINDOW_TILEVERTICALLY:
            TileWindows(MDITILE_SKIPDISABLED | MDITILE_VERTICAL);
            break;
        }
        return MDIFrame::OnCommand(NotifyCode, ID, hWnd);
    }

    virtual LRESULT OnSize(UINT Type, int cx, int cy)
    {
        LRESULT ret = MDIFrame::OnSize(Type, cx, cy);

        RECT r;
        GetClientRect(&r);

        m_toolbar.SetWindowPos(NULL, r.left, r.top, r.right - r.left, 0, SWP_NOACTIVATE | SWP_NOZORDER);
        RECT tr;
        m_toolbar.GetWindowRect(&tr);

        r.top += tr.bottom - tr.top;

        //GetMDIClient().SetWindowPos(NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOACTIVATE | SWP_NOZORDER);
        GetMDIClient().MoveWindow(r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);

        return ret;
    }

private:
    ToolBarWnd m_toolbar;
};

int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_ HINSTANCE /*hPrevInstance*/,
    _In_ LPSTR     /*lpCmdLine*/,
    _In_ int       nCmdShow
)
{
    InitCommonControls();

    MyMDIFrame* f = MyMDIFrame::Create(hInstance);
    f->ShowWindow(nCmdShow);

    (new Window())->CreateMDIChildWnd(_T("MDI Child A"), f);

    f->CreateChild(new Window(), _T("MDI Child 1"));
    f->CreateChild(new Window(), _T("MDI Child 2"));
    f->CreateChild(new Window(), _T("MDI Child 3"));

    return (int) DoMessageLoop(f->GetHWND(), f->GetMDIClient().GetHWND(), NULL);
}
