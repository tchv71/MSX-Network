// SendCrc.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <Windows.h>
#include "..\..\crc32\crc32.h"
#include <random>

const size_t BUF_SIZE = 2048;

unsigned pos = 0;

BOOL GenRnd(LPVOID lpBuffer, DWORD nSize, DWORD* pCount)
{
	std::random_device rd; // Non-deterministic seed
	std::mt19937 gen(rd()); // Mersenne Twister engine
	std::uniform_int_distribution<int> dist(0, 255); // Uniform distribution
	BYTE* pBuf = (BYTE*)lpBuffer;
#if 1
	for (DWORD i = 0; i < nSize; ++i)
	{
		pBuf[i] = dist(gen);
	}
#else
	for (DWORD i = 0; i < nSize/2; ++i)
	{
		pBuf[i * 2] = pos & 255;
		pBuf[i * 2 + 1] = pos++ >> 8;
	}
#endif
	if (pCount)
		*pCount = nSize;
	return TRUE;
}


void copy_file_crc(HANDLE fp_out)
{
	unsigned char data[BUF_SIZE] = { 0 };
	int nStart = 0;
	DWORD count;

	BOOL r;
	while ((r = GenRnd(data, BUF_SIZE, &count)) && count != 0)
	{
		if (!WriteFile(fp_out, data, count, nullptr, nullptr))
			goto err_write;
		unsigned crc = xcrc32(data, BUF_SIZE, -1);
		if (!WriteFile(fp_out, &crc, sizeof(unsigned), nullptr, nullptr))
			goto err_write;
		RtlZeroMemory(data, BUF_SIZE);
		printf("*");
	}
	CloseHandle(fp_out);
	return;
err_write:
	perror("[-] Error in sending data");
	exit(1);
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("SendCrc - send random data over com with crc32\n");
		printf("\tSendCrc comn\n");
		return 2;
	}
	const char* comNameShort = argv[1];
	if (_strnicmp(comNameShort, "COM", 3) == 0)
	{
		HANDLE fp_out = NULL;
		char comName[] = "\\\\.\\" "\0    ";
		strcat_s(comName, sizeof(comName), comNameShort);
		fp_out = CreateFileA(comName,
			GENERIC_READ | GENERIC_WRITE,
			0,              // exclusive access
			NULL,           // no security attrs
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (fp_out == INVALID_HANDLE_VALUE)
		{
			perror("[-]Error in writing file.");
			exit(1);
		}
		copy_file_crc(fp_out);
		printf("[+] File data send successfully. \n");
		return 0;
	}
}

