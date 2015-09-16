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
#include <queue>
#include "./Common/StringUtil.h"
#include "StringTable.h"
#include "TempManager.h"
#include "Settings.h"
//#include "ProjectManager.h"
#include "LangUtil.h"
#include "AudioUtil.h"
//#include "InfraRecorder.h"
#include "TreeManager.h"

CTreeManager g_TreeManager;

// Specifies what column index each data column has.
#define COLUMN_SUBINDEX_NAME				0
#define COLUMN_SUBINDEX_SIZE				1
#define COLUMN_SUBINDEX_TYPE				2
#define COLUMN_SUBINDEX_MODIFIED			3
#define COLUMN_SUBINDEX_PATH				4

// Specifies what column index each audio column has.
#define COLUMN_SUBINDEX_TRACK				0
#define COLUMN_SUBINDEX_TITLE				1
#define COLUMN_SUBINDEX_LENGTH				2
#define COLUMN_SUBINDEX_LOCATION			3

/*
	CItemData
*/
CItemData::CItemData()
{
	m_szFileName[0] = '\0';
	m_szFilePath[0] = '\0';

	m_pAudioData = NULL;
	m_pIsoData = NULL;

	szFullPath[0] = '\0';
	szFileType[0] = '\0';
	usFileDate = 0;
	usFileTime = 0;
	uiSize = 0;

	/*uiTrackLength = 0;
	szTrackTitle[0] = '\0';
	szTrackArtist[0] = '\0';*/

	ucFlags = 0;
}

CItemData::~CItemData()
{
	// Free any optional data.
	if (m_pAudioData != NULL)
	{
		delete m_pAudioData;
		m_pAudioData = NULL;
	}

	if (m_pIsoData != NULL)
	{
		delete m_pIsoData;
		m_pIsoData = NULL;
	}
}

void CItemData::FileNameChanged()
{
	/*if (ucFlags & PROJECTITEM_FLAG_ISBOOTIMAGE)
	{
		CProjectBootImage *pBootImage = g_ProjectSettings.GetBootImage(szFullPath);
		if (pBootImage != NULL)
		{
			pBootImage->m_LocalName = m_szFileName;
		}
	}*/
}

void CItemData::FilePathChanged()
{
	/*if (ucFlags & PROJECTITEM_FLAG_ISBOOTIMAGE)
	{
		CProjectBootImage *pBootImage = g_ProjectSettings.GetBootImage(szFullPath);
		if (pBootImage != NULL)
		{
			pBootImage->m_LocalPath = m_szFilePath;
		}
	}*/
}

void CItemData::SetFileName(const TCHAR *szFileName)
{
	lstrcpy(m_szFileName,szFileName);
	FileNameChanged();
}

const TCHAR *CItemData::GetFileName()
{
	return m_szFileName;
}

void CItemData::SetFilePath(const TCHAR *szFilePath)
{
	lstrcpy(m_szFilePath,szFilePath);
	FilePathChanged();
}

const TCHAR *CItemData::GetFilePath()
{
	return m_szFilePath;
}

TCHAR *CItemData::BeginEditFileName()
{
	return m_szFileName;
}

void CItemData::EndEditFileName()
{
	FileNameChanged();
}

TCHAR *CItemData::BeginEditFilePath()
{
	return m_szFilePath;
}

void CItemData::EndEditFilePath()
{
	FilePathChanged();
}

CItemData::CAudioData *CItemData::GetAudioData()
{
	if (m_pAudioData == NULL)
		m_pAudioData = new CAudioData();

	return m_pAudioData;
}

bool CItemData::HasAudioData()
{
	return m_pAudioData != NULL;
}

CItemData::CIso9660Data *CItemData::GetIsoData()
{
	if (m_pIsoData == NULL)
		m_pIsoData = new CIso9660Data();
	
	return m_pIsoData;
}

bool CItemData::HasIsoData()
{
	return m_pIsoData != NULL;
}

/*
	CProjectNode
*/
void CProjectNode::Sort(unsigned int uiSortColumn,bool bSortUp,bool bSortAudio)
{
	CChildComparator ChildComparator(uiSortColumn,bSortUp,bSortAudio);
	CFileComparator FileComparator(uiSortColumn,bSortUp,bSortAudio);

	m_Children.sort(ChildComparator);
	m_Files.sort(FileComparator);
}

/*
	CChildComparator
*/
bool CChildComparator::operator() (const CProjectNode *pSafeNode1,
								   const CProjectNode *pSafeNode2)
{
	CItemData *pItemData1,*pItemData2;

	if (m_bSortUp)
	{
		pItemData1 = pSafeNode1->pItemData;
		pItemData2 = pSafeNode2->pItemData;
	}
	else
	{
		pItemData1 = pSafeNode2->pItemData;
		pItemData2 = pSafeNode1->pItemData;
	}

	switch (m_uiSortColumn)
	{
		case COLUMN_SUBINDEX_NAME:
			return lstrcmp(pItemData1->GetFileName(),pItemData2->GetFileName()) < 0;

		// No reason to sort these columns, they are all the same.
		case COLUMN_SUBINDEX_TYPE:
		case COLUMN_SUBINDEX_SIZE:
			return false;

		case COLUMN_SUBINDEX_MODIFIED:
			FILETIME ftFileTime1,ftFileTime2;
				DosDateTimeToFileTime(pItemData1->usFileDate,pItemData1->usFileTime,&ftFileTime1);
				DosDateTimeToFileTime(pItemData2->usFileDate,pItemData2->usFileTime,&ftFileTime2);
			return CompareFileTime(&ftFileTime1,&ftFileTime2) < 0;

		case COLUMN_SUBINDEX_PATH:
			return lstrcmp(pItemData1->GetFilePath(),pItemData2->GetFilePath()) < 0;
	}

	return true;
}

/*
	CFileComparator
*/
bool CFileComparator::operator() (const CItemData *pSafeItemData1,
								  const CItemData *pSafeItemData2)
{
	CItemData *pItemData1,*pItemData2;

	if (m_bSortUp)
	{
		pItemData1 = (CItemData *)pSafeItemData1;
		pItemData2 = (CItemData *)pSafeItemData2;
	}
	else
	{
		pItemData1 = (CItemData *)pSafeItemData2;
		pItemData2 = (CItemData *)pSafeItemData1;
	}

	if (!m_bSortAudio)
	{
		switch (m_uiSortColumn)
		{
			case COLUMN_SUBINDEX_NAME:
				return lstrcmp(pItemData1->GetFileName(),pItemData2->GetFileName()) < 0;

			case COLUMN_SUBINDEX_TYPE:
				return lstrcmp(pItemData1->szFileType,pItemData2->szFileType) < 0;

			case COLUMN_SUBINDEX_MODIFIED:
				FILETIME ftFileTime1,ftFileTime2;
					DosDateTimeToFileTime(pItemData1->usFileDate,pItemData1->usFileTime,&ftFileTime1);
					DosDateTimeToFileTime(pItemData2->usFileDate,pItemData2->usFileTime,&ftFileTime2);
				return CompareFileTime(&ftFileTime1,&ftFileTime2) < 0;

			case COLUMN_SUBINDEX_SIZE:
				if (pItemData1->uiSize < pItemData2->uiSize)
					return true;
				else
					return false;

			case COLUMN_SUBINDEX_PATH:
				return lstrcmp(pItemData1->GetFilePath(),pItemData2->GetFilePath()) < 0;
		}
	}
	else
	{
		switch (m_uiSortColumn)
		{
			case COLUMN_SUBINDEX_TRACK:
				return true;

			case COLUMN_SUBINDEX_TITLE:
				{
					// If no title is specified the file name is displayed.
					const TCHAR *szTitle1 = pItemData1->GetAudioData()->szTrackTitle;
					if (szTitle1[0] == '\0')
						szTitle1 = pItemData1->GetFileName();

					const TCHAR *szTitle2 = pItemData2->GetAudioData()->szTrackTitle;
					if (szTitle2[0] == '\0')
						szTitle2 = pItemData2->GetFileName();

					return lstrcmp(szTitle1,szTitle2) < 0;
				}

			case COLUMN_SUBINDEX_LENGTH:
				if (pItemData1->GetAudioData()->uiTrackLength < pItemData2->GetAudioData()->uiTrackLength)
					return true;
				else
					return false;

			case COLUMN_SUBINDEX_LOCATION:
				return lstrcmp(pItemData1->szFullPath,pItemData2->szFullPath) < 0;
		}
	}

	return true;
}



/*
	CTreeManager
*/
CTreeManager::CTreeManager():
m_pRootNode(NULL)
{
	//m_pTreeView = NULL;
	//m_pListView = NULL;
}

CTreeManager::~CTreeManager()
{
	// Destroy the tree if it hasn't been destroyed.
	DestroyTree();
}

