#pragma once
class FileSystem {
protected:

	HANDLE fileHander;

	long GetDec(byte vBootRecord[],int size)
	{
		long sum=0;
		for (int i = 0; i < size; i++)
		{
			sum+=vBootRecord[i]*std::pow(16,i*2);
		}
		return sum;
	}

    void ReedBootRecord(wchar_t* path, byte *dataBuffer){

    fileHander=CreateFileW(path,
	GENERIC_READ,
	FILE_SHARE_READ | FILE_SHARE_WRITE,
	NULL,
	OPEN_EXISTING,
	FILE_ATTRIBUTE_NORMAL,
	NULL);

	if(fileHander == INVALID_HANDLE_VALUE)
	{
		std::wcout<<L"������ ������ �����. ����������� ����� ��������������.\n";
		CloseHandle(fileHander);
		system("pause");
	}

	LARGE_INTEGER sectorOffset;
	sectorOffset.QuadPart=0;


	unsigned long currentPosition = SetFilePointer(fileHander,sectorOffset.LowPart,&sectorOffset.HighPart ,FILE_BEGIN);

	 if(currentPosition != sectorOffset.LowPart)  {
		 CloseHandle(fileHander);
		 delete[] dataBuffer;
	 }

	 DWORD bytesToRead=512;
	 DWORD bytesRead;
	 bool readResult = ReadFile(fileHander,dataBuffer,bytesToRead,&bytesRead,NULL);

	 if(readResult == false || bytesRead != bytesToRead){
		CloseHandle(fileHander);
		delete[] dataBuffer;
	 }

	}


public:
	unsigned long long size;
	unsigned long long countCluster;
	int clusterSize;

	void ViewInfo(){
	std::wcout<<L"������: "<< size<<L" ����"<<std::endl;
	std::wcout<<L"���������� ���������: "<< countCluster<<std::endl;
	std::wcout<<L"������ ��������: "<< clusterSize<<L" ����"<<std::endl;
	}

    void ReedCluster(int number, BYTE *outBuffer){
	LARGE_INTEGER sectorOffset;
	sectorOffset.QuadPart=clusterSize*number;

	unsigned long currentPosition = SetFilePointer(fileHander,sectorOffset.LowPart,&sectorOffset.HighPart ,FILE_BEGIN);

	 if(currentPosition != sectorOffset.LowPart)  {
	 std::wcout<<L"������ ��������� �������.\n";
	 }

	 DWORD bytesToRead=clusterSize;
	 DWORD bytesRead;
	 bool readResult = ReadFile(fileHander,outBuffer,bytesToRead,&bytesRead,NULL);

	 if(readResult == false || bytesRead != bytesToRead){
		 std::wcout<<L"������ ������ ��������.\n";
	 }
	}

};
