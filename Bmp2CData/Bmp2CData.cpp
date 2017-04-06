// Bmp2CData.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"

#include <windows.h>
#include <atlstr.h>
#include <strsafe.h>

void DisplayErrorBox(LPTSTR lpszFunction);

_TCHAR *get_filename_extT(_TCHAR *filename)
{
	_TCHAR *dot = _tcsrchr(filename, _T('.'));
	if (!dot || dot == filename) return NULL;
	return dot + 1;
}

int get_filename_extT_(_TCHAR *filename)
{
	_TCHAR *dot = _tcsrchr(filename, _T('.'));
	if (!dot || dot == filename) return NULL;
	return (int)(dot - filename);
}

char *get_filename_ext(char *filename)
{
	char *dot = strchr(filename, '.');
	if (!dot || dot == filename) return NULL;
	return dot + 1;
}

int get_filename_ext_(char *filename)
{
	char *dot = strchr(filename, '.');
	if (!dot || dot == filename) return NULL;
	return (int)(dot - filename);
}

const unsigned char pPad[] = { 0xFF, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };

char *bin2CData(char *pData, unsigned long datasize, unsigned long widthBit, unsigned long shift, unsigned long *pCDataLen, char *pArrayName)
{
	char dLenStr[10];
	unsigned long cSrcdataLLen = 0;
	unsigned long cSrcdataSize = 35;
	unsigned long bytePerLine = 0;
	unsigned long widthByte = 0;
	unsigned long dataIndex = 0;
	unsigned long bitIndex = 0;
	unsigned long bitR = 0;
	unsigned long i = 0;
	unsigned long j = 0;
	unsigned int tempV = 0;
	unsigned char tempC = 0;

	cSrcdataSize += (datasize * 6); // "0xFF, "
	cSrcdataSize += ((datasize + 15) / 16) * 3; // "\t\r\n"
	if (strlen(pArrayName))
		cSrcdataSize += (unsigned long)(strlen(pArrayName) - 4);

	char *pCSrcbuf = new char[cSrcdataSize];

	if (pCSrcbuf == NULL)
	{
		printf("Malloc memory fail [C].");
		return (0);
	}
	widthByte = (widthBit + 7) / 8;
	bytePerLine = widthByte + shift;
	strcpy(pCSrcbuf, "unsigned char ");
	if (strlen(pArrayName))
		strcat(pCSrcbuf, pArrayName);
	else
		strcat(pCSrcbuf, "data");
	strcat(pCSrcbuf, "[");
	_itoa(datasize, dLenStr, 10);
	strcat(pCSrcbuf, dLenStr);
	strcat(pCSrcbuf, "] =\r\n{");
	bitIndex = 8;
	bitR = widthBit % 8;
	for (i = 0, j = 1, dataIndex = 0; i < datasize; i++, j++)
	{
		if ((i % 16) == 0)
		{
			strcat(pCSrcbuf, "\r\n\t");
		}
		strcat(pCSrcbuf, "0x");
		tempC = pData[dataIndex];
		if (bitIndex > widthBit)
		{
			tempC = ~tempC & pPad[bitR];
			bitIndex = 8;
		}
		else
		{
			tempC = ~tempC;
			bitIndex += 8;
		}
		_itoa(tempC, dLenStr, 16);
		if (strlen(dLenStr) == 1) // Align two digits
		{
			strcat(pCSrcbuf, "0");
		}
		// to Uppercase
		if (dLenStr[0] >= 0x61 && dLenStr[0] <= 0x6F) /* a ~ f */
		{
			dLenStr[0] -= 0x20;
		}
		if (dLenStr[1] >= 0x61 && dLenStr[1] <= 0x6F) /* a ~ f */
		{
			dLenStr[1] -= 0x20;
		}

		strcat(pCSrcbuf, dLenStr);
		if (j < datasize)
		{
			strcat(pCSrcbuf, ", ");
		}
		dataIndex++;
		if (((dataIndex + shift) % bytePerLine) == 0)
		{
			dataIndex += shift;
		}
	}
	strcat(pCSrcbuf, "\r\n};");
	cSrcdataLLen = (unsigned long)strlen(pCSrcbuf);
	*pCDataLen = cSrcdataLLen;
	return pCSrcbuf;
}

