/*===========================================================================
====                                                                     ====
====    File name           :  CxStatic.cpp                              ====
====    Creation date       :  09/2004                                   ====
====    Author(s)           :  Vincent Richomme                          ====
====    Thanks to norm.net and Chen-Cha Hsu                              ====
===========================================================================*/

#include "stdafx.h"
#include "CxStatic.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CxStatic
BEGIN_MESSAGE_MAP(CxStatic, CStatic)
	ON_WM_PAINT()
	//ON_WM_DRAWITEM_REFLECT()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_WM_MOUSEMOVE()
	//ON_MESSAGE(WM_DROPFILES)
	ON_WM_DROPFILES()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
	ON_WM_RBUTTONDOWN()
	ON_WM_SIZE()

	//ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////
// ctor
CxStatic::CxStatic()
{
	hinst_msimg32	  = NULL;
	m_strText		  = _T("");
	m_bTransparentBk  = FALSE;
	m_bAutoWrapping	  = TRUE;
	m_bAutoAdjustFont = TRUE;
	m_bNotifyParent   =	FALSE;
	m_bRounded		  = TRUE;
	m_rgbText		  = ::GetSysColor(COLOR_WINDOWTEXT);
	m_rgbBkgnd		  = ::GetSysColor(COLOR_BTNFACE);
	m_clrBackground	  = RGB(255,255,255); // white by default
	m_cr3DHiliteColor =	RGB(0,0,255);
	//m_rgbTransp		  = 0xFF000000;
	m_EDispMode		  = 0;
	m_pBrush          = new CBrush(m_rgbBkgnd);
	m_fillmode		  = Normal;
	m_crHiColor		  =	m_rgbBkgnd;
	m_crLoColor		  =	m_rgbBkgnd;
	m_nFontSize		  = m_nFontSizeVar = 8;
	m_csFontFamilly	  = _T("");
	m_bFont3d		  = FALSE;
	m_hBitmap		  = NULL;
	m_bBitmap		  = FALSE;
	m_nResourceID	  = -1;
	m_bHover		  = FALSE;
	m_bTracking		  = FALSE;
	m_bAllowMove	  = FALSE;
	m_dwTxtFlags	  = 0;
	m_pImage		  = NULL;
	m_nImgType		  = 0;
	m_nFrames		  = 0;
	m_bIsPlaying	  = FALSE;
	m_bExitThread	  = FALSE;
	m_hExitEvent	  = CreateEvent(NULL,TRUE,FALSE,NULL);
	m_hThread		  = NULL;
	m_bAnimPainting	  = FALSE;
	m_nCurrFrame	  = 0;
	m_PictureSize.cx  = 0;
	m_PictureSize.cy  = 0;

	m_strResourceName.Empty();

	
}

CxStatic::~CxStatic()
{
	//TRACE(_T("in CxStatic::~CxStatic\n"));

	StopAnimatedGif();
	CloseHandle(m_hExitEvent);

	// Clean up
	if (m_pBrush){
		delete m_pBrush;
		m_pBrush = NULL;
	}
	if ( m_hBitmap )
		::DeleteObject(m_hBitmap);

	if ( hinst_msimg32 )
		::FreeLibrary( hinst_msimg32 );

	// Free our image(s)
	UnLoad();
}

/////////////////////////////////////////////////////////////////////////////////
//// PreSubclassWindow
void CxStatic::PreSubclassWindow()
{
	//TRACE(_T("in CxStatic::PreSubclassWindow\n"));

	//m_dwTxtFlags = GetStyle();
	//ModifyStyle(SS_TYPEMASK, SS_OWNERDRAW);
	// get current font
	CFont* pFont = GetFont();
	if (!pFont)
	{
		HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		if (hFont == NULL)
			hFont = (HFONT) GetStockObject(ANSI_VAR_FONT);
		if (hFont)
			pFont = CFont::FromHandle(hFont);
	}
	ASSERT(pFont);
	ASSERT(pFont->GetSafeHandle());

	// create the font for this control
	LOGFONT lf;
	pFont->GetLogFont(&lf);
	pFont->GetLogFont(&m_DefLogFont);

	m_font.CreateFontIndirect(&lf);

	this->GetWindowText(m_strText);

	DragAcceptFiles(TRUE);

	Invalidate();
}