//void CTreeManager::AssignControls(CTreeViewCtrlEx *pTreeView,CListViewCtrl *pListView)
//{
//	m_pTreeView = pTreeView;
//	m_pListView = pListView;
//}
//
//HTREEITEM CTreeManager::AddTreeNode(HTREEITEM hParentItem,CProjectNode *pNode)
//{
//	TV_ITEM tvItem = { 0 };
//	TV_INSERTSTRUCT tvInsert = { 0 };
//
//	tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
//	tvItem.pszText = LPSTR_TEXTCALLBACK;
//	tvItem.iImage = PROJECTTREE_IMAGEINDEX_FOLDER;
//	tvItem.lParam = (LPARAM)pNode;
//	tvItem.cChildren = false;
//
//	tvInsert.item = tvItem;
//	tvInsert.hInsertAfter = TVI_LAST;
//	tvInsert.hParent = hParentItem;
//
//	HasChildren(hParentItem,true);
//
//	// Insert the tree item.
//	pNode->m_hTreeItem = m_pTreeView->InsertItem(&tvInsert);
//	return pNode->m_hTreeItem;
//}
//
//HTREEITEM CTreeManager::GetTreeChildFromParent(HTREEITEM hParentItem,TCHAR *szText)
//{
//	HTREEITEM hItem = m_pTreeView->GetChildItem(hParentItem);
//
//	while (hItem)
//	{
//		CProjectNode *pNode = (CProjectNode *)m_pTreeView->GetItemData(hItem);
//
//		if (pNode)
//		{
//			if (!lstrcmp(pNode->pItemData->GetFileName(),szText))
//				return hItem;
//		}
//
//		hItem = m_pTreeView->GetNextVisibleItem(hItem);
//	}
//
//    return NULL;
//}
//
//bool CTreeManager::HasChildren(HTREEITEM hItem,bool bHasChildren)
//{
//	TVITEM tvItem = { 0 };
//	tvItem.hItem = hItem;
//	tvItem.mask = TVIF_CHILDREN;
//	m_pTreeView->GetItem(&tvItem);
//
//	tvItem.cChildren = bHasChildren;
//
//	if (SUCCEEDED(m_pTreeView->SetItem(&tvItem)))
//		return true;
//
//	return false;
//}
//
CProjectNode *CTreeManager::GetDirFromParent(CProjectNode *pParent,const TCHAR *szName)
{
	std::list <CProjectNode *>::iterator itNodeObject;
	for (itNodeObject = pParent->m_Children.begin(); itNodeObject != pParent->m_Children.end(); itNodeObject++)
	{
		CProjectNode *pChild = *itNodeObject;

		if (!lstrcmp(pChild->pItemData->GetFileName(),szName))
            return pChild;
	}

    return m_pRootNode;
}

///*
//	CTreeManager::GetNodeFromPath
//	-----------------------------
//	Returns the CProjectNode that corresponds to the specified path.
//*/
CProjectNode *CTreeManager::GetNodeFromPath(const TCHAR *szPath)
{
    int iLastDelimiter = 0;
    CProjectNode *pCurrentNode = m_pRootNode;

    for (int i = 0; i < lstrlen(szPath); i++)
    {
        if (szPath[i] == '\\' || szPath[i] == '/')
        {
			TCHAR *szSubPath = SubString(szPath,iLastDelimiter,i - iLastDelimiter);
				pCurrentNode = GetDirFromParent(pCurrentNode,szSubPath);
			delete [] szSubPath;

            iLastDelimiter = i + 1;
        }
    }

	// UPDATE: 2007-02-01
    //return pCurrentNode;
	return pCurrentNode == m_pRootNode ? NULL : pCurrentNode;
}

CProjectNode *CTreeManager::AddPath(const TCHAR *szPath)
{
	CProjectNode *pFoundNode = GetNodeFromPath(szPath);
	if (pFoundNode != NULL)
		return pFoundNode;

    return NodalizePath(szPath);
}

///*
//	FIXME: The following function is very similar to GetDirFromParent.
//*/
CProjectNode *CTreeManager::GetChildFromParent(CProjectNode *pParentNode,const TCHAR *szText)
{
	std::list <CProjectNode *>::iterator itNodeObject;
	for (itNodeObject = pParentNode->m_Children.begin(); itNodeObject != pParentNode->m_Children.end(); itNodeObject++)
	{
		CProjectNode *pChildNode = *itNodeObject;

        if (pChildNode)
        {
			if (!lstrcmp(pChildNode->pItemData->GetFileName(),szText))
                return pChildNode;
        }
	}

    return NULL;
}

CProjectNode *CTreeManager::NodalizePath(const TCHAR *szPath)
{
    CProjectNode *pParentNode = m_pRootNode;
	HTREEITEM hParentTreeNode = m_pRootNode->m_hTreeItem;

    int iLastDelimiter = 1;
	int iPathLength = lstrlen(szPath);

	// The first character should always be '\\' or '/'.
    for (int i = 1; i < iPathLength; i++)
    {
        if (szPath[i] == '\\' || szPath[i] == '/')
        {
			TCHAR *szText = SubString(szPath,iLastDelimiter,i - iLastDelimiter);
            CProjectNode *pTemp = GetChildFromParent(pParentNode,szText);

            // Local tree item.
			/*HTREEITEM hTreeTemp = NULL;
			if (m_pTreeView != NULL)
				hTreeTemp = GetTreeChildFromParent(hParentTreeNode,szText);*/

            // Create the new node.
            if (pTemp == NULL)
            {
                CProjectNode *pChildNode = new CProjectNode(pParentNode);

				// Item text.
				pChildNode->pItemData->SetFileName(szText);

				// Get the system icon index.
				SHFILEINFO shFileInfo;

				// HACK: Force a folder icon to be used.
				if (SHGetFileInfo(_T(""),FILE_ATTRIBUTE_DIRECTORY,&shFileInfo,
					sizeof(shFileInfo),SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_TYPENAME))
				{
					pChildNode->iIconIndex = shFileInfo.iIcon;
					lstrcpy(pChildNode->pItemData->szFileType,shFileInfo.szTypeName);
				}

				TCHAR *szFilePath = pChildNode->pItemData->BeginEditFilePath();
				lstrcpy(szFilePath,pParentNode->pItemData->GetFilePath());
				lstrcat(szFilePath,pParentNode->pItemData->GetFileName());
				lstrcat(szFilePath,_T("/"));
				pChildNode->pItemData->EndEditFilePath();

				pParentNode->m_Children.push_back(pChildNode);
                pParentNode = pChildNode;

				//// Local tree item.
				//if (m_pTreeView != NULL)
				//	hParentTreeNode = AddTreeNode(hParentTreeNode,pChildNode);
            }
            else
            {
                pParentNode = pTemp;

				// Local tree item.
				//hParentTreeNode = hTreeTemp;
            }

            iLastDelimiter = i + 1;
			delete [] szText;
        }
    }

    return pParentNode;
}
//
//void CTreeManager::ListNode(CProjectNode *pNode)
//{
//	unsigned int uiItemCount = 0;
//
//	std::list <CProjectNode *>::iterator itNodeObject;
//	for (itNodeObject = pNode->m_Children.begin(); itNodeObject != pNode->m_Children.end(); itNodeObject++)
//	{
//		// Add the item to the listview.
//		LVITEM lvi = { 0 };
//		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
//
//		lvi.iItem = uiItemCount++;
//		lvi.iSubItem = 0;
//		lvi.pszText = LPSTR_TEXTCALLBACK;
//		lvi.cchTextMax = MAX_PATH - 1;
//		lvi.iImage = I_IMAGECALLBACK;
//		lvi.lParam = (LPARAM)((CProjectNode *)*itNodeObject)->pItemData;
//
//		m_pListView->InsertItem(&lvi);
//	}
//
//	std::list <CItemData *>::iterator itFileObject;
//	for (itFileObject = pNode->m_Files.begin(); itFileObject != pNode->m_Files.end(); itFileObject++)
//	{
//		// Add the item to the listview.
//		LVITEM lvi = { 0 };
//		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
//
//		lvi.iItem = uiItemCount++;
//		lvi.iSubItem = 0;
//		lvi.pszText = LPSTR_TEXTCALLBACK;
//		lvi.cchTextMax = MAX_PATH - 1;
//		lvi.iImage = I_IMAGECALLBACK;
//		lvi.lParam = (LPARAM)*itFileObject;
//
//		m_pListView->InsertItem(&lvi);
//	}
//}
//
//void CTreeManager::SelectPath(const TCHAR *szPath)
//{
//    int iLastDelimiter = 0;
//    CProjectNode *pCurrentNode = m_pRootNode;
//
//	m_pListView->DeleteAllItems();
//
//    for (int i = 0; i < lstrlen(szPath); i++)
//    {
//        if (szPath[i] == '\\' || szPath[i] == '/')
//        {
//			TCHAR *szTempPath = SubString(szPath,iLastDelimiter,i - iLastDelimiter);
//            pCurrentNode = GetDirFromParent(pCurrentNode,szTempPath);
//			delete [] szTempPath;
//
//            iLastDelimiter = i + 1;
//        }
//    }
//
//	// FIXME: Update the status bar path pane.
//	/*g_MainFrame->m_StatusBar.SetPaneText(ID_PANE_PATH,szPath);
//
//	// Update the default status bar pane.
//	TCHAR szSelObjPane[32];
//#ifdef UNICODE
//	swprintf(szSelObjPane,_T(PANE_STR_SELOBJECTS),0);
//#else
//	sprintf(szSelObjPane,_T(PANE_STR_SELOBJECTS),0);
//#endif
//	g_MainFrame->m_StatusBar.SetPaneText(ID_DEFAULT_PANE,szSelObjPane);*/
//
//    ListNode(pCurrentNode);
//
//	// Copy the path to m_szCurrentPath.
//	if (pCurrentNode == m_pRootNode)
//		lstrcpy(m_szCurrentPath,_T("/"));
//	else
//		lstrcpy(m_szCurrentPath,szPath);
//
//	m_pCurrentNode = pCurrentNode;
//
//	// FIXME: Sort the list view.
//	/*tSortData SortData;
//	SortData.iSubItem = g_MainFrame->m_ListViewHeader.m_iSortCol;
//	SortData.bSortUp = g_MainFrame->m_ListViewHeader.m_bSortUp;
//
//	g_MainFrame->m_ListView.SortItemsEx(g_MainFrame->MainListViewCompareProc,(LPARAM)&SortData);*/
//}
//
//void CTreeManager::Refresh()
//{
//	m_pListView->DeleteAllItems();
//
//	ListNode(m_pCurrentNode);
//
//	m_pTreeView->Expand(m_pRootNode->m_hTreeItem);
//}

