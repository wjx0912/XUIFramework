/* Generated by Together */

#ifndef EPROPERTYFILE_H
#define EPROPERTYFILE_H

#include "EPropertyString.h"

class IPropertyHost;

class EPropertyFile : public EPropertyString
{

private:

	CString m_sFilters;

public:

	//
	// construction/destruction
	//
    EPropertyFile( IPropertyHost* pHost , const CString& sName , CString* Property , const CString& sFilters = "All Files (*.*)|*.*||" );
	~EPropertyFile();

	//
	// EProperty overrides
	//
	virtual bool OnCommand( EPropWnd* pParent , WPARAM wParam , LPARAM lParam );

	//
	// EIconTextButtonProperty
	//
	virtual bool HasButton();

	//
	// EPropertyString overrides
	//

	//
	// this
	//
	void AddFilterItem( CString sDescription , CString sFilter );


};
#endif //EPROPERTYSTRING_H