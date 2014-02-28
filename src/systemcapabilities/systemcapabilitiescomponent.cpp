/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2014                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include "systemcapabilities/systemcapabilitiescomponent.h"

#include "logging/logmanager.h"

namespace ghoul {
namespace systemcapabilities {

#ifdef GHOUL_USE_WMI
    IWbemLocator* SystemCapabilitiesComponent::_iwbemLocator = nullptr;
    IWbemServices* SystemCapabilitiesComponent::_iwbemServices = nullptr;
#endif

SystemCapabilitiesComponent::SystemCapabilitiesComponent()
    : _isInitialized(false)
{}

SystemCapabilitiesComponent::~SystemCapabilitiesComponent() {}

#ifdef GHOUL_USE_WMI
void SystemCapabilitiesComponent::initialize(bool initializeWMI) {
    if (initializeWMI && !isWMIinitialized()) {
        SystemCapabilitiesComponent::initializeWMI();
    }
#else
void SystemCapabilitiesComponent::initialize(bool) {
#endif
    _isInitialized = true;
}

void SystemCapabilitiesComponent::deinitialize() {
#ifdef GHOUL_USE_WMI
    if (isWMIinitialized())
        deinitializeWMI();
#endif
    clearCapabilities();
    _isInitialized = false;
}

bool SystemCapabilitiesComponent::isInitialized() const {
    return _isInitialized;
}

#ifdef GHOUL_USE_WMI
void SystemCapabilitiesComponent::initializeWMI() {
    const std::string _loggerCat = "SystemCapabilitiesComponent.WMI";
    // This code is based on
    // http://msdn.microsoft.com/en-us/library/aa390423.aspx
    // All rights remain with their original copyright owners

    if (isWMIinitialized()) {
        LWARNING("The WMI initialization has already been initialized.");
        return;
    }

    HRESULT hRes = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (FAILED(hRes)) {
        LERROR("WMI initialization failed. 'CoInitializeEx' failed." 
            << " Error Code: " << hRes);
        return;
    }
    //if (hRes = S_FALSE)
    //    // WMI was already initialized
    //    return;
    hRes = CoInitializeSecurity(
        NULL, 
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
        );
    if (FAILED(hRes))
        LDEBUG("CoInitializeSecurity failed with error code: 0x" << hRes);
    else
        LDEBUG("CoInitializeSecurity successful.");

    hRes = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        reinterpret_cast<LPVOID*>(&_iwbemLocator));
    if (FAILED(hRes)) {
        LERROR("WMI initialization failed. Failed to create IWbemLocator object." 
            << " Error Code: " << hRes);
        _iwbemLocator = 0;
        CoUninitialize();
        return;
    }

    LDEBUG("IWbemLocator object successfully created.");

    hRes = _iwbemLocator->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
        NULL,                    // User name. NULL = current user
        NULL,                    // User password. NULL = current
        0,                       // Locale. NULL indicates current
        NULL,                    // Security flags.
        0,                       // Authority (for example, Kerberos)
        0,                       // Context object 
        &_iwbemServices          // pointer to IWbemServices proxy
        );
    if (FAILED(hRes)) {
        LERROR("WMI initialization failed. Failed to connect to WMI server."
            << " Error Code: " << hRes);
        _iwbemLocator->Release();
        CoUninitialize();
        _iwbemLocator = 0;
        _iwbemServices = 0;
        return;
    }

    LDEBUG("Connected to ROOT\\CIMV2 WMI namespace.");

