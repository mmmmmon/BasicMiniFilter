#pragma once

namespace Tools
{
	enum class os_version
	{
		unknow,
		win7,
		win8,
		win8_1,
		win10
	};
	HANDLE CreateKey(PUNICODE_STRING pRegKeyPath);
	bool SetValueKey(HANDLE pRegKeyHandle, wchar_t* pRegValueName, PVOID pRegData, ULONG pRegDataSize, ULONG pType);
	bool CloseHandle(HANDLE pHandle);
	os_version GetOsVersion();
	unsigned long GetOsBuildNumber();
	bool FilterGetFileName(PFLT_CALLBACK_DATA pData, FLT_FILE_NAME_OPTIONS pNameOption, PFLT_FILE_NAME_INFORMATION* pFileNameInformation);
	bool RtlAppendPath(PUNICODE_STRING pPath, wchar_t* pAddPath);
}