void CxStatic::SetHAlign(EHAlign enuHAlign)
{
	DWORD dwStyle = 0;

	if (enuHAlign == Left)
		dwStyle = SS_LEFT;
	
	if (enuHAlign == Center)
		dwStyle = SS_CENTER;
	
	if (enuHAlign == Right)
		dwStyle = SS_RIGHT;
	

	ModifyStyle(SS_RIGHT|SS_CENTER|SS_LEFT, dwStyle);
	
	Invalidate();
}


void CxStatic::SetVAlign(EVAlign enuVAlign)
{
	DWORD dwStyle = 0;

	if (enuVAlign == Top)
		dwStyle = 0;
	if (enuVAlign == Middle)
		dwStyle = SS_CENTERIMAGE;
	if (enuVAlign == Center)
		dwStyle = 0;

	ModifyStyle(0, SS_CENTERIMAGE);
	Invalidate();
}


void CxStatic::SetMoveable(BOOL moveAble)
{
	ModifyStyle(0, SS_NOTIFY);
	m_bAllowMove = moveAble;
}

///////////////////////////////////////////////////////////////////////////////
// SetBackgroundColor
void CxStatic::SetBkColor(COLORREF rgbBkgnd, COLORREF rgbBkgndHigh, FillMode mode)
{
	m_crLoColor = rgbBkgnd;
	m_crHiColor = rgbBkgndHigh;
	m_fillmode = mode;

	m_rgbBkgnd = rgbBkgnd;
	if (m_pBrush){
		delete m_pBrush;
		m_pBrush = new CBrush(m_rgbBkgnd);
	}
	else
		m_pBrush = new CBrush(m_rgbBkgnd);

	RedrawWindow();
}

/////////////////////////////////////////////////////////////////////////////////
//// SetTransparent
void CxStatic::SetTransparent(BOOL bTranspMode)
{
	m_bTransparentBk = bTranspMode;
	if (m_bTransparentBk)
		ModifyStyleEx(0,WS_EX_TRANSPARENT);
	else
		ModifyStyleEx(WS_EX_TRANSPARENT,0);

	RedrawWindow();
}

void CxStatic::SetTextColor(COLORREF col)
{
	m_rgbText = col;
	RedrawWindow();
}

void CxStatic::SetAutoAdjustFont(BOOL bAutoAdjustFont)
{
	m_bAutoAdjustFont = bAutoAdjustFont;
}

void CxStatic::SetRounded(BOOL bRounded){
	m_bRounded = bRounded;
}

/////////////////////////////////////////////////////////////////////////////////
//// FONT

//void CxStatic::ReconstructFont()
//{
//	m_pFont->DeleteObject();
//	BOOL bCreated = m_pFont->CreateFontIndirect(&m_lf);
//	ASSERT(bCreated);
//
//	RedrawWindow();
//}

void CxStatic::SetFont(LPCTSTR szFont,int nPointSize, int nWeight,
					   BOOL bRedraw /*= TRUE*/)
{
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));       // Zero out the structure.

	_tcscpy(lf.lfFaceName,szFont);			// Set Font Familly
	lf.lfHeight = nPointSize;// Set Height
	lf.lfWeight = nWeight;					// Set Weight

	SetFont(&lf, bRedraw);

	//m_csFontFamilly = strFont;
	//m_nFontSizeVar = m_nFontSize = nPointSize;
}

void CxStatic::SetFont(LOGFONT *pLogFont, BOOL bRedraw /*= TRUE*/)
{
	ASSERT(pLogFont);
	if (!pLogFont)
		return;

	if (m_font.GetSafeHandle())
		m_font.DeleteObject();

	LOGFONT lf = *pLogFont;

	m_font.CreateFontIndirect(&lf);

	if (bRedraw)
		RedrawWindow();
}

void CxStatic::SetFont(CFont *pFont, BOOL bRedraw /*= TRUE*/)
{
	ASSERT(pFont);
	if (!pFont)
		return;

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));

	pFont->GetLogFont(&lf);

	SetFont(&lf, bRedraw);
}

LRESULT CxStatic::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	LRESULT lrReturn(Default());

	SetFont(CFont::FromHandle((HFONT)wParam) );

	RedrawWindow();

	return lrReturn;
}


