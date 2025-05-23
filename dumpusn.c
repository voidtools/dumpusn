
//
// Copyright (C) 2024 David Carpenter
// 
// Permission is hereby granted, free of charge, 
// to any person obtaining a copy of this software 
// and associated documentation files (the "Software"), 
// to deal in the Software without restriction, 
// including without limitation the rights to use, 
// copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit 
// persons to whom the Software is furnished to do so, 
// subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be 
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#pragma warning(disable : 4996)

#define _WIN32_WINNT  0x0500

#define _DUMPUSN_BUF_SIZE	65536

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include <winioctl.h>

// update resource version!
#define VERSION	"1.0.0.0"

typedef unsigned __int64 QWORD;

static int _dumpusn_is_drive_letter(int ch);
static QWORD _dumpusn_frn_from_filename(const wchar_t *filename);

QWORD string_to_qword(const wchar_t *s)
{
	QWORD v;
	const wchar_t *p;
	
	p = s;
	v = 0;
	
	if ((*p == '0') && ((p[1] == 'x') || (p[1] == 'X')))
	{
		// hex.
		p += 2;
		
		while (*p)
		{
			if ((*p >= '0') && (*p <= '9'))
			{
				v <<= 4;
				v += *p - '0';
			}
			else
			if ((*p >= 'A') && (*p <= 'F'))
			{
				v <<= 4;
				v += *p - 'A' + 10;
			}
			else
			if ((*p >= 'a') && (*p <= 'f'))
			{
				v <<= 4;
				v += *p - 'a' + 10;
			}
			else
			{
				break;
			}
			
			p++;
		}
	}
	else
	{
		int is_signed;
		
		is_signed = 0;
		
		if (*p == '-')
		{
			is_signed = 1;
			p++;
		}
		
		while(*p)	
		{
			if (!((*p >= '0') && (*p <= '9')))
			{
				break;
			}

			v *= 10;
			v += *p - '0';
			p++;
		}
		
		if (is_signed)
		{	
			v = -(__int64)v;
		}
	}
	
	return v;
}

static QWORD _dumpusn_frn_from_filename(const wchar_t *filename)
{
	QWORD frn;
	HANDLE file_handle;
	
	frn = 0;
	
	// find filename..
	file_handle = CreateFile(filename,0,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT,0);
	if (file_handle != INVALID_HANDLE_VALUE)
	{
		BY_HANDLE_FILE_INFORMATION bhfi;
		
		if (GetFileInformationByHandle(file_handle,&bhfi))
		{
			frn = (((QWORD)bhfi.nFileIndexHigh) << 32) | bhfi.nFileIndexLow;
		}
		
		CloseHandle(file_handle);
	}
	
	return frn;
}

static int _dumpusn_is_drive_letter(int ch)
{
	if ((ch >= 'a') && (ch <= 'z'))
	{
		return 1;
	}

	if ((ch >= 'A') && (ch <= 'Z'))
	{
		return 1;
	}
	
	return 0;
}