void CTreeManager::CreateTree(const TCHAR *szRootName,int iImage)
{
	// We need to make sure that any previous tree is destroyed.
	DestroyTree();

	// Allocate memory for a new root node.
	m_pRootNode = new CProjectNode(NULL);
	m_pRootNode->iIconIndex = iImage;
	m_pRootNode->pItemData->ucFlags |= PROJECTITEM_FLAG_ISPROJECTROOT;

	//if (m_pTreeView != NULL)
	//{
	//	TV_ITEM tvRootNode = { 0 };
	//	TV_INSERTSTRUCT tvInsert = { 0 };

	//	// Set the tree view item parameters.
	//	tvRootNode.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
	//	tvRootNode.lParam = (LPARAM)m_pRootNode;
	//	tvRootNode.pszText = (TCHAR *)szRootName;
	//	tvRootNode.iImage = tvRootNode.iSelectedImage = iImage;
	//	tvRootNode.cChildren = false;

	//	tvInsert.item = tvRootNode;
	//	tvInsert.hInsertAfter = TVI_LAST;

	//	// Insert the item to the tree, select it and then expand it.
	//	m_pRootNode->m_hTreeItem = m_pTreeView->InsertItem(&tvInsert);

	//	// Finally we select the root node.
	//	m_pTreeView->SelectItem(m_pRootNode->m_hTreeItem);
	//}

	// Initialize the paths.
	m_szCurrentPath[0] = '/';
	m_szCurrentPath[1] = '\0';
	m_pCurrentNode = m_pRootNode;
}

//CProjectNode *CTreeManager::InsertVirtualRoot(const TCHAR *szNodeName,int iImage)
//{
//	// Allocate memory for a new node.
//	CProjectNode *pNode = new CProjectNode(m_pRootNode);
//	pNode->iIconIndex = iImage;
//	pNode->pItemData->SetFileName(szNodeName);
//	pNode->pItemData->SetFilePath(_T("/"));
//	pNode->pItemData->ucFlags |= PROJECTITEM_FLAG_ISPROJECTROOT;
//
//	m_pRootNode->m_Children.push_back(pNode);
//
//	if (m_pTreeView != NULL)
//	{
//		TV_ITEM tvNode = { 0 };
//		TV_INSERTSTRUCT tvInsert = { 0 };
//
//		// Set the tree view item parameters.
//		tvNode.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
//		tvNode.lParam = (LPARAM)pNode;
//		tvNode.pszText = (TCHAR *)szNodeName;
//		tvNode.iImage = tvNode.iSelectedImage = iImage;
//		tvNode.cChildren = false;
//
//		tvInsert.item = tvNode;
//		tvInsert.hInsertAfter = TVI_LAST;
//		tvInsert.hParent = m_pRootNode->m_hTreeItem;
//
//		// Insert the item to the tree, select it and then expand it.
//		pNode->m_hTreeItem = m_pTreeView->InsertItem(&tvInsert);
//
//		HasChildren(m_pRootNode->m_hTreeItem,true);
//		m_pTreeView->Expand(m_pRootNode->m_hTreeItem);
//	}
//
//	return pNode;
//}
//
void CTreeManager::DestroyTree()
{
	// If the root node hasn't yet been free we free it.
	if (m_pRootNode)
	{
		delete m_pRootNode;
		m_pRootNode = NULL;
	}
}