int _tmain(int argc, _TCHAR* argv[])
{
	FILE *pfile;
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	TCHAR szDir[MAX_PATH];
	TCHAR newDir[MAX_PATH * 4];
	TCHAR *pDir;
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	DWORD currentPahtlength = 0;

	// If the directory is not specified as a command-line argument,
	// print usage.

#if _DEBUG//[.4test.
	argc = 1;
	argv[0] = _T("Bmp2CData.exe");
	if (argc != 1)
	{
		_tprintf(TEXT("\nUsage: Bmp2CData.exe\n"));
		return (-1);
	}
#else
	if (argc == 2 && (!_tcscmp(argv[1], _T("-?")) || !_tcscmp(argv[1], _T("-h")) || !_tcscmp(argv[1], _T("-help"))))
	{
		printf("Version 1.01\r\n");
		_tprintf(TEXT("\nUsage: %s\n"), argv[0]);
		//printf("This program built for Win8.1x64-Win32\n");
		printf("Report error to kai.cheng.wang@gmail.com\n");
		return 0;
	}
	if (argc != 1)
	{
		_tprintf(TEXT("\nUsage: %s\n"), argv[0]);
		return (-1);
	}
#endif//].

	currentPahtlength = GetCurrentDirectory(0, NULL);
	LPWSTR pBuf = new TCHAR[currentPahtlength + 1];
	dwError = GetCurrentDirectory(currentPahtlength, pBuf);
	if (dwError == 0)
	{
		dwError = GetLastError();
		if (pBuf) delete[] pBuf;
		return dwError;
	}
	// Check that the input path plus 3 is not longer than MAX_PATH.
	// Three characters are for the "\*" plus NULL appended below.

	StringCchLength(pBuf, MAX_PATH, &length_of_arg);

	if (length_of_arg > (MAX_PATH - 3))
	{
		_tprintf(TEXT("\nDirectory path is too long.\n"));
		if (pBuf) delete[] pBuf;
		return (-1);
	}

	_tprintf(TEXT("\nTarget directory is %s\n\n"), pBuf);

	// Prepare string for use with FindFile functions.  First, copy the
	// string to a buffer, then append '\*' to the directory name.

	StringCchCopy(szDir, MAX_PATH, pBuf);
	StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

	// Find the first file in the directory.

	hFind = FindFirstFile(szDir, &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		DisplayErrorBox(TEXT("FindFirstFile"));
		if (pBuf) delete[] pBuf;
		return dwError;
	}

	// List all the files in the directory with some info about them.
	CString csFname_R;
	CString csFname;
	unsigned long  cSrcdataLLen = 0;
	unsigned long  datasize = 0;
	unsigned long  retSize = 0;
	unsigned long  widthByte = 0;
	unsigned long  bytePerLine = 0;
	unsigned long  shiftSize = 0;
	unsigned long  actualDataSize = 0;
	PBITMAPFILEHEADER pBmpHd;
	PBITMAPINFOHEADER pBmpInfoHd;
	char *pBitDate;
	char *pCSrcDate;
	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			//_tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
		}
		else
		{
			filesize.LowPart = ffd.nFileSizeLow;
			filesize.HighPart = ffd.nFileSizeHigh;

			//_tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
			//if (ffd.cFileName )
			if (_tcsstr(ffd.cFileName, _T(".bmp")))
			{
				cSrcdataLLen = 0;
				csFname_R.Format(_T("%s"), ffd.cFileName);
				csFname_R.Delete(get_filename_extT_(ffd.cFileName), 4);

				_tprintf(TEXT("Get \"%s\"\n"), ffd.cFileName);
				// Open BMP file and reading data to pBuf
				dwError = _wfopen_s(&pfile, ffd.cFileName, _T("rb"));
				if (pfile == NULL)
				{
					_tprintf(TEXT("\nOpen Error:%s, %s\n"), ffd.cFileName, TEXT("rb"));
					continue;
				}
				fseek(pfile, 0, SEEK_END);
				datasize = ftell(pfile);

				char *pbuf = new char[datasize + 4];
				if (pbuf == NULL)
				{
					printf("Malloc memory fail.\r\n");
					continue;
				}

				fseek(pfile, 0, SEEK_SET);
				retSize = (unsigned long)fread(pbuf, sizeof(unsigned char), datasize, pfile);
				fclose(pfile);
				if (retSize != datasize)
				{

					delete[] pbuf;
					pbuf = NULL;
					printf("Read file fail.\r\n");
					continue;
				}
				// check bmp header, get datasize and get data
				pBmpHd = (PBITMAPFILEHEADER)pbuf;
				pBitDate = (pbuf + pBmpHd->bfOffBits);
				pBmpInfoHd = (PBITMAPINFOHEADER)(pbuf + sizeof(BITMAPFILEHEADER));

				if (pBmpInfoHd->biBitCount != 1)
				{
					if (pBmpInfoHd->biBitCount == 0)
					{
						printf("Not BMP format, 0 bit per pixel.\r\n");
					}
					else if (pBmpInfoHd->biBitCount == 4)
					{
						printf("Not support 16 Color BMP file, 4 bit per pixel.\r\n");
					}
					else if (pBmpInfoHd->biBitCount == 8)
					{
						printf("Not support 256 Color BMP file, 8 bit per pixel.\r\n");
					}
					else if (pBmpInfoHd->biBitCount == 16)
					{
						printf("Not support 65536 Color BMP file, 16 bit per pixel.\r\n");
					}
					else if (pBmpInfoHd->biBitCount == 24)
					{
						printf("Not support 24 bits BMP file, 24 bit per pixel.\r\n");
					}
					else if (pBmpInfoHd->biBitCount == 32)
					{
						printf("Not support 32 bits BMP file, 32 bit per pixel.\r\n");
					}
					else
					{
						printf("Not support BMP format, 32 bit per pixel.");
						_tprintf(TEXT("Not support BMP format, %d bit per pixel.\r\n"), pBmpInfoHd->biBitCount);
					}
					delete[] pbuf;
					pbuf = NULL;
					continue;
				}

				bytePerLine = pBmpInfoHd->biSizeImage / pBmpInfoHd->biHeight;
				widthByte = (pBmpInfoHd->biWidth + 7) / 8;
				actualDataSize = widthByte * (pBmpInfoHd->biHeight);
				//if (pBmpInfoHd->biWidth < 32) shiftSize = 4 - widthByte;
				shiftSize = bytePerLine - widthByte;

				// Get array name
				pDir = get_filename_extT(ffd.cFileName);
				pDir--;
				size_t lenT = _tcslen(ffd.cFileName);
				lenT -= (pDir - ffd.cFileName);
				char *arrayName = new char[lenT + 1];
#ifdef UNICODE
				// TCHAR is unicode, convert to char
				int ansiSize = WideCharToMultiByte(CP_ACP, 0, ffd.cFileName, -1, NULL, 0, NULL, false);
				char *ansiStr = new char[ansiSize + 1];
				// wideChar write to char
				WideCharToMultiByte(CP_ACP, 0, ffd.cFileName, -1, ansiStr, ansiSize, NULL, false);
				memset(ansiStr + get_filename_ext_(ansiStr), 0, ansiSize - get_filename_ext_(ansiStr));
				strcpy(arrayName, ansiStr);
				if (ansiStr) delete[] ansiStr;
#endif
				//printf("arrayName: %s\r\n", arrayName);
				// Convert to C source data
				pCSrcDate = bin2CData(pBitDate, actualDataSize, pBmpInfoHd->biWidth, shiftSize, &cSrcdataLLen, arrayName);
				if (pCSrcDate == NULL)
				{
					printf("Error:Convert to C source\n");
					delete[] pbuf;
					pbuf = NULL;
					continue;
				}

				// Get new file name
				_tcscpy(newDir, ffd.cFileName);
				pDir = get_filename_extT(newDir);
				_tcscpy(pDir, _T("c")); // *.c

				// open new file
				dwError = _wfopen_s(&pfile, newDir, _T("w+b"));
				if (pfile == NULL)
				{
					delete[] pbuf;
					pbuf = NULL;
					delete[] pCSrcDate;
					pCSrcDate = NULL;
					printf("Error:open file, w+b\n");
					continue;
				}
				// write data to file
				retSize = (unsigned long)fwrite(pCSrcDate, sizeof(char), cSrcdataLLen, pfile);
				fclose(pfile);

				if (retSize != cSrcdataLLen)
				{
					delete[] pbuf;
					pbuf = NULL;
					delete[] pCSrcDate;
					pCSrcDate = NULL;
					printf("Write file fail.");
					continue;
				}
			}
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES)
	{
		DisplayErrorBox(TEXT("FindFirstFile"));
	}

	FindClose(hFind);

	if (pBuf) delete[] pBuf;
	printf("Pause ENTER key to exit.");
	getchar(); // wait new line
	return (dwError == ERROR_NO_MORE_FILES) ? 0 : dwError;
}

void DisplayErrorBox(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and clean up

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40)*sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}


