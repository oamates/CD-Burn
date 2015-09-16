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

#pragma once
#include <vector>
#include <ckcore/crcstream.hh>
#include <ckfilesystem/filesystemhelper.hh>
#include "./Common/XMLProcessor.h"
//#include "SpaceMeter.h"
#include "TreeManager.h"
//#include "CustomContainer.h"
#include "AdvancedProgress.h"
//#include "ConfirmFileReplaceDlg.h"

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

#define PROJECTVIEWTYPE_DATA				0
#define PROJECTVIEWTYPE_AUDIO				1

#define PROJECTTYPE_DATA					0
#define PROJECTTYPE_AUDIO					1
#define PROJECTTYPE_MIXED					2

// What project file version does this build use.
#define PROJECTMANAGER_FILEVERSION			3

#define SPACEMETER_SIZE_DVD					1
#define PROJECTTREE_IMAGEINDEX_DATA			1

/// Class for project content management.
/**
	Implements core project functionallity such as creating and loading projects,
	adding, removing and moving files.
*/
class CProjectManager
{
public:
	/// Class for performing file transactions within a project.
	/**
		Implements support for adding and moving files to/within a project.
	*/
	class CFileTransaction
	{
	public:
		enum eMode
		{
			MODE_NORMAL,
			MODE_IMPORT
		};

	private:
		eMode m_Mode;
		//CConfirmFileReplaceDlg m_ReplaceDlg;

		bool AddDataFile(CProjectNode *pParentNode,const TCHAR *szFileName,
			const TCHAR *szFilePath,const TCHAR *szFullPath,FILETIME *pFileTime,
			unsigned __int64 uiSize);
		
		CProjectNode *AddFolder(CProjectNode *pParentNode,const TCHAR *szFolderName,
			const TCHAR *szFolderPath,const TCHAR *szFullPath,FILETIME *pFileTime);
		
		bool AddAudioFile(CProjectNode *pParentNode,const TCHAR *szFullPath);

	public:
		CFileTransaction(eMode Mode = MODE_NORMAL);
        CFileTransaction(CTreeManager *pTreeManager);
		~CFileTransaction();

        CItemData *AddDataFile(CProjectNode *pParentNode,const TCHAR *szFullPath);//wj private org
        CProjectNode *AddFolder(CProjectNode *pParentNode,const TCHAR *szFullPath);//wj private org
        void AddFilesInFolder(CProjectNode *pParentNode,std::vector<CProjectNode *> &FolderStack);//wj private org

		bool AddFile(const TCHAR *szFullPath,CProjectNode *pTargetNode = NULL);
		CItemData *AddFile(const TCHAR *szFullPath,const TCHAR *szProjectPath);

		bool MoveFile(CProjectNode *pItemParent,CItemData *pItemData,CProjectNode *pNewParent);
		bool MoveFileToCurrent(CProjectNode *pItemParent,CItemData *pItemData);

        CTreeManager * GetTreeManager() const;
        void SetTreeManager(CTreeManager * val);
        CTreeManager *m_pTreeManager;

	};

private:
	int m_iProjectType;
//	int m_iProjectMedia;
//	int m_iViewType;
//	int m_iActiveView;				// Tells us what view (tree or list) that last had focus.
//	bool m_bModified;				// Set to true if the project has been modified since the last save.
//
//	CProjectNode *m_pActionNode;	// Used for random internal temporary purposes.
//
//	CSplitterWindow *m_pProjectView;
//	CCustomContainer *m_pContainer;
//	CSpaceMeter *m_pSpaceMeter;
//	CListViewCtrl *m_pListView;
//	CTreeViewCtrlEx *m_pTreeView;
//
	CProjectNode *m_pMixDataNode;
//	CProjectNode *m_pMixAudioNode;
//
//	void SetupDataListView();
//	void SetupAudioListView();
//
//	bool DecodeAudioTrack(const TCHAR *szFullPath,const TCHAR *szFullTempPath,
//		CAdvancedProgress *pProgress);

