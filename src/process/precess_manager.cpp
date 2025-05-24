#include "../include/process_manager.h"
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>

#include <initguid.h>  // Đảm bảo GUID được định nghĩa
#include <wbemidl.h>
#include <comdef.h>
#include <mutex>
#include <thread>  // Thêm thư viện này
#include <chrono>
#include <psapi.h>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "psapi.lib")


bool monitor_running = true;
bool monitor_silent = false; // Mặc định là chế độ hiển thị bình thường
static std::vector<ProcessInfo> process_list;

// Hàm kiểm tra process có đang chạy không
bool is_process_running(DWORD pid) {
    if (pid == 0) return false;
    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess == NULL) {
        return false;
    }
    
    DWORD exitCode;
    bool isRunning = false;
    if (GetExitCodeProcess(hProcess, &exitCode)) {
        isRunning = (exitCode == STILL_ACTIVE);
    }
    
    CloseHandle(hProcess);
    return isRunning;
}

// Hàm đồng bộ process_list với hệ thống
void sync_process_list() {
    auto it = process_list.begin();
    while (it != process_list.end()) {
        if (!is_process_running(it->pid)) {
            std::cout << "[AUTO REMOVED] PID: " << it->pid 
                      << " | Name: " << it->name << "\n";
            it = process_list.erase(it); // Xóa tiến trình và cập nhật iterator
        } else {
            ++it; // Tiến tới phần tử tiếp theo
        }
    }
}

// Thread đồng bộ chạy độc lập
void sync_thread_function() {
    while (monitor_running) {
        std::this_thread::sleep_for(std::chrono::seconds(2)); // Kiểm tra mỗi 2 giây
        sync_process_list();
    }
}

void list_processes() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create snapshot.\n";
        return;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        std::cout << "PID\tName\n";
        do {
            std::wcout << pe.th32ProcessID << "\t" << pe.szExeFile << "\n";
        } while (Process32Next(hSnapshot, &pe));
    } else {
        std::cerr << "Failed to retrieve process list.\n";
    }

    CloseHandle(hSnapshot);
}

int find_process_index(DWORD pid) {
    for (size_t i = 0; i < process_list.size(); ++i) {
        if (process_list[i].pid == pid) return static_cast<int>(i);
    }
    return -1;
}

bool stop_process(DWORD pid) {
    if (pid == GetCurrentProcessId()) {
        std::cerr << "Cannot suspend the shell itself!\n";
        return false;
    }

    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid);
    if (!hProcess) {
        std::cerr << "Failed to open process.\n";
        return false;
    }

    typedef LONG (NTAPI *NtSuspendProcess)(HANDLE);
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (!ntdll) return false;
    NtSuspendProcess suspend = (NtSuspendProcess)GetProcAddress(ntdll, "NtSuspendProcess");
    if (!suspend) return false;

    suspend(hProcess);
    CloseHandle(hProcess);

    int index = find_process_index(pid);
    if (index != -1) {
        process_list[index].status = "Suspended";   
    }

    return true;
}

bool resume_process(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid);
    if (!hProcess) {
        std::cerr << "Failed to open process.\n";
        return false;
    }

    typedef LONG (NTAPI *NtResumeProcess)(HANDLE);
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (!ntdll) return false;
    NtResumeProcess resume = (NtResumeProcess)GetProcAddress(ntdll, "NtResumeProcess");
    if (!resume) return false;

    resume(hProcess);
    CloseHandle(hProcess);

    int index = find_process_index(pid);
    if (index != -1) {
        process_list[index].status = "Running";
    }

    return true;
}

bool kill_process(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!hProcess) {
        std::cerr << "Failed to open process.\n";
        return false;
    }

    BOOL result = TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);

    if (result) {
        int index = find_process_index(pid);
        if (index != -1) {
            process_list.erase(process_list.begin() + index);
        }
        std::cout << "[TERMINATED PROCESS] PID: " << pid << " has been terminated from the system.\n";
    }

    return result;
}

