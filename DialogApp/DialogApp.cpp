#include <Rad\GUI\Dialog.h>
#include <Rad\GUI\MessageLoop.h>

#include "resource.h"

using namespace rad;

int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_ HINSTANCE /*hPrevInstance*/,
    _In_ LPSTR     /*lpCmdLine*/,
    _In_ int       nCmdShow
)
{
    bool modal = true;
    if (modal)
    {
        Dialog dlg;
        return (int) dlg.DoModal(hInstance, IDD_DIALOG1, NULL);
    }
    else
    {
        Dialog* dlg = new Dialog();
        dlg->CreateDlg(hInstance, IDD_DIALOG1, NULL);
        dlg->ShowWindow(nCmdShow);

        return (int) DoMessageLoop();
    }
}
