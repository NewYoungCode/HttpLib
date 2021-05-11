#pragma once
#include <stdio.h>
#include <malloc.h>
#include <string>
#include <direct.h> //����Ŀ¼���
#include <functional>
#include <fstream>
#include <vector>
#include <sstream>
#include <Windows.h>
#include <sys/stat.h>

#include "String.hpp"

namespace File {
	//�����ļ�
	bool Create(const std::string& filename);
	//ɾ���ļ�
	bool Delete(const std::string& filename);
	//�ж��ļ��Ƿ����
	bool Exists(const std::string& filename);
	//�ļ��ƶ����߸���
	bool Move(const std::string& oldname, const std::string& newname);
	//��ȡ�ļ���out����
	void ReadFile(const  std::string& filename, std::string& outData);

	//д���ļ�
	void WriteFile(const std::stringstream& data, const std::string& filename);
}
namespace Path {
	//�Լ�д���ļ������
	class FileWatcher {
	private:
		std::string math = "*.*";
		std::string path;
		std::function<void(const std::string& filename)> callback = NULL;
		size_t sleep;
		bool ctn = true;
		void TaskFunc();
	public:
		FileWatcher(const std::string& path, const std::string& math, const std::function<void(const std::string& filename)>& callback, size_t sleep = 500);
		~FileWatcher();
	};

	//����·��  MultiDir:�Ƿ񴴽��༶Ŀ¼
	bool Create(const std::string& path);
	//ɾ��·�� ����������ļ��л����ļ� ����ݹ�ɾ��
	bool Delete(const std::string& directoryName);
	//ͨ��������ļ�
	std::vector<std::string> SearchFiles(const std::string& path, const std::string& pattern);
	//���·���Ƿ����
	bool Exists(const std::string& path);
	//��ȡ�ļ�����(�ļ�����)
	std::string GetFileNameWithoutExtension(const std::string& _filename);
	//��ȡ�ļ�Ŀ¼����(����Ŀ¼)
	std::string GetDirectoryName(const std::string& _filename);
	//��ȡ�ļ�����+��׺
	std::string GetFileName(const std::string& _filename);
	//��ȡ�ļ���׺��(��׺��)
	std::string GetExtension(const std::string& _filename);
	//��ȡ�������ھ���·��Ŀ¼
	std::string	StartPath();
	//��ȡ�������ھ���·��Fullpath
	std::string	GetModuleFileName();
}

namespace FileSystem {
	typedef enum :unsigned char {
		None,
		File,
		Directory
	}FileType;
	struct FileInfo
	{
	private:
		std::ifstream* fs = NULL;
	public:
		unsigned long long StreamPos = 0;
		struct _stat64 __stat;
		FileType FileType = FileSystem::FileType::None;
		std::string Extension;
		std::string FullName;
		std::string FileName;
		bool ReadOnly = false;
		size_t Read(char* _buf_, size_t _rdCount = 512) {
			size_t rdbufCount = _rdCount;
			if (StreamPos + _rdCount >= __stat.st_size) {
				rdbufCount = __stat.st_size - StreamPos;
			}
			if (rdbufCount == 0) {
				return 0;
			}
			if (fs == NULL) {
				fs = new std::ifstream(FullName, std::ios::binary);
			}
			fs->seekg(StreamPos);
			fs->read(_buf_, rdbufCount);
			StreamPos += rdbufCount;
			return rdbufCount;
		}
		FileInfo() {}
		FileInfo(const std::string& filename) {
			int status = _stat64(filename.c_str(), &__stat);
			if (status == 0 && (__stat.st_mode & S_IFREG) == S_IFREG) {
				Extension = Path::GetExtension(filename);
				FileName = Path::GetFileName(filename);
				FullName = filename;
				FileType = FileType::File;
			}
		}
		void Close() {
			if (fs) {
				fs->close();
				delete fs;
				fs = NULL;
			}
		}
		~FileInfo() {
			if (fs) {
				fs->close();
				delete fs;
			}
		}
	};
	void ReadFileInfoWin32(const std::string& directory, WIN32_FIND_DATAA& pNextInfo, std::vector<FileSystem::FileInfo>& result);
	size_t  Find(const std::string& directory, std::vector<FileSystem::FileInfo>& result, const std::string& pattern = "*.*");
}
//����....................................................................................................................
namespace FileSystem {
	
