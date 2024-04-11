#include <iostream>
#include <Windows.h>
#include <vector>
#include <cstdint>
#include <filesystem>

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
			if (buffer[i + j] != hexArray[j] && hexArray[j] != 0x2A) {
				found = false;
				break;
			}
		}

		if (found)
		{
			std::cout << "[INFO] " << "Found position at buffer index: " << i << std::endl;
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
		for (size_t i = 0; i < replacedArray.size(); ++i)
		{
			if (replacedArray[i] != 0x2A)
			{
				buffer[pos + i] = replacedArray[i];
			}
		}
	}

	if (!writeFile(fileName, buffer))
	{
		return;
	}

	std::cout << "[SUCCESS] " << fileName << " has been patched." << std::endl;
}

static std::string SelectPEFile()
{
	TCHAR szBuffer[MAX_PATH] = { 0 };
	OPENFILENAME file = { 0 };

	ZeroMemory(&file, sizeof(file));

	file.hwndOwner = NULL;
	file.lStructSize = sizeof(file);
	file.lpstrFilter = L"vscript.dll(vscript.dll)\0vscript.dll\0所有文件(*.*)\0*.*\0\0";
	file.lpstrInitialDir = L"";
	file.lpstrFile = szBuffer;
	file.nMaxFile = sizeof(szBuffer) / sizeof(*szBuffer);
	file.nFilterIndex = 0;
	file.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&file))
	{
		int size = WideCharToMultiByte(CP_ACP, 0, file.lpstrFile, -1, NULL, 0, NULL, NULL);
		if (size == 0)
		{
			return std::string();
		}
		std::string result(size - 1, 0);
		WideCharToMultiByte(CP_ACP, 0, file.lpstrFile, -1, &result[0], size, NULL, NULL);
		return result;
	}
	else
	{
		return std::string();
	}
}

static void Pause()
{
	std::cout << "Press any key to continue..." << std::flush;
	HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
	FlushConsoleInputBuffer(h);
	WaitForSingleObject(h, INFINITE);
	FlushConsoleInputBuffer(h);
}

int main(int argc, const char * const * argv)
{
	std::string filePath;

	SetConsoleTitleA("黄埔军校 VScript Patcher");

	std::cout << "[VScript Patcher]" << std::endl;
	std::cout << "Created by Kroytz (https://github.com/Kroytz)" << std::endl;
	std::cout << "Method from Source2ZE (https://github.com/Source2ZE/CS2Fixes)" << std::endl;
	std::cout << "Code mainly from RealSkid (https://www.unknowncheats.me/forum/members/3339879.html)" << std::endl;
	//std::cout << "Place this executable in your <game> folder" << std::endl;
	std::cout << "  " << std::endl;
	std::cout << "  " << std::endl;

	//PatchFile(filePath, { 0xBE, 0x01, 0x2A, 0x2A, 0x2A, 0x2B, 0xD6, 0x74, 0x2A, 0x3B, 0xD6 }, { 0xBE, 0x02 });
	
	try
	{
		if (argc > 1)
		{
			filePath = argv[1];
			if (!std::filesystem::exists(filePath))
			{
				std::cerr << "Specified file path does not exist." << std::endl;
				return 2;
			}
		}
		else
		{
			filePath = "./bin/win64/vscript.dll";
		}

		if (!std::filesystem::exists(filePath))
		{
			filePath = SelectPEFile();
		}
		if (std::filesystem::exists(filePath))
		{
			std::filesystem::copy(filePath, filePath + ".bak");

			PatchFile(filePath, { 0xBE, 0x01, 0x2A, 0x2A, 0x2A, 0x2B, 0xD6, 0x74, 0x2A, 0x3B, 0xD6 }, { 0xBE, 0x02 });
		}
		else
		{
			std::cerr << "File doesn't exist" << std::endl;
			return 2;
		}

	}
	catch (const std::exception & e)
	{
		std::cerr << "An error occurred: " << e.what() << std::endl;

		Pause();

		return 1;
	}

	std::cout << "  " << std::endl;
	std::cout << "  " << std::endl;

	//system("pause");
	Pause();

	return 0;
}