void CxStatic::SetFont3D(BOOL bFont3D, Type3D type)
{
	m_bFont3d = bFont3D;
	m_3dType = type;

	RedrawWindow();
}


LRESULT CxStatic::OnSetText(WPARAM wParam, LPARAM lParam)
{
	LRESULT lrReturn(Default());

	this->GetWindowText(m_strText);

	return lrReturn;
}

void CxStatic::SetSizeMode(long nWidth, long nHeight)
{
	HDC hDC = ::GetDC(m_hWnd);
	long nDCWidth  = MulDiv(nWidth, GetDeviceCaps(hDC,LOGPIXELSX), 2540);
	long nDCHeight = MulDiv(nHeight, GetDeviceCaps(hDC,LOGPIXELSY), 2540);
	::ReleaseDC(m_hWnd,hDC);

	SetWindowPos(NULL,0,0,nDCWidth,nDCHeight,SWP_NOMOVE | SWP_NOZORDER);
}


int CxStatic::HasImage(CString& strImgType, int& nImgWidth, int& nImgHeight, COLORREF& rgbTransp)
{
	m_pImage = NULL;
	int nImages = m_arrFrames.size();
	if ( nImages == 0)
		return 0;

	vector<TFrame>::iterator it = m_arrFrames.begin();
	m_pImage = (CxImage*) (*it).m_pPicture;
	if (m_pImage == NULL)
		return 0;


	strImgType = FindImgType ( m_pImage->GetType() );
	nImgWidth  = m_pImage->GetWidth();
	nImgHeight = m_pImage->GetHeight();

	rgbTransp = m_pImage->RGBQUADtoRGB( m_pImage->GetTransColor() );
	
	return TRUE;

}

void CxStatic::UnLoad()
{
	vector<TFrame>::iterator it;

	for (it = m_arrFrames.begin(); it != m_arrFrames.end(); ++it){
		m_pImage = (CxImage*) (*it).m_pPicture;

		if (m_pImage){
			delete m_pImage;	m_pImage = NULL;
		}
	}

	m_arrFrames.clear();
	m_pImage = NULL;
}

BOOL CxStatic::SetBitmap(HBITMAP hBitmap, ImageSize Emode, COLORREF rgbTransparent)
{
	UnLoad();

	m_pImage = new CxImage();
	if (m_pImage == NULL)
		return FALSE;

	if (! m_pImage->CreateFromHBITMAP(hBitmap) )
		delete 	m_pImage;	m_pImage = NULL;
		return FALSE;

	if (Emode == OriginalSize){
		SetSizeMode(m_pImage->GetWidth(), m_pImage->GetHeight());
	}

	m_frame.m_pPicture = m_pImage;
	m_arrFrames.push_back( m_frame );

	m_bBitmap = TRUE;

	Invalidate();
	m_pImage = NULL;

	return TRUE;
}

BOOL CxStatic::SetBitmap(UINT nIDResource, ImageSize Emode, COLORREF rgbTransparent)
{

	UnLoad();

		m_pImage = new CxImage();
		if (m_pImage == NULL)
			return FALSE;

		HBITMAP hBmp = ::LoadBitmap( AfxGetInstanceHandle(),
			MAKEINTRESOURCE(nIDResource));

		if (! m_pImage->CreateFromHBITMAP(hBmp) ){
			delete m_pImage;	m_pImage = NULL;
			return FALSE;
		}

		if (Emode == OriginalSize){
			SetSizeMode(m_pImage->GetWidth(), m_pImage->GetHeight());
		}

		m_frame.m_pPicture = m_pImage;
		m_arrFrames.push_back( m_frame );

		m_bBitmap = TRUE;
		Invalidate();
		
		// Clear Text
		m_strText = _T("");

		return TRUE;
}

