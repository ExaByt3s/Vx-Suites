#include "main.h"
#include "mac.h"
#include "mainctrl.h"

CMac::CMac() { luStart.clear(); llStart.clear(); }
CMac::~CMac() { luStart.clear(); llStart.clear(); }

void CMac::Init(){	
	g_cMainCtrl.m_cCommands.RegisterCommand(&m_cmdLogout,	dp(12,15,7,15,21,20,0).CStr(),	this);
	g_cMainCtrl.m_cCommands.RegisterCommand(&m_cmdLogin,	dp(19,5,20,0).CStr(),			this); 
}

login *CMac::FindLogin(CString sIRCUsername){
	if(!sIRCUsername.CStr()) return NULL;
	list<login*>::iterator il; for(il=llStart.begin(); il!=llStart.end(); ++il)
	if(!(*il)->sIRCUsername.Compare(sIRCUsername)) return (*il);
	return NULL; }

user *CMac::FindUser(CString sUsername)
{	if(!sUsername.CStr()) return NULL;
	list<user*>::iterator iu; for(iu=luStart.begin(); iu!=luStart.end(); ++iu)
	{	user *pUser=(*iu); login *pLogin=FindLogin(sUsername);
		if(pLogin) pUser=pLogin->pUser;
		if(!pUser->sUsername.Compare(sUsername) || pLogin) return pUser; }
	return NULL; }

func *CMac::FindFunc(CString sFuncname, list<func*> lStart)
{	if(!sFuncname.CStr()) return NULL;
	list<func*>::iterator i; for(i=lStart.begin(); i!=lStart.end(); ++i)
		if(!(*i)->sFuncname.Compare(sFuncname)) return (*i);
	return NULL; }

bool CMac::CheckPassword(CString sPassword, user *pUser)
{	if(!sPassword.CStr()) return false;
	md5::MD5_CTX md5; md5::MD5Init(&md5); unsigned char szMD5[16]; CString sMD5; sMD5.Assign("");
	md5::MD5Update(&md5, (unsigned char*)sPassword.Str(), sPassword.GetLength());
	md5::MD5Final(szMD5, &md5); for(int i=0;i<16;i++)
	{	CString sTemp; sTemp.Format("%2.2X", szMD5[i]); sMD5.Append(sTemp); }
	if(!pUser->sPassword.Compare(sMD5)) return true;
	return false; }

bool CMac::CheckBadFunc(CString sFuncname, CString sUsername)
{	user *pUser=FindUser(sUsername); if(!pUser) return false;
	if(FindFunc(sFuncname, pUser->lDeny)) return true;
	return false; }
bool CMac::CheckBadFunc(char *szFuncname, char *szUsername)
{	return CheckBadFunc(CString(szFuncname), CString(szUsername)); }

void CMac::AddBadFunc(CString sFuncname, user *pUser)
{	if(!pUser || !sFuncname.CStr()) return; int iToken=0;
	while(sFuncname.Token(iToken, ":").Compare(""))
	{	func *pFunc=new func; pFunc->sFuncname=sFuncname.Token(iToken, ":"); pUser->lDeny.push_back(pFunc); iToken++; } }
void CMac::AddBadFunc(char *szFuncname, user *pUser)
{	AddBadFunc(CString(szFuncname), pUser); }

bool CMac::AddLogin(CString sUsername, CString sPassword, CString sIRCUsername, CString sHost, CString sIdentd)
{	if(FindLogin(sIRCUsername)) return false;
	user *pUser=FindUser(sUsername); if(!pUser) return false;
	if(pUser) if(CheckPassword(sPassword, pUser))
	{	
		if(sHost.Compare("")) if(strcmp(sHost.CStr(), pUser->sHost.CStr())!=0) return false;
		if(sIdentd.Compare("")) if(strcmp(sIdentd.CStr(), pUser->sIdentd.CStr())!=0) return false;
		login *pLogin=new login; pLogin->pUser=pUser; pLogin->sUsername=sUsername;
		pLogin->sIRCUsername=sIRCUsername; llStart.push_back(pLogin); return true; }
	return false; }

