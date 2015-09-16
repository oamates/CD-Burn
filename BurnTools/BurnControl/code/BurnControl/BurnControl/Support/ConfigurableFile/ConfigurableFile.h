//////////////////////////////////////////////////////////////////////////
// design ideas
// 
// 1 
// use c++ std, because this class will use tinyxml, tinyxml use std. 
//
// 2
// use std::map make retrieval operation easy.
// 

#ifndef _CONFIGURABLE_FILE_H_
#define _CONFIGURABLE_FILE_H_

#include <string>
#include <map>
#include "tinystr.h"
#include "tinyxml.h"

#define TIXML_USE_STL

class ConfigurableFile
{
public:
    /**
     * Load special xml file. xml file has at least 3 layers
     * such as:
     * ----------------------------------------
     * <?xml version="1.0" encoding="utf-8"?>
     * <config>
     *   <section1>
     *     <keyname1>value1</keyname1>
     *     <keyname2>value2</keyname2>
     *   </section1>
     *   <section2>
     *     <keyname1>value1</keyname1>
     *     <keyname2>value2</keyname2>
     *   </section2>
     * </config>
     * ----------------------------------------
     * 
     * @strFilePath
     * full path, include dir and filename
     *
     * 
     * return 0 : success.
     * return -1: the xml file cannot load successfully.
     * return -2: the xml file structure is wrong.
     */
    int LoadFile(std::string strFilePath);


    /**
     * Save file
     * 
     * @strFilePath
     * full path, include dir and filename
     * if strFilePath is "", use strFilePath from LoadFile.
     * 
     *
     * return 0 : success.
     * return -1: failed.
     */
    int SaveFile(std::string strFilePath = "");


    /**
     * get the value of the firstchild of the specified third layer
     *
     * @sSectionName
     * second layer, section name
     * @sKeyName
     * third layer, key name
     * @bExist
     * whether the value of the key is exist
     * 
	 *
     * return not "": success
     * return ""    : there are two cases return "": 1. the key matches with @sSectionName and @sKeyName exist, but the key value is ""; 2. the key does not exist.
	 *                if the initial value of @bExist is NULL,the result will not show the specific case;
	 *                if the initial value of @bExist is not NULL,the result of @bExist is true means case 1, the result of @bExist is false means case 2.
     */
    std::string GetValue(std::string sSectionName, std::string sKeyName, bool *bExist = NULL);


    /**
     * set the value of the firstchild of the specified third layer
     *
     * @sSectionName
     * second layer, section name
     * @sKeyName
     * third layer, key name
     * @sKeyValue
     * the key value
     * 
	 *
     * return true : set success
     * return false: set failed
     */
    bool SetValue(std::string sSectionName, std::string sKeyName, std::string sKeyValue);

    /**
     * set key value
     *
     * @sSectionName
     * second layer, section name
     * @sKeyName
     * third layer, key name
     * 
	 *
     * return not "" : if the sKeyName == "",returns the TiXmlNode of the Section,or the TiXmlNode of the key
     * return ""     : the section or the key does not exist
     */
    TiXmlNode* GetKeyNode(std::string sSectionName, std::string sKeyName = "");

    /**
     * set key value
     *
     * @newElement
     * the TiXmlElement will be insert
     * @sSectionName
     * second layer, section name
     * @sKeyName
     * third layer, key name,will be replaced by the newElement
     * 
	 *
     * return true : success
     * return false: the section or the key doesn't exist
     */

    bool SetKeyNode(const TiXmlNode& newNode,std::string sSectionName, std::string sKeyName);

        /**
     * add key
     *
     * @sAddSectionName
     * second layer, section name,the new key will be added in this layer;
     * if the section does not exist,a new section will be created.
     * @AddNode
     * TiXmlNode,the new TiXmlNode will be added;
     * if the section does not exist,a new section will be created.
     * @sAfterThisKey
     * third layer, key name,the new key will be added after this layer;
     * if sAfterThisKey is "",the key will be add to the first or the last of the second layer named @sAddSectionName.
     * @bInsertHead
     * under the case of  sAfterThisKey is "",if bInsertHead if false, the key will be added to the last,else to the first.
     * 
	 *
     * return 0 : add success
     * return -1: add failed,the section exits,but the @sAfterThisKey does not exit
	 * return -2: add failed,the section does not exit,but the @sAfterThisKey is not ""
     */
    int AddKeyNode(std::string sAddSectionName, const TiXmlNode& AddNode, std::string sAfterThisKey = "", bool bInsertHead = false);

    /**
     * add key
     *
     * @sAddSectionName
     * second layer, section name,the new key will be added in this layer;
     * if the section does not exist,a new section will be created.
     * @sAddKeyName
     * third layer, key name,the new key will be added with this name;
     * @sAddKeyValue
     * key value,the new key will be added with this value;
     * if the section does not exist,a new section will be created.
     * @sAfterThisKey
     * third layer, key name,the new key will be added after this layer;
     * if sAfterThisKey is "",the key will be add to the first or the last of the second layer named @sAddSectionName.
     * @bInsertHead
     * under the case of  sAfterThisKey is "",if bInsertHead if false, the key will be added to the last,else to the first.
     * 
	 *
     * return 0 : add success
     * return -1: add failed,the section exits,but the @sAfterThisKey does not exit
	 * return -2: add failed,the section does not exit,but the @sAfterThisKey is not ""
     */
    int AddKey(std::string sAddSectionName, std::string sAddKeyName, std::string sAddKeyValue, std::string sAfterThisKey = "", bool bInsertHead = false);


     /**
     * delete key
     *
     * @sSectionName
     * second layer, section name
     * @sKeyName
     * third layer, key name
     * @sKeyValue
     * the key value
	 * if @sKeyValue is "",the value will not be check,else the key will be deleted only in the case of @sSectionName,@sKeyName,@sKeyValue are matched.
     * 
	 *
     * return true : delete success
     * return false: delete failed
     */
    bool DeleteKey(std::string sSectionName, std::string sKeyName, std::string sKeyValue = "");

	/**
     * delete section
     *
     * @sSectionName
     * second layer, section name
	 * 
	 *
	 * return true : delete success
	 * return false: delete failed
	 */
    bool DeleteSection(std::string sSectionName);
public:
    ConfigurableFile();
    ~ConfigurableFile();
private:
    TiXmlDocument m_TiXmlDoc;
    std::map<std::string, TiXmlNode *>     m_mapKeyValue;
	std::map<std::string, TiXmlNode *>     m_mapSection;
};

#endif //_CONFIGURABLE_FILE_H_
