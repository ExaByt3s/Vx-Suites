/*
  Dokan : user-mode file system library for Windows

  Copyright (C) 2008 Hiroki Asakawa info@dokan-dev.net

  http://dokan-dev.net/en

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation; either version 3 of the License, or (at your option) any
later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#include "devioctl.h"

#define DOKAN_VERSION	0x0000170

#define EVENT_CONTEXT_MAX_SIZE		(1024*32)

#define IOCTL_TEST \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_UNREGFS \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_EVENT_WAIT \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_EVENT_INFO \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_EVENT_RELEASE \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_EVENT_START \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_EVENT_WRITE \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x806, METHOD_OUT_DIRECT, FILE_ANY_ACCESS )

#define IOCTL_ALTSTREAM_ON \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_KEEPALIVE_ON \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x808, METHOD_NEITHER, FILE_ANY_ACCESS )

#define IOCTL_KEEPALIVE \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x809, METHOD_NEITHER, FILE_ANY_ACCESS )

#define IOCTL_SERVICE_WAIT \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x80A, METHOD_BUFFERED, FILE_ANY_ACCESS )


#define DRIVER_FUNC_INSTALL     0x01
#define DRIVER_FUNC_REMOVE      0x02

#define DOKAN_MOUNTED	1
#define DOKAN_USED		2

#define DOKAN_DEVICE_MAX	10

#define DOKAN_SECTOR_SIZE			512
#define DOKAN_ALLOCATION_UNIT_SIZE	512


#define DOKAN_FILE_DIRECTORY	1


typedef struct _CREATE_CONTEXT {
	ULONG	FileAttributes;
	ULONG	CreateOptions;
	ULONG	DesiredAccess;
	ULONG	ShareAccess;
	
	ULONG	FileNameLength;
	WCHAR	FileName[1];

} CREATE_CONTEXT, *PCREATE_CONTEXT;


typedef struct _CLEANUP_CONTEXT {
	CHAR	DeleteOnClose;
	ULONG	FileNameLength;
	WCHAR	FileName[1];

} CLEANUP_CONTEXT, *PCLEANUP_CONTEXT;


typedef struct _CLOSE_CONTEXT {
	ULONG	FileNameLength;
	WCHAR	FileName[1];

} CLOSE_CONTEXT, *PCLOSE_CONTEXT;


typedef struct _DIRECTORY_CONTEXT {
	ULONG	FileInformationClass;
	ULONG	FileIndex;
	ULONG	BufferLength;
	ULONG	DirectoryNameLength;
	ULONG	SearchPatternLength;
	ULONG	SearchPatternOffset;
	WCHAR	DirectoryName[1];
	WCHAR	SearchPatternBase[1];

} DIRECTORY_CONTEXT, *PDIRECTORY_CONTEXT;


typedef struct _READ_CONTEXT {
	LARGE_INTEGER	ByteOffset;
	ULONG			BufferLength;
	ULONG			FileNameLength;
	WCHAR			FileName[1];
} READ_CONTEXT, *PREAD_CONTEXT;


typedef struct _WRITE_CONTEXT {
	LARGE_INTEGER	ByteOffset;
	ULONG			BufferLength;
	ULONG			BufferOffset;
	ULONG			RequestLength;
	ULONG			FileNameLength;
	WCHAR			FileName[2];
	// "2" means to keep last null of contents to write
} WRITE_CONTEXT, *PWRITE_CONTEXT;


typedef struct _FILEINFO_CONTEXT {
	ULONG	FileInformationClass;
	ULONG	BufferLength;
	ULONG	FileNameLength;
	WCHAR	FileName[1];
} FILEINFO_CONTEXT, *PFILEINFO_CONTEXT;


typedef struct _SETFILE_CONTEXT {
	ULONG	FileInformationClass;
	ULONG	BufferLength;
	ULONG	BufferOffset;
	ULONG	FileNameLength;
	WCHAR	FileName[1];
} SETFILE_CONTEXT, *PSETFILE_CONTEXT;


typedef struct _VOLUME_CONTEXT {
	ULONG	FsInformationClass;
	ULONG	BufferLength;
} VOLUME_CONTEXT, *PVOLUME_CONTEXT;


typedef struct _LOCK_CONTEXT {
	LARGE_INTEGER	ByteOffset;
	LARGE_INTEGER	Length;
	ULONG			Key;
	ULONG			FileNameLength;
	WCHAR			FileName[1];
} LOCK_CONTEXT, *PLOCK_CONTEXT;


typedef struct _FLUSH_CONTEXT {
	ULONG	FileNameLength;
	WCHAR	FileName[1];
} FLUSH_CONTEXT, *PFLUSH_CONTEXT;


typedef struct _EVENT_CONTEXT {
	ULONG	Length;
	ULONG	SerialNumber;
	ULONG	ProcessId;
	UCHAR	MajorFunction;
	UCHAR	MinorFunction;
	ULONG	Flags;
	ULONG64	Context;
	union {
		DIRECTORY_CONTEXT	Directory;
		READ_CONTEXT		Read;
		WRITE_CONTEXT		Write;
		FILEINFO_CONTEXT	File;
		CREATE_CONTEXT		Create;
		CLOSE_CONTEXT		Close;
		SETFILE_CONTEXT		SetFile;
		CLEANUP_CONTEXT		Cleanup;
		LOCK_CONTEXT		Lock;
		VOLUME_CONTEXT		Volume;
		FLUSH_CONTEXT		Flush;
	};
} EVENT_CONTEXT, *PEVENT_CONTEXT;


#define WRITE_MAX_SIZE				(EVENT_CONTEXT_MAX_SIZE-sizeof(EVENT_CONTEXT)-256*sizeof(WCHAR))


typedef struct _EVENT_INFORMATION {
	ULONG		SerialNumber;
	ULONG		Status;
	ULONG		Flags;
	union {
		struct {
			ULONG	Index;
		} Directory;
		struct {
			LARGE_INTEGER CurrentByteOffset;
		} Read;
		struct {
			LARGE_INTEGER CurrentByteOffset;
		} Write;
		struct {
			ULONG	Flags;
			ULONG	Information;
		} Create;
		struct {
			UCHAR	DeleteOnClose;
		} Delete;
	};
	ULONG64		Context;
	ULONG		BufferLength;
	UCHAR		Buffer[8];

} EVENT_INFORMATION, *PEVENT_INFORMATION;


typedef struct _EVENT_START {
	ULONG	Version;
	ULONG	Status;
	ULONG	DeviceNumber;
} EVENT_START, *PEVENT_START;

#endif // _PUBLIC_H_