// main entry
int main(int argc,char **argv)
{
	wchar_t *drive;
	QWORD frn;
	int wargc;
	wchar_t **wargv;
	int main_ret;
	
	wargv = CommandLineToArgvW(GetCommandLine(),&wargc);

	wargc--;
	main_ret = 0;
	
	if (wargc == 1)
	{
		drive = wargv[1];
		frn = 0;
		
		// a filename?
		if ((_dumpusn_is_drive_letter(drive[0])) && (drive[1] == ':') && (drive[2] == '\\'))
		{
			frn = _dumpusn_frn_from_filename(drive);
			
			if (!frn)
			{
				printf("Error %u: unable to get FRN from filename.\n",GetLastError());
				
				return 1;
			}
			
		}
	}
	else
	if (wargc == 2)
	{
		wchar_t *frnarg;
		
		drive = wargv[1];
		frnarg = wargv[2];
		
		if ((frnarg[0] >= '0') && (frnarg[0] <= '9'))
		{
			frn = string_to_qword(frnarg);
		}
		else
		{
			frn = _dumpusn_frn_from_filename(frnarg);
	
			if (!frn)
			{
				printf("Error %u: unable to get FRN from filename.\n",GetLastError());
				
				return 1;
			}
			
		}
	}
	else
	{
		printf("\n");
		printf("dumpusn "VERSION" (c) voidtools 2024\n");
		printf("\n");
		printf("Usage:\n");
		printf("dumpusn.exe <volume> [file-reference-number]\n");
		printf("-or-\n");
		printf("dumpusn.exe <filename>\n");
		printf("\n");
		printf("<volume>\n");
		printf("\tThe volume path.\n");
		printf("\tFor example:\n");
		printf("\tdumpusn.exe C:\n");
		printf("\tdumpusn.exe \\\\.\\C:\n");
		printf("\tdumpusn.exe \\\\?\\Volume{00000000-0000-0000-0000-000000000000}\n");
		printf("\n");
		printf("[file-reference-number]\n");
		printf("\tThe file reference number in hexidecimal or decimal.\n");
		printf("\tThe sequence number is ignored if 0.\n");
		printf("\tAll USN journal events are shown if omitted.\n");
		printf("\tFor example:\n");
		printf("\tdumpusn.exe C: 0x0005000000000005\n");
		printf("\tdumpusn.exe C: 5\n");
		printf("\n");
		printf("<filename>\n");
		printf("\tShow USN journal events for the specified file or folder.\n");
		printf("\tFor example:\n");
		printf("\tdumpusn.exe C:\\windows\\explorer.exe\n");
		
		return 1;
	}
	
	{
		HANDLE h;
		wchar_t volume_path[MAX_PATH];
		
		// convert c: => \\.\c:
		// convert c:\\.... => \\.\c:
		if ((_dumpusn_is_drive_letter(drive[0])) && (drive[1] == ':'))
		{
			volume_path[0] = '\\';
			volume_path[1] = '\\';
			volume_path[2] = '.';
			volume_path[3] = '\\';
			volume_path[4] = drive[0];
			volume_path[5] = ':';
			volume_path[6] = 0;
			
			drive = volume_path;
		}
		
		h = CreateFile(drive,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,0);
		if (h != INVALID_HANDLE_VALUE)
		{
			USN_JOURNAL_DATA usnjd;
			DWORD usnjdsize;
			
			// this will set the last error to 87 if there is no media plugged in.
			// for example a floppy drive with no floppy disk.
			if (DeviceIoControl(h,FSCTL_QUERY_USN_JOURNAL,0,0,&usnjd,sizeof(USN_JOURNAL_DATA),&usnjdsize,0))
			{
				char *buf;
				
				buf = malloc(_DUMPUSN_BUF_SIZE);
				if (buf)
				{
					USN next_usn;
					
					next_usn = usnjd.FirstUsn;
					
					for(;;)
					{
						READ_USN_JOURNAL_DATA rujd;
						DWORD readsize;
						
						rujd.StartUsn = next_usn;
						rujd.ReasonMask = 0xffffffff;
						rujd.ReturnOnlyOnClose = 0;
						rujd.Timeout = 0;
						rujd.BytesToWaitFor = 0;
						rujd.UsnJournalID = usnjd.UsnJournalID;
						
						if (DeviceIoControl(h,FSCTL_READ_USN_JOURNAL,&rujd,sizeof(READ_USN_JOURNAL_DATA),buf,_DUMPUSN_BUF_SIZE,&readsize,0))
						{
							BYTE *p;
							DWORD run;
							
							p = buf;
							run = readsize;
							
							if (run >= sizeof(USN))
							{
								next_usn = *(USN *)p;
								
								p += sizeof(USN);
								run -= sizeof(USN);
							}
							else
							{
								printf("no next USN.\n");
								
								break;
							}
							
							if (!run)
							{
								// empty buffer = EOF
								break;
							}
							
							while(run)
							{
								int did_reason;
								USN_RECORD *usn_record;
								WCHAR filename[MAX_PATH];
								
								usn_record = (USN_RECORD *)p;
								
								if (run < sizeof(USN_RECORD))
								{
									printf("bad record length %u < %u.\n",run,sizeof(USN_RECORD));
									
									break;
								}
								
								if (usn_record->FileNameLength > 255*2)
								{
									printf("bad FileNameLength %u\n",usn_record->FileNameLength);
									
									break;
								}
								
								if (frn)
								{
									if (frn & 0xffff000000000000)
									{
										// full match.
										if (usn_record->FileReferenceNumber != frn)
										{
											goto next_record;
										}
									}
									else
									{
										// id match only.
										if ((usn_record->FileReferenceNumber & 0x0000FFFFFFFFFFFFI64) != frn)
										{
											goto next_record;
										}
									}
								}
								
								CopyMemory(filename,p + usn_record->FileNameOffset,usn_record->FileNameLength);
								filename[usn_record->FileNameLength / sizeof(wchar_t)] = 0;
							
								did_reason = 0;
							
								if (usn_record->Reason & USN_REASON_DATA_OVERWRITE)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("DATA_OVERWRITE");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_DATA_EXTEND)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("DATA_EXTEND");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_DATA_TRUNCATION)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("DATA_TRUNCATION");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_NAMED_DATA_OVERWRITE)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("NAMED_DATA_OVERWRITE");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_NAMED_DATA_EXTEND)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("NAMED_DATA_EXTEND");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_NAMED_DATA_TRUNCATION)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("NAMED_DATA_TRUNCATION");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_FILE_CREATE)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("FILE_CREATE");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_FILE_DELETE)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("FILE_DELETE");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_EA_CHANGE)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("EA_CHANGE");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_SECURITY_CHANGE)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("SECURITY_CHANGE");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_RENAME_OLD_NAME)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("RENAME_OLD_NAME");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_RENAME_NEW_NAME)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("RENAME_NEW_NAME");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_INDEXABLE_CHANGE)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("INDEXABLE_CHANGE");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_BASIC_INFO_CHANGE)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("BASIC_INFO_CHANGE");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_HARD_LINK_CHANGE)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("HARD_LINK_CHANGE");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_COMPRESSION_CHANGE)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("COMPRESSION_CHANGE");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_ENCRYPTION_CHANGE)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("ENCRYPTION_CHANGE");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_OBJECT_ID_CHANGE)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("OBJECT_ID_CHANGE");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_REPARSE_POINT_CHANGE)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("REPARSE_POINT_CHANGE");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_STREAM_CHANGE)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("STREAM_CHANGE");
									did_reason = 1;
								}
								
								if (usn_record->Reason & USN_REASON_CLOSE)
								{
									if (did_reason)
									{
										printf("|");
									}
									
									printf("CLOSE");
									did_reason = 1;
								}
								
								printf(" 0x%016I64x %S\n",usn_record->ParentFileReferenceNumber,filename);
								
next_record:								
								
								if (usn_record->RecordLength > run)
								{
									printf("bad RecordLength %u\n",usn_record->RecordLength);
									
									break;
								}
								
								p += usn_record->RecordLength;
								run -= usn_record->RecordLength;
							}
						}
						else
						{
							printf("Error %u: unable to read USN journal.\n",GetLastError());
							
							main_ret = 1;

							break;
						}
					}
					
					free(buf);
				}
				else
				{
					printf("Error %u: unable to allocate USN buffer.\n",ERROR_OUTOFMEMORY);

					main_ret = 1;
				}
			}
			else
			{
				printf("Error %u: unable to query USN journal.\n",GetLastError());

				main_ret = 1;
			}
		
			CloseHandle(h);
		}
		else
		{
			printf("Error %u: unable to open drive.\n",GetLastError());

			main_ret = 1;
		}
	}
		
	return main_ret;
}
