/*

  Extract Authenticode signature data from PE format file

  Copyright (C) 2014 Masamichi Hosoda. All rights reserved.
 
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  SUCH DAMAGE.

 */

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include<windows.h>
#include<wintrust.h>
#include<stdio.h>
#include<io.h>
#include<fcntl.h>
#include<stdlib.h>
#include<locale.h>

#define SIZEOF_WIN_CERTIFICATE_HDR 8

DWORD parse(FILE *fin);
int parse_dos_header(FILE *fin);
DWORD parse_nt_header(FILE *fin);
DWORD parse_datadirectory(FILE *fin, PIMAGE_DATA_DIRECTORY pdd);
DWORD parse_authenticode_signature(FILE *fin);
int copy_file(FILE *fin, FILE *fout, DWORD size);

int wmain(int argc, wchar_t *argv[])
{
  FILE *fin, *fout;
  DWORD size;
  int retval;

  _wsetlocale(LC_ALL, L"");

  wprintf(L"Extract Authenticode signature data from PE format file\n"
	  L"Copyright (C) 2014 Masamichi Hosoda. All rights reserved.\n\n");

  if(argc!=3)
    {
      fwprintf(stderr, 
	       L"usage: extract-authenticode [input output]\n\n"
	       L"  input:  Authenticode signed PE format filename.\n"
	       L"          (e.g. .exe, .dll, etc.)\n"
	       L"  output: PKCS#7 signed data filename.\n");
      return 1;
    }

  fin=_wfopen(argv[1], L"rb");
  if(!fin)
    {
      fwprintf(stderr, L"input file %s open failed: %s\n", argv[1],
	       _wcserror(errno));
      return 1;
    }

  size=parse(fin);
  if(!size)
    {
      fclose(fin);
      return 1;
    }

  fout=_wfopen(argv[2], L"wb");
  if(!fout)
    {
      fwprintf(stderr, L"output file %s open failed: %s\n", argv[2],
	       _wcserror(errno));
      fclose(fin);
      return 1;
    }

  retval=copy_file(fin, fout, size);

  fclose(fout);
  fclose(fin);

  return retval;
}

DWORD parse(FILE *fin)
{
  DWORD size, length;

  if(parse_dos_header(fin))
    {
      return 0;
    }

  size=parse_nt_header(fin);

  if(size)
    {
      length=parse_authenticode_signature(fin);
      if(size>length)
	{
	  return length;
	}
      else
	{
	  return 0;
	}
    }
  return 0;
}

int parse_dos_header(FILE *fin)
{
  IMAGE_DOS_HEADER idh;
  size_t rc;
  fpos_t pos;
  int retval;

  rc=fread(&idh, sizeof(idh), 1, fin);
  if(rc<0)
    {
      _wperror(L"fread IMAGE_DOS_HEADER failed");
      return 1;
    }
  else if(rc==0)
    {
      wprintf(L"fread IMAGE_DOS_HEADER failed: EOF\n");
      return 1;
    }

  if(idh.e_magic != IMAGE_DOS_SIGNATURE)
    {
      wprintf(L"IMAGE_DOS_HEADER e_magic = unknown (0x%04x)\n",
	      idh.e_magic);
      return 1;
    }

  wprintf(L"IMAGE_DOS_HEADER e_lfanew = 0x%08x\n", idh.e_lfanew);

  pos=idh.e_lfanew;
  retval=fsetpos(fin, &pos);
  if(retval!=0)
    {
      _wperror(L"fsetpos IMAGE_DOS_HEADER e_lfanew failed");
      return 1;
    }
  return 0;
}