BOOL CxStatic::SetBitmap(LPCTSTR lpszFileName, ImageSize Emode, COLORREF rgbTransparent)
{
	UnLoad();

	// If filename is empty remove bitmap
	if (_tcsclen( lpszFileName )== 0 ){
		m_bBitmap = FALSE;
		return TRUE;
	}

	CString csExt = lpszFileName;
	int nDotPos = csExt.ReverseFind('.');
	if (nDotPos != -1)
		csExt = csExt.Mid(++nDotPos);

	m_nImgType = CxCommon::FindImgType(csExt);

	// Declare a new CxImage to load our unknown image
	m_pImage = new CxImage();
	if (m_pImage == NULL)
		return FALSE;

	// Load Image
	bool bRet = m_pImage->Load(lpszFileName,m_nImgType);
	if (!bRet){
		delete 	m_pImage;	m_pImage = NULL;
		return FALSE;
	}

	//Get Frame count (GIF or NOT ?)
	int nFrameCount = m_pImage->GetNumFrames();

	if (nFrameCount == 0){
		m_frame.m_pPicture = m_pImage;
		m_arrFrames.push_back( m_frame );
	}
	else{
		// FREE our CxImage allocated above 
		delete 	m_pImage;	
		m_pImage = NULL;

		// Parse frames and allocate image for each
		for(int i=0; i < nFrameCount; i++){
			CxImage *newImage = new CxImage();
			
			newImage->SetFrame(i);
			newImage->Load(lpszFileName,m_nImgType);

			m_frame.m_pPicture = newImage;
			m_frame.m_frameOffset.cx= 0; 
			m_frame.m_frameOffset.cy = 0;
			m_frame.m_frameSize.cx = newImage->GetWidth();  
			m_frame.m_frameSize.cy = newImage->GetHeight();
			m_frame.m_nDelay = newImage->GetFrameDelay();
			m_frame.m_nDisposal = 1;

			m_arrFrames.push_back(m_frame);
		}
		m_bAnimPainting = TRUE;
	}



	if (Emode == OriginalSize){
		SetSizeMode(m_pImage->GetWidth(), m_pImage->GetHeight());
	}


	m_bBitmap = TRUE;

	Invalidate();

	// Clear Text
	m_strText = _T("");

	return TRUE;
}

BOOL CxStatic::SetBitmap(const CByteArray& ImgBuffer, int nImgType, COLORREF rgbTransparent)
{
	UnLoad();

	m_pImage = new CxImage();
	if (m_pImage == NULL)
		return FALSE;

	m_nImgType = nImgType;

	ULONG nBufferLen = ImgBuffer.GetSize();
	BYTE* pBuffer = new BYTE [ nBufferLen ];
	memset(pBuffer, 0, nBufferLen);

	for (ULONG i = 0; i < nBufferLen; i++)
		pBuffer[i] = ImgBuffer.GetAt(i);

	CxMemFile memfile((BYTE*)pBuffer, nBufferLen);
	bool bRet = m_pImage->Decode(&memfile,m_nImgType);
	
	m_frame.m_pPicture = m_pImage;
	m_arrFrames.push_back( m_frame );

	

	m_bBitmap = TRUE;

	Invalidate();

	// Clear Text
	m_strText = _T("");

	// Free buffer
	if (pBuffer){
		delete pBuffer; 
		pBuffer = NULL;
	}

	return TRUE;
}


int	CxStatic::GetImgBuffer(CByteArray& ImgBuffer)
{
	if (m_bBitmap == FALSE || m_pImage == NULL)
		return 0;

	CxMemFile memfile;
	memfile.Open();

	m_pImage->Encode(&memfile, m_nImgType);

	BYTE*	buffer = memfile.GetBuffer();
	unsigned long size = memfile.Size();
	
	for (unsigned long i = 0; i < size; i++)
		ImgBuffer.Add(buffer[i]);

	if (buffer){
		delete [] buffer;
		buffer = NULL;
	}

	return 1;
}

//void CxStatic::BlinkBmp(UINT nIDBmp1, UINT nIDBmp2, UINT Timeout)
//{
//	SetTimer(0, Timeout);
//
//
//
//}

///////////////////////////////////////////////////////////////////////////////
// GetFontPointSize()
int CxStatic::GetFontPointSize(int nHeight)
{
	HDC hdc = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	ASSERT(hdc);
	int cyPixelsPerInch = ::GetDeviceCaps(hdc, LOGPIXELSY);
	::DeleteDC(hdc);

	int nPointSize = MulDiv(nHeight, 72, cyPixelsPerInch);
	if (nPointSize < 0)
		nPointSize = -nPointSize;

	return nPointSize;
}

