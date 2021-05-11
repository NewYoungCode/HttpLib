#pragma once
#include <string>
#include <vector>
#include <Windows.h>
//继承std::string性能会好一点
class String :public std::string {
public:
	String() {}
	String(const char* cstr) : std::string(cstr) {}
	String(const std::string& str) : std::string(str) {}
	size_t Split(const String& ch_, std::vector<String>& result) const {
		String buf = *this;
		size_t pos = buf.find(ch_);
		if (pos == size_t(-1)) {
			result.push_back(*this);
			return result.size();
		}
		for (; pos != size_t(-1);) {
			result.push_back(buf.substr(0, pos));
			buf = buf.erase(0, pos + ch_.size());
			pos = buf.find(ch_);
			if (pos == size_t(-1)) {
				result.push_back(buf);
			}
		}
		return result.size();
	}
	String ToLower()const {
		const std::string& str = *this;
		char* cStr = (char*)malloc(str.size() + 1);
		size_t pos = 0;
		for (auto ch : str) {
			char newCh = ch;
			if (ch >= 65 && ch <= 90) {
				newCh = ch + 32;
			}
			cStr[pos] = newCh;
			pos++;
		}
		cStr[str.size()] = 0;
		std::string newStr = cStr;
		free(cStr);
		return newStr;
	}
	String ToUpper() const {
		const std::string& str = *this;
		char* cStr = (char*)malloc(str.size() + 1);
		size_t pos = 0;
		for (auto ch : str) {
			char newCh = ch;
			if (ch >= 97 && ch <= 122) {
				newCh = ch - 32;
			}
			cStr[pos] = newCh;
			pos++;
		}
		cStr[str.size()] = 0;
		std::string newStr = cStr;
		free(cStr);
		return newStr;
	}
	String& Replace(const String& oldText, const String& newText) {
		size_t pos;
		pos = (*this).find(oldText);
		size_t count = 0;
		for (; pos != std::string::npos;) {
			(*this).replace(pos, oldText.size(), newText);
			count++;
			pos = (*this).find(oldText);
		}
		return *this;
	}
	String& Trim() {
		char* buf = new char[size() + 1]{ 0 };
		int i = 0;
		for (char it : *this) {
			if (it != 32) {
				buf[i] = it;
				i++;
			}
		}
		this->erase(0, size());
		this->append(buf);
		delete[] buf;
		return *this;
	}
	String ANSIToUTF8()const {
		int nwLen = ::MultiByteToWideChar(CP_ACP, 0, c_str(), -1, NULL, 0);
		wchar_t* pwBuf = new wchar_t[nwLen + 1];
		ZeroMemory(pwBuf, nwLen * 2 + 2);
		::MultiByteToWideChar(CP_ACP, 0, c_str(), size(), pwBuf, nwLen);
		int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
		char* pBuf = new char[nLen + 1];
		ZeroMemory(pBuf, nLen + 1);
		::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
		std::string strRet(pBuf);
		delete[]pwBuf;
		delete[]pBuf;
		pwBuf = NULL;
		pBuf = NULL;
		return strRet;
	}
	String UTF8ToANSI()const {
		int nwLen = MultiByteToWideChar(CP_UTF8, 0, c_str(), -1, NULL, 0);
		wchar_t* pwBuf = new wchar_t[nwLen + 1];
		memset(pwBuf, 0, nwLen * 2 + 2);
		MultiByteToWideChar(CP_UTF8, 0, c_str(), length(), pwBuf, nwLen);
		int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
		char* pBuf = new char[nLen + 1];
		memset(pBuf, 0, nLen + 1);
		WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
		std::string strRet = pBuf;

		delete[]pBuf;
		delete[]pwBuf;
		pBuf = NULL;
		pwBuf = NULL;
		return strRet;
	}
	std::wstring ANSIToUniCode()const {
		int bytes = ::MultiByteToWideChar(CP_ACP, 0, c_str(), size(), NULL, 0);
		std::wstring wstrCmd;
		wstrCmd.resize(bytes);
		bytes = ::MultiByteToWideChar(CP_ACP, 0, c_str(), size(), const_cast<wchar_t*>(wstrCmd.c_str()), wstrCmd.size());
		return wstrCmd;
	}
};