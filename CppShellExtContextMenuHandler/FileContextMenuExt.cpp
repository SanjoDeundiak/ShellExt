/****************************** Module Header ******************************\
Module Name:  FileContextMenuExt.cpp
Project:      CppShellExtContextMenuHandler
Copyright (c) Microsoft Corporation.

The code sample demonstrates creating a Shell context menu handler with C++. 

A context menu handler is a shell extension handler that adds commands to an 
existing context menu. Context menu handlers are associated with a particular 
file class and are called any time a context menu is displayed for a member 
of the class. While you can add items to a file class context menu with the 
registry, the items will be the same for all members of the class. By 
implementing and registering such a handler, you can dynamically add items to 
an object's context menu, customized for the particular object.

The example context menu handler adds the menu item "Display File Name (C++)"
to the context menu when you right-click a .cpp file in the Windows Explorer. 
Clicking the menu item brings up a message box that displays the full path 
of the .cpp file.

This source is subject to the Microsoft Public License.
See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma warning(push)
#pragma warning(disable: 4995)
#include <fstream> //for file output

#include <locale> //for char_type toupper(char_type)
#pragma warning(pop)

#include <WinSock2.h>

#include "FileContextMenuExt.h"
#include "resource.h"
#include <strsafe.h>
#include <Shlwapi.h>
#include "boost/filesystem.hpp"




std::locale loc; //locale

using std::endl;

#pragma comment(lib, "shlwapi.lib")


extern HINSTANCE g_hInst;
extern long g_cDllRef;

#define IDM_DISPLAY             0  // The command's identifier offset
#define MAX32BIT 4294967295
#define buffSize 1024*4

FileContextMenuExt::FileContextMenuExt(void) : m_cRef(1), 
    m_pszMenuText(L"&Load list to log.txt"),
    m_pszVerb("cppdisplay"),
    m_pwszVerb(L"cppdisplay"),
    m_pszVerbCanonicalName("CppDisplayFileName"),
    m_pwszVerbCanonicalName(L"CppDisplayFileName"),
    m_pszVerbHelpText("Display File Name (C++)"),
    m_pwszVerbHelpText(L"Display File Name (C++)"),
	m_hMenuBmp(NULL),
	proceededNumber(0)
{
	std::locale::global(std::locale(""));		
	checkSumIsReady = CreateEvent(NULL, true, false, NULL);
    InterlockedIncrement(&g_cDllRef);

    // Load the bitmap for the menu item. 
    // If you want the menu item bitmap to be transparent, the color depth of 
    // the bitmap must not be greater than 8bpp.

    //m_hMenuBmp = LoadImage(g_hInst, MAKEINTRESOURCE(IDB_OK), 
    //    IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADTRANSPARENT);
}

FileContextMenuExt::~FileContextMenuExt(void)
{	
	//std::wofstream debug;
	//debug.open(L"F:\\//debug_Destructor.txt", std::ios_base::out | std::ios_base::app);
	//debug << L"Started" << endl;
    if (m_hMenuBmp)
    {		
        DeleteObject(m_hMenuBmp);
        m_hMenuBmp = NULL;
    }

	for (auto i=m_szSelectedFilesList.begin(); i!=m_szSelectedFilesList.end(); i++)
	{
		//debug << L"Deleting path" << endl;
		delete [] i->path;
		//debug << L"Closing event" << endl;
		CloseHandle(i->checkSumEvent);
		//debug << L"Event is closed" << endl;
		//debug << L"Event address is " << i->checkSum << endl;
	}	

	CloseHandle(checkSumIsReady);

    InterlockedDecrement(&g_cDllRef);
	//debug << L"Finished" << endl;
	//debug.close();
}

bool compareStrings(const fileInfo& s1, const fileInfo& s2)
{	
	wchar_t *st1=s1.path, *st2=s2.path;
	int i=0;
	size_t n1 = 0,n2 = 0;

	StringCchLength(st1, MAX_PATH, &n1);
	StringCchLength(st1, MAX_PATH, &n2);	

	while (i<n1 && i<n2 && std::toupper(st1[i],loc)==std::toupper(st2[i],loc))
		i++;
	return (std::toupper(st1[i],loc)<std::toupper(st2[i],loc));
}


std::wstring TimeToString(const LPSYSTEMTIME creationTime)
{
	std::wstring tempSt, creationTimeSt;

	creationTimeSt.clear();

	tempSt = std::to_wstring(DWORDLONG((*creationTime).wDay));
	creationTimeSt += ((*creationTime).wDay>9) ? tempSt : L"0" + tempSt;	

	tempSt = std::to_wstring(DWORDLONG((*creationTime).wMonth));
	creationTimeSt += L"/" + (((*creationTime).wMonth>9) ? tempSt : L"0" + tempSt);

	tempSt = std::to_wstring(DWORDLONG((*creationTime).wYear));
	creationTimeSt += L"/" + tempSt + L" ";

	tempSt = std::to_wstring(DWORDLONG((*creationTime).wHour));
	creationTimeSt += ((*creationTime).wHour>9) ? tempSt : L"0" + tempSt;

	tempSt = std::to_wstring(DWORDLONG((*creationTime).wMinute));
	creationTimeSt += L":" + (((*creationTime).wMinute>9) ? tempSt : L"0" + tempSt);

	tempSt = std::to_wstring(DWORDLONG((*creationTime).wSecond));
	creationTimeSt += L":" + (((*creationTime).wSecond>9) ? tempSt : L"0" + tempSt);

	return creationTimeSt;
}

void findCheckSum(fileInfo* data)
{		
	//std::wofstream debug;
	//debug.open(L"F:\\//debug_findCheckSum.txt",std::ios_base::out | std::ios_base::app);

	//debug << "Calculating checksum for " << data->path << endl;
	std::fstream file;

	DWORD sum = 0;
	char buff[buffSize];
	DWORD temp[4], T;
	
	file.open(data->path, std::ios_base::binary | std::ios_base::in);

	while (file.good())
	{
		int i=0;
		file.read(buff, sizeof(buff));
		std::streamsize wasRead = file.gcount();
		while (wasRead > i)
		{
			T = 0;
			for (int j = 0; j < 4; j++, i++)
			{
				temp[j] = (i<wasRead) ? buff[i] : 0;
				T += temp[j] << (4-j);
			}
			sum = sum ^ T;
		}
	}		
	
	file.clear();
	file.close();	

	data->checkSum = sum;
	SetEvent(data->checkSumEvent);
	//if (!SetEvent(data->checkSumEvent))
		//debug << "Set Ivent error. ErrorNumber=" << GetLastError() << endl;

	//debug << "Checksum = " << sum << endl;	
	//debug.close();
}

void FileContextMenuExt::threadFunc()
{
	std::list<fileInfo>::iterator it;
	
	while (this->proceededNumber < this->m_szSelectedFilesList.size())
		{
			{
				boost::lock_guard<boost::recursive_mutex> lock(this->m_guard);
				this->proceededNumber++;
				it = (this->currentCheckSum)++;
			}			
			findCheckSum(&*it);		
		}
}

void FileContextMenuExt::OnVerbDisplayFileName(HWND hWnd)
{		
	//std::wofstream debug;
	//debug.open(L"F:\\//debug_OnVerbDisplayFileName.txt", std::ios_base::app | std::ios_base::out);	

	if (!this->m_szSelectedFilesList.empty())
	{
		//Detecting folder where the first file is
		wchar_t szPath[MAX_PATH], szDir[MAX_PATH];

		size_t i=0;
		StringCchLength(this->m_szSelectedFilesList.begin()->path, MAX_PATH, &i);
	
		while (i>0 && (this->m_szSelectedFilesList.begin()->path)[i]!=L'\\')
			--i;
	
		StringCchCopy(szDir, i + 2, this->m_szSelectedFilesList.begin()->path);
		StringCchPrintf(szPath, ARRAYSIZE(szPath), L"%slog.txt", szDir);

		std::wofstream logFile;	
		logFile.open(szPath, std::ios::app | std::ios::out); 
		
		this->m_szSelectedFilesList.sort(compareStrings);

		if (logFile.good())
			logFile << L"Total " << this->m_szSelectedFilesList.size() << L" file(s)!" <<endl;
		
		WIN32_FILE_ATTRIBUTE_DATA fileInfo;
		SYSTEMTIME creationTime;
		FILETIME localFileTime;
		DWORDLONG highSize, lowSize, fileSize;
		std::wstring creationTimeSt;										

		int threadNumber = boost::thread::hardware_concurrency() - 1;
		if (threadNumber == 0)
			threadNumber++;
		this->currentCheckSum = this->m_szSelectedFilesList.begin();

		boost::thread_group trGroup;
		boost::thread **threads = new boost::thread*[threadNumber];
		for (int i = 0; i < threadNumber; i++)
		{
			threads[i] = new boost::thread(&FileContextMenuExt::threadFunc, this);
			trGroup.add_thread(threads[i]);
		}
		
		//trGroup.join_all();

		for (auto j = this->m_szSelectedFilesList.begin(); 
			logFile.good() 
			&& j != this->m_szSelectedFilesList.end()
			&& GetFileAttributesEx(j->path, GetFileExInfoStandard, &fileInfo);
			j++)
		{								
			FileTimeToLocalFileTime(&fileInfo.ftCreationTime, &localFileTime);
			FileTimeToSystemTime(&localFileTime, &creationTime);

			highSize = fileInfo.nFileSizeHigh;
			lowSize = fileInfo.nFileSizeLow;
			fileSize = lowSize + highSize*MAX32BIT + highSize;					
			//debug << "Waiting for " << j->path << " event" << endl;
			//if(WAIT_OBJECT_0 == WaitForSingleObject(j->checkSumEvent, INFINITE))
			WaitForSingleObject(j->checkSumEvent, INFINITE);
			logFile << j->path					
					<< L" CheckSum: " << std::hex << j->checkSum << std::dec
					<< L" Size: " << boost::filesystem::file_size(j->path) << L" bytes"					
					<< L" Creating time: " << TimeToString(&creationTime)
					<< endl;
			//else debug << "Event waiting error" << endl;
		}		

		//debug << "join_all()" << endl;
		trGroup.join_all();
		//debug << "delete threads" << endl;
		if (threads)
			delete []threads;

		logFile.close();
	}
	//debug.close();
}


#pragma region IUnknown

// Query to the interface the component supported.
IFACEMETHODIMP FileContextMenuExt::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(FileContextMenuExt, IContextMenu),
        QITABENT(FileContextMenuExt, IShellExtInit), 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

// Increase the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) FileContextMenuExt::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

// Decrease the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) FileContextMenuExt::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
    }

    return cRef;
}

#pragma endregion


#pragma region IShellExtInit

// Initialize the context menu handler.
IFACEMETHODIMP FileContextMenuExt::Initialize(
    LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hKeyProgID)
{			    		
	//std::wofstream debug;
	//debug.open(L"F:\\//debug_Initialize.txt", std::ios_base::out | std::ios_base::app);
	//debug << "Инициализация" << endl;
	
	if (!this->m_szSelectedFilesList.empty())
	{
		//debug << "Not empty list" << endl;
		for (auto i = this->m_szSelectedFilesList.begin(); i != this->m_szSelectedFilesList.end(); i++)
		{
			if (i->checkSumEvent) 
				delete i->checkSumEvent;
			CloseHandle(i->checkSumEvent);
		}
	}
	this->m_szSelectedFilesList.clear();

	UINT nFiles;

    if (NULL == pDataObj)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = E_FAIL;

    FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stm;

    // The pDataObj pointer contains the objects being acted upon. In this 
    // example, we get an HDROP handle for enumerating the selected files and 
    // folders.
    if (SUCCEEDED(pDataObj->GetData(&fe, &stm)))
	{

        // Get an HDROP handle.
        HDROP hDrop = static_cast<HDROP>(GlobalLock(stm.hGlobal));
        if (hDrop != NULL)
        {           
			hr = S_OK;
            nFiles = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);						
            if (nFiles > 0)
            {				
                // Get the path of the files.
				size_t nameSize;
				wchar_t temp[MAX_PATH];				
				wchar_t* fileName;

				//debug << nFiles << " elements where received" << endl;

				for (UINT i = 0; i < nFiles; i++)
				{
					
					if (0 == DragQueryFile(hDrop, i, temp, ARRAYSIZE(temp)))
					{
						//debug << "DragQueryError" << endl;
						return E_FAIL;								
					}

					StringCchLength(temp,MAX_PATH,&nameSize);		

					//debug << "Working on " << temp << endl;						

					if (boost::filesystem::exists(temp) && boost::filesystem::is_regular_file(temp))
					{						
						fileName = new wchar_t[nameSize+1];
						StringCchCopy(fileName,nameSize+1,temp);

						fileInfo t;
						t.checkSum = 0;
						t.path = fileName;												
						
						//debug << "Creating event" << endl;
						t.checkSumEvent = CreateEvent(NULL, true, false, NULL);
						//debug << "Created. Address=" << t.checkSumEvent << endl;						
						//debug << "Pushing to the list" << endl;
						this->m_szSelectedFilesList.push_back(t);
						//debug << "Pushing finished" << endl;
					}				
				}							               
            }
			
            GlobalUnlock(stm.hGlobal);
        }
		
        ReleaseStgMedium(&stm);
    }
    	

	// If any value other than S_OK is returned from the method, the context 
    // menu item is not displayed.	
	//debug << "Initialization finished " << hr << endl;
	//debug.close();

	return hr;		    
}

#pragma endregion


#pragma region IContextMenu

//
//   FUNCTION: FileContextMenuExt::QueryContextMenu
//
//   PURPOSE: The Shell calls IContextMenu::QueryContextMenu to allow the 
//            context menu handler to add its menu items to the menu. It 
//            passes in the HMENU handle in the hmenu parameter. The 
//            indexMenu parameter is set to the index to be used for the 
//            first menu item that is to be added.
//
IFACEMETHODIMP FileContextMenuExt::QueryContextMenu(
    HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	//std::wofstream debug;
	//debug.open(L"F:\\//debug_QueryContextMenu.txt", std::ios_base::out | std::ios_base::app);
	//debug << L"Started" << endl;
	
    // If uFlags include CMF_DEFAULTONLY then we should not do anything.
    if (CMF_DEFAULTONLY & uFlags)
    {
        return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
    }

    // Use either InsertMenu or InsertMenuItem to add menu items.
    // Learn how to add sub-menu from:
    // http://www.codeproject.com/KB/shell/ctxextsubmenu.aspx

    MENUITEMINFO mii = { sizeof(mii) };
    mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
    mii.wID = idCmdFirst + IDM_DISPLAY;
    mii.fType = MFT_STRING;
    mii.dwTypeData = m_pszMenuText;
    mii.fState = MFS_ENABLED;
    mii.hbmpItem = static_cast<HBITMAP>(m_hMenuBmp);
    if (!InsertMenuItem(hMenu, indexMenu, TRUE, &mii))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Add a separator.
    MENUITEMINFO sep = { sizeof(sep) };
    sep.fMask = MIIM_TYPE;
    sep.fType = MFT_SEPARATOR;
    if (!InsertMenuItem(hMenu, indexMenu + 1, TRUE, &sep))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Return an HRESULT value with the severity set to SEVERITY_SUCCESS. 
    // Set the code value to the offset of the largest command identifier 
    // that was assigned, plus one (1).
	//debug << L"Success" << endl;
	//debug.close();
    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IDM_DISPLAY + 1));
}


//
//   FUNCTION: FileContextMenuExt::InvokeCommand
//
//   PURPOSE: This method is called when a user clicks a menu item to tell 
//            the handler to run the associated command. The lpcmi parameter 
//            points to a structure that contains the needed information.
//
IFACEMETHODIMP FileContextMenuExt::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
	//std::wofstream debug;
	//debug.open(L"F:\\//debug_InvokeCommand.txt", std::ios_base::out | std::ios_base::app);
	//debug << L"Started" << endl;	
    BOOL fUnicode = FALSE;

    // Determine which structure is being passed in, CMINVOKECOMMANDINFO or 
    // CMINVOKECOMMANDINFOEX based on the cbSize member of lpcmi. Although 
    // the lpcmi parameter is declared in Shlobj.h as a CMINVOKECOMMANDINFO 
    // structure, in practice it often points to a CMINVOKECOMMANDINFOEX 
    // structure. This struct is an extended version of CMINVOKECOMMANDINFO 
    // and has additional members that allow Unicode strings to be passed.
    if (pici->cbSize == sizeof(CMINVOKECOMMANDINFOEX))
    {
        if (pici->fMask & CMIC_MASK_UNICODE)
        {
            fUnicode = TRUE;
        }
    }

    // Determines whether the command is identified by its offset or verb.
    // There are two ways to identify commands:
    // 
    //   1) The command's verb string 
    //   2) The command's identifier offset
    // 
    // If the high-order word of lpcmi->lpVerb (for the ANSI case) or 
    // lpcmi->lpVerbW (for the Unicode case) is nonzero, lpVerb or lpVerbW 
    // holds a verb string. If the high-order word is zero, the command 
    // offset is in the low-order word of lpcmi->lpVerb.

    // For the ANSI case, if the high-order word is not zero, the command's 
    // verb string is in lpcmi->lpVerb. 
    if (!fUnicode && HIWORD(pici->lpVerb))
    {
        // Is the verb supported by this context menu extension?
        if (StrCmpIA(pici->lpVerb, m_pszVerb) == 0)
        {
			//debug << L"ANSI verb-string" << endl;
			//debug.close();
            OnVerbDisplayFileName(pici->hwnd);
        }
        else
        {
            // If the verb is not recognized by the context menu handler, it 
            // must return E_FAIL to allow it to be passed on to the other 
            // context menu handlers that might implement that verb.
            return E_FAIL;
        }
    }

    // For the Unicode case, if the high-order word is not zero, the 
    // command's verb string is in lpcmi->lpVerbW. 
    else if (fUnicode && HIWORD(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW))
    {
        // Is the verb supported by this context menu extension?
        if (StrCmpIW(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW, m_pwszVerb) == 0)
        {
			//debug << L"UNICODE verb-string" << endl;
			//debug.close();
            OnVerbDisplayFileName(pici->hwnd);
        }
        else
        {
            // If the verb is not recognized by the context menu handler, it 
            // must return E_FAIL to allow it to be passed on to the other 
            // context menu handlers that might implement that verb.
            return E_FAIL;
        }
    }

    // If the command cannot be identified through the verb string, then 
    // check the identifier offset.
    else
    {
        // Is the command identifier offset supported by this context menu 
        // extension?
        if (LOWORD(pici->lpVerb) == IDM_DISPLAY)
        {
			//debug << L"offset" << endl;			
            OnVerbDisplayFileName(pici->hwnd);
			//debug << L"Verb is done" << endl;
			//debug.close();
        }
        else
        {
            // If the verb is not recognized by the context menu handler, it 
            // must return E_FAIL to allow it to be passed on to the other 
            // context menu handlers that might implement that verb.
            return E_FAIL;
        }
    }	
	return S_OK;
}


//
//   FUNCTION: CFileContextMenuExt::GetCommandString
//
//   PURPOSE: If a user highlights one of the items added by a context menu 
//            handler, the handler's IContextMenu::GetCommandString method is 
//            called to request a Help text string that will be displayed on 
//            the Windows Explorer status bar. This method can also be called 
//            to request the verb string that is assigned to a command. 
//            Either ANSI or Unicode verb strings can be requested. This 
//            example only implements support for the Unicode values of 
//            uFlags, because only those have been used in Windows Explorer 
//            since Windows 2000.
//
IFACEMETHODIMP FileContextMenuExt::GetCommandString(UINT_PTR idCommand, 
    UINT uFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax)
{
	//std::wofstream debug;
	//debug.open(L"F:\\//debug_GetCommandString.txt", std::ios_base::out | std::ios_base::app);
	//debug << L"Started" << endl;
	
    HRESULT hr = E_INVALIDARG;

    if (idCommand == IDM_DISPLAY)
    {
        switch (uFlags)
        {
        case GCS_HELPTEXTW:
            // Only useful for pre-Vista versions of Windows that have a 
            // Status bar.
            hr = StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, 
                m_pwszVerbHelpText);
            break;

        case GCS_VERBW:
            // GCS_VERBW is an optional feature that enables a caller to 
            // discover the canonical name for the verb passed in through 
            // idCommand.
            hr = StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, 
                m_pwszVerbCanonicalName);
            break;

        default:
            hr = S_OK;
        }
    }

    // If the command (idCommand) is not supported by this context menu 
    // extension handler, return E_INVALIDARG.
	//debug << L"Finished " << hr << endl;
	//debug.close();
    return hr;
}

#pragma endregion