///////////////////////////////////////////////////////////////////////////////
// GetFontHeight()
int CxStatic::GetFontHeight(int nPointSize)
{
	HDC hdc = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	ASSERT(hdc);
	int cyPixelsPerInch = ::GetDeviceCaps(hdc, LOGPIXELSY);
	::DeleteDC(hdc);

	int nHeight = -MulDiv(nPointSize, cyPixelsPerInch, 72);

	return nHeight;
}

void CxStatic::SetWindowText( LPCTSTR strText)
{
	m_strText = strText;

	CRect Rect;
	GetClientRect(&Rect);

	if ( m_bTransparentBk ){
		ClientToScreen(&Rect);
		Rect.InflateRect(1,1,1,1);
		CWnd *Parent = GetParent();
		Parent->ScreenToClient(&Rect);
		Parent->InvalidateRect(&Rect);
		Parent->UpdateWindow();
		//SendMessage(WM_ERASEBKGND);
	}
	else
		RedrawWindow();
}

void CxStatic::Format(LPCTSTR szFmt, ...)
{
	ULONG	bytesWriten;
	TCHAR	szBuffer[2048];
	va_list args;

	va_start(args, szFmt);
	bytesWriten = _vstprintf(szBuffer, szFmt, args);
	va_end(args);

	m_strText = szBuffer;



	RedrawWindow();
}

void CxStatic::AppendText(LPCTSTR szFmt, ...)
{
	ULONG	bytesWriten;
	TCHAR	szBuffer[2048];
	va_list args;

	va_start(args, szFmt);
	bytesWriten = _vstprintf(szBuffer, szFmt, args);

	m_strText += szBuffer;

	va_end(args);

	RedrawWindow();

}


BOOL CxStatic::RedrawWindow()
{
	/*CRect rect;

	GetClientRect( rect );
	InvalidateRect( rect );*/
	Invalidate();
	return TRUE;
}


void CxStatic::OnDraw(CDC* pDC, LPRECT lpRect)
{
	CPaintDC	dc(this); // device context for painting
	CDC*		pMyDC = NULL;
	CRect		rect;

	if (pDC != NULL){
		
		pMyDC = pDC;
	

		pMyDC->SetWindowOrg((int) lpRect->left, (int)lpRect->top);
	}
	else{
		pMyDC = &dc;
	}
		
	
	CBitmap		bmp;
	CBitmap*	pOldBitmap = NULL;

	// GET Client area
	CRect rc;
	GetClientRect(rc);
	m_rc = rc;

	//Double Buffering - Modify MemDC for transparent background but doesn't work
	// because I have to invalidate parent when SetWindowText - Need to find a fix!!!
	dc.SetBkColor(m_rgbBkgnd);
	CMemDC MemDC(pMyDC, &rc, m_bTransparentBk);

	// PAINT SOLID BACKGROUND IF NO BITMAP
	if ( !m_bBitmap){
		if ( !m_bTransparentBk ){
			if (m_fillmode == Normal)
				MemDC.FillRect(&rc, m_pBrush);
			else
				DrawGradientFill(&MemDC, &rc, m_fillmode, m_crLoColor, m_crHiColor);
		}
	}
	else{// DISPLAY BACKGROUND BITMAP
		DrawBitmap(&MemDC, &rc);
	}


	// TEXT RENDERING
	CFont* pOldFont  = MemDC.SelectObject( &m_font );
	COLORREF rgbText = MemDC.SetTextColor(m_rgbText);
	DrawText(&MemDC, &rc, m_strText);


	// Restore DC's State
	MemDC.SelectObject(pOldFont);
	MemDC.SetTextColor(rgbText);
}

void CxStatic::OnPaint()
{
	OnDraw(NULL);
}



//void CxStatic::OnPaint()
//{
//	CPaintDC	dc(this); // device context for painting
//	CBitmap		bmp;
//	CBitmap*	pOldBitmap = NULL;
//
//	// GET Client area
//	CRect rc;
//	GetClientRect(rc);
//	m_rc = rc;
//
//	//Double Buffering - Modify MemDC for transparent background but doesn't work
//	// because I have to invalidate parent when SetWindowText - Need to find a fix!!!
//	dc.SetBkColor(m_rgbBkgnd);
//	CMemDC MemDC(&dc, &rc, m_bTransparentBk);
//}



