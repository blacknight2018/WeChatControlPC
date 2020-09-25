// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

DWORD dwFunc = 0;
typedef int  (__stdcall *ptrSrc)(int);


//@wxid 微信ID,群聊@chat
//wsmsg 消息内容
void RecvMsgCallBack(wstring wxid,wstring wsmsg)
{
    if (wsmsg == L"SHUTDOWN") {
        MessageBoxW(NULL, L"执行关机", NULL, MB_OK);
    }
}



//[[[ESP+4]] + 0x40] == wxid_
//[[[ESP+4]] + 0x68] == 消息内容
int __stdcall Hook(int Param)
{
    int t = 0;
    //由于进行了一次HOOK后到另一个函数进行了push ebp;mov ebp,esp;
    //所以偏移改成下面
    //[[EBP+8]]+0x40
    __asm {
        mov t,ebp
    }
    int* wxid = (int*)(t+8);
    wxid = (int*)*wxid;
    wxid = (int*)*wxid;
    wxid = (int*)((int)wxid + 0x40);
    wxid = (int*)*wxid;
    //MessageBoxW(NULL, (LPCWSTR)wxid, NULL, MB_OK);

    int *wxmsg = (int*)(t + 8);
    wxmsg = (int*)*wxmsg;
    wxmsg = (int*)*wxmsg;
    wxmsg = (int*)((int)wxmsg + 0x68);
    wxmsg = (int*)*wxmsg;
    //MessageBoxW(NULL, (LPCWSTR)wxmsg, NULL, MB_OK);

    wstring wxidStr((LPCWSTR)wxid),wxmsgStr((LPCWSTR)wxmsg);
    RecvMsgCallBack(wxidStr, wxmsgStr);
    return ((ptrSrc)(dwFunc))(Param);
}

void Enter()
{
    //WeChatWin.dll + 0x2C9650
    HMODULE dllModule = LoadLibraryA("WeChatWin.dll");
    if (dllModule == 0) {
        MessageBoxA(NULL, "WeChatWin.dll Not Loaded", "", MB_OK);
        return;
    }

    MessageBoxA(NULL, "WeChatWin.dll has Loaded", "", MB_OK);

    dwFunc = (DWORD)dllModule + 0x2C9650;

    //不知道这两个什么鸟用
    DetourRestoreAfterWith();
    DetourTransactionBegin();

    DetourUpdateThread(GetCurrentThread());

    DetourAttach(&(PVOID&)dwFunc, Hook);
    DetourTransactionCommit();
    
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Enter();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