	//bool VerifyLocalFiles(CProjectNode *pNode,std::vector<CProjectNode *> &FolderStack,
	//	CAdvancedProgress *pProgress,TCHAR *szFileNameBuffer,int iPathStripLen,
	//	ckcore::Progresser &FileProgresser,unsigned __int64 &uiFailCount,
	//	std::map<tstring,tstring> &FilePathMap);

//	bool GenerateNewFolderName(CProjectNode *pParent,TCHAR *szFolderName,
//		unsigned int uiFolderNameSize);
//
	void CloseProject();
//	void SaveProjectData(CXmlProcessor *pXML);
//	bool LoadProjectData(CXmlProcessor *pXML);
//	void SaveProjectFileSys(CXmlProcessor *pXML);
//	bool LoadProjectFileSys(CXmlProcessor *pXML);
//	void SaveProjectISO(CXmlProcessor *pXML);
//	bool LoadProjectISO(CXmlProcessor *pXML);
//	void SaveProjectFields(CXmlProcessor *pXML);
//	bool LoadProjectFields(CXmlProcessor *pXML);
//	void SaveProjectBoot(CXmlProcessor *pXML);
//	bool LoadProjectBoot(CXmlProcessor *pXML);
//
//	enum eActiveView
//	{
//		AV_TREE,
//		AV_LIST
//	};
//
//	bool ImportFile(ckcore::Path &BasePath,ckcore::tstring &FilePath,
//		CFileTransaction &Transaction);
//
public:
	CProjectManager();
	~CProjectManager();
//
//	LRESULT OnNewFolder(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
//	LRESULT OnRename(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
//	LRESULT OnRemove(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
//
//	void AssignControls(CSplitterWindow *pProjectView,CCustomContainer *pContainer,
//		CSpaceMeter *pSpaceMeter,CListViewCtrl *pListView,CTreeViewCtrlEx *pTreeView);
//
//	void EnableAll(int iID,bool bEnable,HMENU hMenu = NULL);
//
	void NewDataProject(/*int iDiscMedia*/);
//	void NewAudioProject(int iDiscMedia);
//	void NewMixedProject(int iDiscMedia);
//
//	void DataSelected();
//	void AudioSelected();
//
//	bool ListAddNewFolder();
//	bool TreeAddNewFolder(CProjectNode *pParentNode);
//
//	void RemoveFile(CProjectNode *pParentNode,CItemData *pItemData);
//
//	void ListRemoveSel();
//	void TreeRemoveNode(CProjectNode *pNode);
//	void NotifyListSelChanged(unsigned int uiSelCount);
//	void NotifyTreeSelChanged(CProjectNode *pNode);
//	void TreeSetActionNode(CProjectNode *pNode);
//
//	void TreeSetActive();
//	void ListSetActive();
//
//	void DeleteImportedItems();
//
//	int GetViewType();
	int GetProjectType();
//	void GetProjectContents(unsigned __int64 &uiFileCount,unsigned __int64 &uiFolderCount,
//		unsigned __int64 &uiTrackCount);
//	unsigned __int64 GetProjectSize();
//	//unsigned __int64 GetProjectAudioSize();
	CProjectNode *GetMixDataRootNode();
//	CProjectNode *GetMixAudioRootNode();
//
//	void ListAudioTracks(CListViewCtrl *pListView);
//	void GetAudioTracks(std::vector<TCHAR *> &AudioTracks);
//	bool DecodeAudioTracks(std::vector<TCHAR *> &AudioTracks,
//		std::vector<TCHAR *> &DecodedTracks,CAdvancedProgress *pProgress);
//	bool SaveCDText(const TCHAR *szFullPath);
//
//	bool VerifyCompilation(CAdvancedProgress *pProgress,const TCHAR *szDriveLetter,
//		std::map<tstring,tstring> &FilePathMap);
//
//	void SetDiscLabel(TCHAR *szLabelName);
//
//	void SetModified(bool bModified);
//	bool GetModified();
//
//	bool SaveProject(const TCHAR *szFullPath);
//	bool LoadProject(const TCHAR *szFullPath);
//
//	bool Import(const TCHAR *szFullPath);

    CTreeManager * GetTreeManager() const;
    void SetTreeManager(CTreeManager * val);
    CTreeManager *m_pTreeManager;

    CProjectSettings * GetProjectSettings() const;
    void SetProjectSettings(CProjectSettings * val);
    CProjectSettings *m_pProjectSettings;
};
