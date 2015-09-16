/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2009 Christian Kindahl
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
#include <ckcore/filestream.hh>
#include <ckcore/directory.hh>
#include <ckcore/convert.hh>
#include <ckcore/linereader.hh>
#include "./Common/StringUtil.h"
#include "StringTable.h"
#include "AudioUtil.h"
#include "Settings.h"
#include "CdText.h"
#include "LangUtil.h"
#include "ProjectManager.h"

CProjectManager::CFileTransaction::CFileTransaction(eMode Mode) : m_Mode(Mode)
{
}
CProjectManager::CFileTransaction::CFileTransaction(CTreeManager *pTreeManager)
{
    m_pTreeManager=pTreeManager;
}
CProjectManager::CFileTransaction::~CFileTransaction()
{
}

bool CProjectManager::CFileTransaction::AddDataFile(CProjectNode *pParentNode,const TCHAR *szFileName,
													const TCHAR *szFilePath,const TCHAR *szFullPath,
													FILETIME *pFileTime,unsigned __int64 uiSize)
{
	// Make sure that a file with this name does not already exist.
	CItemData *pExistingItemData = GetTreeManager()->GetChildItem(pParentNode,szFileName);
	if (pExistingItemData != NULL)
	{
		/*if (m_ReplaceDlg.Execute(szFullPath,pExistingItemData))
			g_ProjectManager.RemoveFile(pParentNode,pExistingItemData);
		else
			return false;*/
	}

	// Add the new item.
	CItemData *pItemData = new CItemData();
	if (m_Mode == MODE_IMPORT)
		pItemData->ucFlags |= PROJECTITEM_FLAG_ISIMPORTED | PROJECTITEM_FLAG_ISLOCKED;

	// Paths.
	pItemData->SetFileName(szFileName);
	pItemData->SetFilePath(szFilePath);
	lstrcpy(pItemData->szFullPath,szFullPath);

	// File type.
	SHFILEINFO shFileInfo;
	if (SHGetFileInfo(szFileName,FILE_ATTRIBUTE_NORMAL,&shFileInfo,
		sizeof(shFileInfo),SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME))
	{
		lstrcpy(pItemData->szFileType,shFileInfo.szTypeName);
	}

	// File time.
	FILETIME LocalFileTime;
	if (FileTimeToLocalFileTime(pFileTime,&LocalFileTime) == TRUE)
		FileTimeToDosDateTime(&LocalFileTime,&pItemData->usFileDate,&pItemData->usFileTime);

	// File size.
	pItemData->uiSize = uiSize;

	pParentNode->m_Files.push_back(pItemData);

	// Increase the total length counter.
	//g_ProjectManager.m_pSpaceMeter->IncreaseAllocatedSize(pItemData->uiSize);
	//g_ProjectManager.m_pSpaceMeter->ForceRedraw();

	return true;
}

CItemData *CProjectManager::CFileTransaction::
	AddDataFile(CProjectNode *pParentNode,const TCHAR *szFullPath)
{
	// Make sure that a file with this name does not already exist.
	TCHAR szFileName[MAX_PATH];
	lstrcpy(szFileName,szFullPath);
	ExtractFileName(szFileName);

	CItemData *pExistingItemData = GetTreeManager()->GetChildItem(pParentNode,szFileName);
	if (pExistingItemData != NULL)
	{
		/*if (m_ReplaceDlg.Execute(szFullPath,pExistingItemData))
			g_ProjectManager.RemoveFile(pParentNode,pExistingItemData);
		else
			return false;*/
	}

	// Add the new item.
	CItemData *pItemData = new CItemData();
	if (m_Mode == MODE_IMPORT)
		pItemData->ucFlags |= PROJECTITEM_FLAG_ISIMPORTED | PROJECTITEM_FLAG_ISLOCKED;

	// Paths.
	pItemData->SetFileName(szFileName);

	TCHAR *szSafeFilePath = pItemData->BeginEditFilePath();
		lstrcpy(szSafeFilePath,pParentNode->pItemData->GetFilePath());
		lstrcat(szSafeFilePath,pParentNode->pItemData->GetFileName());
		lstrcat(szSafeFilePath,_T("/"));
	pItemData->EndEditFilePath();

	lstrcpy(pItemData->szFullPath,szFullPath);

	// File type.
	SHFILEINFO shFileInfo;
	if (SHGetFileInfo(szFullPath,FILE_ATTRIBUTE_NORMAL,&shFileInfo,
		sizeof(shFileInfo),SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME))
	{
		lstrcpy(pItemData->szFileType,shFileInfo.szTypeName);
	}

	// File time.
	struct tm AccessTime,ModifyTime,CreateTime;
	ckcore::File::time(szFullPath,AccessTime,ModifyTime,CreateTime);
	ckcore::convert::tm_to_dostime(ModifyTime,pItemData->usFileDate,pItemData->usFileTime);

	// File size.
	pItemData->uiSize = ckcore::File::size(szFullPath);

	pParentNode->m_Files.push_back(pItemData);

	// Increase the total length counter.
	//g_ProjectManager.m_pSpaceMeter->IncreaseAllocatedSize(pItemData->uiSize);
	//g_ProjectManager.m_pSpaceMeter->ForceRedraw();

	return pItemData;
}

CProjectNode *CProjectManager::CFileTransaction::
	AddFolder(CProjectNode *pParentNode,const TCHAR *szFolderName,
		const TCHAR *szFolderPath,const TCHAR *szFullPath,
		FILETIME *pFileTime)
{
	// Make sure that this folder does not already exist.
	CProjectNode *pExistingNode = GetTreeManager()->GetChildNode(pParentNode,szFolderName);
	if (pExistingNode != NULL)
		return pExistingNode;

	// Add a new node.
	CProjectNode *pNode = new CProjectNode(pParentNode);
	if (m_Mode == MODE_IMPORT)
		pNode->pItemData->ucFlags |= PROJECTITEM_FLAG_ISIMPORTED | PROJECTITEM_FLAG_ISLOCKED;

	// Paths.
	pNode->pItemData->SetFileName(szFolderName);
	pNode->pItemData->SetFilePath(szFolderPath);
	lstrcpy(pNode->pItemData->szFullPath,szFullPath);

	// File type.
	SHFILEINFO shFileInfo;
	if (SHGetFileInfo(_T(""),FILE_ATTRIBUTE_DIRECTORY,&shFileInfo,
		sizeof(shFileInfo),SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME))
	{
		lstrcpy(pNode->pItemData->szFileType,shFileInfo.szTypeName);
	}
	else
	{
		lstrcpy(pNode->pItemData->szFileType,_T(""));
	}

	// File time.
	FILETIME LocalFileTime;
	if (FileTimeToLocalFileTime(pFileTime,&LocalFileTime) == TRUE)
		FileTimeToDosDateTime(&LocalFileTime,&pNode->pItemData->usFileDate,&pNode->pItemData->usFileTime);

	pParentNode->m_Children.push_back(pNode);

	//g_TreeManager.AddTreeNode(pParentNode->m_hTreeItem,pNode);
	return pNode;
}

CProjectNode *CProjectManager::CFileTransaction::
	AddFolder(CProjectNode *pParentNode,const TCHAR *szFullPath)
{
    if (NULL == pParentNode)
    {
        pParentNode=GetTreeManager()->GetRootNode();
    }

	// Make sure that this folder does not already exist.
	TCHAR szFolderName[MAX_PATH];
	lstrcpy(szFolderName,szFullPath);
	ExtractFileName(szFolderName);

	CProjectNode *pExistingNode = GetTreeManager()->GetChildNode(pParentNode,szFolderName);
	if (pExistingNode != NULL)
		return pExistingNode;

	// Add a new node.
	CProjectNode *pNode = new CProjectNode(pParentNode);
	if (m_Mode == MODE_IMPORT)
		pNode->pItemData->ucFlags |= PROJECTITEM_FLAG_ISIMPORTED | PROJECTITEM_FLAG_ISLOCKED;

	// Paths.
	pNode->pItemData->SetFileName(szFolderName);

	TCHAR *szSafeFilePath = pNode->pItemData->BeginEditFilePath();
		lstrcpy(szSafeFilePath,pParentNode->pItemData->GetFilePath());
		lstrcat(szSafeFilePath,pParentNode->pItemData->GetFileName());
		lstrcat(szSafeFilePath,_T("/"));
	pNode->pItemData->EndEditFilePath();

	lstrcpy(pNode->pItemData->szFullPath,szFullPath);

	// File type.
	SHFILEINFO shFileInfo;
	if (SHGetFileInfo(_T(""),FILE_ATTRIBUTE_DIRECTORY,&shFileInfo,
		sizeof(shFileInfo),SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME))
	{
		lstrcpy(pNode->pItemData->szFileType,shFileInfo.szTypeName);
	}
	else
	{
		lstrcpy(pNode->pItemData->szFileType,_T(""));
	}

	// Directory time.
	struct tm AccessTime,ModifyTime,CreateTime;
	ckcore::Directory::time(szFullPath,AccessTime,ModifyTime,CreateTime);
	ckcore::convert::tm_to_dostime(ModifyTime,pNode->pItemData->usFileDate,
								   pNode->pItemData->usFileTime);

	pParentNode->m_Children.push_back(pNode);

	//g_TreeManager.AddTreeNode(pParentNode->m_hTreeItem,pNode);
	return pNode;
}

CTreeManager * CProjectManager::CFileTransaction::GetTreeManager() const 
{ 
    return m_pTreeManager; 
}
void CProjectManager::CFileTransaction::SetTreeManager(CTreeManager * val) 
{ 
    m_pTreeManager = val; 
}

CProjectSettings * CProjectManager::GetProjectSettings() const 
{ 
    return m_pProjectSettings; 
}
void CProjectManager::SetProjectSettings(CProjectSettings * val)
{ 
    m_pProjectSettings = val; 
}

