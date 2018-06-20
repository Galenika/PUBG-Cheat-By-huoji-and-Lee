#include "stdafx.h"
#include "strenc.h"
char *MD5_file(char *path, int md5_len)
{
	FILE *fp = fopen(path, "rb");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];
	char *file_md5;
	int i;
	if (fp == NULL) {
		fprintf(stderr, "fopen %s failed\n", path);
		return NULL;
	}
	MD5Init(&mdContext);
	while ((bytes = fread(data, 1, 1024, fp)) != 0)
	{
		MD5Update(&mdContext, data, bytes);
	}
	MD5Final(&mdContext, data);

	file_md5 = (char *)malloc((md5_len + 1) * sizeof(char));
	if (file_md5 == NULL)
	{
		fprintf(stderr, "malloc failed.\n");
		return NULL;
	}
	memset(file_md5, 0, (md5_len + 1));

	if (md5_len == 16)
	{
		for (i = 4; i<12; i++)
		{
			sprintf(&file_md5[(i - 4) * 2], "%02x", data[i]);
		}
	}
	else if (md5_len == 32)
	{
		for (i = 0; i<16; i++)
		{
			sprintf(&file_md5[i * 2], "%02x", data[i]);
		}
	}
	else
	{
		fclose(fp);
		free(file_md5);
		return NULL;
	}

	fclose(fp);
	return file_md5;
}
char* ConvertLPWSTRToLPSTR(LPWSTR lpwszStrIn)
{
	LPSTR pszOut = NULL;
	if (lpwszStrIn != NULL)
	{
		int nInputStrLen = wcslen(lpwszStrIn);

		// Double NULL Termination  
		int nOutputStrLen = WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, NULL, 0, 0, 0) + 2;
		pszOut = new char[nOutputStrLen];

		if (pszOut)
		{
			memset(pszOut, 0x00, nOutputStrLen);
			WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, pszOut, nOutputStrLen, 0, 0);
		}
	}
	return pszOut;
}
std::wstring string_to_wstring(const std::string& astr, UINT cp = GetACP())
{
	if (astr.empty())
		return std::wstring();

	const char* a_str = astr.c_str();
	int a_len = (int)astr.size();

	int u_len = MultiByteToWideChar(cp, 0, a_str, a_len, NULL, 0);
	if (u_len == 0)
		return std::wstring();

	std::vector<wchar_t> wstr(u_len);
	wchar_t* u_str = &wstr.front();
	MultiByteToWideChar(cp, 0, a_str, a_len, u_str, u_len);

	return std::wstring(wstr.begin(), wstr.end());
}
string GetFieMd5()
{
	char *md5;

	TCHAR szPath[MAX_PATH];

	if (!GetModuleFileName(NULL, szPath, MAX_PATH))
	{
		return 0;
	}
	OutputDebugString(szPath);
	//	wprintf(L"%s", szPath);
	char *shit = ConvertLPWSTRToLPSTR(szPath);

	md5 = MD5_file((char*)shit, 32);
	//	printf("32: %s\n", md5);
	string s(md5);
	return s;
}
string GetStringMd5(string strPlain)
{
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];

	MD5Init(&mdContext);
	MD5Update(&mdContext, (unsigned char*)const_cast<char*>(strPlain.c_str()), strPlain.size());
	MD5Final(&mdContext, data);

	string md5;
	char buf[3];
	for (int i = 0; i < 16; i++)
	{
		sprintf(buf, "%02x", data[i]);
		md5.append(buf);
	}
	return md5;
}
bool AntiLeak::MainCheck()
{
	Vars.Lazy.Flags = false;
	//PIP_ADAPTER_INFO�ṹ��ָ��洢����������Ϣ
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
	//�õ��ṹ���С,����GetAdaptersInfo����
	unsigned long stSize = sizeof(IP_ADAPTER_INFO);
	//����GetAdaptersInfo����,���pIpAdapterInfoָ�����;����stSize��������һ��������Ҳ��һ�������
	int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	//��¼��������
	int netCardNum = 0;
	//��¼ÿ�������ϵ�IP��ַ����
	int IPnumPerNetCard = 0;
	string MD5;
	if (ERROR_BUFFER_OVERFLOW == nRel)
	{
		//����������ص���ERROR_BUFFER_OVERFLOW
		//��˵��GetAdaptersInfo�������ݵ��ڴ�ռ䲻��,ͬʱ�䴫��stSize,��ʾ��Ҫ�Ŀռ��С
		//��Ҳ��˵��ΪʲôstSize����һ��������Ҳ��һ�������
		//�ͷ�ԭ�����ڴ�ռ�
		delete pIpAdapterInfo;
		//���������ڴ�ռ������洢����������Ϣ
		pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
		//�ٴε���GetAdaptersInfo����,���pIpAdapterInfoָ�����
		nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	}
	if (ERROR_SUCCESS == nRel)
	{
		//���������Ϣ
		//�����ж�����,���ͨ��ѭ��ȥ�ж�
		while (pIpAdapterInfo)
		{
			string s(pIpAdapterInfo->AdapterName);
			MD5 = MD5 + s;
			pIpAdapterInfo = pIpAdapterInfo->Next;

		}

	}


	if (pIpAdapterInfo)
	{
		delete pIpAdapterInfo;
	}
	//cout << GetFieMd5().c_str() << endl;
	/*
	time_t tt = time(NULL);
	tm* t = localtime(&tt);
	char buffer[128];
	sprintf_s(buffer, "%d-%02d-%02d%02d:%02d",
		t->tm_year + 1900,
		t->tm_mon + 1,
		t->tm_mday,
		t->tm_hour,
		t->tm_min);
	
	string aTime(buffer);
	
	Vars.Lazy.Flags = true;
	Vars.Lazy.key = GetStringMd5(GetStringMd5(GetFieMd5() + MD5) + aTime);
	*/
	Vars.Lazy.Flags = true;
	Vars.Lazy.key = GetStringMd5(GetStringMd5(GetFieMd5() + MD5) + Vars.Lazy.Time);
	if (string_to_wstring(Vars.Lazy.GCmdLine) != string_to_wstring(Vars.Lazy.key))
	{
		exit(1);//���������е����˳�����
		Vars.Lazy.Flags = false;
	}


	return true;
}
void AntiLeak::SecCheck()
{
	if (!Vars.Lazy.Flags)
	{
	//	MessageBox(0, L"exit!!!!", L"exit!!!!", MB_OK | MB_ICONERROR);
		exit(1);
	}
	
	//PIP_ADAPTER_INFO�ṹ��ָ��洢����������Ϣ
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
	//�õ��ṹ���С,����GetAdaptersInfo����
	unsigned long stSize = sizeof(IP_ADAPTER_INFO);
	//����GetAdaptersInfo����,���pIpAdapterInfoָ�����;����stSize��������һ��������Ҳ��һ�������
	int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	//��¼��������
	int netCardNum = 0;
	//��¼ÿ�������ϵ�IP��ַ����
	int IPnumPerNetCard = 0;
	string MD5;
	if (ERROR_BUFFER_OVERFLOW == nRel)
	{
		//����������ص���ERROR_BUFFER_OVERFLOW
		//��˵��GetAdaptersInfo�������ݵ��ڴ�ռ䲻��,ͬʱ�䴫��stSize,��ʾ��Ҫ�Ŀռ��С
		//��Ҳ��˵��ΪʲôstSize����һ��������Ҳ��һ�������
		//�ͷ�ԭ�����ڴ�ռ�
		delete pIpAdapterInfo;
		//���������ڴ�ռ������洢����������Ϣ
		pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
		//�ٴε���GetAdaptersInfo����,���pIpAdapterInfoָ�����
		nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	}
	if (ERROR_SUCCESS == nRel)
	{
		//���������Ϣ
		//�����ж�����,���ͨ��ѭ��ȥ�ж�
		while (pIpAdapterInfo)
		{
			string s(pIpAdapterInfo->AdapterName);
			MD5 = MD5 + s;
			pIpAdapterInfo = pIpAdapterInfo->Next;

		}

	}
	if (pIpAdapterInfo)
	{
		delete pIpAdapterInfo;
	}
//	cout << GetFieMd5().c_str() << endl;

	
	Vars.Lazy.key = GetStringMd5(GetStringMd5(GetFieMd5() + MD5) + Vars.Lazy.Time);
//	delete buffer;
	if (string_to_wstring(Vars.Lazy.GCmdLine) != string_to_wstring(Vars.Lazy.key))
	{
		//MessageBox(0, L"string_to_wstring!!!!", L"string_to_wstring!!!!", MB_OK | MB_ICONERROR);
		exit(1);//���������е����˳�����
		Vars.Lazy.Flags = false;
	}

}
