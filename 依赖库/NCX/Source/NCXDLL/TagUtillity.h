#ifndef _TAGUTILLITY_H_
#define _TAGUTILLITY_H_

#define MAX_TAGNAME_LENGTH		(256)
//////////////////////////////////////////////////////////////////////////

class TagUtillity
{
public:
	static bool	IsProtocolComplete(const char *sInput, const char *sOuterMostTagName, 
		int *nStartPos, int *nProtocolLength);
};

#endif //_TAGUTILLITY_H_
//////////////////////////////////////////////////////////////////////////