//void CTreeManager::RebuildLocalPaths(CProjectNode *pNode,std::vector<CProjectNode *> &FolderStack)
//{
//	TCHAR szPath[MAX_PATH];
//	lstrcpy(szPath,pNode->pItemData->GetFilePath());
//	lstrcat(szPath,pNode->pItemData->GetFileName());
//	lstrcat(szPath,_T("/"));
//
//	std::list <CProjectNode *>::iterator itNodeObject;
//	for (itNodeObject = pNode->m_Children.begin(); itNodeObject != pNode->m_Children.end(); itNodeObject++)
//	{
//		(*itNodeObject)->pItemData->SetFilePath(szPath);
//
//		FolderStack.push_back(*itNodeObject);
//	}
//
//	std::list <CItemData *>::iterator itFileObject;
//	for (itFileObject = pNode->m_Files.begin(); itFileObject != pNode->m_Files.end(); itFileObject++)
//		(*itFileObject)->SetFilePath(szPath);
//}
//
//void CTreeManager::RebuildPaths(const TCHAR *szStartPath)
//{
//	CProjectNode *pCurNode = GetNodeFromPath(szStartPath);
//
//	std::vector<CProjectNode *> FolderStack;
//	RebuildLocalPaths(pCurNode,FolderStack);
//
//	while (FolderStack.size() > 0)
//	{ 
//		pCurNode = FolderStack[FolderStack.size() - 1];
//		FolderStack.pop_back();
//
//		RebuildLocalPaths(pCurNode,FolderStack);
//	}
//}
//
//bool CTreeManager::RemoveEntry(CProjectNode *pNode)
//{
//	HTREEITEM hParentItem = m_pTreeView->GetParentItem(pNode->m_hTreeItem);
//	if (!hParentItem)
//		return false;
//
//	// If the item we're deleting is the current directory we need to update
//	// the current directory to the parent.
//	if (pNode == m_pCurrentNode)
//		m_pTreeView->SelectItem(hParentItem);
//
//	// Remove the node from the path list.
//	TCHAR szFullName[MAX_PATH];
//	lstrcpy(szFullName,pNode->pItemData->GetFilePath());
//	lstrcat(szFullName,pNode->pItemData->GetFileName());
//
//	// Delete the node.
//	m_pTreeView->DeleteItem(pNode->m_hTreeItem);
//
//	CProjectNode *pParent = (CProjectNode *)m_pTreeView->GetItemData(hParentItem);
//	pParent->m_Children.remove(pNode);
//	delete pNode;
//
//	if (pParent->m_Children.size() == 0)
//		HasChildren(pParent->m_hTreeItem,false);
//
//	return true;
//}
//
/*
	CTreeManager::RemoveEntry
	-------------------------
	Removes the child or file entry in pNode that points to the same data as
	the pItemData pointer. It returns true if the item was found, otherwise false.
	All data associated with the pItemData object will be deallocated.
*/
bool CTreeManager::RemoveEntry(CProjectNode *pNode,CItemData *pItemData)
{
	// Look for matching nodes.
	CProjectNode *pFoundNode = NULL;
	std::list <CProjectNode *>::iterator itNodeObject;
	for (itNodeObject = pNode->m_Children.begin(); itNodeObject != pNode->m_Children.end(); itNodeObject++)
	{
		if ((*itNodeObject)->pItemData == pItemData)
		{
			pFoundNode = *itNodeObject;
			break;
		}
	}

	// It's not safe to remove the node from the iterating loop so we need
	// to remove it here.
	if (pFoundNode != NULL)
	{
		// Delete the node.
		//m_pTreeView->DeleteItem(pFoundNode->m_hTreeItem);

		delete pFoundNode;
		pNode->m_Children.remove(pFoundNode);

		/*if (pNode->m_Children.size() == 0)
			HasChildren(pNode->m_hTreeItem,false);*/

		return true;
	}

	// If we have reached this far we know that the pItemData belongs to a file.
	delete pItemData;
	pNode->m_Files.remove(pItemData);

	return true;
}
//
//bool CTreeManager::RemoveEntry(const TCHAR *szLocalPath,const TCHAR *szFullPath)
//{
//	CProjectNode *pNode = GetNodeFromPath(szLocalPath);
//
//	std::list <CItemData *>::iterator itFileObject;
//	for (itFileObject = pNode->m_Files.begin(); itFileObject != pNode->m_Files.end(); itFileObject++)
//	{
//		if (!ComparePaths((*itFileObject)->szFullPath,szFullPath))
//			return RemoveEntry(pNode,(*itFileObject));
//	}
//
//	return false;
//}
//
///*
//	CTreeManager::MoveEntry
//	-----------------------
//	Moves the item with the specified pItemData in the specified parent to the
//	new parent. The function returns true if successfull, false otherwise (the
//	only valid cause of a failure is when the destination folder is a subfolder
//	of the source folder).
//*/
//bool CTreeManager::MoveEntry(CProjectNode *pParent,CItemData *pItemData,CProjectNode *pNewParent)
//{
//	// Check if we are about to move a file or folder.
//	if (pItemData->ucFlags & PROJECTITEM_FLAG_ISFOLDER)
//	{
//		// Locate the tree node accosiacted with the folder item.
//		CProjectNode *pItemNode = ResolveNode(pParent,pItemData);
//
//		// The node was not found.
//		if (pItemNode == NULL)
//			return false;
//
//		// Make sure that the destination folder is not a subfolder of the source folder.
//		if (IsSubNode(pItemNode,pNewParent))
//			return false;
//
//		pParent->m_Children.remove(pItemNode);
//		pNewParent->m_Children.push_back(pItemNode);
//
//		class CParentChild
//		{
//		public:
//			HTREEITEM m_hParent;
//			HTREEITEM m_hChild;
//
//			CParentChild(HTREEITEM hParent,HTREEITEM hChild)
//			{
//				m_hParent = hParent;
//				m_hChild = hChild;
//			}
//		};
//
//		// We need to remember the old node so we can delete it when we're done.
//		HTREEITEM hOldNode = pItemNode->m_hTreeItem;
//
//		std::queue<CParentChild> NodeQueue;
//		NodeQueue.push(CParentChild(pNewParent->m_hTreeItem,pItemNode->m_hTreeItem));
//
//		while (!NodeQueue.empty())
//		{
//			// Insert the current item at the new position.
//			TVITEM tvItem = { 0 };
//			tvItem.hItem = NodeQueue.front().m_hChild;
//			tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
//
//			m_pTreeView->GetItem(&tvItem);
//
//			TVINSERTSTRUCT tvInsert = { 0 };
//			tvInsert.item = tvItem;
//			tvInsert.hInsertAfter = TVI_LAST;
//			tvInsert.hParent = NodeQueue.front().m_hParent;
//
//			HTREEITEM hNewItem = m_pTreeView->InsertItem(&tvInsert);
//			CProjectNode *pNewNode = (CProjectNode *)m_pTreeView->GetItemData(hNewItem);
//			pNewNode->m_hTreeItem = hNewItem;
//
//			// Update all file paths.
//			TCHAR *szNewNodePath = pNewNode->pItemData->BeginEditFilePath();
//				CProjectNode *pParentNode = (CProjectNode *)m_pTreeView->GetItemData(NodeQueue.front().m_hParent);
//				lstrcpy(szNewNodePath,pParentNode->pItemData->GetFilePath());
//				//IncludeTrailingBackslash(szFilePath);
//				lstrcat(szNewNodePath,pParentNode->pItemData->GetFileName());
//				lstrcat(szNewNodePath,_T("/"));
//
//				// Set the same file path for all files in the folder.
//				std::list<CItemData *>::iterator itFile;
//				for (itFile = pNewNode->m_Files.begin(); itFile != pNewNode->m_Files.end(); itFile++)
//				{
//					TCHAR *szFilePath = (*itFile)->BeginEditFilePath();
//						lstrcpy(szFilePath,szNewNodePath);
//						lstrcat(szFilePath,pNewNode->pItemData->GetFileName());
//						lstrcat(szFilePath,_T("/"));
//					(*itFile)->EndEditFilePath();
//				}
//			pNewNode->pItemData->EndEditFilePath();
//
//			// Add the childs to the queue.
//			HTREEITEM hChild = m_pTreeView->GetChildItem(NodeQueue.front().m_hChild);
//			if (hChild != NULL)
//			{
//				NodeQueue.push(CParentChild(hNewItem,hChild));
//				while ((hChild = m_pTreeView->GetNextSiblingItem(hChild)) != NULL)
//					NodeQueue.push(CParentChild(hNewItem,hChild));
//			}
//
//			NodeQueue.pop();
//		}
//
//		// Delete the old node.
//		m_pTreeView->DeleteItem(hOldNode);
//
//		// Update the old parent's child status if it no longer has any children.
//		if (m_pTreeView->GetChildItem(pParent->m_hTreeItem) == NULL)
//			HasChildren(pParent->m_hTreeItem,false);
//
//		// Make sure that the new parent knows that it has children.
//		HasChildren(pNewParent->m_hTreeItem,true);
//	}
//	else
//	{
//		pParent->m_Files.remove(pItemData);
//		pNewParent->m_Files.push_back(pItemData);
//	}
//
//	return true;
//}

///*
//	CTreeManager::IsSubNode
//	-----------------------
//	Returns true if pNode2 is a sub node of pNode1.
//*/
bool CTreeManager::IsSubNode(CProjectNode *pNode1,CProjectNode *pNode2)
{
	CProjectNode *pCurNode = pNode2;
	while (pCurNode != NULL)
	{
		if (pCurNode == pNode1)
			return true;

		pCurNode = pCurNode->m_pParent;
	}

	return false;
}
//
///*
//	CTreeManager::GetChildItem
//	--------------------------
//	Checks if pParent has a child node or contains a file with the specified
//	name szName. The function returns the CItemData object of that item. If no
//	such item exists the function returns NULL.
//*/
CItemData *CTreeManager::GetChildItem(CProjectNode *pParent,const TCHAR *szName)
{
	// Check all children of pParent.
	std::list <CProjectNode *>::iterator itNodeObject;
	for (itNodeObject = pParent->m_Children.begin(); itNodeObject != pParent->m_Children.end(); itNodeObject++)
	{
		if (!lstrcmp((*itNodeObject)->pItemData->GetFileName(),szName))
			return (*itNodeObject)->pItemData;
	}

	// Check all files in pParent.
	std::list <CItemData *>::iterator itFileObject;
	for (itFileObject = pParent->m_Files.begin(); itFileObject != pParent->m_Files.end(); itFileObject++)
	{
		if (!lstrcmp((*itFileObject)->GetFileName(),szName))
			return *itFileObject;
	}

	return NULL;
}
//
///*
//	CTreeManager::GetChildNode
//	--------------------------
//	Checks if pParent has a child node with the specified name szName. The
//	function returns the CProjectNode object of that child. If no such node
//	exists the function returns NULL.
//*/
CProjectNode *CTreeManager::GetChildNode(CProjectNode *pParent,const TCHAR *szName)
{
	// Check all children of pParent.
	if (pParent != NULL)
	{
        std::list <CProjectNode *>::iterator itNodeObject;
        for (itNodeObject = pParent->m_Children.begin(); itNodeObject != pParent->m_Children.end(); itNodeObject++)
        {
            if (!lstrcmp((*itNodeObject)->pItemData->GetFileName(),szName))
                return *itNodeObject;
        }
	}

	return NULL;
}

///*
//	CTreeManager::ResolveNode
//	-------------------------
//	Resolved the CItemData object in the pParent node to the actual node
//	object. The function returns NULL if unsuccessfull.
//*/
CProjectNode *CTreeManager::ResolveNode(CProjectNode *pParent,CItemData *pNodeItem)
{
	std::list <CProjectNode *>::iterator itNodeObject;
	for (itNodeObject = pParent->m_Children.begin(); itNodeObject != pParent->m_Children.end(); itNodeObject++)
	{
		if ((*itNodeObject)->pItemData == pNodeItem)
			return *itNodeObject;
	}

	return NULL;
}

void CTreeManager::GetCurrentPath(TCHAR *szCurrentPath)
{
	lstrcpy(szCurrentPath,m_szCurrentPath);
}

void CTreeManager::SetCurrentPath(const TCHAR *szCurrentPath)
{
	lstrcpy(m_szCurrentPath,szCurrentPath);
}

CProjectNode *CTreeManager::GetCurrentNode()
{
	return m_pCurrentNode;
}

CProjectNode *CTreeManager::GetRootNode()
{
	return m_pRootNode;
}

///*
//	CTreeManager::GetDirFromParent
//	------------------------------
//	Returns the CProjectNode object of the sub directory with the name szText
//	which is located inside pParent.
//*/
CProjectNode *CTreeManager::GetDirFromParent(CProjectNode *pParent,TCHAR *szText)
{
	std::list <CProjectNode *>::iterator itNodeObject;
	for (itNodeObject = pParent->m_Children.begin(); itNodeObject != pParent->m_Children.end(); itNodeObject++)
	{
		CProjectNode *pChild = (CProjectNode *)*itNodeObject;

		if (!lstrcmp(pChild->pItemData->GetFileName(),szText))
            return pChild;
	}

    return m_pRootNode;
}

