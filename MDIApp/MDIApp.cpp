#include <Rad\GUI\Window.h>
#include <Rad\GUI\MDIFrame.h>
#include <Rad\GUI\WindowCreate.h>
#include <Rad\GUI\RegClass.h>
#include <Rad\GUI\MessageLoop.h>
#include <Rad\GUI\CommCtrl.h>
#include <Rad\GUI\WindowChain.h>

#include <WinUser.h>

#include "resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace rad;

LRESULT CALLBACK MDIChildSubclass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR /*uIdSubclass*/, DWORD_PTR /*dwRefData*/)
{
    if (uMsg == WM_MDIACTIVATE)
    {
        if (hWnd == (HWND) lParam)
            SendMessage(GetParent(hWnd), WM_PARENTNOTIFY, MAKELONG(WM_MDIACTIVATE, GetWindowID(hWnd)), (LPARAM) hWnd);
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK MDIClientSubclass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR /*uIdSubclass*/, DWORD_PTR /*dwRefData*/)
{
    if (uMsg == WM_PARENTNOTIFY)
    {
        switch (LOWORD(wParam))
        {
        case WM_CREATE:
            SendMessage(GetParent(hWnd), WM_PARENTNOTIFY, MAKELONG(WM_MDICREATE, HIWORD(wParam)), lParam);
            SetWindowSubclass((HWND) lParam, MDIChildSubclass, 0, 0);
            break;
        case WM_DESTROY:
            SendMessage(GetParent(hWnd), WM_PARENTNOTIFY, MAKELONG(WM_MDIDESTROY, HIWORD(wParam)), lParam);
            break;
        case WM_MDIACTIVATE:
            SendMessage(GetParent(hWnd), WM_PARENTNOTIFY, wParam, lParam);
            break;
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

class MDITabChain : public WindowChain
{
protected:
    void OnCreate(Window& Window, LPCREATESTRUCT /*CreateStruct*/)
    {
        m_tabctrl.Create(Window, s_nTabID, RECT({}), TCS_FOCUSNEVER | TCS_SINGLELINE);
    }

    void OnNotify(Window& Window, int CtrlID, LPNMHDR Header)
    {
        switch (CtrlID)
        {
        case s_nTabID:
            switch (Header->code)
            {
            case TCN_SELCHANGE:
                {
                    int sel = m_tabctrl.GetCurSel();
                    HWND hChildWnd = (HWND) m_tabctrl.GetParam(sel);
                    dynamic_cast<MDIFrame&>(Window).MDIActivate(hChildWnd);
                }
                break;
            }
            break;
        }
    }

    void OnParentNotify(Window& Window, UINT uMsg, int /*Id*/, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_MDICREATE:
            {
                WindowProxy c((HWND) lParam);
                TCHAR title[1024];
                c.GetWindowText(title);
                TCITEM item = {};
                item.mask = TCIF_TEXT | TCIF_PARAM;
                item.pszText = title;
                item.lParam = lParam;
                m_tabctrl.InsertItem(m_tabctrl.GetItemCount(), &item);

                ForceLayout(Window);
            }
            break;

        case WM_MDIDESTROY:
            {
                int i = m_tabctrl.FindParam(lParam);
                if (i >= 0)
                    m_tabctrl.DeleteItem(i);
            }
            break;

        case WM_MDIACTIVATE:
            {
                int i = m_tabctrl.FindParam(lParam);
                if (i >= 0)
                    m_tabctrl.SetCurSel(i);
            }
            break;
        }
    }

    void OnSize(Window& Window, UINT /*Type*/, int /*cx*/, int /*cy*/)
    {
        WindowProxy mdiclient = dynamic_cast<MDIFrame&>(Window).GetMDIClient();

        RECT r;
        mdiclient.GetWindowRect(&r);
        MapWindowPoints(NULL, Window.GetHWND(), (LPPOINT) &r, 2);

        m_tabctrl.SetWindowPos(NULL, r, SWP_NOACTIVATE | SWP_NOZORDER);
        //m_tabctrl.GetWindowRect(&cr);
        //r.top += cr.bottom - cr.top;
        m_tabctrl.AdjustRect(&r, FALSE);

        mdiclient.SetWindowPos(NULL, r, SWP_NOACTIVATE | SWP_NOZORDER);
    }

    LRESULT OnMessage(Window* Window, UINT Message, WPARAM wParam, LPARAM lParam) override
    {
        switch (Message)
        {
        case WM_CREATE:         OnCreate(*Window, (LPCREATESTRUCT) lParam); break;
        case WM_NOTIFY:         OnNotify(*Window, (int) wParam, (LPNMHDR) lParam); break;
        case WM_PARENTNOTIFY:   OnParentNotify(*Window, LOWORD(wParam), HIWORD(wParam), lParam); break;
        }
        LRESULT ret =  WindowChain::OnMessage(Window, Message, wParam, lParam);
        switch (Message)
        {
        case WM_SIZE:           OnSize(*Window, (UINT) wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); break;
        }
        return ret;
    }

private:
    static const int s_nTabID = 123;

    void ForceLayout(Window& Window)
    {
        RECT r;
        Window.GetWindowRect(&r);
        Window.SendMessage(WM_SIZE, 0, MAKELONG(r.right - r.left, r.bottom - r.top));
    }

private:
    TabCtrlWnd m_tabctrl;
};

class BorderLayout : public WindowChain
{
public:
    void AddTop(const WindowProxy& Window)
    {
        m_top.push_back(Window);
    }

    void SetCenter(const WindowProxy& Window)
    {
        m_center = Window;
    }

protected:
    void OnSize(Window& Window, UINT /*Type*/, int /*cx*/, int /*cy*/)
    {
        RECT r, cr;
        Window.GetClientRect(&r);

        for (WindowProxy& Child : m_top)
        {
            Child.SetWindowPos(NULL, r, SWP_NOACTIVATE | SWP_NOZORDER);
            Child.GetWindowRect(&cr);
            r.top += cr.bottom - cr.top;
        }

        m_center.SetWindowPos(NULL, r, SWP_NOACTIVATE | SWP_NOZORDER);
    }

    LRESULT OnMessage(Window* Window, UINT Message, WPARAM wParam, LPARAM lParam) override
    {
        LRESULT ret = WindowChain::OnMessage(Window, Message, wParam, lParam);
        switch (Message)
        {
        case WM_SIZE:           OnSize(*Window, (UINT) wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); break;
        }
        return ret;
    }

private:
    std::vector<WindowProxy> m_top;
    WindowProxy m_center;
};

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
    MyMDIFrame()
    {
        Add(&m_tabchain);
        Add(&m_layout);
    }

    ~MyMDIFrame() {}

    MDIChildCreate GetMDIChildCreate(HINSTANCE hInstance) override
    {
        MDIChildCreate wc(MDIFrame::GetMDIChildCreate(hInstance));
        wc.Style |= WS_MAXIMIZE;
        return wc;
    }

    LRESULT OnCreate(LPCREATESTRUCT CreateStruct) override
    {
        LRESULT r = MDIFrame::OnCreate(CreateStruct);

        SetWindowSubclass(GetMDIClient().GetHWND(), MDIClientSubclass, 0, 0);

        const int ImageListID = 0;
        const DWORD buttonStyles = BTNS_AUTOSIZE;
        TBBUTTON tbButtons[] =
        {
            { MAKELONG(STD_FILENEW,  ImageListID), ID_WINDOW_NEW,               TBSTATE_ENABLED, buttonStyles, { 0 }, 0, (INT_PTR) L"New" },
            { MAKELONG(STD_FILEOPEN, ImageListID), 0,                           0,               buttonStyles, { 0 }, 0, (INT_PTR) L"Open" },
            { MAKELONG(STD_FILESAVE, ImageListID), 0,                           0,               buttonStyles, { 0 }, 0, (INT_PTR) L"Save" },
            { MAKELONG(STD_HELP,     ImageListID), ID_HELP_ABOUT,               TBSTATE_ENABLED, buttonStyles, { 0 }, 0, (INT_PTR) L"Help" },
        };

        m_toolbar.Create256(*this, WS_CHILD | TBSTYLE_WRAPABLE | TBSTYLE_TOOLTIPS | TBSTYLE_LIST, 456, HINST_COMMCTRL, IDB_STD_SMALL_COLOR, tbButtons, ARRAYSIZE(tbButtons), 16, 16);
        m_toolbar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
        m_toolbar.ShowWindow();

        m_layout.AddTop(m_toolbar);
        m_layout.SetCenter(GetMDIClient());

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
            CreateMDIChild(new Window(), (HINSTANCE) GetWindowLongPtr(GWLP_HINSTANCE), _T("MDI Child New"));
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

private:
    BorderLayout m_layout;
    MDITabChain m_tabchain;
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

    (new Window())->CreateMDIChildWnd(hInstance, _T("MDI Child A"), f);

    f->CreateMDIChild(new Window(), hInstance, _T("MDI Child 1"));
    f->CreateMDIChild(new Window(), hInstance, _T("MDI Child 2"));
    f->CreateMDIChild(new Window(), hInstance, _T("MDI Child 3"));

    return (int) DoMessageLoop(f->GetHWND(), f->GetMDIClient().GetHWND(), NULL);
}
