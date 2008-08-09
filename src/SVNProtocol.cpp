// SVNProtocol.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"
#include "SVNProtocol.h"
#include "svn_dso.h"


class CSVNProtocolModule : public CAtlDllModuleT< CSVNProtocolModule >
{
public :
	DECLARE_LIBID(LIBID_SVNProtocolLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_SVNPROTOCOL, "{E066CD38-CD6D-4fde-A58D-54F1A81B271E}")
};

CSVNProtocolModule _AtlModule;


#ifdef _MANAGED
#pragma managed(push, off)
#endif

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		apr_initialize();
		svn_dso_initialize();
		break;
	case DLL_PROCESS_DETACH:
		apr_terminate();
		break;
	}
    return _AtlModule.DllMain(dwReason, lpReserved); 
}

#ifdef _MANAGED
#pragma managed(pop)
#endif




// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}


// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllRegisterServer();
	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
	return hr;
}

