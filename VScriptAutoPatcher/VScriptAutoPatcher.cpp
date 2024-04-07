#include <iostream>
#include <Windows.h>
#include <vector>
#include <cstdint>

bool readFile(const std::string& filename, std::vector<uint8_t>& buffer)
{
	HANDLE fileHandle = CreateFileA(
		filename.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		std::cerr << "[ERROR] " << "Failed to create handle to: " << filename << std::endl;
		//std::cout << "[REMINDER] " << "Make sure the executable is in same folder with: " << filename << std::endl;
		return false;
	}

	DWORD fileSize = GetFileSize(fileHandle, NULL);
	buffer.resize(fileSize);

	DWORD bytesRead = 0;
	ReadFile(fileHandle, buffer.data(), fileSize, &bytesRead, NULL);

	CloseHandle(fileHandle);

	return true;
}

std::vector<size_t> findHexArray(std::vector<uint8_t>& buffer, const std::vector<uint8_t>& hexArray)
{
	std::vector<size_t> positions;

	size_t bufferSize = buffer.size();
	size_t arraySize = hexArray.size();

	for (size_t i = 0; i <= bufferSize - arraySize; ++i)
	{
		bool found = true;

		for (size_t j = 0; j < arraySize; ++j) {
			if (buffer[i + j] != hexArray[j] && hexArray[j] != 0x2a) {
				found = false;
				break;
			}
		}

		if (found)
		{
			for (size_t j = 0; j < arraySize; ++j) {
				buffer[i + j] = hexArray[j];
			}

			positions.push_back(i);
		}
	}

	return positions;
}

bool writeFile(const std::string& filename, const std::vector<uint8_t>& buffer)
{
	HANDLE fileHandle = CreateFileA(
		filename.c_str(),
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		std::cerr << "[ERROR] " << "Failed to create handle to: " << filename << std::endl;
		//std::cout << "[REMINDER] " << "Make sure the executable is in same folder with: " << filename << std::endl;

		return false;
	}

	DWORD bytesWritten = 0;
	WriteFile(fileHandle, buffer.data(), buffer.size(), &bytesWritten, NULL);

	CloseHandle(fileHandle);

	return (bytesWritten == buffer.size());
}

void PatchFile(std::string fileName, std::vector<uint8_t> originalArray, std::vector<uint8_t> replacedArray)
{
	std::vector<uint8_t> buffer;
	if (!readFile(fileName, buffer))
	{
		return;
	}

	std::vector<size_t> positions = findHexArray(buffer, originalArray);

	if (positions.empty())
	{
		std::cout << "[ERROR] " << fileName << " is already patched" << std::endl;
		return;
	}

	for (size_t pos : positions)
	{
		for (size_t i = 0; i < originalArray.size(); ++i)
		{
			buffer[pos + i] = replacedArray[i];
		}
	}

	if (!writeFile(fileName, buffer))
	{
		return;
	}

	std::cout << "[SUCCESS] " << fileName << " has been patched." << std::endl;
}

int main()
{
	std::string filePath = "./bin/win64/vscript.dll";

	SetConsoleTitleA("���Ҿ�У VScript Patcher");

	std::cout << "[VScript Patcher]" << std::endl;
	std::cout << "Created by Kroytz (https://github.com/Kroytz)" << std::endl;
	std::cout << "Code mainly from RealSkid (https://www.unknowncheats.me/forum/members/3339879.html)" << std::endl;
	std::cout << "Method from Source2ZE (https://github.com/Source2ZE/CS2Fixes)" << std::endl;
	std::cout << "Place this executable in your <game> folder" << std::endl;
	std::cout << "  " << std::endl;
	std::cout << "  " << std::endl;

	PatchFile(filePath, { 0xBE, 0x01, 0x2A, 0x2A, 0x2A, 0x2B, 0xD6, 0x74, 0x2A, 0x3B, 0xD6 }, { 0xBE, 0x02, 0x2A, 0x2A, 0x2A, 0x2B, 0xD6, 0x74, 0x2A, 0x3B, 0xD6 });
	std::cout << "  " << std::endl;
	std::cout << "  " << std::endl;

	system("pause");

	return 0;
}