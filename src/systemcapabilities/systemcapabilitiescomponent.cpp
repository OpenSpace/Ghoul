/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2018                                                               *
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

#include <ghoul/systemcapabilities/systemcapabilitiescomponent.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/assert.h>
#include <ghoul/misc/stringconversion.h>
#include <map>

namespace ghoul {

template <>
std::string to_string
              (const ghoul::systemcapabilities::SystemCapabilitiesComponent::Verbosity& v)
{
    using Verbosity = ghoul::systemcapabilities::SystemCapabilitiesComponent::Verbosity;
    switch (v) {
        case Verbosity::None:    return "None";
        case Verbosity::Minimal: return "Minimal";
        case Verbosity::Default: return "Default";
        case Verbosity::Full:    return "Full";
        default:                 throw ghoul::MissingCaseException();
    }
}

template <>
ghoul::systemcapabilities::SystemCapabilitiesComponent::Verbosity
from_string(const std::string& string)
{
    using Verbosity = ghoul::systemcapabilities::SystemCapabilitiesComponent::Verbosity;

    static const std::map<std::string, Verbosity> VerbosityMap = {
        { "None", Verbosity::None },
        { "Minimal", Verbosity::Minimal },
        { "Default", Verbosity::Default },
        { "Full", Verbosity::Full }
    };

    // Throws a std::out_of_range exception if the type could not be found
    return VerbosityMap.at(string);
}

} // namespace ghoul

namespace ghoul::systemcapabilities {

#ifdef GHOUL_USE_WMI
IWbemLocator* SystemCapabilitiesComponent::_iwbemLocator = nullptr;
IWbemServices* SystemCapabilitiesComponent::_iwbemServices = nullptr;

SystemCapabilitiesComponent::WMIError::WMIError(std::string msg, HRESULT code)
    : RuntimeError(msg + ". Error Code: " + std::to_string(code), "WMI")
    , message(std::move(msg))
    , errorCode(std::move(code))
{}
#endif

SystemCapabilitiesComponent::SystemCapabilitiesComponent(InitializeWMI initializeWMI) {
#ifdef GHOUL_USE_WMI
    if (initializeWMI && !isWMIInitialized()) {
        SystemCapabilitiesComponent::initializeWMI();
    }
#else
    // Remove 'unused parameter' warning
    (void)(initializeWMI);
#endif
}

SystemCapabilitiesComponent::~SystemCapabilitiesComponent() {
#ifdef GHOUL_USE_WMI
    if (isWMIInitialized()) {
        deinitializeWMI();
    }
#endif
}

#ifdef GHOUL_USE_WMI
void SystemCapabilitiesComponent::initializeWMI() {
    constexpr const char* _loggerCat = "SystemCapabilitiesComponent.WMI";

    ghoul_assert(!isWMIInitialized(), "WMI must not have been initialized");

    LDEBUG("Begin initializing WMI");
    // This code is based on
    // http://msdn.microsoft.com/en-us/library/aa390423.aspx
    // All rights remain with their original copyright owners

    HRESULT hRes = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hRes)) {
        throw WMIError("WMI initialization failed. 'CoInitializeEx' failed", hRes);
    }

    hRes = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        reinterpret_cast<LPVOID*>(&_iwbemLocator)
    );
    if (FAILED(hRes)) {
        _iwbemLocator = nullptr;
        CoUninitialize();
        throw WMIError(
            "WMI initialization failed. Failed to create IWbemLocator object", hRes
        );
    }

    LDEBUG("IWbemLocator object successfully created.");

    hRes = _iwbemLocator->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
        nullptr,                 // User name. NULL = current user
        nullptr,                 // User password. NULL = current
        nullptr,                 // Locale. NULL indicates current
        0,                       // Security flags.
        nullptr,                 // Authority (for example, Kerberos)
        nullptr,                 // Context object
        &_iwbemServices          // pointer to IWbemServices proxy
    );
    if (FAILED(hRes)) {
        _iwbemLocator->Release();
        _iwbemLocator = nullptr;
        CoUninitialize();
        _iwbemServices = nullptr;
        throw WMIError(
            "WMI initialization failed. Failed to connect to WMI server", hRes
        );
    }

    LDEBUG("Connected to ROOT\\CIMV2 WMI namespace.");

    hRes = CoSetProxyBlanket(
        _iwbemServices,              // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        nullptr,                     // Server principal name
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        nullptr,                     // client identity
        EOAC_NONE                    // proxy capabilities
    );
    if (FAILED(hRes)) {
        _iwbemServices->Release();
        _iwbemServices = nullptr;
        _iwbemLocator->Release();
        _iwbemLocator = nullptr;
        CoUninitialize();
        throw WMIError("WMI initialization failed. Could not set proxy blanket", hRes);
    }
    LDEBUG("WMI successfully initialized.");
}