void CxStatic::DrawText(CDC* pDCMem, CRect* pRect, CString csText)
{
	DWORD dwStyle = GetStyle();
	DWORD dwFlags = 0;

	// Map "Static Styles" to "Text Styles" - WARNING MACROS
#define MAP_STYLE(src, dest) if(dwStyle & (src)) dwFlags |= (dest)
#define NMAP_STYLE(src, dest) if(!(dwStyle & (src))) dwFlags |= (dest)

	MAP_STYLE(	SS_RIGHT,			DT_RIGHT   					);
	MAP_STYLE(	SS_CENTER,			DT_CENTER					);
	MAP_STYLE(	SS_LEFT,			DT_LEFT					    );
	//MAP_STYLE(	SS_CENTERIMAGE,		DT_VCENTER | DT_SINGLELINE	);
	MAP_STYLE(	SS_NOPREFIX,		DT_NOPREFIX					);
	MAP_STYLE(	SS_WORDELLIPSIS,	DT_WORD_ELLIPSIS			);
	MAP_STYLE(	SS_ENDELLIPSIS,		DT_END_ELLIPSIS				);
	MAP_STYLE(	SS_PATHELLIPSIS,	DT_PATH_ELLIPSIS			);

	// TAb expansion
	if (csText.Find( _T('\t') ) != -1)
		dwFlags |= DT_EXPANDTABS;

	//csText.Replace(
	// TEXT WRAPPING - Inspired by Chen-Cha Hsu and improved
	CRect		newRC;
	TEXTMETRIC	tag;
	CSize		sz;
	::GetTextExtentPoint32(pDCMem->GetSafeHdc(), csText,csText.GetLength(), &sz);
	CStringArray	asText;
	int				nLine = 0;
	CString			s2;
	int				nX = 0;
	int				nId = 0;
	char			nCR = 0;

	// Autowrapping mode enabled
	if ( m_bAutoWrapping ){
		for (int i = 1; i <= csText.GetLength(); i++){
			s2 = csText.Left(i);
			//CARRIAGE RETURN not recognised with SS_CENTERIMAGE - manual handling
			if (csText.GetAt(i-1) == '\r' || csText.GetAt(i-1) == '\n'){
				if (nCR == 0){
					nCR = csText.GetAt(i-1);
				}
				else if (nCR != csText.GetAt(i-1)){ // "\r\n" or "\n\r"
					s2 = csText.Left(i-2);
					asText.Add(s2);
					csText = csText.Mid(i);
					i = nCR = 0;
				}
			}
			::GetTextExtentPoint32(pDCMem->GetSafeHdc(), s2, s2.GetLength(), &sz);
			if ( sz.cx > pRect->Width() ){// We found how many letters fits
				s2 = csText.Left(i-1);
				if ( IsASingleWord(s2) ){
					asText.Add(s2);
					csText = csText.Mid(i-1);
					i = 0;
				}
				else{ // Do not break a word
					nId = s2.ReverseFind(' ');
					s2 = s2.Left(nId);
					asText.Add(s2);
					csText = csText.Mid(nId + 1);
					i = 0;
				}
			}
		}
		if ( ! csText.IsEmpty() )
			asText.Add(csText);
	}
	else{// Standard CStatic behaviour
		asText.SetAtGrow(0, csText);
	}

	nLine = asText.GetSize();
	pDCMem->GetTextMetrics(&tag);
	newRC = *pRect;

	LONG nDiff = pRect->bottom - pRect->top - tag.tmHeight * nLine;
	if (dwStyle & SS_CENTERIMAGE)
		pRect->top = nDiff/2;

	//TRACE( "The value of nDiff is %d\n", nDiff );
	if (m_bAutoAdjustFont){
		if (nDiff < 0){
			//m_nFontSizeVar--;
			//SetFont( m_lf.lfFaceName, m_nFontSizeVar, m_lf.lfWeight );
		}
		//pDCMem->SelectObject( m_pFont ); TODO CHECK WITH AUTOADJUST
		//RedrawWindow();
	}
	for (int j = 0; j < nLine; j++){
		newRC.top = pRect->top + tag.tmHeight * j;
		pDCMem->DrawText(asText[j], &newRC,dwFlags);
		if (m_bFont3d){
			if (m_3dType == Raised)
				newRC.OffsetRect(-1,-1);
			else
				newRC.OffsetRect(1,1);
			pDCMem->DrawText(asText[j], &newRC,dwFlags);
		}
	}
}


