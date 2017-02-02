/* Generated by Together */

#ifndef EPROPERTYSEPARATOR_H
#define EPROPERTYSEPARATOR_H

#include "EProperty.h"

class IPropertyHost;

class EPropertySeparator : public EProperty
{

public:

	//
	// construction/destruction
	//
    EPropertySeparator( IPropertyHost* pHost , const CString& sName );
	~EPropertySeparator();

	//
	// EProperty
	//
	virtual int      GetHeight( CDC* pDC );
	virtual COLORREF GetColorKey( EPropWnd* pCtrl );
    virtual void     Draw( CDC* pDC ,  EPropWnd* pCtrl , const CRect& r );
	virtual bool     IsBoldKey();
	virtual bool     IsSelectable();

};
#endif //EPropertySeparator_H