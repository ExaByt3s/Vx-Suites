/*
  ������ ����.
*/
#pragma once

#include "..\common\config0.h"

namespace BuildBot
{
  /*
    �������������.
  */
  void init(void);

  /*
    ���������������.
  */
  void uninit(void);

  /*    Run the build bot.

    IN owner - osnvonoe window.
    IN output - a window to display status.
    IN config - configuration.
    IN destFolder - destination folder for the destination file.

    Return - true - if successful,
                    false - if an error occurs.
  */
  bool _run(HWND owner, HWND output, Config0::CFGDATA *config, LPWSTR destFolder);
};