BOOL CxStatic::IsASingleWord(const CString & csText)
{
	char	cEnd = 0;

	cEnd = csText.GetAt(csText.GetLength() - 1);
	if ( ( csText.Find(" ") == -1 ) || (cEnd == ' ') )
		return TRUE;
	else
		return FALSE;
}

/*
void CxStatic::UnLoad()
{
	vector<TFrame>::iterator it = NULL;

	for (it = m_arrFrames.begin(); it != m_arrFrames.end(); ++it){
		m_pImage = (CxImage*) (*it).m_pPicture;

		if (m_pImage){
			delete m_pImage;	m_pImage = NULL;
		}
	}

	m_arrFrames.clear();
	m_pImage = NULL;
}
*/

void CxStatic::DrawBitmap(CDC* pDCMem, CRect* pRect)
{
	vector<TFrame>::iterator it;

	if (m_bAnimPainting == FALSE)
	{
		
		it = m_arrFrames.begin();
		m_pImage = (CxImage*) (*it).m_pPicture;
		
		if (m_pImage){
			m_pImage->Stretch(*pDCMem, *pRect);
		}
	}
	else
	{
		unsigned int nDummy;
		
		// Thread params (this & current CDC)
		m_thrParams.pThis	= this;
		m_thrParams.pDC		= pDCMem;
		m_thrParams.pRect	= pRect;

		m_hThread = (HANDLE) _beginthreadex(NULL,0,_ThreadAnimation, &m_thrParams,
			CREATE_SUSPENDED,&nDummy);
		if (!m_hThread){
			TRACE(_T("Draw: Couldn't start a GIF animation thread\n"));
			return;
		} 
		else 
			ResumeThread(m_hThread);
	}
}




///////////////////////////////////////////////////////////////////////////////
// OnEraseBkgnd
BOOL CxStatic::OnEraseBkgnd(CDC* pDC)
{


	return TRUE; //CStatic::OnEraseBkgnd(pDC);
}

void CxStatic::OnSize(UINT nType, int cx, int cy)
{
  CStatic::OnSize(nType, cx, cy);
  Invalidate();
}


//HBRUSH CxStatic::CtlColor(CDC* pDC, UINT nCtlColor)
//{
//	pDC->SetBkMode(TRANSPARENT);
//	HBRUSH brush = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
//	return brush;
//}

//Credits goes to Aquiruse http://www.thecodeproject.com/buttonctrl/hoverbuttonex.asp
void CxStatic::OnMouseMove(UINT nFlags, CPoint point)
{
	TRACE("OnMouseMove : %d %d\r\n", point.x, point.y);

	if (!m_bTracking){
		/*TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE|TME_HOVER;
		tme.dwHoverTime = 1;
		m_bTracking = _TrackMouseEvent(&tme);
		m_point = point;*/
	}
	CStatic::OnMouseMove(nFlags, point);
}

LRESULT CxStatic::OnMouseHover(WPARAM wparam, LPARAM lparam)
{
	m_bHover=TRUE;
	::SetActiveWindow(GetParent()->GetSafeHwnd());
	Invalidate();

	return 0;
}

LRESULT CxStatic::OnMouseLeave(WPARAM wparam, LPARAM lparam)
{
	m_bTracking = FALSE;
	m_bHover=FALSE;
	Invalidate();

	return 0;
}

void CxStatic::OnDropFiles(HDROP hDropInfo)
{
	int		nFiles = 0;
	TCHAR	szFile[MAX_PATH];


	nFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);

	if (nFiles != 1)
		return;

	DragQueryFile(hDropInfo, 0, szFile, sizeof(szFile));
	SetBitmap(szFile);


	DragFinish(hDropInfo);

}