void addProcess(DWORD pid, const std::wstring &cmdline, HANDLE hProcess, bool is_background) {
    int len = WideCharToMultiByte(CP_UTF8, 0, cmdline.c_str(), -1, NULL, 0, NULL, NULL);
    std::string name(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, cmdline.c_str(), -1, &name[0], len, NULL, NULL);
    process_list.push_back({ pid, name, "Running", is_background });
}

void print_managed_processes() {
    std::cout << "PID\tStatus\t\tType\t\tName\n";
    for (const auto& p : process_list) {
        std::cout << p.pid << "\t" << p.status << "\t"
                  << (p.is_background ? "Background" : "Foreground") << "\t"
                  << p.name << "\n";
    }
}

void print_process_info(DWORD pid) {
    int index = find_process_index(pid);
    if (index == -1) {
        std::cout << "Process with PID " << pid << " not found in managed list.\n";
        return;
    }
    const auto& p = process_list[index];
    std::cout << "PID:        " << p.pid << "\n"
              << "Status:     " << p.status << "\n"
              << "Type:       " << (p.is_background ? "Background" : "Foreground") << "\n"
              << "Name:       " << p.name << "\n";
}




class EventSink : public IWbemObjectSink {
    LONG m_lRef;
    std::mutex mtx;

public:
    EventSink() : m_lRef(0) {}
    virtual ~EventSink() {}

    virtual ULONG STDMETHODCALLTYPE AddRef() {
        return InterlockedIncrement(&m_lRef);
    }

    virtual ULONG STDMETHODCALLTYPE Release() {
        LONG lRef = InterlockedDecrement(&m_lRef);
        if (lRef == 0) delete this;
        return lRef;
    }

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) {
        if (riid == IID_IUnknown || riid == IID_IWbemObjectSink) {
            *ppv = (IWbemObjectSink*)this;
            AddRef();
            return WBEM_S_NO_ERROR;
        } else {
            return E_NOINTERFACE;
        }
    }

    virtual HRESULT STDMETHODCALLTYPE Indicate(LONG lObjectCount, IWbemClassObject** apObjArray) {
        for (LONG i = 0; i < lObjectCount; i++) {
            VARIANT vtClass;
            HRESULT hr = apObjArray[i]->Get(L"__CLASS", 0, &vtClass, NULL, NULL);
            if (FAILED(hr) || vtClass.vt != VT_BSTR) continue;

            std::wstring className = vtClass.bstrVal;
            VariantClear(&vtClass);

            VARIANT vtProp;
            hr = apObjArray[i]->Get(L"TargetInstance", 0, &vtProp, NULL, NULL);
            if (FAILED(hr) || vtProp.vt != VT_UNKNOWN) continue;

            IUnknown* pUnk = vtProp.punkVal;
            IWbemClassObject* pObj = NULL;
            pUnk->QueryInterface(IID_IWbemClassObject, (void**)&pObj);

            VARIANT vtName, vtPid;
            pObj->Get(L"Name", 0, &vtName, NULL, NULL);
            pObj->Get(L"ProcessId", 0, &vtPid, NULL, NULL);

            std::wstring name = vtName.bstrVal;
            DWORD pid = vtPid.uintVal;

            if (className == L"__InstanceCreationEvent") {
                if (!monitor_silent) { // Chỉ in ra khi không ở chế độ silent
                    std::string utf8name;
                    int len = WideCharToMultiByte(CP_UTF8, 0, name.c_str(), -1, nullptr, 0, nullptr, nullptr);
                    utf8name.resize(len);
                    WideCharToMultiByte(CP_UTF8, 0, name.c_str(), -1, utf8name.data(), len, nullptr, nullptr);

                    std::cout << "[NEW PROCESS] PID: " << pid << " | Name: " << utf8name << "\n";
                }
            } else if (className == L"__InstanceDeletionEvent") {
                if (!monitor_silent) { // Chỉ in ra khi không ở chế độ silent
                    std::string utf8name;
                    int len = WideCharToMultiByte(CP_UTF8, 0, name.c_str(), -1, nullptr, 0, nullptr, nullptr);
                    utf8name.resize(len);
                    WideCharToMultiByte(CP_UTF8, 0, name.c_str(), -1, utf8name.data(), len, nullptr, nullptr);

                    std::cout << "[TERMINATED PROCESS] PID: " << pid << " | Name: " << utf8name << "\n";
                }
            }

            VariantClear(&vtName);
            VariantClear(&vtPid);
            pObj->Release();
            VariantClear(&vtProp);
        }
        return WBEM_S_NO_ERROR;
    }

    virtual HRESULT STDMETHODCALLTYPE SetStatus(LONG lFlags, HRESULT hResult, BSTR strParam, IWbemClassObject* pObjParam) {
        return WBEM_S_NO_ERROR;
    }
};