    hRes = CoSetProxyBlanket(
        _iwbemServices,              // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name 
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities 
        );
    if (FAILED(hRes)) {
        LERROR("WMI initialization failed. Could not set proxy blanket. Error Code: 0x"
            << hRes);
        _iwbemServices->Release();
        _iwbemServices = 0;
        _iwbemLocator->Release();
        _iwbemLocator = 0;
        CoUninitialize();
        return;
    }
    LDEBUG("WMI successfully initialized.");
}

void SystemCapabilitiesComponent::deinitializeWMI() {
    const std::string _loggerCat = "SystemCapabilitiesComponent.WMI";
    if (!isWMIinitialized()) {
        LWARNING("WMI is not initialized.");
        return;
    }

    LDEBUG("Deinitializing WMI.");
    if (_iwbemLocator)
        _iwbemLocator->Release();
    _iwbemLocator = 0;
    if (_iwbemServices)
        _iwbemServices->Release();
    _iwbemServices = 0;

    CoUninitialize();
}

std::wstring str2wstr(const std::string& str) {
    int stringLength = static_cast<int>(str.length() + 1);
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), stringLength, 0, 0); 
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), stringLength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

std::string wstr2str(const std::wstring& wstr) {
    int stringLength = static_cast<int>(wstr.length() + 1);
    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), stringLength, 0, 0, 0, 0); 
    char* buf = new char[len];
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), stringLength, buf, len, 0, 0);
    std::string r(buf);
    delete[] buf;
    return r;
}

bool SystemCapabilitiesComponent::isWMIinitialized() {
    return ((_iwbemLocator != nullptr) && (_iwbemServices != nullptr));
}

VARIANT* SystemCapabilitiesComponent::queryWMI(const std::string& wmiClass,
                                               const std::string& attribute)
{
    const std::string _loggerCat = "SystemCapabilitiesComponent.WMI";
    if (!isWMIinitialized()) {
        LERROR("WMI is not initialized.");
        return nullptr;
    }

    VARIANT* result = nullptr;
    IEnumWbemClassObject* enumerator = nullptr;
    std::string query = "SELECT " + attribute + " FROM " + wmiClass;
    HRESULT hRes = _iwbemServices->ExecQuery(
        bstr_t("WQL"),
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
        NULL,
        &enumerator);
    if (FAILED(hRes)) {
        LERROR("WMI query failed. Error Code: 0x" << hRes);
        return nullptr;
    }

    IWbemClassObject* pclsObject = 0;
    ULONG returnValue;
    if (enumerator) {
        HRESULT hr = enumerator->Next(
            WBEM_INFINITE,
            1,
            &pclsObject,
            &returnValue);
        if (returnValue) {
            result = new VARIANT;
            hr = pclsObject->Get(
                LPCWSTR(str2wstr(attribute).c_str()),
                0,
                result,
                0,
                0);
        }
    }

    if (!result)
        LINFO("No WMI query result.");

    if (enumerator)
        enumerator->Release();
    if (pclsObject)
        pclsObject->Release();

    return result;
}

bool SystemCapabilitiesComponent::queryWMI(
    const std::string& wmiClass,
    const std::string& attribute,
    std::string& value) 
{
    VARIANT* variant = queryWMI(wmiClass, attribute);
    if (!variant)
        return false;
    else {
        value = wstr2str(std::wstring(variant->bstrVal));
        VariantClear(variant);
        return true;
    }
}

bool SystemCapabilitiesComponent::queryWMI(
    const std::string& wmiClass,
    const std::string& attribute,
    int& value)
{
    VARIANT* variant = queryWMI(wmiClass, attribute);
    if (!variant)
        return false;
    else {
        value = variant->intVal;
        VariantClear(variant);
        return true;
    }
}

bool SystemCapabilitiesComponent::queryWMI(const std::string& wmiClass, const std::string& attribute,
                                           unsigned int& value)
{
    VARIANT* variant = queryWMI(wmiClass, attribute);
    if (!variant)
        return false;
    else {
        value = variant->uintVal;
        VariantClear(variant);
        return true;
    }
}

bool SystemCapabilitiesComponent::queryWMI(const std::string& wmiClass, const std::string& attribute,
                                           unsigned long long& value)
{
    VARIANT* variant = queryWMI(wmiClass, attribute);
    if (!variant)
        return false;
    else {
        value = variant->ullVal;
        VariantClear(variant);
        return true;
    }
}

#endif

} // namespace systemcapabilities
} // namespace ghoul
