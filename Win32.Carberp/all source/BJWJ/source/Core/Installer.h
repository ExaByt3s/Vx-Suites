//-----------------------------------------------------------------------------
//  ������ ���������� ����
//-----------------------------------------------------------------------------
#ifndef InstallerH
#define InstallerH
//-----------------------------------------------------------------------------

#include "Strings.h"

//----------------------------------------------------
//  Install - ������� ����������� ����
//
//  IsUpdate - ������� ����, ��� ��� ���������� ����
//             � �� ������ ����������
//
//  DeleteSourceFile - ������� �� �������� ����
//
//  SourceFileProcessPID - PID ��������, ������
//                         ���������� ��������� �����
//                         ��������� ��������� �����
//----------------------------------------------------
BOOL WINAPI Install(const char* FileName, BOOL IsUpdate, BOOL DeleteSourceFile, DWORD SourceFileProcessPID);




//**************************************************************
//        ������ ��� ������ � �������� ���� bot.plug
//**************************************************************


//----------------------------------------------
//  LoadBotPlug - ������� ��������� ������
//
//  ���� ������ ���� �� �����, �� �� �����������
//  � �����. � ��������� ������ ������ �����-
//  ������ � ������� � ������������ �����������
//  �� �����
//----------------------------------------------
BOOL WINAPI LoadBotPlug(LPVOID *Buf, DWORD *BufSize);


//----------------------------------------------
//  DeleteBotPlug
//  ������� ��������� ������
//----------------------------------------------
BOOL WINAPI DeleteBotPlug();

//----------------------------------------------
//  UpdateBotPlug 
//  ������� ��������� ������
//----------------------------------------------
BOOL WINAPI UpdateBotPlug();


//----------------------------------------------
//  FreeBotPlug - ������� ����������� ������
//                ���������� ��� ������
//
//  ��������� ��� dll ����
//----------------------------------------------
VOID WINAPI FreeBotPlug(LPVOID Buf);




//-----------------------------------------------------------------------------
#endif