///*
//	CTreeManager::GetLocalSizeFromNode
//	----------------------------------
//	Calculates the size of all the files (not recursivly) in the specified node.
//*/
unsigned __int64 CTreeManager::GetLocalSizeFromNode(CProjectNode *pNode,std::vector<CProjectNode *> &FolderStack)
{
	unsigned __int64 uiSize = 0;

	std::list <CProjectNode *>::iterator itNodeObject;
	for (itNodeObject = pNode->m_Children.begin(); itNodeObject != pNode->m_Children.end(); itNodeObject++)
		FolderStack.push_back(*itNodeObject);

	std::list <CItemData *>::iterator itFileObject;
	for (itFileObject = pNode->m_Files.begin(); itFileObject != pNode->m_Files.end(); itFileObject++)
		uiSize += (*itFileObject)->uiSize;

	return uiSize;
}
//
///*
//	CTreeManager::GetNodeSize
//	-------------------------
//	Calculates the size of the specified node.
//*/
unsigned __int64 CTreeManager::GetNodeSize(CProjectNode *pNode)
{
	std::vector<CProjectNode *> FolderStack;
	unsigned __int64 uiSize = GetLocalSizeFromNode(pNode,FolderStack);

	while (FolderStack.size() > 0)
	{ 
		pNode = FolderStack[FolderStack.size() - 1];
		FolderStack.pop_back();

		uiSize += GetLocalSizeFromNode(pNode,FolderStack);
	}

	return uiSize;
}

///*
//	CTreeManager::GetNodeSize
//	-------------------------
//	Calculates the size of the node matching the pItemData pointer in the
//	pParentNode.
//*/
unsigned __int64 CTreeManager::GetNodeSize(CProjectNode *pParentNode,CItemData *pItemData)
{
	CProjectNode *pCurNode = NULL;

	// Look for matching nodes.
	std::list <CProjectNode *>::iterator itNodeObject;
	for (itNodeObject = pParentNode->m_Children.begin(); itNodeObject != pParentNode->m_Children.end(); itNodeObject++)
	{
		if ((*itNodeObject)->pItemData == pItemData)
		{
			pCurNode = *itNodeObject;
			break;
		}
	}

	if (pCurNode == NULL)
		return 0;

	return GetNodeSize(pCurNode);

	// Calulate the size.
	/*std::vector<CProjectNode *> FolderStack;
	unsigned __int64 uiSize = GetLocalSizeFromNode(pCurNode,FolderStack);

	while (FolderStack.size() > 0)
	{ 
		pCurNode = FolderStack[FolderStack.size() - 1];
		FolderStack.pop_back();

		uiSize += GetLocalSizeFromNode(pCurNode,FolderStack);
	}

	return uiSize;*/
}

void CTreeManager::GetLocalNodeContents(CProjectNode *pNode,std::vector<CProjectNode *> &FolderStack,
										unsigned __int64 &uiFileCount,unsigned __int64 &uiNodeCount)
{
	std::list <CProjectNode *>::iterator itNodeObject;
	for (itNodeObject = pNode->m_Children.begin(); itNodeObject != pNode->m_Children.end(); itNodeObject++)
		FolderStack.push_back(*itNodeObject);

	uiFileCount += pNode->m_Files.size();
	uiNodeCount += pNode->m_Children.size();
}

void CTreeManager::GetNodeContents(CProjectNode *pRootNode,unsigned __int64 &uiFileCount,unsigned __int64 &uiNodeCount)
{
	CProjectNode *pCurNode = pRootNode;

	uiFileCount = 0;
	uiNodeCount = 0;

	// Save the information.
	std::vector<CProjectNode *> FolderStack;
	GetLocalNodeContents(pCurNode,FolderStack,uiFileCount,uiNodeCount);

	while (FolderStack.size() > 0)
	{ 
		pCurNode = FolderStack[FolderStack.size() - 1];
		FolderStack.pop_back();

		GetLocalNodeContents(pCurNode,FolderStack,uiFileCount,uiNodeCount);
	}
}

///*
//	CTreeManager::RecursiveLocalSetFlags
//	------------------------------------
//	Recursivly sets the specified flags to all files and folder of the
//	specified node.
//*/
//void CTreeManager::RecursiveLocalSetFlags(CProjectNode *pNode,std::vector<CProjectNode *> &FolderStack,
//										  unsigned char ucFlags)
//{
//	std::list <CProjectNode *>::iterator itNodeObject;
//	for (itNodeObject = pNode->m_Children.begin(); itNodeObject != pNode->m_Children.end(); itNodeObject++)
//	{
//		FolderStack.push_back(*itNodeObject);
//		(*itNodeObject)->pItemData->ucFlags |= ucFlags;
//	}
//
//	std::list <CItemData *>::iterator itFileObject;
//	for (itFileObject = pNode->m_Files.begin(); itFileObject != pNode->m_Files.end(); itFileObject++)
//		(*itFileObject)->ucFlags |= ucFlags;
//}
//
//void CTreeManager::RecursiveSetFlags(CProjectNode *pRootNode,unsigned char ucFlags)
//{
//	CProjectNode *pCurNode = pRootNode;
//
//	// Save the information.
//	std::vector<CProjectNode *> FolderStack;
//	RecursiveLocalSetFlags(pCurNode,FolderStack,ucFlags);
//
//	while (FolderStack.size() > 0)
//	{ 
//		pCurNode = FolderStack[FolderStack.size() - 1];
//		FolderStack.pop_back();
//
//		RecursiveLocalSetFlags(pCurNode,FolderStack,ucFlags);
//	}
//}
//
//void CTreeManager::DeleteImportedItems(CProjectNode *pRootNode)
//{
//	// Select the root.
//	m_pTreeView->SelectItem(pRootNode->m_hTreeItem);
//
//	// Find the locked nodes.
//	std::vector<CProjectNode *> FolderStack;
//	std::list <CProjectNode *>::iterator itNodeObject;
//	for (itNodeObject = pRootNode->m_Children.begin(); itNodeObject != pRootNode->m_Children.end(); itNodeObject++)
//	{
//		if ((*itNodeObject)->pItemData->ucFlags & PROJECTITEM_FLAG_ISIMPORTED)
//			FolderStack.push_back(*itNodeObject);
//	}
//
//	// Remove the locked nodes.
//	TCHAR szFullName[MAX_PATH];
//
//	while (FolderStack.size() > 0)
//	{
//		CProjectNode *pCurNode = FolderStack[FolderStack.size() - 1];
//		FolderStack.pop_back();
//
//		// Remove the node from the path list.
//		lstrcpy(szFullName,pCurNode->pItemData->GetFilePath());
//		lstrcat(szFullName,pCurNode->pItemData->GetFileName());
//
//		// Delete the node.
//		m_pTreeView->DeleteItem(pCurNode->m_hTreeItem);
//
//		delete pCurNode;
//		pRootNode->m_Children.remove(pCurNode);
//	}
//
//	if (pRootNode->m_Children.size() == 0)
//		HasChildren(pRootNode->m_hTreeItem,false);
//
//	// Find the locked files.
//	std::vector<CItemData *> FileStack;
//	std::list <CItemData *>::iterator itFileObject;
//	for (itFileObject = pRootNode->m_Files.begin(); itFileObject != pRootNode->m_Files.end(); itFileObject++)
//	{
//		if ((*itFileObject)->ucFlags & PROJECTITEM_FLAG_ISIMPORTED)
//			FileStack.push_back(*itFileObject);
//	}
//
//	// Remove the locked files.
//	while (FileStack.size() > 0)
//	{
//		CItemData *pCurFile = FileStack[FileStack.size() - 1];
//		FileStack.pop_back();
//
//		// Delete the file.
//		delete pCurFile;
//		pRootNode->m_Files.remove(pCurFile);
//	}
//}
//
void CTreeManager::GetNodeFullPaths(CProjectNode *pRootNode,std::vector<TCHAR *> &FullPaths)
{
	std::list <CItemData *>::iterator itFileObject;
	for (itFileObject = pRootNode->m_Files.begin(); itFileObject != pRootNode->m_Files.end(); itFileObject++)
		FullPaths.push_back(((CItemData *)*itFileObject)->szFullPath);
}

void CTreeManager::GetNodeFiles(CProjectNode *pNode,std::vector<CItemData *> &Files)
{
	std::list <CItemData *>::iterator itFileObject;
	for (itFileObject = pNode->m_Files.begin(); itFileObject != pNode->m_Files.end(); itFileObject++)
		Files.push_back(*itFileObject);
}