//
//bool CProjectManager::CFileTransaction::
//	AddAudioFile(CProjectNode *pParentNode,const TCHAR *szFullPath)
//{
//	// Check if M3U play list, in that case import the contents from it.
//	ckcore::Path FullPath(szFullPath);
//	if (!ckcore::string::astrcmpi(FullPath.ext_name().c_str(),ckT("m3u")))
//		return g_ProjectManager.Import(szFullPath);
//
//	bool bEncoded = false;
//	unsigned __int64 uiDuration = 0;
//
//	// If the audio file is not a Wave file, try to find a codec that can handle it.
//	if (GetAudioFormat(szFullPath) != AUDIOFORMAT_WAVE)
//	{
//		// Audio file information.
//		int iNumChannels = -1;
//		int iSampleRate = -1;
//		int iBitRate = -1;
//
//		for (unsigned int i = 0; i < g_CodecManager.m_Codecs.size(); i++)
//		{
//			// We're only interested in decoders.
//			if ((g_CodecManager.m_Codecs[i]->irc_capabilities() & IRC_HAS_DECODER) == 0)
//				continue;
//
//			if (g_CodecManager.m_Codecs[i]->irc_decode_init(szFullPath,iNumChannels,
//				iSampleRate,iBitRate,uiDuration))
//			{
//				// Exit the decoder immediately since we don't want to decode the file yet.
//				g_CodecManager.m_Codecs[i]->irc_decode_exit();
//				bEncoded = true;
//				break;
//			}
//		}
//
//		if (!bEncoded)
//		{
//			lngMessageBox(*g_pMainFrame,FAILURE_UNSUPAUDIO,GENERAL_ERROR,MB_OK | MB_ICONERROR);
//			return false;
//		}
//	}
//
//	CItemData *pItemData = new CItemData();
//	if (m_Mode == MODE_IMPORT)
//		pItemData->ucFlags |= PROJECTITEM_FLAG_ISIMPORTED | PROJECTITEM_FLAG_ISLOCKED;
//
//	// Paths.
//	TCHAR *szSafeFileName = pItemData->BeginEditFileName();
//		lstrcpy(szSafeFileName,szFullPath);
//		ExtractFileName(szSafeFileName);
//	pItemData->EndEditFileName();
//
//	TCHAR *szSafeFilePath = pItemData->BeginEditFilePath();
//		lstrcpy(szSafeFilePath,pParentNode->pItemData->GetFilePath());
//		lstrcat(szSafeFilePath,pParentNode->pItemData->GetFileName());
//		lstrcat(szSafeFilePath,_T("/"));
//	pItemData->EndEditFilePath();
//
//	lstrcpy(pItemData->szFullPath,szFullPath);
//
//	// Track length.
//	if (bEncoded)
//		pItemData->GetAudioData()->uiTrackLength = uiDuration;
//	else
//		pItemData->GetAudioData()->uiTrackLength = GetAudioLength(szFullPath);
//
//	if (g_ProjectManager.m_iProjectType == PROJECTTYPE_MIXED)	// Count using the Mode-1 sector size since the spacemeter in these projects are based on that disc size.
//		pItemData->uiSize = (pItemData->GetAudioData()->uiTrackLength / 1000) * 75 * 2048;
//	else
//		pItemData->uiSize = pItemData->GetAudioData()->uiTrackLength;
//
//	pParentNode->m_Files.push_back(pItemData);
//
//	// Increase the total length counter.
//	g_ProjectManager.m_pSpaceMeter->IncreaseAllocatedSize(pItemData->uiSize);
//	g_ProjectManager.m_pSpaceMeter->ForceRedraw();
//	return true;
//}
//
void CProjectManager::CFileTransaction::AddFilesInFolder(CProjectNode *pParentNode,
														 std::vector<CProjectNode *> &FolderStack)
{
	// Real parent path.
	TCHAR szRealParentPath[MAX_PATH];
	lstrcpy(szRealParentPath,pParentNode->pItemData->szFullPath);
	IncludeTrailingBackslash(szRealParentPath);

	// Search path.
	TCHAR szSearchPath[MAX_PATH];
	lstrcpy(szSearchPath,szRealParentPath);
	lstrcat(szSearchPath,TEXT("*"));

	// Internal parent path.
	TCHAR szParentPath[MAX_PATH];
	lstrcpy(szParentPath,pParentNode->pItemData->GetFilePath());
	lstrcat(szParentPath,pParentNode->pItemData->GetFileName());
	lstrcat(szParentPath,_T("/"));

	WIN32_FIND_DATA FileData;
	HANDLE hFind = FindFirstFile(szSearchPath,&FileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		if (lstrcmp(FileData.cFileName,TEXT(".")) &&
            lstrcmp(FileData.cFileName,TEXT("..")) &&
            !(FileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
		{
			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				TCHAR szFullName[MAX_PATH];
				lstrcpy(szFullName,szRealParentPath);
				lstrcat(szFullName,FileData.cFileName);

				FolderStack.push_back(AddFolder(pParentNode,FileData.cFileName,szParentPath,szFullName,&FileData.ftLastWriteTime));
			}
			else
			{
				TCHAR szFullName[MAX_PATH];
				lstrcpy(szFullName,szRealParentPath);
				lstrcat(szFullName,FileData.cFileName);

				unsigned __int64 uiFileSize = 0;

				if (FileData.nFileSizeHigh == 0)
					uiFileSize = FileData.nFileSizeLow;
				else
					uiFileSize = ((unsigned __int64)FileData.nFileSizeHigh << 32) | FileData.nFileSizeLow;

				AddDataFile(pParentNode,FileData.cFileName,szParentPath,szFullName,&FileData.ftLastWriteTime,uiFileSize);
			}
		}

		while (FindNextFile(hFind,&FileData) != 0) 
		{
			if (lstrcmp(FileData.cFileName,TEXT(".")) &&
                lstrcmp(FileData.cFileName,TEXT("..")) &&
                !(FileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
			{
				if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					TCHAR szFullName[MAX_PATH];
					lstrcpy(szFullName,szRealParentPath);
					lstrcat(szFullName,FileData.cFileName);

					FolderStack.push_back(AddFolder(pParentNode,FileData.cFileName,szParentPath,szFullName,&FileData.ftLastWriteTime));
				}
				else
				{
					TCHAR szFullName[MAX_PATH];
					lstrcpy(szFullName,szRealParentPath);
					lstrcat(szFullName,FileData.cFileName);

					unsigned __int64 uiFileSize = 0;

					if (FileData.nFileSizeHigh == 0)
						uiFileSize = FileData.nFileSizeLow;
					else
						uiFileSize = ((unsigned __int64)FileData.nFileSizeHigh << 32) | FileData.nFileSizeLow;

					AddDataFile(pParentNode,FileData.cFileName,szParentPath,szFullName,&FileData.ftLastWriteTime,uiFileSize);
				}
			}
		}
	}

	FindClose(hFind);
}
//
///**
//	Adds the specified file to specifed node. If pTargetNode is NULL then the
//	file will be added to the current folder (node) in the project view.
//	@param szFullPath the absolute path to a file on the file system.
//	@param pTargetNode the node that the file should be added to.
//	@return true if successfull, otherwise false.
//*/
bool CProjectManager::CFileTransaction::
	AddFile(const TCHAR *szFullPath,CProjectNode *pTargetNode)
{
	// Check if we're realing with a folder.
	if (ckcore::Directory::exist(szFullPath))
	{
		//// We can't add folder to an audio disc.
		//if (g_ProjectManager.m_iViewType != PROJECTVIEWTYPE_DATA)
		//{
		//	lngMessageBox(*g_pMainFrame,ERROR_AUDIOADDFOLDER,GENERAL_ERROR,MB_OK | MB_ICONERROR);
		//	return false;
		//}

		// Add this folder.
		if (pTargetNode == NULL)
			pTargetNode = AddFolder(GetTreeManager()->GetCurrentNode(),szFullPath);
		else
			pTargetNode = AddFolder(pTargetNode,szFullPath);

		std::vector<CProjectNode *> FolderStack;
		AddFilesInFolder(pTargetNode,FolderStack);

		while (FolderStack.size() > 0)
		{ 
			pTargetNode = FolderStack[FolderStack.size() - 1];
			FolderStack.pop_back();

			AddFilesInFolder(pTargetNode,FolderStack);
		}

		//g_TreeManager.Refresh();
		//g_ProjectManager.m_pTreeView->Expand(g_TreeManager.GetRootNode()->m_hTreeItem);
	}
	else
	{
		if (pTargetNode == NULL)
			pTargetNode = GetTreeManager()->GetCurrentNode();

		AddDataFile(pTargetNode,szFullPath);

		/*if (pTargetNode == NULL)
			pTargetNode = g_TreeManager.GetCurrentNode();

		if (g_ProjectManager.m_iViewType == PROJECTVIEWTYPE_DATA &&
			pTargetNode != g_ProjectManager.m_pMixAudioNode)
		{
			AddDataFile(pTargetNode,szFullPath);
		}
		else
		{
			AddAudioFile(pTargetNode,szFullPath);
		}

		g_TreeManager.Refresh();*/
	}

	//g_ProjectManager.m_bModified = true;
	return true;
}

///**
//	Adds the specified file to specifed path in the project.
//	@param szFullPath the absolute path to a file on the file system.
//	@param szProjectPath the target path in the project.
//	@return If successfull, a pointer to the CItemData object of the new item. If
//	unsuccessfull the function returns NULL.
//	@see AddFile(const TCHAR *szFullPath,CProjectNode *pTargetNode)
//*/
CItemData *CProjectManager::CFileTransaction::
	AddFile(const TCHAR *szFullPath,const TCHAR *szProjectPath)
{
	CProjectNode *pParent = GetTreeManager()->AddPath(szProjectPath);
	if (pParent == NULL)
		return NULL;

	return AddDataFile(pParent,szFullPath);
}
//
///**
//	Moves the specified item to the specified new parent node.
//	@param pItemParent the current parent node of pItemData.
//	@param pItemData the item to move.
//	@param pNewParent the new node that pItemData should be moved to.
//	@return true if successfull, otherwise false.
//*/
//bool CProjectManager::CFileTransaction::
//	MoveFile(CProjectNode *pItemParent,CItemData *pItemData,CProjectNode *pNewParent)
//{
//	// We can only move files in data projects.
//	if (g_ProjectManager.m_iViewType != PROJECTVIEWTYPE_DATA)
//		return false;
//
//	// Check if a file or folder already exists in the destination folder.
//	CItemData *pExistingItemData = g_TreeManager.GetChildItem(pNewParent,pItemData->GetFileName());
//	if (pExistingItemData != NULL)
//	{
//		// It does not make sense to move oneself to itself.
//		if (pExistingItemData == pItemData)
//			return false;
//
//		if (m_ReplaceDlg.Execute(pItemData,pExistingItemData))
//			g_ProjectManager.RemoveFile(pNewParent,pExistingItemData);
//		else
//			return false;
//	}
//
//	if (!g_TreeManager.MoveEntry(pItemParent,pItemData,pNewParent))
//		lngMessageBox(*g_pMainFrame,ERROR_MOVESAMESRCDST,GENERAL_ERROR,MB_OK | MB_ICONERROR);
//
//	g_TreeManager.Refresh();
//
//	g_ProjectManager.m_bModified = true;
//	return true;
//}
//
///**
//	Moves the specified item to the current folder (node) in the project view.
//	@param pItemParent the current parent node of pItemData.
//	@param pItemData the item to move.
//	@return true if successfull, otherwise false.
//	@see MoveFile(CProjectNode *pItemParent,CItemData *pItemData,CProjectNode *pNewParent)
//*/
//bool CProjectManager::CFileTransaction::
//	MoveFileToCurrent(CProjectNode *pItemParent,CItemData *pItemData)
//{
//	return MoveFile(pItemParent,pItemData,g_TreeManager.GetCurrentNode());
//}

CProjectManager::CProjectManager()
{
	m_iProjectType = PROJECTTYPE_DATA;
	//m_iProjectMedia = -1;
	/*m_iViewType = -1;
	m_iActiveView = AV_TREE;

	m_pProjectView = NULL;
	m_pContainer = NULL;
	m_pSpaceMeter = NULL;
	m_pListView = NULL;
	m_pTreeView = NULL;*/
}

CProjectManager::~CProjectManager()
{
}

//LRESULT CProjectManager::OnNewFolder(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
//{
//	// Make sure that this action is allowed to happen (accelerators does not care about menu item state).
//	if (g_pMainFrame->UIGetState(ID_EDIT_NEWFOLDER) & g_pMainFrame->UPDUI_DISABLED)
//		return 0;
//
//	switch (m_iActiveView)
//	{
//		case AV_TREE:
//			TreeAddNewFolder(m_pActionNode);
//			break;
//
//		case AV_LIST:
//			ListAddNewFolder();
//			break;
//	};
//
//	m_bModified = true;
//	return 0;
//}
//
//LRESULT CProjectManager::OnRename(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
//{
//	// Make sure that this action is allowed to happen (accelerators does not care about menu item state).
//	if (g_pMainFrame->UIGetState(ID_EDIT_RENAME) & g_pMainFrame->UPDUI_DISABLED)
//		return 0;
//
//	switch (m_iActiveView)
//	{
//		case AV_TREE:
//			// We can't rename locked items.
//			if (!(m_pActionNode->pItemData->ucFlags & PROJECTITEM_FLAG_ISLOCKED))
//				m_pTreeView->EditLabel(m_pActionNode->m_hTreeItem);
//			break;
//
//		case AV_LIST:
//			int iItemIndex = -1;
//			iItemIndex = m_pListView->GetNextItem(iItemIndex,LVNI_SELECTED | LVNI_FOCUSED);
//
//			// Make sure that atleast one item is selected with focus.
//			if (iItemIndex == -1)
//				break;
//
//			CItemData *pItemData = (CItemData *)m_pListView->GetItemData(iItemIndex);
//
//			if (!(pItemData->ucFlags & PROJECTITEM_FLAG_ISLOCKED))
//				m_pListView->EditLabel(iItemIndex);
//			break;
//	};
//
//	return 0;
//}
//
//LRESULT CProjectManager::OnRemove(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
//{
//	// Make sure that this action is allowed to happen (accelerators does not care about menu item state).
//	if (g_pMainFrame->UIGetState(ID_EDIT_REMOVE) & g_pMainFrame->UPDUI_DISABLED)
//		return 0;
//
//	// We need to store the current active view since it will change after the message box has been shown.
//	int iActiveView = m_iActiveView;
//
//	if (lngMessageBox(*g_pMainFrame,CONFIRM_REMOVEITEMS,GENERAL_QUESTION,MB_YESNO | MB_ICONQUESTION) != IDYES)
//		return 0;
//
//	switch (iActiveView)
//	{
//		case AV_TREE:
//			TreeRemoveNode(m_pActionNode);
//			break;
//
//		case AV_LIST:
//			ListRemoveSel();
//			break;
//	};
//
//	m_iActiveView = iActiveView;
//
//	m_bModified = true;
//	return 0;
//}
//
//void CProjectManager::SetupDataListView()
//{
//	m_iViewType = PROJECTVIEWTYPE_DATA;
//
//	// Remove all columns.
//	int iColCount = m_pListView->GetHeader().GetItemCount();
//
//	for (int i = 0; i < iColCount; i++)
//		m_pListView->DeleteColumn(0);
//
//	// Add the new columns.
//	m_pListView->InsertColumn(0,lngGetString(COLUMN_NAME),LVCFMT_LEFT,250,COLUMN_SUBINDEX_NAME);
//	m_pListView->InsertColumn(1,lngGetString(COLUMN_SIZE),LVCFMT_RIGHT,100,COLUMN_SUBINDEX_SIZE);
//	m_pListView->InsertColumn(2,lngGetString(COLUMN_TYPE),LVCFMT_LEFT,180,COLUMN_SUBINDEX_TYPE);
//	m_pListView->InsertColumn(3,lngGetString(COLUMN_MODIFIED),LVCFMT_LEFT,120,COLUMN_SUBINDEX_MODIFIED);
//	m_pListView->InsertColumn(4,lngGetString(COLUMN_PATH),LVCFMT_LEFT,120,COLUMN_SUBINDEX_PATH);
//
//	// Update menu items.
//	EnableAll(ID_EDIT_NEWFOLDER,true,g_pMainFrame->m_hProjListNoSelMenu);
//	EnableAll(ID_EDIT_RENAME,false,g_pMainFrame->m_hProjListSelMenu);
//	EnableAll(ID_EDIT_REMOVE,false);
//
//	// Enable editing of names.
//	unsigned long ulViewStyle = m_pListView->GetWindowLong(GWL_STYLE);
//	m_pListView->SetWindowLong(GWL_STYLE,ulViewStyle | LVS_EDITLABELS);
//
//	// Specifiy the sort-column.
//	SendMessage(m_pListView->GetHeader(),WM_CHC_SETSORTCOLUMN,0,0);
//}
//
//void CProjectManager::SetupAudioListView()
//{
//	m_iViewType = PROJECTVIEWTYPE_AUDIO;
//
//	// Remove all columns.
//	int iColCount = m_pListView->GetHeader().GetItemCount();
//
//	for (int i = 0; i < iColCount; i++)
//		m_pListView->DeleteColumn(0);
//
//	// Add the new columns.
//	m_pListView->InsertColumn(0,lngGetString(COLUMN_TRACK),LVCFMT_LEFT,45,COLUMN_SUBINDEX_TRACK);
//	m_pListView->InsertColumn(1,lngGetString(COLUMN_TITLE),LVCFMT_LEFT,220,COLUMN_SUBINDEX_TITLE);
//	m_pListView->InsertColumn(2,lngGetString(COLUMN_LENGTH),LVCFMT_LEFT,60,COLUMN_SUBINDEX_LENGTH);
//	m_pListView->InsertColumn(3,lngGetString(COLUMN_LOCATION),LVCFMT_LEFT,450,COLUMN_SUBINDEX_LOCATION);
//
//	// Update menu items.
//	EnableAll(ID_EDIT_NEWFOLDER,false,g_pMainFrame->m_hProjListNoSelMenu);
//	EnableAll(ID_EDIT_RENAME,false,g_pMainFrame->m_hProjListSelMenu);
//	EnableAll(ID_EDIT_REMOVE,false);
//
//	// Disable editing of names.
//	unsigned long ulViewStyle = m_pListView->GetWindowLong(GWL_STYLE);
//	m_pListView->SetWindowLong(GWL_STYLE,ulViewStyle & ~LVS_EDITLABELS);
//
//	// Specifiy the sort-column.
//	SendMessage(m_pListView->GetHeader(),WM_CHC_SETSORTCOLUMN,1,0);
//}
//
///**
//	Assigns graphical controls to the project manage so that they will be
//	properly updated when the project changes.
//	@param pProjectView project splitter view control.
//	@param pContainer project list view container control.
//	@param pSpaceMeter space meter control.
//	@param pListView project list view control.
//	@param pTreeView project tree view control.
//*/
//void CProjectManager::AssignControls(CSplitterWindow *pProjectView,CCustomContainer *pContainer,
//									 CSpaceMeter *pSpaceMeter,CListViewCtrl *pListView,
//									 CTreeViewCtrlEx *pTreeView)
//{
//	m_pProjectView = pProjectView;
//	m_pContainer = pContainer;
//	m_pSpaceMeter = pSpaceMeter;
//	m_pListView = pListView;
//	m_pTreeView = pTreeView;
//}
//
///**
//	Creates a new empty data project.
//	@param bDVD specifies whether the project should be recorded to DVD media or
//	not.
//*/
void CProjectManager::NewDataProject(/*int iDiscMedia*/)
{
	// Reset the old project settings.
	GetProjectSettings()->Reset();

	//m_bModified = false;

	/*if (m_pProjectView != NULL)
		m_pProjectView->SetSinglePaneMode(SPLIT_PANE_NONE);*/

	CloseProject();

	m_iProjectType = PROJECTTYPE_DATA;
	//m_iProjectMedia = iDiscMedia;

	/*if (m_pProjectView != NULL)
		SetupDataListView();*/

	// Get system date and time.
	SYSTEMTIME st;
	GetLocalTime(&st);
	TCHAR szDate[8];
	GetDateFormat(LOCALE_USER_DEFAULT,0,&st,_T("yyMMdd_"),szDate,8);
	TCHAR szDateTime[12];
	lsprintf(szDateTime,_T("%s%.2d%.2d"),szDate,st.wHour,st.wMinute);

	// Insert tree root item.
	GetTreeManager()->CreateTree(szDateTime,PROJECTTREE_IMAGEINDEX_DATA);

	// Update the space meter.
	/*if (m_pSpaceMeter != NULL)
	{
		m_pSpaceMeter->SetDisplayMode(PROJECTVIEWTYPE_DATA);
		m_pSpaceMeter->SetDiscSize(m_iProjectMedia);

		m_pSpaceMeter->SetAllocatedSize(0);
		m_pSpaceMeter->ForceRedraw();
	}*/

	// Project settings.
	//lstrcpy(GetProjectSettings()->m_szLabel,szDateTime);

	// Enable/disable menu items.
	/*g_pMainFrame->UIEnable(ID_BURNCOMPILATION_DISCIMAGE,true);
	g_pMainFrame->UIEnable(ID_ACTIONS_IMPORTSESSION,true);*/
}

///**
//	Creates a mew empty audio project.
//*/
//void CProjectManager::NewAudioProject(int iDiscMedia)
//{
//	// Reset the old project settings.
//	g_ProjectSettings.Reset();
//
//	m_bModified = false;
//
//	if (m_pProjectView != NULL)
//		m_pProjectView->SetSinglePaneMode(SPLIT_PANE_RIGHT);
//
//	CloseProject();
//
//	m_iProjectType = PROJECTTYPE_AUDIO;
//	m_iProjectMedia = iDiscMedia;
//
//	if (m_pListView != NULL)
//		SetupAudioListView();
//
//	// Insert tree root item.
//	g_TreeManager.CreateTree(lngGetString(PROJECT_AUDIO),PROJECTTREE_IMAGEINDEX_AUDIO);
//
//	// Update the space meter.
//	if (m_pSpaceMeter != NULL)
//	{
//		m_pSpaceMeter->SetDisplayMode(PROJECTVIEWTYPE_AUDIO);
//		m_pSpaceMeter->SetDiscSize(m_iProjectMedia);
//		m_pSpaceMeter->SetAllocatedSize(0);
//		m_pSpaceMeter->ForceRedraw();
//	}
//
//	// Project settings.
//	g_ProjectSettings.m_szLabel[0] = '\0';
//
//	// Enable/disable menu items.
//	g_pMainFrame->UIEnable(ID_BURNCOMPILATION_DISCIMAGE,false);
//	g_pMainFrame->UIEnable(ID_ACTIONS_IMPORTSESSION,false);
//}
//
///**
//	Creates a new empty mixed mode project.
//*/
//void CProjectManager::NewMixedProject(int iDiscMedia)
//{
//	// Reset the old project settings.
//	g_ProjectSettings.Reset();
//
//	m_bModified = false;
//
//	if (m_pProjectView != NULL)
//		m_pProjectView->SetSinglePaneMode(SPLIT_PANE_NONE);
//
//	CloseProject();
//
//	m_iProjectType = PROJECTTYPE_MIXED;
//	m_iProjectMedia = iDiscMedia;
//
//	// By default we display the data view.
//	if (m_pListView != NULL)
//		SetupDataListView();
//
//	// Insert tree root item.
//	g_TreeManager.CreateTree(lngGetString(PROJECT_MIXED),PROJECTTREE_IMAGEINDEX_MIXED);
//
//	// Get system date and time.
//	SYSTEMTIME st;
//	GetLocalTime(&st);
//	TCHAR szDate[8];
//	GetDateFormat(LOCALE_USER_DEFAULT,0,&st,_T("yyMMdd_"),szDate,8);
//	TCHAR szDateTime[12];
//	lsprintf(szDateTime,_T("%s%.2d%.2d"),szDate,st.wHour,st.wMinute);
//
//	m_pMixDataNode = g_TreeManager.InsertVirtualRoot(szDateTime,PROJECTTREE_IMAGEINDEX_DATA);
//	m_pMixAudioNode = g_TreeManager.InsertVirtualRoot(lngGetString(PROJECT_AUDIO),PROJECTTREE_IMAGEINDEX_AUDIO);
//
//	if (m_pTreeView != NULL)
//		m_pTreeView->Select(m_pMixDataNode->m_hTreeItem,TVGN_CARET);
//
//	// Update the space meter.
//	if (m_pSpaceMeter != NULL)
//	{
//		m_pSpaceMeter->SetDisplayMode(PROJECTVIEWTYPE_DATA);
//		m_pSpaceMeter->SetDiscSize(m_iProjectMedia);
//		m_pSpaceMeter->SetAllocatedSize(0);
//		m_pSpaceMeter->ForceRedraw();
//	}
//
//	// Project settings.
//	lstrcpy(g_ProjectSettings.m_szLabel,szDateTime);
//
//	// Enable/disable menu items.
//	g_pMainFrame->UIEnable(ID_BURNCOMPILATION_DISCIMAGE,true);
//	g_pMainFrame->UIEnable(ID_ACTIONS_IMPORTSESSION,false);
//}
//
///**
//	Changes to project data view for mixed mode projects.
//*/
//void CProjectManager::DataSelected()
//{
//	if (m_iProjectType != PROJECTTYPE_MIXED)
//		return;
//
//	SetupDataListView();
//}
//
///**
//	Changes to project audio view for mixed mode projects.
//*/
//void CProjectManager::AudioSelected()
//{
//	if (m_iProjectType != PROJECTTYPE_MIXED)
//		return;
//
//	SetupAudioListView();
//}
//
///**
//	Generates a new sub folder name for the specified parent node. For example
//	if a folder named "New Folder" already exists in that parent, the new one
//	should be nameed "New Folder (2)" (unless that one already exists).
//	@param pParent parent node that the new folder should be created in.
//	@param szFolderName the output string that should hold the generated folder
//	name.
//	@param uiFolderNameSize the size of the szFolderName variable (in TCHARs).
//	@return true if successfull, false otherwise.
//*/
//bool CProjectManager::GenerateNewFolderName(CProjectNode *pParent,TCHAR *szFolderName,
//											unsigned int uiFolderNameSize)
//{
//	if (g_TreeManager.GetChildItem(pParent,lngGetString(MISC_NEWFOLDER)) != NULL)
//	{
//		TCHAR szFolderNamePattern[64];
//
//		size_t iMaxLen = sizeof(szFolderNamePattern) - (1 + 10 + 3);
//		if ((size_t)lstrlen(lngGetString(MISC_NEWFOLDER)) > iMaxLen)
//			lstrncpy(szFolderNamePattern,lngGetString(MISC_NEWFOLDER),iMaxLen);
//		else
//			lstrcpy(szFolderNamePattern,lngGetString(MISC_NEWFOLDER));
//		lstrcat(szFolderNamePattern,_T(" (%u)"));
//
//		for (unsigned int i = 2; i < 0xFFFFFFFF; i++)
//		{
//			lsprintf(szFolderName,szFolderNamePattern,i);
//			if (!g_TreeManager.GetChildItem(pParent,szFolderName) != NULL)
//				return true;
//		}
//
//		return false;
//	}
//	else
//	{
//		if ((unsigned int)lstrlen(lngGetString(MISC_NEWFOLDER)) >= uiFolderNameSize)
//			lstrncpy(szFolderName,lngGetString(MISC_NEWFOLDER),uiFolderNameSize - 1);
//		else
//			lstrcpy(szFolderName,lngGetString(MISC_NEWFOLDER));
//	}
//
//	return true;
//}
//
///**
//	Adds a new empty folder to the current folder (node) in the project through
//	the project list view.
//*/
//bool CProjectManager::ListAddNewFolder()
//{
//	CProjectNode *pCurNode = g_TreeManager.GetCurrentNode();
//
//	TCHAR szFolderName[64];
//	if (!GenerateNewFolderName(pCurNode,szFolderName,sizeof(szFolderName)))
//		return false;
//
//	// Create the new node.
//	CProjectNode *pNode = new CProjectNode(pCurNode);
//
//	// Paths.
//	pNode->pItemData->SetFileName(szFolderName);
//
//	TCHAR *szSafeFilePath = pNode->pItemData->BeginEditFilePath();
//		g_TreeManager.GetCurrentPath(szSafeFilePath);
//	pNode->pItemData->EndEditFilePath();
//
//	// File type.
//	SHFILEINFO shFileInfo;
//	if (SHGetFileInfo(_T(""),FILE_ATTRIBUTE_DIRECTORY,&shFileInfo,
//		sizeof(shFileInfo),SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME))
//	{
//		lstrcpy(pNode->pItemData->szFileType,shFileInfo.szTypeName);
//	}
//	else
//	{
//		lstrcpy(pNode->pItemData->szFileType,_T(""));
//	}
//
//	// File time.
//	SYSTEMTIME SystemTime;
//	GetLocalTime(&SystemTime);
//
//	FILETIME FileTime;
//	SystemTimeToFileTime(&SystemTime,&FileTime);
//
//	FileTimeToDosDateTime(&FileTime,&pNode->pItemData->usFileDate,&pNode->pItemData->usFileTime);
//
//	// Add the new node as a child to the current.
//	pCurNode->m_Children.push_back(pNode);
//
//	g_TreeManager.AddTreeNode(pCurNode->m_hTreeItem,pNode);
//	g_TreeManager.Refresh();
//
//	m_pTreeView->Expand(pCurNode->m_hTreeItem);
//	return true;
//}
//
///**
//	Adds a new empty folder to the current folder (node) in the project through
//	the project tree view.
//*/
//bool CProjectManager::TreeAddNewFolder(CProjectNode *pParentNode)
//{
//	TCHAR szFolderName[64];
//	if (!GenerateNewFolderName(pParentNode,szFolderName,sizeof(szFolderName)))
//		return false;
//
//	// Create the new node.
//	CProjectNode *pNode = new CProjectNode(pParentNode);
//
//	// Paths.
//	pNode->pItemData->SetFileName(szFolderName);
//
//	TCHAR *szSafeFilePath = pNode->pItemData->BeginEditFilePath();
//		lstrcpy(szSafeFilePath,pParentNode->pItemData->GetFilePath());
//		lstrcat(szSafeFilePath,pParentNode->pItemData->GetFileName());
//		lstrcat(szSafeFilePath,_T("/"));
//	pNode->pItemData->EndEditFilePath();
//
//	// File type.
//	SHFILEINFO shFileInfo;
//	if (SHGetFileInfo(_T(""),FILE_ATTRIBUTE_DIRECTORY,&shFileInfo,
//		sizeof(shFileInfo),SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME))
//	{
//		lstrcpy(pNode->pItemData->szFileType,shFileInfo.szTypeName);
//	}
//	else
//	{
//		lstrcpy(pNode->pItemData->szFileType,_T(""));
//	}
//
//	// File time.
//	SYSTEMTIME SystemTime;
//	GetLocalTime(&SystemTime);
//
//	FILETIME FileTime;
//	SystemTimeToFileTime(&SystemTime,&FileTime);
//
//	FileTimeToDosDateTime(&FileTime,&pNode->pItemData->usFileDate,&pNode->pItemData->usFileTime);
//
//	pParentNode->m_Children.push_back(pNode);
//
//	g_TreeManager.AddTreeNode(pParentNode->m_hTreeItem,pNode);
//	g_TreeManager.Refresh();
//
//	m_pTreeView->Expand(pParentNode->m_hTreeItem);
//	return true;
//}
//
///**
//	Removes the specified file from the specified parent folder (node).
//	@param pParentNode parent of the folder file to be removed from the project.
//	@param pItemData item that should be removed.
//*/
//void CProjectManager::RemoveFile(CProjectNode *pParentNode,CItemData *pItemData)
//{
//	// Update the space meter.
//	m_pSpaceMeter->DecreaseAllocatedSize(pItemData->uiSize);
//	m_pSpaceMeter->ForceRedraw();
//
//	// Remove the items.
//	g_TreeManager.RemoveEntry(pParentNode,pItemData);
//}
//
///**
//	Removes the selected files and folders (in the list view) from the project.
//*/
//void CProjectManager::ListRemoveSel()
//{
//	int iItemIndex = -1;
//	iItemIndex = m_pListView->GetNextItem(iItemIndex,LVNI_SELECTED);
//
//	while (iItemIndex != -1)
//	{
//		CItemData *pItemData = (CItemData *)m_pListView->GetItemData(iItemIndex);
//
//		// If the item is locked, skip it.
//		if (pItemData->ucFlags & PROJECTITEM_FLAG_ISLOCKED)
//		{
//			iItemIndex = m_pListView->GetNextItem(iItemIndex,LVNI_SELECTED);
//			continue;
//		}
//
//		// Update the space meter.
//		if (pItemData->ucFlags & PROJECTITEM_FLAG_ISFOLDER)
//			m_pSpaceMeter->DecreaseAllocatedSize(g_TreeManager.GetNodeSize(g_TreeManager.GetCurrentNode(),pItemData));
//		else
//			m_pSpaceMeter->DecreaseAllocatedSize(pItemData->uiSize);
//
//		m_pSpaceMeter->ForceRedraw();
//
//		// Remove the items.
//		g_TreeManager.RemoveEntry(g_TreeManager.GetCurrentNode(),pItemData);
//
//		iItemIndex = m_pListView->GetNextItem(iItemIndex,LVNI_SELECTED);
//	}
//
//	g_TreeManager.Refresh();
//}
//
///**
//	Removes the selected folder (in the tree view) from the project.
//*/
//void CProjectManager::TreeRemoveNode(CProjectNode *pNode)
//{
//	// It's not possible to remove locked items.
//	if (pNode->pItemData->ucFlags & PROJECTITEM_FLAG_ISLOCKED)
//		return;
//
//	// Update the space meter.
//	m_pSpaceMeter->DecreaseAllocatedSize(g_TreeManager.GetNodeSize(pNode));
//	m_pSpaceMeter->ForceRedraw();
//
//	// Remove the node.
//	g_TreeManager.RemoveEntry(pNode);
//	g_TreeManager.Refresh();
//
//	NotifyTreeSelChanged((CProjectNode *)m_pTreeView->GetSelectedItem().GetData());
//}
//
///**
//	Used for notifying the project manager that the list view selection has
//	changed.
//	@param uiSelCount new number of selected item in the list view.
//*/
//void CProjectManager::NotifyListSelChanged(unsigned int uiSelCount)
//{
//	if (m_iViewType == PROJECTVIEWTYPE_DATA)
//	{
//		if (uiSelCount > 0)
//		{
//			EnableAll(ID_EDIT_RENAME,true,g_pMainFrame->m_hProjListSelMenu);
//			EnableAll(ID_EDIT_REMOVE,true,g_pMainFrame->m_hProjListSelMenu);
//		}
//		else
//		{
//			EnableAll(ID_EDIT_RENAME,false,g_pMainFrame->m_hProjListSelMenu);
//			EnableAll(ID_EDIT_REMOVE,false,g_pMainFrame->m_hProjListSelMenu);
//		}
//	}
//	else if (m_iViewType == PROJECTVIEWTYPE_AUDIO)
//	{
//		if (uiSelCount > 0)
//			EnableAll(ID_EDIT_REMOVE,true,g_pMainFrame->m_hProjListSelMenu);
//		else
//			EnableAll(ID_EDIT_REMOVE,false,g_pMainFrame->m_hProjListSelMenu);
//	}
//
//	// Currently the properties menu item only works when right-clicking on the project tree root.
//	g_ProjectManager.EnableAll(ID_POPUPMENU_PROPERTIES,false,g_pMainFrame->m_hProjListSelMenu);
//}
//
///**
//	Used for notifying the project manager that the tree view selection has
//	changed.
//	@param pNode new selected node.
//*/
//void CProjectManager::NotifyTreeSelChanged(CProjectNode *pNode)
//{
//	if (m_iProjectType == PROJECTTYPE_DATA)
//	{
//		g_ProjectManager.EnableAll(ID_EDIT_RENAME,true,g_pMainFrame->m_hProjListSelMenu);
//		g_ProjectManager.EnableAll(ID_EDIT_REMOVE,pNode != g_TreeManager.GetRootNode(),g_pMainFrame->m_hProjListSelMenu);
//	}
//	else if (m_iProjectType == PROJECTTYPE_MIXED)
//	{
//		if (pNode == m_pMixAudioNode)
//		{
//			g_ProjectManager.EnableAll(ID_EDIT_RENAME,false,g_pMainFrame->m_hProjListSelMenu);
//			g_ProjectManager.EnableAll(ID_EDIT_REMOVE,false,g_pMainFrame->m_hProjListSelMenu);
//		}
//		else
//		{
//			g_ProjectManager.EnableAll(ID_EDIT_RENAME,true,g_pMainFrame->m_hProjListSelMenu);
//			g_ProjectManager.EnableAll(ID_EDIT_REMOVE,pNode != m_pMixDataNode,g_pMainFrame->m_hProjListSelMenu);
//		}
//	}
//
//	// Currently the properties menu item only works when right-clicking on the project tree root.
//	if (pNode == g_TreeManager.GetRootNode())
//		g_ProjectManager.EnableAll(ID_POPUPMENU_PROPERTIES,true,g_pMainFrame->m_hProjListSelMenu);
//	else
//		g_ProjectManager.EnableAll(ID_POPUPMENU_PROPERTIES,false,g_pMainFrame->m_hProjListSelMenu);
//}
//
///**
//	Sets the current active (selected) tree node.
//	@param pNode the new node that should be treated as active.
//*/
//void CProjectManager::TreeSetActionNode(CProjectNode *pNode)
//{
//	m_pActionNode = pNode;
//}
//
///**
//	Used for notifying the project manager that the project tree view has been
//	activated and now has focus.
//*/
//void CProjectManager::TreeSetActive()
//{
//	m_iActiveView = AV_TREE;
//}
//
///**
//	Used for notifying the project manager that the project list view has been
//	activated and now has focus.
//*/
//void CProjectManager::ListSetActive()
//{
//	m_iActiveView = AV_LIST;
//}
//
///**
//	Deletes all items imported from multisession discs from the project.
//*/
//void CProjectManager::DeleteImportedItems()
//{
//	switch (m_iProjectType)
//	{
//		case PROJECTTYPE_DATA:
//			g_TreeManager.DeleteImportedItems(g_TreeManager.GetRootNode());
//			break;
//	};
//
//	g_TreeManager.Refresh();
//}
//
///**
//	Closes the project.
//*/
void CProjectManager::CloseProject()
{
	//m_iActiveView = AV_TREE;

	//// Remove all items from the list and tree view.
	//if (m_pTreeView != NULL)
	//	m_pTreeView->DeleteAllItems();
	//if (m_pListView != NULL)
	//	m_pListView->DeleteAllItems();

	GetTreeManager()->DestroyTree();
}
//
//void CProjectManager::EnableAll(int iID,bool bEnable,HMENU hMenu)
//{
//	g_pMainFrame->UIEnable(iID,bEnable);
//	m_pContainer->EnableToolbarButton(iID,bEnable);
//
//	if (hMenu != NULL)
//		::EnableMenuItem(hMenu,iID,bEnable ? MF_ENABLED : MF_GRAYED);
//}
//
///**
//	Returns the current view type.
//	@return the current view type.
//*/
//int CProjectManager::GetViewType()
//{
//	return m_iViewType;
//}
//
///**
//	Returns the current project type.
//	@return the current project type.
//*/
int CProjectManager::GetProjectType()
{
	return m_iProjectType;
}

///**
//	Saves the current project to the specified Xml structure.
//	@param pXml the Xml container which the project should be saved to.
//*/
//void CProjectManager::SaveProjectData(CXmlProcessor *pXml)
//{
//	m_bModified = false;
//
//	switch (m_iProjectType)
//	{
//		case PROJECTTYPE_DATA:
//			pXml->AddElement(_T("Data"),_T(""),true);
//				g_TreeManager.SaveNodeFileData(pXml,g_TreeManager.GetRootNode());
//			pXml->LeaveElement();
//			break;
//
//		case PROJECTTYPE_AUDIO:
//			pXml->AddElement(_T("Audio"),_T(""),true);
//				g_TreeManager.SaveNodeAudioData(pXml,g_TreeManager.GetRootNode());
//			pXml->LeaveElement();
//			break;
//
//		case PROJECTTYPE_MIXED:
//			pXml->AddElement(_T("Data"),_T(""),true);
//				g_TreeManager.SaveNodeFileData(pXml,m_pMixDataNode);
//			pXml->LeaveElement();
//
//			pXml->AddElement(_T("Audio"),_T(""),true);
//				g_TreeManager.SaveNodeAudioData(pXml,m_pMixAudioNode);
//			pXml->LeaveElement();
//			break;
//	};
//}
//
///**
//	Loads the project from the specified Xml structure.
//	@param pXml the Xml container which the project should be loaded from.
//	@return true if the project was successfylly loaded, false otherwise.
//*/
//bool CProjectManager::LoadProjectData(CXmlProcessor *pXml)
//{
//	switch (m_iProjectType)
//	{
//		case PROJECTTYPE_DATA:
//			if (!pXml->EnterElement(_T("Data")))
//				return true;
//
//			g_TreeManager.LoadNodeFileData(pXml,g_TreeManager.GetRootNode());
//
//			pXml->LeaveElement();
//			break;
//
//		case PROJECTTYPE_AUDIO:
//			if (!pXml->EnterElement(_T("Audio")))
//				return true;
//
//			g_TreeManager.LoadNodeAudioData(pXml,g_TreeManager.GetRootNode(),PROJECTTYPE_AUDIO);
//
//			pXml->LeaveElement();
//			break;
//
//		case PROJECTTYPE_MIXED:
//			if (!pXml->EnterElement(_T("Data")))
//				return true;
//
//			g_TreeManager.LoadNodeFileData(pXml,m_pMixDataNode);
//
//			pXml->LeaveElement();
//
//			if (!pXml->EnterElement(_T("Audio")))
//				return true;
//
//			g_TreeManager.LoadNodeAudioData(pXml,m_pMixAudioNode,PROJECTTYPE_MIXED);
//
//			pXml->LeaveElement();
//			break;
//	};
//
//	return true;
//}
//
//void CProjectManager::SaveProjectFileSys(CXmlProcessor *pXml)
//{
//	pXml->AddElement(_T("FileSystem"),_T(""),true);
//		pXml->AddElement(_T("Identifier"),g_ProjectSettings.m_iFileSystem);
//	pXml->LeaveElement();
//}
//
//bool CProjectManager::LoadProjectFileSys(CXmlProcessor *pXml)
//{
//	if (!pXml->EnterElement(_T("FileSystem")))
//		return false;
//
//	pXml->GetSafeElementData(_T("Identifier"),&g_ProjectSettings.m_iFileSystem);
//	pXml->LeaveElement();
//	return true;
//}
//
//void CProjectManager::SaveProjectISO(CXmlProcessor *pXml)
//{
//	pXml->AddElement(_T("ISO"),_T(""),true);
//		pXml->AddElement(_T("Level"),g_ProjectSettings.m_iIsoLevel);
//		pXml->AddElement(_T("Format"),g_ProjectSettings.m_iIsoFormat);
//		pXml->AddElement(_T("DeepDirs"),g_ProjectSettings.m_bDeepDirs);
//		pXml->AddElement(_T("Joliet"),_T(""),true);
//			pXml->AddElementAttr(_T("enable"),g_ProjectSettings.m_bJoliet);
//			pXml->AddElement(_T("LongNames"),g_ProjectSettings.m_bJolietLongNames);
//		pXml->LeaveElement();
//		pXml->AddElement(_T("OmitVerNum"),g_ProjectSettings.m_bOmitVerNum);
//	pXml->LeaveElement();
//}
//
//bool CProjectManager::LoadProjectISO(CXmlProcessor *pXml)
//{
//	if (!pXml->EnterElement(_T("ISO")))
//		return false;
//
//	pXml->GetSafeElementData(_T("Level"),&g_ProjectSettings.m_iIsoLevel);
//	pXml->GetSafeElementData(_T("Format"),&g_ProjectSettings.m_iIsoFormat);
//	pXml->GetSafeElementData(_T("DeepDirs"),&g_ProjectSettings.m_bDeepDirs);
//
//	if (!pXml->EnterElement(_T("Joliet")))
//	{
//		pXml->LeaveElement();
//		return false;
//	}
//
//	pXml->GetSafeElementAttrValue(_T("enable"),&g_ProjectSettings.m_bJoliet);
//	pXml->GetSafeElementData(_T("LongNames"),&g_ProjectSettings.m_bJolietLongNames);
//	pXml->LeaveElement();
//
//	pXml->GetSafeElementData(_T("OmitVerNum"),&g_ProjectSettings.m_bOmitVerNum);
//
//	pXml->LeaveElement();
//	return true;
//}
//
//void CProjectManager::SaveProjectFields(CXmlProcessor *pXml)
//{
//	pXml->AddElement(_T("Fields"),_T(""),true);
//		pXml->AddElement(_T("Publisher"),g_ProjectSettings.m_szPublisher);
//		pXml->AddElement(_T("Preparer"),g_ProjectSettings.m_szPreparer);
//		pXml->AddElement(_T("System"),g_ProjectSettings.m_szSystem);
//		pXml->AddElement(_T("VolumeSet"),g_ProjectSettings.m_szVolumeSet);
//
//		pXml->AddElement(_T("Files"),_T(""),true);
//			pXml->AddElement(_T("Copyright"),g_ProjectSettings.m_szCopyright);
//			pXml->AddElement(_T("Abstract"),g_ProjectSettings.m_szAbstract);
//			pXml->AddElement(_T("Bibliographic"),g_ProjectSettings.m_szBibliographic);
//		pXml->LeaveElement();
//	pXml->LeaveElement();
//}
//
//bool CProjectManager::LoadProjectFields(CXmlProcessor *pXml)
//{
//	if (!pXml->EnterElement(_T("Fields")))
//		return false;
//
//	pXml->GetSafeElementData(_T("Publisher"),g_ProjectSettings.m_szPublisher,127);
//	pXml->GetSafeElementData(_T("Preparer"),g_ProjectSettings.m_szPreparer,127);
//	pXml->GetSafeElementData(_T("System"),g_ProjectSettings.m_szSystem,127);
//	pXml->GetSafeElementData(_T("VolumeSet"),g_ProjectSettings.m_szVolumeSet,127);
//
//	if (!pXml->EnterElement(_T("Files")))
//	{
//		pXml->LeaveElement();
//		return false;
//	}
//
//	pXml->GetSafeElementData(_T("Copyright"),g_ProjectSettings.m_szCopyright,36);
//	pXml->GetSafeElementData(_T("Abstract"),g_ProjectSettings.m_szAbstract,36);
//	pXml->GetSafeElementData(_T("Bibliographic"),g_ProjectSettings.m_szBibliographic,36);
//	pXml->LeaveElement();
//
//	pXml->LeaveElement();
//	return true;
//}
//
//void CProjectManager::SaveProjectBoot(CXmlProcessor *pXml)
//{
//	pXml->AddElement(_T("Boot"),_T(""),true);
//		pXml->AddElement(_T("Images"),_T(""),true);
//			int iCounter = 0;
//			TCHAR szName[32];
//
//			std::list <CProjectBootImage *>::iterator itNodeObject;
//			for (itNodeObject = g_ProjectSettings.m_BootImages.begin(); itNodeObject != g_ProjectSettings.m_BootImages.end(); itNodeObject++)
//			{
//				CProjectBootImage *pBootImage = *itNodeObject;
//				lsnprintf_s(szName,32,_T("Image%d"),iCounter);
//
//				pXml->AddElement(szName,_T(""),true);
//					pXml->AddElement(_T("FullPath"),pBootImage->m_FullPath.c_str());
//					pXml->AddElement(_T("LocalName"),pBootImage->m_LocalName.c_str());
//					pXml->AddElement(_T("Emulation"),pBootImage->m_iEmulation);
//					pXml->AddElement(_T("NoBoot"),pBootImage->m_bNoBoot);
//					pXml->AddElement(_T("LoadSegment"),pBootImage->m_uiLoadSegment);
//					pXml->AddElement(_T("LoadSize"),pBootImage->m_uiLoadSize);
//				pXml->LeaveElement();
//			}
//		pXml->LeaveElement();
//	pXml->LeaveElement();
//}
//
//bool CProjectManager::LoadProjectBoot(CXmlProcessor *pXml)
//{
//	if (!pXml->EnterElement(_T("Boot")))
//		return false;
//
//	if (!pXml->EnterElement(_T("Images")))
//	{
//		pXml->LeaveElement();
//		return false;
//	}
//
//	TCHAR szFileName[MAX_PATH];
//
//	for (unsigned int i = 0; i < pXml->GetElementChildCount(); i++)
//	{
//		if (!pXml->EnterElement(i))
//		{
//			pXml->LeaveElement();
//			pXml->LeaveElement();
//			return false;
//		}
//
//		CProjectBootImage *pBootImage = new CProjectBootImage();
//
//		pXml->GetSafeElementData(_T("FullPath"),szFileName,MAX_PATH - 1);
//		pBootImage->m_FullPath = szFileName;
//
//		pXml->GetSafeElementData(_T("LocalName"),szFileName,MAX_PATH - 1);
//		pBootImage->m_LocalName = szFileName;
//
//		pXml->GetSafeElementData(_T("Emulation"),&pBootImage->m_iEmulation);
//		pXml->GetSafeElementData(_T("NoBoot"),&pBootImage->m_bNoBoot);
//
//		int iLoadSegment = 0;
//		pXml->GetSafeElementData(_T("LoadSegment"),&iLoadSegment);
//		pBootImage->m_uiLoadSegment = static_cast<ckcore::tuint16>(iLoadSegment);
//
//		int iLoadSize = 0;
//		pXml->GetSafeElementData(_T("LoadSize"),&iLoadSize);	
//		pBootImage->m_uiLoadSize = static_cast<ckcore::tuint16>(iLoadSize);
//
//		g_ProjectSettings.m_BootImages.push_back(pBootImage);
//
//		pXml->LeaveElement();
//	}
//
//	pXml->LeaveElement();
//	pXml->LeaveElement();
//	return true;
//}
//
///**
//	Saves the current project to the specified file. The file will be created if
//	it does not exists and overwritten otherwise.
//	@param szFullPath the absolute path to a project file on the file system.
//	@return true if the project was successfully saved, false otherwise.
//*/
//bool CProjectManager::SaveProject(const TCHAR *szFullPath)
//{
//	CXmlProcessor Xml;
//
//	Xml.AddElement(_T("InfraRecorder"),_T(""),true);
//		Xml.AddElement(_T("Project"),_T(""),true);
//			Xml.AddElementAttr(_T("version"),PROJECTMANAGER_FILEVERSION);
//			Xml.AddElementAttr(_T("type"),m_iProjectType);
//			Xml.AddElementAttr(_T("media"),m_iProjectMedia);
//
//			switch (m_iProjectType)
//			{
//				case PROJECTTYPE_DATA:
//					Xml.AddElement(_T("Label"),g_ProjectSettings.m_szLabel);
//					SaveProjectFileSys(&Xml);
//					SaveProjectISO(&Xml);
//					SaveProjectFields(&Xml);
//					SaveProjectBoot(&Xml);
//					break;
//
//				case PROJECTTYPE_AUDIO:
//					Xml.AddElement(_T("AlbumName"),g_ProjectSettings.m_szAlbumName);
//					Xml.AddElement(_T("AlbumArtist"),g_ProjectSettings.m_szAlbumArtist);
//					break;
//
//				case PROJECTTYPE_MIXED:
//					Xml.AddElement(_T("Label"),g_ProjectSettings.m_szLabel);
//					Xml.AddElement(_T("AlbumName"),g_ProjectSettings.m_szAlbumName);
//					Xml.AddElement(_T("AlbumArtist"),g_ProjectSettings.m_szAlbumArtist);
//
//					SaveProjectFileSys(&Xml);
//					SaveProjectISO(&Xml);
//					SaveProjectFields(&Xml);
//					break;
//			};
//
//			SaveProjectData(&Xml);
//		Xml.LeaveElement();
//	Xml.LeaveElement();
//
//	return Xml.Save(szFullPath) == XMLRES_OK;
//}
//
///**
//	Loads project data from the specified file.
//	@param szFullPath the absolute path to a project file on the file system.
//	@return true if the project was successfully loaded, false otherwise.
//*/
//bool CProjectManager::LoadProject(const TCHAR *szFullPath)
//{
//	CXmlProcessor Xml;
//
//	int iResult = Xml.Load(szFullPath);
//	if (iResult != XMLRES_OK && iResult != XMLRES_FILEERROR)
//	{
//		TCHAR szMessage[128];
//		lsnprintf_s(szMessage,128,lngGetString(ERROR_LOADPROJECTXML),iResult);
//
//		MessageBox(*g_pMainFrame,szMessage,lngGetString(GENERAL_ERROR),MB_OK | MB_ICONERROR);
//		return false;
//	}
//
//	if (!Xml.EnterElement(_T("InfraRecorder")))
//		return false;
//
//	if (!Xml.EnterElement(_T("Project")))
//		return false;
//
//	// Version check.
//	int iVersion = 0;
//	Xml.GetSafeElementAttrValue(_T("version"),&iVersion);
//
//	if (iVersion > PROJECTMANAGER_FILEVERSION)
//	{
//		lngMessageBox(*g_pMainFrame,ERROR_PROJECTVERSION,GENERAL_ERROR,MB_OK | MB_ICONERROR);
//		return false;
//	}
//
//	if (iVersion < PROJECTMANAGER_FILEVERSION)
//		lngMessageBox(*g_pMainFrame,WARNING_OLDPROJECT,GENERAL_WARNING,MB_OK | MB_ICONWARNING);
//
//	// Project type.
//	int iType = -1;
//	Xml.GetSafeElementAttrValue(_T("type"),&iType);
//
//	int iMedia = false;
//	Xml.GetSafeElementAttrValue(_T("media"),&iMedia);
//
//	switch (iType)
//	{
//		case PROJECTTYPE_DATA:
//			NewDataProject(iMedia);
//
//			// Label.
//			Xml.GetSafeElementData(_T("Label"),g_ProjectSettings.m_szLabel,MAX_PATH - 1);
//			SetDiscLabel(g_ProjectSettings.m_szLabel);
//
//			// Data information.
//			LoadProjectFileSys(&Xml);
//			LoadProjectISO(&Xml);
//			LoadProjectFields(&Xml);
//			LoadProjectBoot(&Xml);
//			break;
//
//		case PROJECTTYPE_AUDIO:
//			NewAudioProject(iMedia);
//
//			// Album information.
//			Xml.GetSafeElementData(_T("AlbumName"),g_ProjectSettings.m_szAlbumName,159);
//			Xml.GetSafeElementData(_T("AlbumArtist"),g_ProjectSettings.m_szAlbumArtist,159);
//			break;
//
//		case PROJECTTYPE_MIXED:
//			NewMixedProject(iMedia);
//
//			// Label.
//			Xml.GetSafeElementData(_T("Label"),g_ProjectSettings.m_szLabel,MAX_PATH - 1);
//			SetDiscLabel(g_ProjectSettings.m_szLabel);
//
//			// Album information.
//			Xml.GetSafeElementData(_T("AlbumName"),g_ProjectSettings.m_szAlbumName,159);
//			Xml.GetSafeElementData(_T("AlbumArtist"),g_ProjectSettings.m_szAlbumArtist,159);
//
//			// Data information.
//			LoadProjectFileSys(&Xml);
//			LoadProjectISO(&Xml);
//			LoadProjectFields(&Xml);
//			break;
//
//		default:
//			lngMessageBox(*g_pMainFrame,ERROR_LOADPROJECT,GENERAL_ERROR,MB_OK | MB_ICONERROR);
//			return false;
//	}
//
//	// Project data.
//	if (!LoadProjectData(&Xml))
//	{
//		lngMessageBox(*g_pMainFrame,ERROR_LOADPROJECT,GENERAL_ERROR,MB_OK | MB_ICONERROR);
//		return false;
//	}
//
//	// Try to expand the root item for data projects.
//	if (iType == PROJECTTYPE_DATA)
//		m_pTreeView->Expand(g_TreeManager.GetRootNode()->m_hTreeItem);
//
//	// Update the space meter.
//	if (m_pSpaceMeter != NULL)
//		m_pSpaceMeter->SetAllocatedSize(g_TreeManager.GetNodeSize(g_TreeManager.GetRootNode()));
//
//	Xml.LeaveElement();
//	Xml.LeaveElement();
//
//	return true;
//}
//
///**
//	Obtains project information.
//	@param uiFileCount number of files in the project.
//	@param uiFolderCount number of folders in the project.
//	@param uiTrackCount number of tracks in the project.
//*/
//void CProjectManager::GetProjectContents(unsigned __int64 &uiFileCount,unsigned __int64 &uiFolderCount,
//										 unsigned __int64 &uiTrackCount)
//{
//	switch (m_iProjectType)
//	{
//		case PROJECTTYPE_DATA:
//			g_TreeManager.GetNodeContents(g_TreeManager.GetRootNode(),uiFileCount,uiFolderCount);
//			uiTrackCount = 0;
//			break;
//
//		case PROJECTTYPE_AUDIO:
//			g_TreeManager.GetNodeContents(g_TreeManager.GetRootNode(),uiTrackCount,uiFolderCount);
//			uiFileCount = 0;
//			uiFolderCount = 0;
//			break;
//
//		case PROJECTTYPE_MIXED:
//			g_TreeManager.GetNodeContents(m_pMixAudioNode,uiTrackCount,uiFolderCount);
//			g_TreeManager.GetNodeContents(m_pMixDataNode,uiFileCount,uiFolderCount);
//			break;
//	};
//}
//
///**
//	Returns the project size in bytes.
//	@return the project size in bytes.
//*/
//unsigned __int64 CProjectManager::GetProjectSize()
//{
//	return m_pSpaceMeter->GetAllocatedSize();
//}
//
///*
//	CProjectManager::GetProjectAudioSize
//	------------------------------------
//	Returns the number of bytes of CD audio that the project contains.
//*/
//// Working, but not used.
///*unsigned __int64 CProjectManager::GetProjectAudioSize()
//{
//	switch (GetProjectType())
//	{
//	case PROJECTTYPE_AUDIO:
//		return GetProjectSize();
//
//	case PROJECTTYPE_MIXED:
//		return g_TreeManager.GetNodeSize(m_pMixAudioNode);
//	}
//
//	return 0;
//}*/
//
///**
//	Returns the data root node of a mixed mode project.
//	@return the data root node of a mixed mode project.
//*/
CProjectNode *CProjectManager::GetMixDataRootNode()
{
	return m_pMixDataNode;
}

CTreeManager * CProjectManager::GetTreeManager() const 
{ 
    return m_pTreeManager; 
}
void CProjectManager::SetTreeManager(CTreeManager * val) 
{ 
    m_pTreeManager = val; 
}

///**
//	Returns the audio root node of a mixed mode project.
//	@return the audio root node of a mixed mode project.
//*/
//CProjectNode *CProjectManager::GetMixAudioRootNode()
//{
//	return m_pMixAudioNode;
//}
//
///**
//	Updates the specifed list view with all audio files in an audio or mixed mode
//	project.
//	@param pListView the list view which should be updated.
//*/
//void CProjectManager::ListAudioTracks(CListViewCtrl *pListView)
//{
//	if (m_iProjectType == PROJECTTYPE_AUDIO)
//		g_TreeManager.ListNodeFiles(g_TreeManager.GetRootNode(),pListView);
//	else if (m_iProjectType == PROJECTTYPE_MIXED)
//		g_TreeManager.ListNodeFiles(m_pMixAudioNode,pListView);
//}
//
///**
//	Fills the specified vector with the absolute file system paths of all audio
//	tracks in project.
//	@param AudioTracks the target vector which should be updated with the
//	absolute paths.
//*/
//void CProjectManager::GetAudioTracks(std::vector<TCHAR *> &AudioTracks)
//{
//	if (m_iProjectType == PROJECTTYPE_AUDIO)
//		g_TreeManager.GetNodeFullPaths(g_TreeManager.GetRootNode(),AudioTracks);
//	else if (m_iProjectType == PROJECTTYPE_MIXED)
//		g_TreeManager.GetNodeFullPaths(m_pMixAudioNode,AudioTracks);
//}
//
///**
//	Decodes the specified audio file to a file with the specified temporary file
//	path.
//	@param szFullPath absolute path to the file to be decoded.
//	@param szFullTempPath absolute path to the decoded file which should be
//	created.
//	@param pProgress progress feedback object.
//	@return true if successfull, false otherwise.
//*/
//bool CProjectManager::DecodeAudioTrack(const TCHAR *szFullPath,const TCHAR *szFullTempPath,
//									   CAdvancedProgress *pProgress)
//{
//	// Find which codec that can be uses for decoding the source file.
//	CCodec *pDecoder = NULL;
//
//	// Audio file information.
//	int iNumChannels = -1;
//	int iSampleRate = -1;
//	int iBitRate = -1;
//	unsigned __int64 uiDuration = 0;
//
//	for (unsigned int i = 0; i < g_CodecManager.m_Codecs.size(); i++)
//	{
//		// We're only interested in decoders.
//		if ((g_CodecManager.m_Codecs[i]->irc_capabilities() & IRC_HAS_DECODER) == 0)
//			continue;
//
//		if (g_CodecManager.m_Codecs[i]->irc_decode_init(szFullPath,iNumChannels,
//			iSampleRate,iBitRate,uiDuration))
//		{
//			pDecoder = g_CodecManager.m_Codecs[i];
//			break;
//		}
//	}
//
//	if (pDecoder == NULL)
//	{
//		TCHAR szNameBuffer[MAX_PATH];
//		lstrcpy(szNameBuffer,szFullPath);
//		ExtractFileName(szNameBuffer);
//
//		pProgress->notify(ckcore::Progress::ckERROR,lngGetString(ERROR_NODECODER),szNameBuffer);
//		return false;
//	}
//
//	// Find the wave encoder.
//	CCodec *pEncoder = NULL;
//
//	for (unsigned int i = 0; i < g_CodecManager.m_Codecs.size(); i++)
//	{
//		if ((g_CodecManager.m_Codecs[i]->irc_capabilities() & IRC_HAS_ENCODER) == 0)
//			continue;
//
//		if (!lstrcmp(g_CodecManager.m_Codecs[i]->irc_string(IRC_STR_FILEEXT),_T(".wav")))
//		{
//			pEncoder = g_CodecManager.m_Codecs[i];
//			break;
//		}
//	}
//
//	if (pEncoder == NULL)
//	{
//		pProgress->notify(ckcore::Progress::ckERROR,lngGetString(ERROR_WAVECODEC));
//
//		pDecoder->irc_decode_exit();
//		return false;
//	}
//
//	// Initialize the encoder.
//	if (!pEncoder->irc_encode_init(szFullTempPath,iNumChannels,iSampleRate,iBitRate))
//	{
//		pProgress->notify(ckcore::Progress::ckERROR,lngGetString(ERROR_CODECINIT),
//			pEncoder->irc_string(IRC_STR_ENCODER),
//			iNumChannels,iSampleRate,iBitRate,uiDuration);
//
//		pDecoder->irc_decode_exit();
//		return false;
//	}
//
//	// Encode/decode-process.
//	__int64 iBytesRead = 0;
//	unsigned __int64 uiCurrentTime = 0;
//
//	// FIXME: This macro should not be placed here.
//#define ENCODE_BUFFER_FACTOR		1024
//
//	// Allocate buffer memory.
//	unsigned int uiBufferSize = iNumChannels * ((iBitRate / iSampleRate) >> 3) * ENCODE_BUFFER_FACTOR;
//	unsigned char *pBuffer = new unsigned char[uiBufferSize];
//
//	while (true)
//	{
//		iBytesRead = pDecoder->irc_decode_process(pBuffer,uiBufferSize,uiCurrentTime);
//		if (iBytesRead <= 0)
//			break;
//
//		if (pEncoder->irc_encode_process(pBuffer,iBytesRead) < 0)
//		{
//			pProgress->notify(ckcore::Progress::ckERROR,lngGetString(ERROR_ENCODEDATA));
//			break;
//		}
//
//		// Update the progres bar.
//		unsigned char ucPercent = (unsigned char)(((double)uiCurrentTime/uiDuration) * 100);
//		pProgress->set_progress(ucPercent);
//	}
//
//	// Free buffer memory.
//	delete [] pBuffer;
//
//	// Flush.
//	pEncoder->irc_encode_flush();
//	pProgress->set_progress(100);
//
//	// Destroy the codecs.
//	pEncoder->irc_encode_exit();
//	pDecoder->irc_decode_exit();
//
//	TCHAR szNameBuffer[MAX_PATH];
//	lstrcpy(szNameBuffer,szFullPath);
//	ExtractFileName(szNameBuffer);
//
//	pProgress->notify(ckcore::Progress::ckINFORMATION,
//		lngGetString(SUCCESS_DECODETRACK),szNameBuffer);
//
//	return true;
//}
//
///**
//	Decodes any necessary (that are not in a compatible format) track in the
//	specified audio tracks vector. The decoded audio tracks vector will be
//	updated with the new temporary file paths of the decoded tracks. Please
//	note that the caller needs to free the memory allocated for the strings
//	added to the decoded tracks vector.
//	@param AudioTracks vector of aboslute file paths to audio tracks that
//	should be decoded.
//	@param DecodedTracks output vector of absolute file paths to the decoded
//	tracks.
//	@param pProgress progress feedback object.
//	@return true if successfull, false otherwise.
//*/
//bool CProjectManager::DecodeAudioTracks(std::vector<TCHAR *> &AudioTracks,
//										std::vector<TCHAR *> &DecodedTracks,
//										CAdvancedProgress *pProgress)
//{
//	if (pProgress == NULL)
//		return false;
//
//	for (unsigned int i = 0; i < AudioTracks.size(); i++)
//	{
//		// Return if the user has canceled the operaiton.
//		if (pProgress->cancelled())
//			return false;
//
//		// If the track isn't a wave file it needs to be decoded.
//		if (GetAudioFormat(AudioTracks[i]) != AUDIOFORMAT_WAVE)
//		{
//			TCHAR szFileName[MAX_PATH];
//			lstrcpy(szFileName,AudioTracks[i]);
//			ExtractFileName(szFileName);
//
//			// Create temporary file name.
//			TCHAR *szTempName = new TCHAR[MAX_PATH];
//			lstrcpy(szTempName,g_GlobalSettings.m_szTempPath);
//			lstrcat(szTempName,szFileName);
//			ChangeFileExt(szTempName,_T(".wav"));
//
//			// Decode the track.
//			if (DecodeAudioTrack(AudioTracks[i],szTempName,pProgress))
//			{
//				DecodedTracks.push_back(szTempName);
//				AudioTracks[i] = szTempName;
//			}
//			else
//			{
//				delete [] szTempName;
//				return false;
//			}
//		}
//	}
//
//	return true;
//}
//
///**
//	Saves the project CD-Text data to the specified file.
//	@param szFullPath the absolute path of the file that should contain the
//	CD-Text data. The file will be created if it doesn't exist. If it already
//	exists it will be overwritten.
//	@return true if the file was successfully created, false otherwise.
//*/
//bool CProjectManager::SaveCDText(const TCHAR *szFullPath)
//{
//	std::vector<CItemData *> Files;
//
//	switch (m_iProjectType)
//	{
//		case PROJECTTYPE_MIXED:
//			g_TreeManager.GetNodeFiles(m_pMixAudioNode,Files);
//			break;
//
//		case PROJECTTYPE_AUDIO:
//			g_TreeManager.GetNodeFiles(g_TreeManager.GetRootNode(),Files);
//			break;
//	};
//
//	CCdText CdText;
//	return CdText.WriteFileEx(szFullPath,g_ProjectSettings.m_szAlbumName,g_ProjectSettings.m_szAlbumArtist,Files);
//}
//
///**
//	Verifies all files in the specified folder (node). The number of failed
//	readings will be obtained.
//	@param pNode folder (node) containing all files to be verified.
//	@param FolderStack vector of folders (nodes) that will be updated as new
//	sub folders are detected.
//	@param pProgress progress feedback object.
//	@param szFileNameBuffer file name buffer used for improving performance.
//	@param iPathStripLen number of characters that should be stripped from
//	the current project path.
//	@param pCrc32File helper object for calculating a CRC32 checksum.
//	@param uiFailCount number of files that failed the CRC32 verification.
//	@return true if completed (with or without errors), and false if cancelled.
//*/
//bool CProjectManager::VerifyLocalFiles(CProjectNode *pNode,std::vector<CProjectNode *> &FolderStack,
//									   CAdvancedProgress *pProgress,TCHAR *szFileNameBuffer,int iPathStripLen,
//									   ckcore::Progresser &FileProgresser,unsigned __int64 &uiFailCount,
//									   std::map<tstring,tstring> &FilePathMap)
//{
//	ckcore::CrcStream FileCrcStream(ckcore::CrcStream::ckCRC_32);
//
//	TCHAR szStatus[MAX_PATH + 32];
//	tstring FileName;
//
//	TCHAR szDriveLetter[3];
//	szDriveLetter[0] = szFileNameBuffer[0];
//	szDriveLetter[1] = szFileNameBuffer[1];
//	szDriveLetter[2] = '\0';
//
//	std::list <CProjectNode *>::iterator itNodeObject;
//	for (itNodeObject = pNode->m_Children.begin(); itNodeObject != pNode->m_Children.end(); itNodeObject++)
//		FolderStack.push_back(*itNodeObject);
//
//	std::list <CItemData *>::iterator itFileObject;
//	for (itFileObject = pNode->m_Files.begin(); itFileObject != pNode->m_Files.end(); itFileObject++)
//	{
//		CItemData *pItemData = (*itFileObject);
//
//		// Ignore files that are already on the disc. We have nothing to
//		// compare them against.
//		if (pItemData->ucFlags & PROJECTITEM_FLAG_ISIMPORTED)
//			continue;
//
//		// Don't overwrite the drive letter.
//		lstrcpy(szFileNameBuffer + 2,pItemData->GetFilePath() + iPathStripLen);
//		lstrcat(szFileNameBuffer,pItemData->GetFileName());
//
//		lsnprintf_s(szStatus,MAX_PATH + 32,lngGetString(STATUS_VERIFY),szFileNameBuffer);
//		pProgress->set_status(szStatus);
//
//		// Calculate CRC of file on the hard drive.
//		ckcore::FileInStream FileStream1(pItemData->szFullPath);
//		if (!FileStream1.open())
//			return false;
//
//		FileCrcStream.reset();
//		if (!ckcore::stream::copy(FileStream1,FileCrcStream,FileProgresser))
//			return false;
//
//		FileStream1.close();
//		unsigned long ulGoodCrc = FileCrcStream.checksum();
//
//		// Calculate CRC of the file on the disc.
//		FileName = szDriveLetter;
//		FileName.append(FilePathMap[szFileNameBuffer + 2]);
//
//		ckcore::FileInStream FileStream2(FileName.c_str());
//		if (!FileStream2.open())
//			return false;
//
//		FileCrcStream.reset();
//		if (!ckcore::stream::copy(FileStream2,FileCrcStream,FileProgresser))
//			return false;
//
//		FileStream2.close();
//		unsigned long ulTestCrc = FileCrcStream.checksum();
//
//		// Compare the CRC of the file on the disc to the one on the harddrive.
//		if (ulTestCrc != ulGoodCrc)
//		{
//			if (ulTestCrc == 0)
//			{
//				pProgress->notify(ckcore::Progress::ckERROR,
//					lngGetString(FAILURE_VERIFYNOFILE),FileName.c_str() + 3);
//			}
//			else
//			{
//				pProgress->notify(ckcore::Progress::ckERROR,
//					lngGetString(FAILURE_VERIFYREADERROR),/*szFileNameBuffer*/FileName.c_str(),ulTestCrc,ulGoodCrc);
//			}
//
//			uiFailCount++;
//		}
//	}
//
//	return true;
//}

///**
//	 Performs a CRC check comparission on the files on the hard disk and the CD.
//	 @param pProgress progress feedback object.
//	 @param szDriveLetter drive letter of the device containg the CD to be
//	 verified.
//	 @return true of the operation completed successfully (with or without
//	 errors), and false if the operation was cancelled.
//*/
//bool CProjectManager::VerifyCompilation(CAdvancedProgress *pProgress,const TCHAR *szDriveLetter,
//										std::map<tstring,tstring> &FilePathMap)
//{
//	int iPathStripLen = 0;
//
//	CProjectNode *pRootNode = NULL;
//	switch (g_ProjectManager.GetProjectType())
//	{
//		case PROJECTTYPE_MIXED:
//			pRootNode = GetMixDataRootNode();
//
//			iPathStripLen = lstrlen(pRootNode->pItemData->GetFileName()) + 1;
//			break;
//
//		default:
//			pRootNode = g_TreeManager.GetRootNode();
//			break;
//	}
//
//	pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(PROGRESS_BEGINVERIFY));
//
//	// It's important the the first two characters contain <drive letter>:.
//	TCHAR szFileNameBuffer[MAX_PATH];
//	lstrcpy(szFileNameBuffer,szDriveLetter);
//
//	unsigned __int64 uiFailCount = 0;
//
//	ckcore::Progresser FileProgresser(*pProgress,g_TreeManager.GetNodeSize(pRootNode) << 1);
//
//	std::vector<CProjectNode *> FolderStack;
//	if (!VerifyLocalFiles(pRootNode,FolderStack,pProgress,szFileNameBuffer,iPathStripLen,FileProgresser,uiFailCount,FilePathMap))
//		return false;
//
//	while (FolderStack.size() > 0)
//	{ 
//		pRootNode = FolderStack[FolderStack.size() - 1];
//		FolderStack.pop_back();
//
//		if (!VerifyLocalFiles(pRootNode,FolderStack,pProgress,szFileNameBuffer,iPathStripLen,FileProgresser,uiFailCount,FilePathMap))
//			return false;
//	}
//	
//	// Display the final message.
//	if (uiFailCount == 0)
//	{
//		pProgress->notify(ckcore::Progress::ckINFORMATION,
//			lngGetString(SUCCESS_VERIFY));
//	}
//	else
//	{
//		pProgress->notify(ckcore::Progress::ckINFORMATION,
//			lngGetString(FAILURE_VERIFY),uiFailCount);
//	}
//
//	return true;
//}
//
///**
//	Updates the label of the current project.
//	@param szLabelName new label.
//*/
//void CProjectManager::SetDiscLabel(TCHAR *szLabelName)
//{
//	switch (m_iProjectType)
//	{
//		case PROJECTTYPE_DATA:
//			if (m_pTreeView != NULL)
//				m_pTreeView->SetItemText(g_TreeManager.GetRootNode()->m_hTreeItem,szLabelName);
//			break;
//
//		case PROJECTTYPE_AUDIO:
//			szLabelName[0] = '\0';
//			break;
//
//		case PROJECTTYPE_MIXED:
//			if (m_pTreeView != NULL)
//				m_pTreeView->SetItemText(m_pMixDataNode->m_hTreeItem,szLabelName);
//			break;
//	};
//}
//
///**
//	Updates the modified status of the current project.
//	@param bModified true if the project has been modified after last save,
//	false otherwise.
//*/
//void CProjectManager::SetModified(bool bModified)
//{
//	m_bModified = bModified;
//}
//
///**
//	Returns the modified status of the current project.
//	@return the modified status of the current project.
//*/
//bool CProjectManager::GetModified()
//{
//	return m_bModified;
//}
//
///**
// * Imports a single file into the project.
// * @param [in] BasePath The base path to use in order to build full path if the
// *                      specified file path is relative.
// * @param [in] FilePath The full or relative file path.
// * @param [in] Transaction The transaction object to update with the new file.
// * @return If successful true is returned, if not false is returned.
// */
//bool CProjectManager::ImportFile(ckcore::Path &BasePath,
//								 ckcore::tstring &FilePath,
//								 CFileTransaction &Transaction)
//{
//	// Check if full file path.
//	bool bFullPath = FilePath.size() > 1 && (FilePath[1] == ':' ||
//		(FilePath[0] == '\\' && FilePath[1] == '\\'));
//
//	if (bFullPath)
//		return Transaction.AddFile(FilePath.c_str());
//
//	// Create full path from relative path.
//	ckcore::Path FullPath = BasePath;
//	FullPath += FilePath.c_str();
//
//	return Transaction.AddFile(FullPath.name().c_str());
//}
//
///**
// * Imports data from a file list (text file containing one file or folder on
// * each line). The file can be in M3U format, supporting comments starting with
// * the '#'-character.
// * @param [in] szFullPath Path to the file list.
// * @return If successful true is returned, if not false is returned.
// */
//bool CProjectManager::Import(const TCHAR *szFullPath)
//{
//	// Open the selected file for reading.
//	ckcore::FileInStream FileStream(szFullPath);
//	if (!FileStream.open())
//		return false;
//
//	// Calculate the base path name in case the listed files have relative paths.
//	ckcore::Path BasePath(szFullPath);
//	BasePath = BasePath.dir_name().c_str();
//
//	CFileTransaction Transaction;
//
//	// Read the file line by line.
//	ckcore::LineReader<ckcore::tchar>::Encoding FileEnc =
//		ckcore::LineReader<ckcore::tchar>::encoding(FileStream);
//	FileStream.seek(0,ckcore::InStream::ckSTREAM_BEGIN);
//
//	// Check if ANSI or UTF-16 encoding.
//	if (FileEnc == ckcore::LineReader<ckcore::tchar>::ckENCODING_ANSI)
//	{
//		ckcore::LineReader<char> LineReader(FileStream);
//		while (!LineReader.end())
//		{
//			ckcore::tstring Line =
//				ckcore::string::ansi_to_auto<1024>(LineReader.read_line().c_str());
//
//			// Skip empty and commented lines.
//			if (Line.empty())
//				continue;
//
//			if (Line[0] == '#')
//				continue;
//
//			// Remove any trailing path delimiters.
//			if (Line[Line.size() - 1] == '\\' || Line[Line.size() - 1] == '/')
//				Line.resize(Line.size() - 1);
//
//			if (!ImportFile(BasePath,Line,Transaction))
//			{
//				TCHAR szBuffer[1024];
//				lsprintf(szBuffer,lngGetString(ERROR_PROJECT_IMPORT_FILE),
//					Line.c_str());
//
//				MessageBox(HWND_DESKTOP,szBuffer,lngGetString(GENERAL_ERROR),
//					MB_OK | MB_ICONERROR);
//			}
//		}
//	}
//	else
//	{
//		ckcore::LineReader<wchar_t> LineReader(FileStream);
//		while (!LineReader.end())
//		{
//			ckcore::tstring Line =
//				ckcore::string::utf16_to_auto<1024>(LineReader.read_line().c_str());
//			
//			// Skip empty and commented lines.
//			if (Line.empty())
//				continue;
//
//			if (Line[0] == '#')
//				continue;
//
//			// Remove any trailing path delimiters.
//			if (Line[Line.size() - 1] == '\\' || Line[Line.size() - 1] == '/')
//				Line.resize(Line.size() - 1);
//
//			if (!ImportFile(BasePath,Line,Transaction))
//			{
//				TCHAR szBuffer[1024];
//				lsprintf(szBuffer,lngGetString(ERROR_PROJECT_IMPORT_FILE),
//					Line.c_str());
//
//				MessageBox(HWND_DESKTOP,szBuffer,lngGetString(GENERAL_ERROR),
//					MB_OK | MB_ICONERROR);
//			}
//		}
//	}
//
//	return true;
//}