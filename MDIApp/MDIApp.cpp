#include <Rad\GUI\Window.h>
#include <Rad\GUI\MDIFrame.h>
#include <Rad\GUI\WindowCreate.h>
#include <Rad\GUI\RegClass.h>
#include <Rad\GUI\MessageLoop.h>

#include <WinUser.h>

#include "resource.h"

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
    virtual LRESULT OnCommand(WORD NotifyCode, WORD ID, HWND hWnd) override
    {
        switch (ID)
        {
        case ID_FILE_EXIT:
            PostQuitMessage(0);
            break;

        case ID_WINDOW_NEW:
            //(new Window())->CreateMDIChildWnd(_T("MDI Child New"), this);
            CreateChild(new Window(), _T("MDI Child New"));
            break;
        }
        return MDIFrame::OnCommand(NotifyCode, ID, hWnd);
    }
};

int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_ HINSTANCE /*hPrevInstance*/,
    _In_ LPSTR     /*lpCmdLine*/,
    _In_ int       nCmdShow
)
{
    MyMDIFrame* f = MyMDIFrame::Create(hInstance);
    f->ShowWindow(nCmdShow);

    (new Window())->CreateMDIChildWnd(_T("MDI Child A"), f);

    f->CreateChild(new Window(), _T("MDI Child 1"));
    f->CreateChild(new Window(), _T("MDI Child 2"));
    f->CreateChild(new Window(), _T("MDI Child 3"));

    return (int) DoMessageLoop(f->GetHWND(), f->GetMDIClient().GetHWND(), NULL);
}
