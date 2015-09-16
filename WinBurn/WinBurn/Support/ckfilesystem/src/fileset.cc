#include "ckfilesystem/fileset.hh"
#include "windows.h"
#include <TCHAR.H>

namespace ckfilesystem
{
	void FindAndSet(const TCHAR *pFilePath, FileSet &Files, int inter_path_start)
	{
		ckcore::tstring sInteralPath(pFilePath);
		sInteralPath.erase(0, inter_path_start);
		Files.insert(ckfilesystem::FileDescriptor(sInteralPath.c_str(),
			pFilePath,1,NULL)); // error pData!= NULL
		TCHAR szFind[MAX_PATH];
		WIN32_FIND_DATA FindFileData;
		_tcscpy(szFind,pFilePath);
		_tcscat(szFind,_T("*.*"));
		HANDLE hFind=::FindFirstFile(szFind,&FindFileData);
		if(INVALID_HANDLE_VALUE == hFind)
			return;
		while(TRUE)
		{
			if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if(FindFileData.cFileName[0]!=_T('.'))
				{
					_tcscpy(szFind,pFilePath);
					_tcscat(szFind,_T(""));
					_tcscat(szFind,FindFileData.cFileName);
					_tcscat(szFind, _T("/"));
					FindAndSet(szFind, Files, inter_path_start);
				}
			}
			else
			{
				ckcore::tstring sInteralFilePath(FindFileData.cFileName);
				sInteralFilePath.erase(0, inter_path_start);
				Files.insert(ckfilesystem::FileDescriptor(sInteralFilePath.c_str(),
					FindFileData.cFileName,0,NULL)); // error pData!= NULL
			}
			if(!FindNextFile(hFind,&FindFileData))
				break;
		}
		FindClose(hFind);
	}
	
	// file_path 的格式为 "c:/xin/"
	void GetFileSet(FileSet &Files, ckcore::tstring file_path_)
	{
		ckcore::tstring temp = file_path_;
		temp.erase(temp.length() - 1, 1);
		FindAndSet(file_path_.c_str(), Files, temp.find_last_of(_T('/')));
	}	
};