bool CMac::AddLogin(char *szUsername, char *szPassword, char *szIRCUsername, char *szHost, char *szIdentd)
{	return AddLogin(CString(szUsername), CString(szPassword), CString(szIRCUsername), CString(szHost), CString(szIdentd)); }

void CMac::ClearLogins() { llStart.clear(); }

void CMac::AddUser(CString sUsername, CString sPassword, CString sHost, CString sIdentd)
{	user *pUser=new user; pUser->sUsername=sUsername; pUser->sPassword=sPassword;
	pUser->sHost=sHost; pUser->sIdentd=sIdentd; luStart.push_back(pUser); }
void CMac::AddUser(char *szUsername, char *szPassword, char *szHost, char *szIdentd)
{	AddUser(CString(szUsername), CString(szPassword), CString(szHost), CString(szIdentd)); }

bool CMac::DelBadFunc_int(CString sFuncname, user *pUser)
{	func *pRemove=NULL; list<func*>::iterator i; for(i=pUser->lDeny.begin(); i!=pUser->lDeny.end(); ++i)
	{	if(!(*i)->sFuncname.Compare(sFuncname)) pRemove=(*i); }
	if(pRemove) { pUser->lDeny.remove(pRemove); delete pRemove; return true; }
	return false; }

bool CMac::DelBadFunc(CString sFuncname, user *pUser)
{	bool bRetVal=true; int iToken=0;
	while(sFuncname.Token(iToken, ":").Compare(""))
	{	if(!DelBadFunc_int(sFuncname.Token(iToken, ":"), pUser)) bRetVal=false;
		iToken++; }
	return bRetVal; }

bool CMac::DelLogin(CString sUsername, CString sIRCUsername)
{	if(!sUsername.CStr()) return false; login *pRemove=NULL;
	list<login*>::iterator i; for(i=llStart.begin(); i!=llStart.end(); ++i)
	{	if(!(*i)->sUsername.Compare(sUsername) || !(*i)->sIRCUsername.Compare(sIRCUsername))
			pRemove=(*i); }
	if(pRemove) { llStart.remove(pRemove); delete pRemove; return true; }
	return false; }
bool CMac::DelLogin(char *szUsername, char *szIRCUsername)
{	return DelLogin(CString(szUsername), CString(szIRCUsername)); }

bool CMac::DelUser(CString sUsername)
{	if(!sUsername.CStr()) return false; user *pRemove=NULL;
	list<user*>::iterator i; for(i=luStart.begin(); i!=luStart.end(); ++i)
	{	if(!(*i)->sUsername.Compare(sUsername)) pRemove=(*i); }
	if(pRemove) { luStart.remove(pRemove); delete pRemove; return true; }
	return false; }

bool CMac::HandleCommand(CMessage *pMsg){

	if(!pMsg->sCmd.Compare(m_cmdLogout.sName.Str()))
	{	if(g_cMainCtrl.m_cMac.DelLogin(CString(""), pMsg->sSrc))
		{	g_cMainCtrl.m_cIRC.SendMsg(pMsg->bSilent, pMsg->bNotice, dp(33,15,15,4,2,25,5,0).Str(), pMsg->sReplyTo.Str());
			return true; }
		else return false; }

	else if(!pMsg->sCmd.Compare(m_cmdLogin.sName.Str()))
	{	
		if(g_cMainCtrl.m_cMac.AddLogin(pMsg->sChatString.Token(1, " ", true), 
			pMsg->sChatString.Token(2, " ", true), 
			pMsg->sSrc, pMsg->sHost, pMsg->sIdentd))
		{	
			g_cMainCtrl.m_cIRC.SendMsg(
				pMsg->bSilent, 
				pMsg->bNotice, 
				dp(27,3,3,5,16,20,5,4,0).Str(), 
				pMsg->sReplyTo.Str());
			return true; 
		}
		else 
			return false; 
	}

	return false; 
}