//void CTreeManager::ListNodeFiles(CProjectNode *pNode,CListViewCtrl *pListView)
//{
//	unsigned int uiItemCount = 0;
//	TCHAR szBuffer[16];
//
//	std::list <CItemData *>::iterator itFileObject;
//	for (itFileObject = pNode->m_Files.begin(); itFileObject != pNode->m_Files.end(); itFileObject++)
//	{
//		CItemData *pItemData = (*itFileObject);
//
//		lsprintf(szBuffer,_T("%d"),uiItemCount + 1);
//
//		pListView->AddItem(uiItemCount,0,szBuffer);
//		pListView->AddItem(uiItemCount,1,pItemData->GetAudioData()->szTrackTitle);
//		pListView->AddItem(uiItemCount,2,pItemData->GetAudioData()->szTrackArtist);
//		pListView->SetItemData(uiItemCount,(DWORD_PTR)pItemData);
//
//		uiItemCount++;
//	}
//}
//
///*
//	CTreeManager::HasExtraAudioData
//	-------------------------------
//	Returns true if all items in pNode contains both track title and artist
//	information.
//*/
//bool CTreeManager::HasExtraAudioData(CProjectNode *pNode)
//{
//	std::list <CItemData *>::iterator itFileObject;
//	for (itFileObject = pNode->m_Files.begin(); itFileObject != pNode->m_Files.end(); itFileObject++)
//	{
//		CItemData *pItemData = (*itFileObject);
//
//		if (pItemData->GetAudioData()->szTrackArtist[0] == '\0' ||
//			pItemData->GetAudioData()->szTrackTitle[0] == '\0')
//		{
//			return false;
//		}
//	}
//
//	return true;
//}

void CTreeManager::SaveLocalNodeFileData(CXmlProcessor *pXml,CProjectNode *pNode,
										 std::vector<CProjectNode *> &FolderStack,
										 unsigned int &uiFileCount,unsigned int uiRootLength)
{
	// Save the item information.
	TCHAR szEntryName[32];
	TCHAR szInternalName[MAX_PATH];

	std::list <CProjectNode *>::iterator itNodeObject;
	for (itNodeObject = pNode->m_Children.begin(); itNodeObject != pNode->m_Children.end(); itNodeObject++)
	{
		FolderStack.push_back(*itNodeObject);

		CItemData *pItemData = (*itNodeObject)->pItemData;

		// Don't save imported items.
		if (pItemData->ucFlags & PROJECTITEM_FLAG_ISIMPORTED)
			continue;

		lsprintf(szEntryName,_T("File%d"),uiFileCount++);
		pXml->AddElement(szEntryName,_T(""),true);
			pXml->AddElementAttr(_T("flags"),(int)pItemData->ucFlags);

			lstrcpy(szInternalName,pItemData->GetFilePath() + uiRootLength);
			lstrcat(szInternalName,pItemData->GetFileName());

			pXml->AddElement(_T("InternalName"),szInternalName);
			pXml->AddElement(_T("FullPath"),pItemData->szFullPath);

			FILETIME LocalFileTime;
			DosDateTimeToFileTime(pItemData->usFileDate,pItemData->usFileTime,&LocalFileTime);
			ULARGE_INTEGER iLocalFileTime;
			memcpy(&iLocalFileTime,&LocalFileTime,sizeof(FILETIME));
			pXml->AddElement(_T("FileTime"),(__int64)iLocalFileTime.QuadPart);
		pXml->LeaveElement();
	}

	std::list <CItemData *>::iterator itFileObject;
	for (itFileObject = pNode->m_Files.begin(); itFileObject != pNode->m_Files.end(); itFileObject++)
	{
		CItemData *pItemData = (*itFileObject);

		// Don't save imported items.
		if (pItemData->ucFlags & PROJECTITEM_FLAG_ISIMPORTED)
			continue;

		lsprintf(szEntryName,_T("File%d"),uiFileCount++);
		pXml->AddElement(szEntryName,_T(""),true);
			pXml->AddElementAttr(_T("flags"),(int)pItemData->ucFlags);

			lstrcpy(szInternalName,pItemData->GetFilePath() + uiRootLength);
			lstrcat(szInternalName,pItemData->GetFileName());

			pXml->AddElement(_T("InternalName"),szInternalName);
			pXml->AddElement(_T("FullPath"),pItemData->szFullPath);

			FILETIME LocalFileTime;
			DosDateTimeToFileTime(pItemData->usFileDate,pItemData->usFileTime,&LocalFileTime);
			ULARGE_INTEGER iLocalFileTime;
			memcpy(&iLocalFileTime,&LocalFileTime,sizeof(FILETIME));
			pXml->AddElement(_T("FileTime"),(__int64)iLocalFileTime.QuadPart);
		pXml->LeaveElement();
	}
}

void CTreeManager::SaveNodeFileData(CXmlProcessor *pXml,CProjectNode *pRootNode)
{
	CProjectNode *pCurNode = pRootNode;

	unsigned int uiRootLength = lstrlen(pRootNode->pItemData->GetFilePath()) +
		lstrlen(pRootNode->pItemData->GetFileName());

	// Save the information.
	unsigned int uiFileCount = 0;

	std::vector<CProjectNode *> FolderStack;
	SaveLocalNodeFileData(pXml,pCurNode,FolderStack,uiFileCount,uiRootLength);

	while (FolderStack.size() > 0)
	{ 
		pCurNode = FolderStack[FolderStack.size() - 1];
		FolderStack.pop_back();

		SaveLocalNodeFileData(pXml,pCurNode,FolderStack,uiFileCount,uiRootLength);
	}
}

void CTreeManager::SaveNodeAudioData(CXmlProcessor *pXml,CProjectNode *pRootNode)
{
	unsigned int uiRootLength = lstrlen(pRootNode->pItemData->GetFilePath()) +
		lstrlen(pRootNode->pItemData->GetFileName());

	// Save the information.
	unsigned int uiFileCount = 0;
	TCHAR szEntryName[32];
	TCHAR szInternalName[MAX_PATH];

	std::list <CItemData *>::iterator itFileObject;
	for (itFileObject = pRootNode->m_Files.begin(); itFileObject != pRootNode->m_Files.end(); itFileObject++)
	{
		CItemData *pItemData = (*itFileObject);

		lsprintf(szEntryName,_T("File%d"),uiFileCount++);
		pXml->AddElement(szEntryName,_T(""),true);
			lstrcpy(szInternalName,pItemData->GetFilePath() + uiRootLength);
			lstrcat(szInternalName,pItemData->GetFileName());

			pXml->AddElement(_T("InternalName"),szInternalName);
			pXml->AddElement(_T("FullPath"),pItemData->szFullPath);

			if (pItemData->HasAudioData())
			{
				pXml->AddElement(_T("TrackTitle"),pItemData->GetAudioData()->szTrackTitle);
				pXml->AddElement(_T("TrackArtist"),pItemData->GetAudioData()->szTrackArtist);
			}
		pXml->LeaveElement();
	}
}

