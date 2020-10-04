// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

DWORD dwFunc = 0;
typedef int  (__stdcall *ptrSrc)(int);


void ExecuteCMD(const char *szCommandLine)
{
    STARTUPINFOA si = { sizeof(si) };
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = { 0 };
    si.dwFlags = STARTF_USESHOWWINDOW; //指定wShowWindow成员有效
    si.wShowWindow = TRUE; //此成员设为TRUE的话则显示新建进程的主窗口
    BOOL bRet = CreateProcessA(
        NULL, //不在此指定可执行文件的文件名
        (char*)szCommandLine, //命令行参数
        NULL, //默认进程安全性
        NULL, //默认进程安全性
        FALSE, //指定当前进程内句柄不可以被子进程继承
        CREATE_NEW_CONSOLE, //为新进程创建一个新的控制台窗口
        NULL, //使用本进程的环境变量
        NULL, //使用本进程的驱动器和目录
        &si,
        &pi);
}

//@wxid 微信ID,群聊@chat
//wsmsg 消息内容
void RecvMsgCallBack(wstring wxid,wstring wsmsg)
{
    if (wsmsg == L"gao") {
        ExecuteCMD("powercfg -s 10cd17b0-a31c-4de3-96ff-b841e6b8ecce");
    }
    else if (wsmsg == L"di") {
        ExecuteCMD("powercfg -s 8b89927c-6b0a-4aa7-9856-892b574747f6");
    }
    else if (wsmsg == L"gp") {
        //ExecuteCMD("scrnsave.scr /s");
        PostMessageA((HWND)-1, 0x0112, 0xF170, 2);
    }
    else if (wsmsg == L"gj") {
        ExecuteCMD("shutdown -s -t 0");
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

