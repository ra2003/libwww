// WinCom.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "WinCom.h"

#include "MainFrm.h"
#include "WinComDoc.h"
#include "WinComView.h"

#include "Location.h"
#include "EntityInfo.h"
#include "Options.h"
#include "Password.h"

// From libwww
#include "WWWLib.h"			      /* Global Library Include file */
#include "WWWApp.h"
#include "WWWMIME.h"				    /* MIME parser/generator */
#include "WWWHTML.h"				    /* HTML parser/generator */
#include "WWWNews.h"				       /* News access module */
#include "WWWHTTP.h"				       /* HTTP access module */
#include "WWWFTP.h"
#include "WWWFile.h"
#include "WWWGophe.h"
#include "WWWStream.h"
#include "WWWTrans.h"
#include "WWWInit.h"

// Glue code between MFC and libwww
#include "UserParameters.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Libwww event handlers

/*	terminate_handler
**	-----------------
**	This function is registered to handle the result of the request
*/
PRIVATE int terminate_handler (HTRequest * request, HTResponse * response,
			       void * param, int status) 
{
    CRequest * www_request = (CRequest *) HTRequest_context(request);
    exit(0);
    return HT_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CWinComApp

BEGIN_MESSAGE_MAP(CWinComApp, CWinApp)
	//{{AFX_MSG_MAP(CWinComApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinComApp construction

CWinComApp::CWinComApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWinComApp object

CWinComApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWinComApp initialization

BOOL CWinComApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("W3C"));

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

        /* Initialize libwww */
        HTProfile_newNoCacheClient("WebCommandor", "1.0");

        /* Setup our own dialog handlers */
        {
            HTAlert_deleteAll();
            HTAlert_add(UserProgress, HT_A_PROGRESS);
            HTAlert_add(UserPrint, HT_A_MESSAGE);
            HTAlert_add(UserConfirm, HT_A_CONFIRM);
            HTAlert_add(UserPrompt, HT_A_PROMPT);
            HTAlert_add(UserPassword, HT_A_SECRET);
            HTAlert_add(UserNameAndPassword, HT_A_USER_PW);
        }

        /* Get all the warnings we can */
        HTError_setShow((HTErrorShow) (HT_ERR_SHOW_INFO | HT_ERR_SHOW_PARS));

        /* Add our own filter to update the history list */
        HTNet_addAfter(terminate_handler, NULL, NULL, HT_ALL, HT_FILTER_LAST);

        /* We do our own local file suffix bindings */
        HTFile_doFileSuffixBinding(NO);

        /* Get the local directory as a URI */
        Request.mp_cwd = HTGetCurrentDirectoryURL();

        // add and initialize the general page....
	CPropertySheet WinCom( IDS_WINCOM );
	CLocation locationPage( this );
	WinCom.AddPage( &locationPage );

	// add and initialize the root dir page....
	CEntityInfo entityPage( this );
	WinCom.AddPage( &entityPage );

	// add and initialize the name page....
	COptions optionsPage( this );
	WinCom.AddPage( &optionsPage );

       	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

        if (WinCom.DoModal() == IDOK) {

            Request.mp_source = &locationPage.m_source;
            Request.mp_destination = &locationPage.m_destination;
            Request.mp_proxy = &optionsPage.m_proxy;
            Request.mp_proxy_prefix = &optionsPage.m_proxy_prefix;

            /* Do we have any options to set first? */
            if (Request.mp_proxy_prefix && Request.mp_proxy) {
                LPTSTR proxy_prefix = Request.mp_proxy_prefix->GetBuffer( 64 );
                LPTSTR proxy = Request.mp_proxy->GetBuffer( 64 );
                HTProxy_add(proxy_prefix, proxy);
                Request.mp_proxy_prefix->ReleaseBuffer( );
                Request.mp_proxy->ReleaseBuffer( );
            }

            /* Set up the anchors and call libwww */
            {
                LPTSTR source = Request.mp_source->GetBuffer( 64 );
                LPTSTR destination = Request.mp_destination->GetBuffer( 64 );
		char * src = HTParse(source, Request.mp_cwd, PARSE_ALL);
		char * dest = HTParse(destination, Request.mp_cwd, PARSE_ALL);
                HTRequest * www_request = HTRequest_new();
                HTAnchor * www_source = HTAnchor_findAddress(src);
                HTAnchor * www_destination = HTAnchor_findAddress(dest);

                /* Set the context so that we can find it again */
                HTRequest_setContext(www_request, &Request);

                /* Start the PUT */    
                if (HTPutDocumentAnchor(HTAnchor_parent(www_source),
                    www_destination, www_request) != YES) {

                    /* @@@ handle error @@@ */
                    ;

                }

                /* Go into the event loop... */
                HTEventList_loop(www_request);

                Request.mp_source->ReleaseBuffer( );
                Request.mp_destination->ReleaseBuffer( );
            }
        }

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CWinComApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CWinComApp commands