	inline size_t  Find(const std::string& directory, std::vector<FileSystem::FileInfo>& result, const std::string& pattern) {
		HANDLE hFile = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATAA pNextInfo;
		hFile = FindFirstFileA((directory + "\\" + pattern).c_str(), &pNextInfo);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			return 0;
		}
		if (pNextInfo.cFileName[0] != '.') {
			ReadFileInfoWin32(directory, pNextInfo, result);
		}
		while (FindNextFileA(hFile, &pNextInfo))
		{
			if (pNextInfo.cFileName[0] != '.') {
				ReadFileInfoWin32(directory, pNextInfo, result);
			}
		}
		FindClose(hFile);//�����ڴ�й©
		return result.size();
	};
};
namespace File {
#ifdef _WINDEF_
	inline bool Exists(const std::string& filename) {
		DWORD dwAttr = GetFileAttributesA(filename.c_str());
		if (dwAttr == DWORD(-1)) {
			return false;
		}
		if (dwAttr & FILE_ATTRIBUTE_ARCHIVE) {
			return true;
		}
		return false;
	}
#else
	//��stat������windows�����¿��ܻ��������
	inline bool Exists(const std::string& filename) {
		struct stat buf;
		int status = stat(filename.c_str(), &buf);
		if (status == 0 && (buf.st_mode & S_IFREG) == S_IFREG) {
			return true;
		}
		else {
			return false;
		}
	}
#endif
	inline bool Create(const std::string& filename) {
		File::Delete(filename);
		std::ofstream ofs(filename.c_str(), std::ios::app);
		ofs.flush();
		ofs.close();
		return true;
	}
	inline std::string ReadFile(const std::string& filename) {
		std::stringstream ss;
		std::ifstream ifs(filename, std::ios::binary);
		ss << ifs.rdbuf();
		return ss.str();
	}
	inline bool Delete(const std::string& filename) {
		::remove(filename.c_str());
		return !File::Exists(filename);
	}
	inline bool Move(const std::string& oldname, const std::string& newname) {
		if (!File::Delete(newname)) {
			printf("Move Faild ! The target file is in use\n");
			return false;
		}
		int code = ::rename(oldname.c_str(), newname.c_str());
		if (File::Exists(oldname)) {
			return false;
		}
		return true;
	}
	inline void ReadFile(const  std::string& filename, std::string& outData) {
		outData.clear();
		std::ifstream* ifs = new std::ifstream(filename.c_str(), std::ios::binary);
		std::stringstream ss;
		ss << ifs->rdbuf();
		ifs->close();
		outData = ss.str();
		delete ifs;
	}
	inline void WriteFile(const std::stringstream& data, const std::string& filename)
	{
		std::string buf = data.str();
		File::Delete(filename);
		std::ofstream* ofs = new std::ofstream(filename, std::ios::binary);
		ofs->write(buf.c_str(), buf.size());
		ofs->flush();
		ofs->close();
		delete ofs;
	}

};
namespace Path {
	inline void FileWatcher::TaskFunc()
	{
		std::vector<std::string> files;//�������ص�ǰ�ļ�
		//std::vector<std::string> files = Path::SearchFiles(path, math.c_str());
		for (; exit; )
		{
			//�Ƴ������ڵ��ļ�
			for (size_t i = 0; i < files.size(); i++)
			{
				if (!File::Exists(files[i]))
				{
					std::vector<std::string>::iterator it = std::find(files.begin(), files.end(), files[i]);
					if (it != files.end()) {
						files.erase(it);
					}
				}
			}
			//�ж��Ƿ��������ļ�s
			std::vector<std::string> tmp = Path::SearchFiles(path, math.c_str());
			for (auto& item : tmp)
			{
				if (find(files.begin(), files.end(), item) == files.end())
				{
					files.push_back(item);
					if (callback) {
						callback(item);
					}
				}
			}
			//ֵԽСԽ��׼
			Sleep(sleep);
		}
	}
	inline FileWatcher::FileWatcher(const std::string& path, const std::string& math, const std::function<void(const std::string& filename)>& callback, size_t sleep)
	{
		this->sleep = sleep;
		this->callback = callback;
		this->path = path;
		this->math = math;
		TaskFunc();
	}
	inline FileWatcher::~FileWatcher()
	{
		ctn = false;
	}
	
	inline bool Delete(const std::string& directoryName) {
		std::vector<FileSystem::FileInfo>result;
		FileSystem::Find(directoryName, result);
		for (auto& it : result) {
			if (it.FileType == FileSystem::FileType::File) {
				File::Delete(it.FullName);
			}
			if (it.FileType == FileSystem::FileType::Directory) {
				Path::Delete(it.FullName);
			}
		}
		::_rmdir(directoryName.c_str());
		return !Path::Exists(directoryName);
	}
	

#ifdef _WINDEF_
	inline bool Exists(const std::string& path) {
		DWORD dwAttr = GetFileAttributesA(path.c_str());
		if (dwAttr == DWORD(-1)) {
			return false;
		}
		if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
		{
			return true;
		}
		return false;
	}
#else
	//XPϵͳ���жϿ��ܻ�������
	inline bool Exists(const std::string& path)
	{
		struct stat buf;
		int status = stat(path.c_str(), &buf);
		if (status == 0) {
			return true;
		}
		return false;
	}
#endif

	inline std::string GetFileNameWithoutExtension(const std::string& _filename) {
		std::string newDir = String(_filename).Replace("\\", "/");
		int bPos = newDir.rfind("/");
		int ePos = newDir.rfind(".");
		newDir = newDir.substr(bPos + 1, ePos - bPos - 1);
		return newDir;
	}

	inline std::string GetExtension(const std::string& _filename) {
		size_t pos = _filename.rfind(".");
		return pos == size_t(-1) ? "" : _filename.substr(pos);
	}
	inline std::string GetDirectoryName(const std::string& _filename) {
		int pos = String(_filename).Replace("\\", "/").rfind("/");
		return _filename.substr(0, pos);
	}
	inline std::string GetFileName(const std::string& filename) {
		return Path::GetFileNameWithoutExtension(filename) + Path::GetExtension(filename);
	}
	inline std::string StartPath() {
		return Path::GetDirectoryName(GetModuleFileName());
	}
	inline std::string GetModuleFileName() {
		CHAR exeFullPath[MAX_PATH];
		::GetModuleFileNameA(NULL, exeFullPath, MAX_PATH);
		std::string filename = exeFullPath;
		return filename;
	}
}