void MonitorProcessCreation() {
    HRESULT hr;
    IWbemLocator* pLoc = NULL;
    IWbemServices* pSvc = NULL;
    IUnsecuredApartment* pUnsecApp = NULL;
    IWbemObjectSink* pSink = NULL;

    // Khởi tạo COM
    hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM library.\n";
        return;
    }

    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize security.\n";
        CoUninitialize();
        return;
    }

    // Kết nối tới WMI
    hr = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLoc
    );
    if (FAILED(hr)) {
        std::cerr << "Failed to create IWbemLocator object.\n";
        CoUninitialize();
        return;
    }

    BSTR namespaceStr = SysAllocString(L"ROOT\\CIMV2");
    hr = pLoc->ConnectServer(
        namespaceStr,
        NULL,
        NULL,
        0,
        0,
        0,
        0,
        &pSvc
    );
    SysFreeString(namespaceStr);
    if (FAILED(hr)) {
        std::cerr << "Could not connect to WMI namespace.\n";
        pLoc->Release();
        CoUninitialize();
        return;
    }

    // Đăng ký sự kiện
    hr = CoCreateInstance(
        CLSID_UnsecuredApartment,
        NULL,
        CLSCTX_LOCAL_SERVER,
        IID_IUnsecuredApartment,
        (void**)&pUnsecApp
    );
    if (FAILED(hr)) {
        std::cerr << "Failed to create UnsecuredApartment.\n";
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    // Tạo sink object để nhận thông báo
    pSink = new EventSink;
    pSink->AddRef();

    IUnknown* pStubUnk = NULL;
    pUnsecApp->CreateObjectStub(pSink, &pStubUnk);

    IWbemObjectSink* pStubSink = NULL;
    pStubUnk->QueryInterface(IID_IWbemObjectSink, (void**)&pStubSink);

    // Thực hiện truy vấn sự kiện
    BSTR queryLanguage = SysAllocString(L"WQL");
    BSTR creationQuery = SysAllocString(L"SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process'");
    BSTR deletionQuery = SysAllocString(L"SELECT * FROM __InstanceDeletionEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process'");

    hr = pSvc->ExecNotificationQueryAsync(
        queryLanguage,
        creationQuery,
        WBEM_FLAG_SEND_STATUS,
        NULL,
        pStubSink
    );
    SysFreeString(creationQuery);

    hr = pSvc->ExecNotificationQueryAsync(
        queryLanguage,
        deletionQuery,
        WBEM_FLAG_SEND_STATUS,
        NULL,
        pStubSink
    );
    SysFreeString(deletionQuery);
    SysFreeString(queryLanguage);

    if (FAILED(hr)) {
        std::cerr << "Failed to execute WMI query.\n";
        pStubSink->Release();
        pStubUnk->Release();
        pUnsecApp->Release();
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    std::cout << "Monitoring process creation and deletion... Press Ctrl+C to stop.\n";

    // Chờ sự kiện
    std::thread sync_thread(sync_thread_function);
    while (monitor_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Dọn dẹp
    pSvc->CancelAsyncCall(pStubSink);
    pStubSink->Release();
    pStubUnk->Release();
    pUnsecApp->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
    sync_thread.join();

}