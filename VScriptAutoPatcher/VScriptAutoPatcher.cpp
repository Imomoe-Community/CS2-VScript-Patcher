#include <iostream>
#include <vector>
#include <cstdint>
#include <filesystem>
#include <fstream>

#ifdef _WIN32
#include <Windows.h>
#include <conio.h>

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
	(void)_getch();
}

#else

static void Pause()
{

}

#endif // _WIN32

static bool readFile(const std::string & fileName, std::vector<uint8_t> & buffer)
{
	std::ifstream file(fileName, std::ios::binary);

	if (!file.is_open())
	{
		std::cerr << "[ERROR] " << "Failed to open file: " << fileName << std::endl;
		return false;
	}

	file.seekg(0, std::ios::end);
	std::streamsize fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	buffer.resize(static_cast<size_t>(fileSize));
	file.read(reinterpret_cast<char *>(buffer.data()), fileSize);

	file.close();
	return true;
}

static std::vector<size_t> findHexArray(const std::vector<uint8_t> & buffer, const std::vector<uint8_t> & hexArray)
{
	std::vector<size_t> positions;

	size_t bufferSize = buffer.size();
	size_t arraySize = hexArray.size();

	for (size_t i = 0; i <= bufferSize - arraySize; ++i)
	{
		if (std::ranges::equal(buffer.begin() + i, buffer.begin() + i + arraySize, hexArray.begin(), hexArray.end(),
							   [](uint8_t a, uint8_t b)
		{
			return a == b || b == 0x2A;
		}))
		{
			std::cout << "[INFO] " << "Found position at buffer index: " << i << std::endl;
			positions.push_back(i);
		}
	}

	return positions;
}

static bool writeFile(const std::string & fileName, const std::vector<uint8_t> & buffer)
{
	std::ofstream file(fileName, std::ios::binary);

	if (!file.is_open())
	{
		std::cerr << "[ERROR] " << "Failed to open file: " << fileName << std::endl;
		return false;
	}

	file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());

	file.close();

	return true;
}

static void patchFile(const std::string & fileName, const std::vector<uint8_t> & originalArray, const std::vector<uint8_t> & replacedArray)
{
	std::vector<uint8_t> buffer;

	if (!readFile(fileName, buffer))
	{
		return;
	}

	std::vector<size_t> positions = findHexArray(buffer, originalArray);

	if (positions.empty())
	{
		std::cerr << "[ERROR] " << fileName << " is already patched" << std::endl;
		return;
	}

	if (std::filesystem::exists(fileName + ".bak"))
	{
		std::filesystem::remove(fileName + ".bak");
	}
	std::filesystem::copy(fileName, fileName + ".bak");

	for (size_t pos : positions)
	{
		std::ranges::copy(replacedArray.begin(), replacedArray.end(), buffer.begin() + pos);
	}

	if (!writeFile(fileName, buffer))
	{
		return;
	}

	std::cout << "[SUCCESS] " << fileName << " has been patched." << std::endl;
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
#ifdef _WIN32
		if (!std::filesystem::exists(filePath))
		{
			filePath = SelectPEFile();
		}
#endif // _WIN32
		if (std::filesystem::exists(filePath))
		{
			patchFile(filePath, { 0xBE, 0x01, 0x2A, 0x2A, 0x2A, 0x2B, 0xD6, 0x74, 0x2A, 0x3B, 0xD6 }, { 0xBE, 0x02 });
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