void CxStatic::OnRButtonDown(UINT nFlags, CPoint point)
{

	if (IsMoveable() == TRUE)
	{
		m_point = point;
		CRect rect;
		GetWindowRect( rect );
		ScreenToClient( rect );
		m_pTrack = new CRectTracker(&rect, CRectTracker::dottedLine |
								CRectTracker::resizeInside |
								CRectTracker::hatchedBorder);
		// RIGHT-CLICK + CONTROL
		if (nFlags & MK_CONTROL){
			GetWindowRect(rect);
			GetParent()->ScreenToClient(rect);
			m_pTrack->TrackRubberBand(GetParent(), rect.TopLeft());
			m_pTrack->m_rect.NormalizeRect();
		}
		else {
			m_pTrack->Track(GetParent(), point);
		}
		rect = LPRECT(m_pTrack->m_rect);
		MoveWindow(&rect,TRUE);//Move Window to our new position


		// Clean our mess
		if (m_pTrack)
			delete m_pTrack;

		rect = NULL;
	}

	CStatic::OnRButtonDown(nFlags, point);
}

void CxStatic::SetTransColor(COLORREF coltrans)
{
	if(NULL == m_pImage)
		return;

	if( m_pImage->IsIndexed())
		m_pImage->SetTransIndex(m_pImage->GetNearestIndex(m_pImage->RGBtoRGBQUAD(coltrans)));
	else
	{
		m_pImage->SetTransIndex(0);
		m_pImage->SetTransColor(m_pImage->RGBtoRGBQUAD(coltrans));
	}

}



//============================================================================
// GIF ANIMATION THREAD - MAYBE USELESS because CxImage seems to support
// animated gifs.
//============================================================================
void CxStatic::StopAnimatedGif()
{
	m_bIsPlaying = FALSE;
	m_bExitThread = TRUE;
	SetEvent(m_hExitEvent);

	if (m_hThread){
		// we'll wait for 5 seconds then continue execution
		WaitForSingleObject(m_hThread,5000);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	// make it possible to Draw() again
	ResetEvent(m_hExitEvent);
	m_bExitThread = FALSE;
}

UINT WINAPI CxStatic::_ThreadAnimation(LPVOID pParam)
{
	ASSERT(pParam);
	TThreadParams* pThrParams = reinterpret_cast<TThreadParams *> (pParam);

	pThrParams->pThis->m_bIsPlaying = TRUE;
	pThrParams->pThis->ThreadAnimation( pThrParams );
	pThrParams->pThis->m_bIsPlaying = FALSE;

	// this thread has finished its work so we close the handle
	CloseHandle(pThrParams->pThis->m_hThread); 
	// and init the handle to zero (so that Stop() doesn't Wait on it)
	pThrParams->pThis->m_hThread = 0;

	return 0;
}

void CxStatic::ThreadAnimation(TThreadParams* pThrParams)
{

	CDC*		pDC	= pThrParams->pDC;
	LPRECT		pRect = pThrParams->pRect;
	CxStatic*	pThis = pThrParams->pThis;


	while (!m_bExitThread)
	{
		if (m_arrFrames[m_nCurrFrame].m_pPicture)
		{
			long hmWidth = m_arrFrames[m_nCurrFrame].m_pPicture->GetWidth();
			long hmHeight= m_arrFrames[m_nCurrFrame].m_pPicture->GetHeight();
			
			m_arrFrames[m_nCurrFrame].m_pPicture->Draw2(*pDC, 0, 0, 
				hmWidth, 
				hmHeight);

			
			Invalidate(FALSE);
			
			if (m_bExitThread) break;

			// if the delay time is too short (like in old GIFs), wait for 100ms
			if (m_arrFrames[m_nCurrFrame].m_nDelay < 5) 
				WaitForSingleObject(m_hExitEvent, 100);
			else
				WaitForSingleObject(m_hExitEvent, 10*m_arrFrames[m_nCurrFrame].m_nDelay);

			if (m_bExitThread) break;

		};
		m_nCurrFrame++;
		if (m_nCurrFrame == m_arrFrames.size())
		{
			m_nCurrFrame
				= 0; 
		// init the screen for the first frame,
			/*
			HBRUSH hBrush = CreateSolidBrush(m_clrBackground);
			if (hBrush)
			{
				RECT rect = {0,0,m_PictureSize.cx,m_PictureSize.cy};
				FillRect(*pDC,&rect,hBrush);
				DeleteObject(hBrush);
			};
			*/
		};
	};
}