bool CTreeManager::LoadNodeFileData(CXmlProcessor *pXml,CProjectNode *pRootNode)
{
	TCHAR szInternalName[MAX_PATH];
	TCHAR szFullName[MAX_PATH];

	for (unsigned int i = 0; i < pXml->GetElementChildCount(); i++)
	{
		if (!pXml->EnterElement(i))
			return false;

		int iFlags = 0;
		pXml->GetSafeElementAttrValue(_T("flags"),&iFlags);

		// Temporary borrow the szFullName variable. It's safe.
		pXml->GetSafeElementData(_T("InternalName"),szFullName,MAX_PATH - 1);
		lstrcpy(szInternalName,pRootNode->pItemData->GetFilePath());
		lstrcat(szInternalName,pRootNode->pItemData->GetFileName());
		lstrcat(szInternalName,szFullName);

		pXml->GetSafeElementData(_T("FullPath"),szFullName,MAX_PATH - 1);

		// Check that the file exist.
		if (!(iFlags & PROJECTITEM_FLAG_ISFOLDER) && !ckcore::File::exist(szFullName))
		{
			/*MessageBox(*g_pMainFrame,SlowFormatStr(lngGetString(WARNING_MISSPROJFILE),szFullName).c_str(),
					   lngGetString(GENERAL_WARNING),MB_OK | MB_ICONWARNING);*/

			pXml->LeaveElement();
			continue;
		}

		// File time.
		ULARGE_INTEGER iLocalFileTime;
		__int64 iTemp = 0;
		pXml->GetSafeElementData(_T("FileTime"),&iTemp);
		iLocalFileTime.QuadPart = iTemp;

		FILETIME LocalFileTime;
		memcpy(&LocalFileTime,&iLocalFileTime,sizeof(FILETIME));

		//if (bIsFolder)
		if (iFlags & PROJECTITEM_FLAG_ISFOLDER)
		{
			// Include a trailing backslash to indicate that we're dealing with a folder.
			TCHAR szTemp[MAX_PATH];
			lstrcpy(szTemp,szInternalName);
			IncludeTrailingBackslash(szTemp);

			CProjectNode *pNode = AddPath(szTemp);

			pNode->pItemData->ucFlags = (unsigned char)iFlags;

			lstrcpy(pNode->pItemData->szFullPath,szFullName);

			// Copy the modified time.
			FileTimeToDosDateTime(&LocalFileTime,
				&pNode->pItemData->usFileDate,
				&pNode->pItemData->usFileTime);
		}
		else
		{
			CItemData *pItemData = new CItemData();

			pItemData->ucFlags = (unsigned char)iFlags;

			// Paths.
			TCHAR *szFileNameBuffer = pItemData->BeginEditFileName();
				lstrcpy(szFileNameBuffer,szInternalName);
				ExtractFileName(szFileNameBuffer);
			pItemData->EndEditFileName();

			TCHAR *szFilePathBuffer = pItemData->BeginEditFilePath();
				lstrcpy(szFilePathBuffer,szInternalName);
				if (!ExtractFilePath(szFilePathBuffer))
					lstrcpy(szFilePathBuffer,_T("/"));
			pItemData->EndEditFilePath();
			
			lstrcpy(pItemData->szFullPath,szFullName);

			// File type.
			SHFILEINFO shFileInfo;
			if (SHGetFileInfo(pItemData->GetFileName(),FILE_ATTRIBUTE_NORMAL,&shFileInfo,
				sizeof(shFileInfo),SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME))
			{
				lstrcpy(pItemData->szFileType,shFileInfo.szTypeName);
			}

			CProjectNode *pCurrentNode;
			if (pItemData->GetFilePath()[1] != '\0')	// pData->szFilePath[1] == '\0' => pData->szFilePath = "\\"
				pCurrentNode = GetTreeManager()->AddPath(szInternalName);
			else
				pCurrentNode = GetTreeManager()->m_pRootNode;

			// Modified time.
			FileTimeToDosDateTime(&LocalFileTime,&pItemData->usFileDate,&pItemData->usFileTime);

			// Size.
			pItemData->uiSize = ckcore::File::size(szFullName);

			pCurrentNode->m_Files.push_back(pItemData);
		}

		pXml->LeaveElement();
	}

	return true;
}
//
//// FIXME: This function actually duplicates functionallity from the project manager.
//bool CTreeManager::LoadNodeAudioData(CXmlProcessor *pXml,CProjectNode *pRootNode,
//									 int iProjectType)
//{
//	TCHAR szInternalName[MAX_PATH];
//	TCHAR szFullName[MAX_PATH];
//
//	for (unsigned int i = 0; i < pXml->GetElementChildCount(); i++)
//	{
//		if (!pXml->EnterElement(i))
//			return false;
//
//		pXml->GetSafeElementData(_T("InternalName"),szInternalName,MAX_PATH - 1);
//		pXml->GetSafeElementData(_T("FullPath"),szFullName,MAX_PATH - 1);
//
//		CItemData *pItemData = new CItemData();
//
//		// Paths.
//		TCHAR *szFileNameBuffer = pItemData->BeginEditFileName();
//			lstrcpy(szFileNameBuffer,szInternalName);
//			ExtractFileName(szFileNameBuffer);
//		pItemData->EndEditFileName();
//
//		TCHAR *szFilePathBuffer = pItemData->BeginEditFilePath();
//			lstrcpy(szFilePathBuffer,szInternalName);
//			if (!ExtractFilePath(szFilePathBuffer))
//				lstrcpy(szFilePathBuffer,_T("/"));
//		pItemData->EndEditFilePath();
//
//		lstrcpy(pItemData->szFullPath,szFullName);
//
//		// Check that the file exist.
//		if (!ckcore::File::exist(szFullName))
//		{
//			MessageBox(*g_pMainFrame,SlowFormatStr(lngGetString(WARNING_MISSPROJFILE),szFullName).c_str(),
//					   lngGetString(GENERAL_WARNING),MB_OK | MB_ICONWARNING);
//
//			pXml->LeaveElement();
//			continue;
//		}
//
//		// Check file encoding.
//		bool bEncoded = false;
//		unsigned __int64 uiDuration = 0;
//
//		// If the audio file is not a Wave file, try to find a codec that can handle it.
//		if (GetAudioFormat(szFullName) != AUDIOFORMAT_WAVE)
//		{
//			// Audio file information.
//			int iNumChannels = -1;
//			int iSampleRate = -1;
//			int iBitRate = -1;
//
//			for (unsigned int i = 0; i < g_CodecManager.m_Codecs.size(); i++)
//			{
//				// We're only interested in decoders.
//				if ((g_CodecManager.m_Codecs[i]->irc_capabilities() & IRC_HAS_DECODER) == 0)
//					continue;
//
//				if (g_CodecManager.m_Codecs[i]->irc_decode_init(szFullName,iNumChannels,
//					iSampleRate,iBitRate,uiDuration))
//				{
//					// Exit the decoder immediately since we don't want to decode the file yet.
//					g_CodecManager.m_Codecs[i]->irc_decode_exit();
//					bEncoded = true;
//					break;
//				}
//			}
//
//			if (!bEncoded)
//			{
//				lngMessageBox(*g_pMainFrame,FAILURE_UNSUPAUDIO,GENERAL_ERROR,MB_OK | MB_ICONERROR);
//
//				pXml->LeaveElement();
//				continue;
//			}
//		}
//
//		// Track length.
//		if (bEncoded)
//			pItemData->GetAudioData()->uiTrackLength = uiDuration;
//		else
//			pItemData->GetAudioData()->uiTrackLength = GetAudioLength(szFullName);
//
//		// Track size.
//		if (iProjectType == PROJECTTYPE_MIXED)	// Count using the Mode-1 sector size since the spacemeter in these projects are based on that disc size.
//			pItemData->uiSize = (pItemData->GetAudioData()->uiTrackLength / 1000) * 75 * 2048;
//		else
//			pItemData->uiSize = pItemData->GetAudioData()->uiTrackLength;
//
//		// Track information.
//		if (pXml->EnterElement(_T("TrackTitle")))
//		{
//			pXml->LeaveElement();
//			pXml->GetSafeElementData(_T("TrackTitle"),pItemData->GetAudioData()->szTrackTitle,159);	// FIXME: Constant value?!
//		}
//
//		if (pXml->EnterElement(_T("TrackArtist")))
//		{
//			pXml->LeaveElement();
//			pXml->GetSafeElementData(_T("TrackArtist"),pItemData->GetAudioData()->szTrackArtist,159);
//		}
//
//		pRootNode->m_Files.push_back(pItemData);
//
//		pXml->LeaveElement();
//	}
//
//	return true;
//}

void CTreeManager::GetLocalPathList(ckfilesystem::FileSet &Files,CProjectNode *pNode,
									std::vector<CProjectNode *> &FolderStack,int iPathStripLen)
{
	TCHAR szInternalFilePath[MAX_PATH];
	bool bHasChildren = false;

	std::list <CProjectNode *>::iterator itNodeObject;
	for (itNodeObject = pNode->m_Children.begin(); itNodeObject != pNode->m_Children.end(); itNodeObject++)
	{
		CItemData *pItemData = (*itNodeObject)->pItemData;

		// Skip imported folders.
		/*if (pItemData->ucFlags & PROJECTITEM_FLAG_ISIMPORTED)
			continue;*/

		bHasChildren = true;

		FolderStack.push_back(*itNodeObject);

		// Force slash delimiters (no backslash delimiters allowed).
		TCHAR *szFilePathBuffer = pItemData->BeginEditFilePath();
			ForceSlashDelimiters(szFilePathBuffer + iPathStripLen);
		pItemData->EndEditFilePath();

		ForceSlashDelimiters(pItemData->szFullPath);

		lstrcpy(szInternalFilePath,pItemData->GetFilePath() + iPathStripLen);
		lstrcat(szInternalFilePath,pItemData->GetFileName());

		// Handle import data.
		unsigned char ucFlags = ckfilesystem::FileDescriptor::FLAG_DIRECTORY;
		void *pData = NULL;
		if (pItemData->ucFlags & PROJECTITEM_FLAG_ISIMPORTED)
		{
			ucFlags |= ckfilesystem::FileDescriptor::FLAG_IMPORTED;
			pData = pItemData->GetIsoData();
		}

		Files.insert(ckfilesystem::FileDescriptor(szInternalFilePath,
					 pItemData->szFullPath,ucFlags,pData));
	}

	std::list <CItemData *>::iterator itFileObject;
	for (itFileObject = pNode->m_Files.begin(); itFileObject != pNode->m_Files.end(); itFileObject++)
	{
		CItemData *pItemData = (*itFileObject);

		// Skip imported files.
		/*if (pItemData->ucFlags & PROJECTITEM_FLAG_ISIMPORTED)
			continue;*/

		bHasChildren = true;

		// Force slash delimiters (no backslash delimiters allowed).
		TCHAR *szFilePathBuffer = pItemData->BeginEditFilePath();
			ForceSlashDelimiters(szFilePathBuffer + iPathStripLen);
		pItemData->EndEditFilePath();

		ForceSlashDelimiters(pItemData->szFullPath);

		lstrcpy(szInternalFilePath,pItemData->GetFilePath() + iPathStripLen);
		lstrcat(szInternalFilePath,pItemData->GetFileName());

		// Handle import data.
		unsigned char ucFlags = 0;
		void *pData = NULL;
		if (pItemData->ucFlags & PROJECTITEM_FLAG_ISIMPORTED)
		{
			ucFlags |= ckfilesystem::FileDescriptor::FLAG_IMPORTED;
			pData = pItemData->GetIsoData();
		}

		Files.insert(ckfilesystem::FileDescriptor(szInternalFilePath,
					 pItemData->szFullPath,ucFlags,pData));
	}

	// If the folder does not have children, add it manually.
	if (!bHasChildren)
	{
		// Force slash delimiters (no backslash delimiters allowed).
		TCHAR *szFilePathBuffer = pNode->pItemData->BeginEditFilePath();
			ForceSlashDelimiters(szFilePathBuffer + iPathStripLen);
		pNode->pItemData->EndEditFilePath();

		ForceSlashDelimiters(pNode->pItemData->szFullPath);

		lstrcpy(szInternalFilePath,pNode->pItemData->GetFilePath() + iPathStripLen);
		lstrcat(szInternalFilePath,pNode->pItemData->GetFileName());

		// Copy the file name of a temporary empty directory.
		TCHAR szEmptyFolder[MAX_PATH];
		lstrcpy(szEmptyFolder,g_TempManager.GetEmtpyDirectory());
		ForceSlashDelimiters(szEmptyFolder);

		// Handle import data.
		unsigned char ucFlags = ckfilesystem::FileDescriptor::FLAG_DIRECTORY;
		void *pData = NULL;
		if (pNode->pItemData->ucFlags & PROJECTITEM_FLAG_ISIMPORTED)
		{
			ucFlags |= ckfilesystem::FileDescriptor::FLAG_IMPORTED;
			pData = pNode->pItemData->GetIsoData();
		}

		Files.insert(ckfilesystem::FileDescriptor(szInternalFilePath,szEmptyFolder,
					 ucFlags,pData));
	}
}