void SystemCapabilitiesComponent::deinitializeWMI() {
    constexpr const char* _loggerCat = "SystemCapabilitiesComponent.WMI";
    ghoul_assert(isWMIInitialized(), "WMI must have been initialized");

    LDEBUG("Deinitializing WMI.");
    if (_iwbemLocator) {
        _iwbemLocator->Release();
    }
    _iwbemLocator = nullptr;
    if (_iwbemServices) {
        _iwbemServices->Release();
    }
    _iwbemServices = nullptr;

    CoUninitialize();
}

std::wstring str2wstr(const std::string& str) {
    int stringLength = static_cast<int>(str.length() + 1);
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), stringLength, nullptr, 0);
    std::vector<wchar_t> buf(len);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), stringLength, buf.data(), len);
    return std::wstring(buf.begin(), buf.end());
}

std::string wstr2str(const std::wstring& wstr) {
    int stringLength = static_cast<int>(wstr.length() + 1);
    int len = WideCharToMultiByte(
        CP_ACP,
        0,
        wstr.c_str(),
        stringLength,
        nullptr,
        0,
        nullptr,
        nullptr
    );
    std::vector<char> buf(len);
    WideCharToMultiByte(
        CP_ACP,
        0,
        wstr.c_str(),
        stringLength,
        buf.data(),
        len,
        nullptr,
        nullptr
    );
    return std::string(buf.begin(), buf.end());
}

bool SystemCapabilitiesComponent::isWMIInitialized() {
    return (_iwbemLocator && _iwbemServices);
}

VARIANT* SystemCapabilitiesComponent::queryWMI(const std::string& wmiClass,
                                               const std::string& attribute)
{
    ghoul_assert(isWMIInitialized(), "WMI must have been initialized");
    ghoul_assert(!wmiClass.empty(), "wmiClass must not be empty");
    ghoul_assert(!attribute.empty(), "Attribute must not be empty");

    VARIANT* result = nullptr;
    IEnumWbemClassObject* enumerator = nullptr;
    std::string query = "SELECT " + attribute + " FROM " + wmiClass;
    HRESULT hRes = _iwbemServices->ExecQuery(
        bstr_t("WQL"),
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator
    );
    if (FAILED(hRes)) {
        throw WMIError("WMI query failed", hRes);
    }

    IWbemClassObject* pclsObject = nullptr;
    ULONG returnValue = 0;
    HRESULT hr = S_OK;
    if (enumerator) {
        hr = enumerator->Next(WBEM_INFINITE, 1, &pclsObject, &returnValue );
        if (!FAILED(hRes) && returnValue) {
            result = new VARIANT;
            hr = pclsObject->Get(
                LPCWSTR(str2wstr(attribute).c_str()),
                0,
                result,
                nullptr,
                nullptr
            );
        }
    }

    if (FAILED(hr)) {
        if (result) {
            VariantClear(result);
        }
        throw WMIError("No WMI query result", hr);
    }

    if (enumerator) {
        enumerator->Release();
    }
    if (pclsObject) {
        pclsObject->Release();
    }

    return result;
}

void SystemCapabilitiesComponent::queryWMI(const std::string& wmiClass,
                                           const std::string& attribute,
                                           std::string& value)
{
    VARIANT* variant = queryWMI(wmiClass, attribute);
    value = wstr2str(std::wstring(variant->bstrVal));
    VariantClear(variant);
}

void SystemCapabilitiesComponent::queryWMI(const std::string& wmiClass,
                                           const std::string& attribute, int& value)
{
    VARIANT* variant = queryWMI(wmiClass, attribute);
    value = variant->intVal;
    VariantClear(variant);
}

void SystemCapabilitiesComponent::queryWMI(const std::string& wmiClass,
                                           const std::string& attribute,
                                           unsigned int& value)
{
    VARIANT* variant = queryWMI(wmiClass, attribute);
    value = variant->uintVal;
    VariantClear(variant);
}

void SystemCapabilitiesComponent::queryWMI(const std::string& wmiClass,
                                           const std::string& attribute,
                                           unsigned long long& value)
{
    VARIANT* variant = queryWMI(wmiClass, attribute);
    value = variant->ullVal;
    VariantClear(variant);
}

#endif

} // namespace ghoul::systemcapabilities