DWORD parse_nt_header(FILE *fin)
{
  union
  {
    IMAGE_NT_HEADERS32 inh32;
    IMAGE_NT_HEADERS64 inh64;
  } inh;
  size_t rc;

  rc=fread(&inh, sizeof(inh), 1, fin);
  if(rc<0)
    {
      _wperror(L"fread IMAGE_NT_HEADERS failed");
      return 0;
    }
  else if(rc==0)
    {
      wprintf(L"fread IMAGE_NT_HEADER failed: EOF\n");
      return 0;
    }

  if(inh.inh32.Signature != IMAGE_NT_SIGNATURE)
    {
      wprintf(L"IMAGE_NT_HEADERS Signature = unknown (0x%08x)\n",
	      inh.inh32.Signature);
      return 0;
    }

  if(inh.inh32.FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
    {
      wprintf(L"IMAGE_FILE_HEADER Machine = IMAGE_FILE_MACHINE_I386\n");
    }
  else if(inh.inh64.FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
    {
      wprintf(L"IMAGE_FILE_HEADER Machine = IMAGE_FILE_MACHINE_AMD64\n");
    }
  else
    {
      wprintf(L"IMAGE_FILE_HEADER Machine = unknown (0x%04x)\n",
	      inh.inh32.FileHeader.Machine);
    }

  if(inh.inh32.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
    {
      wprintf(L"IMAGE_OPTIONAL_HEADER32 Magic found.\n");
      return parse_datadirectory(fin, inh.inh32.OptionalHeader.DataDirectory);
    }
  else if(inh.inh64.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
    {
      wprintf(L"IMAGE_OPTIONAL_HEADER64 Magic found.\n");
      return parse_datadirectory(fin, inh.inh64.OptionalHeader.DataDirectory);
    }

  wprintf(L"IMAGE_OPTIONAL_HEADER Magic = unknown (0x%04x)\n",
	  inh.inh32.OptionalHeader.Magic);
  return 0;
}

DWORD parse_datadirectory(FILE *fin, PIMAGE_DATA_DIRECTORY pdd)
{
  fpos_t pos;
  DWORD size;
  int retval;

  pos=pdd[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress;
  size=pdd[IMAGE_DIRECTORY_ENTRY_SECURITY].Size;

  wprintf(L"IMAGE_DIRECTORY_ENTRY_SECURITY VirtualAddress = 0x%08x\n",
	  pos);
  wprintf(L"IMAGE_DIRECTORY_ENTRY_SECURITY Size = 0x%08x\n",
	  size);

  if(pos && size)
    {
      retval=fsetpos(fin, &pos);
      if(retval!=0)
	{
	  _wperror(L"fsetpos IMAGE_DIRECTORY_ENTRY_SECURITY "
		   L"VirtualAddress failed");
	  return 0;
	}
      return size;
    }
  wprintf(L"WIN_CERTIFICATE not found.\n");
  return 0;
}

DWORD parse_authenticode_signature(FILE *fin)
{
  size_t rc;
  WIN_CERTIFICATE wc;

  rc=fread(&wc, SIZEOF_WIN_CERTIFICATE_HDR, 1, fin);
  if(rc<0)
    {
      _wperror(L"fread WIN_CERTIFICATE failed");
      return 0;
    }
  else if(rc==0)
    {
      wprintf(L"fread WIN_CERTIFICATE failed: EOF\n");
      return 0;
    }

  wprintf(L"WIN_CERTIFICATE dwLength = 0x%08x\n", wc.dwLength);

  switch(wc.wRevision)
    {
    case WIN_CERT_REVISION_1_0:
      wprintf(L"WIN_CERTIFICATE wRevision = WIN_CERT_REVISION_1_0\n");
      break;
    case WIN_CERT_REVISION_2_0:
      wprintf(L"WIN_CERTIFICATE wRevision = WIN_CERT_REVISION_2_0\n");
      break;
    default:
      wprintf(L"WIN_CERTIFICATE wRevision = unknown (0x%04x)\n",
	      wc.wRevision);
      break;
    }

  if(wc.wCertificateType == WIN_CERT_TYPE_PKCS_SIGNED_DATA)
    {
      wprintf(L"WIN_CERTIFICATE wCertificateType = "
	      L"WIN_CERT_TYPE_PKCS_SIGNED_DATA\n");
    }
  else
    {
      wprintf(L"WIN_CERTIFICATE wCertificateType = unknown (0x%04x)\n",
	     wc.wCertificateType);
    }

  return wc.dwLength - SIZEOF_WIN_CERTIFICATE_HDR;
}

int copy_file(FILE *fin, FILE *fout, DWORD size)
{
  unsigned char *buff;
  size_t count;

  buff=malloc(size);
  if(!buff)
    {
      _wperror(L"malloc failed");
      return 1;
    }

  count=fread(buff, size, 1, fin);
  if(count<0)
    {
      _wperror(L"fread WIN_CERTIFICATE bCertificate failed");
      free(buff);
      return 1;
    }
  else if(count==0)
    {
      wprintf(L"fread WIN_CERTIFICATE bCertificate failed: EOF\n");
      free(buff);
      return 1;
    }

  count=fwrite(buff, size, 1, fout);
  if(count<0)
    {
      _wperror(L"fwrite pkcs#7 failed");
      free(buff);
      return 1;
    }
  else if(count==0)
    {
      wprintf(L"fwrite pkcs#7 failed: EOF\n");
      free(buff);
      return 1;
    }

  free(buff);
  return 0;
}