void CTreeManager::GetPathList(ckfilesystem::FileSet &Files,CProjectNode *pRootNode,int iPathStripLen)
{
	CProjectNode *pCurNode = pRootNode;

	// Save the information.
	std::vector<CProjectNode *> FolderStack;
	GetLocalPathList(Files,pCurNode,FolderStack,iPathStripLen);

	while (FolderStack.size() > 0)
	{ 
		pCurNode = FolderStack[FolderStack.size() - 1];
		FolderStack.pop_back();

		GetLocalPathList(Files,pCurNode,FolderStack,iPathStripLen);
	}
}

CTreeManager * CTreeManager::GetTreeManager() const 
{ 
    return m_pTreeManager; 
}
void CTreeManager::SetTreeManager(CTreeManager * val) 
{ 
    m_pTreeManager = val; 
}

//void CTreeManager::ImportLocalIso9660Tree(ckfilesystem::Iso9660TreeNode *pLocalIsoNode,
//										  CProjectNode *pLocalNode,
//										  std::vector<std::pair<ckfilesystem::Iso9660TreeNode *,
//										  CProjectNode *> > &FolderStack)
//{
//	std::vector<ckfilesystem::Iso9660TreeNode *>::const_iterator itIsoNode;
//	for (itIsoNode = pLocalIsoNode->children_.begin(); itIsoNode !=
//		pLocalIsoNode->children_.end(); itIsoNode++)
//	{
//		if ((*itIsoNode)->file_flags_ & DIRRECORD_FILEFLAG_DIRECTORY)
//		{
//			// Try to locate the corresponding project node.
//			CProjectNode *pCurNode = NULL;
//
//			std::list<CProjectNode *>::const_iterator itNode;
//			for (itNode = pLocalNode->m_Children.begin(); itNode != pLocalNode->m_Children.end(); itNode++)
//			{
//				if (!lstrcmpi((*itNode)->pItemData->GetFileName(),(*itIsoNode)->file_name_.c_str()))
//				{
//					pCurNode = *itNode;
//					break;
//				}
//			}
//
//			// Create a new node if necessary.
//			if (pCurNode == NULL)
//			{
//				pCurNode = new CProjectNode(pLocalNode);
//				pCurNode->pItemData->ucFlags |= PROJECTITEM_FLAG_ISIMPORTED;
//				pCurNode->pItemData->SetFileName((*itIsoNode)->file_name_.c_str());
//
//				TCHAR *szFilePath = pCurNode->pItemData->BeginEditFilePath();
//				lstrcpy(szFilePath,pLocalNode->pItemData->GetFilePath());
//				lstrcat(szFilePath,pLocalNode->pItemData->GetFileName());
//				lstrcat(szFilePath,_T("/"));
//				pCurNode->pItemData->EndEditFilePath();
//
//				ckfilesystem::Iso9660::make_dosdatetime((*itIsoNode)->rec_timestamp_,
//					pCurNode->pItemData->usFileDate,
//					pCurNode->pItemData->usFileTime);
//
//				// Force a folder icon to be used.
//				SHFILEINFO shFileInfo;
//				if (SHGetFileInfo(_T(""),FILE_ATTRIBUTE_DIRECTORY,&shFileInfo,
//					sizeof(shFileInfo),SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_TYPENAME))
//				{
//					pCurNode->iIconIndex = shFileInfo.iIcon;
//					lstrcpy(pCurNode->pItemData->szFileType,shFileInfo.szTypeName);
//				}
//
//				// Add ISO9660 data.
//				CItemData::CIso9660Data *pIsoData = pCurNode->pItemData->GetIsoData();
//
//				pIsoData->file_flags_ = (*itIsoNode)->file_flags_;
//				pIsoData->file_unit_size_ = (*itIsoNode)->file_unit_size_;
//				pIsoData->interleave_gap_size_ = (*itIsoNode)->interleave_gap_size_;
//				pIsoData->volseq_num_ = (*itIsoNode)->volseq_num_;
//				pIsoData->extent_loc_ = (*itIsoNode)->extent_loc_;
//				pIsoData->extent_len_ = (*itIsoNode)->extent_len_;
//
//				memcpy(&pIsoData->rec_timestamp_,&(*itIsoNode)->rec_timestamp_,
//					sizeof(ckfilesystem::tiso_dir_record_datetime));
//
//				// Finalize.
//				pLocalNode->m_Children.push_back(pCurNode);
//
//				AddTreeNode(pLocalNode->m_hTreeItem,pCurNode);
//			}
//
//			FolderStack.push_back(std::make_pair(*itIsoNode,pCurNode));
//		}
//		else
//		{
//			CItemData *pItemData = new CItemData();
//			pItemData->ucFlags |= PROJECTITEM_FLAG_ISIMPORTED;
//			pItemData->uiSize = (*itIsoNode)->extent_len_;
//			pItemData->SetFileName((*itIsoNode)->file_name_.c_str());
//
//			TCHAR *szFilePath = pItemData->BeginEditFilePath();
//			lstrcpy(szFilePath,pLocalNode->pItemData->GetFilePath());
//			lstrcat(szFilePath,pLocalNode->pItemData->GetFileName());
//			lstrcat(szFilePath,_T("/"));
//			pItemData->EndEditFilePath();
//
//			ckfilesystem::Iso9660::make_dosdatetime((*itIsoNode)->rec_timestamp_,
//													pItemData->usFileDate,pItemData->usFileTime);
//
//			// Add ISO9660 data.
//			CItemData::CIso9660Data *pIsoData = pItemData->GetIsoData();
//
//			pIsoData->file_flags_ = (*itIsoNode)->file_flags_;
//			pIsoData->file_unit_size_ = (*itIsoNode)->file_unit_size_;
//			pIsoData->interleave_gap_size_ = (*itIsoNode)->interleave_gap_size_;
//			pIsoData->volseq_num_ = (*itIsoNode)->volseq_num_;
//			pIsoData->extent_loc_ = (*itIsoNode)->extent_loc_;
//			pIsoData->extent_len_ = (*itIsoNode)->extent_len_;
//
//			memcpy(&pIsoData->rec_timestamp_,&(*itIsoNode)->rec_timestamp_,
//				sizeof(ckfilesystem::tiso_dir_record_datetime));
//
//			// Finalize.
//			pLocalNode->m_Files.push_back(pItemData);
//		}
//	}
//}
//
//void CTreeManager::ImportIso9660Tree(ckfilesystem::Iso9660TreeNode *pIsoRootNode,CProjectNode *pRootNode)
//{
//	// Save the information.
//	std::vector<std::pair<ckfilesystem::Iso9660TreeNode *,CProjectNode *> > FolderStack;
//	ImportLocalIso9660Tree(pIsoRootNode,pRootNode,FolderStack);
//
//	while (FolderStack.size() > 0)
//	{ 
//		ckfilesystem::Iso9660TreeNode *pIsoNode = FolderStack.back().first;
//		CProjectNode *pNode = FolderStack.back().second;
//
//		FolderStack.pop_back();
//
//		ImportLocalIso9660Tree(pIsoNode,pNode,FolderStack);
//	}
//}
