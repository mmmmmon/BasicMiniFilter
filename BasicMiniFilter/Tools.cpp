#include "Public.h"

namespace Tools
{
	HANDLE CreateKey(PUNICODE_STRING pRegKeyPath) 
	{
		HANDLE regCreateKeyHandle = nullptr;
		OBJECT_ATTRIBUTES	ObjectAttribute = { 0 };
		ULONG disposition = 0;
		InitializeObjectAttributes(&ObjectAttribute, pRegKeyPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
		ZwCreateKey(&regCreateKeyHandle, KEY_ALL_ACCESS, &ObjectAttribute, 0, NULL, REG_OPTION_VOLATILE, &disposition);
		return regCreateKeyHandle;
	}
	bool SetValueKey(HANDLE pRegKeyHandle, wchar_t* pRegValueName, PVOID pRegData, ULONG pRegDataSize, ULONG pType)
	{
		if (nullptr == pRegValueName || nullptr == pRegData || 0 == pRegDataSize)
		{
			return false;
		}
		UNICODE_STRING regValueName = { 0 };
		RtlInitUnicodeString(&regValueName, pRegValueName);
		return	NT_SUCCESS(ZwSetValueKey(pRegKeyHandle, &regValueName, 0, pType, pRegData, pRegDataSize));
	}
	bool CloseHandle(HANDLE pHandle)
	{
		return NT_SUCCESS(ZwClose(pHandle));
	}
	os_version GetOsVersion()
	{
		os_version osVersion = os_version::unknow;
		RTL_OSVERSIONINFOW osVersionInfor = { 0 };
		osVersionInfor.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

		if (false != NT_SUCCESS(RtlGetVersion(&osVersionInfor)))
		{
			if (osVersionInfor.dwMajorVersion == 6)
			{
				if (osVersionInfor.dwMinorVersion == 1)
				{
					osVersion = os_version::win7;
				}
				if (osVersionInfor.dwMinorVersion == 2)
				{
					osVersion = os_version::win8;
				}
				if (osVersionInfor.dwMinorVersion == 3)
				{
					osVersion = os_version::win8_1;
				}
			}
			else if (osVersionInfor.dwMajorVersion == 10 && osVersionInfor.dwMinorVersion == 0)
			{
				osVersion = os_version::win10;
			}
		}
		return osVersion;
	}
	unsigned long GetOsBuildNumber()
	{
		RTL_OSVERSIONINFOW osVersionInfor = { 0 };
		if (false != NT_SUCCESS(RtlGetVersion(&osVersionInfor)))
		{
			return osVersionInfor.dwBuildNumber;
		}
		return 0;
	}
	bool FilterGetFileName(PFLT_CALLBACK_DATA pData, FLT_FILE_NAME_OPTIONS pNameOption, PFLT_FILE_NAME_INFORMATION *pFileNameInformation)
	{
		bool result = false;
		do
		{
			if (nullptr == pFileNameInformation)
			{
				break;
			}
			if (false == NT_SUCCESS(FltGetFileNameInformation(pData, pNameOption, pFileNameInformation)))
			{
				break;
			}
			if (false == NT_SUCCESS(FltParseFileNameInformation(*pFileNameInformation)))
			{
				break;
			}
			result = true;
		} while (false);
		return result;
	}
	bool RtlAppendPath(PUNICODE_STRING pPath,wchar_t* pAddPath)
	{
		bool result = false;
		UNICODE_STRING tempPath = { 0 };
		if (nullptr != pPath)
		{
			tempPath.Length = pPath->Length;
			tempPath.MaximumLength = pPath->MaximumLength;
			tempPath.Buffer = reinterpret_cast<wchar_t*>(ExAllocatePoolWithTag(NonPagedPool, pPath->MaximumLength, DerKiTag));
			if (false != NT_SUCCESS(RtlUnicodeStringCopy(&tempPath, pPath)))
			{
				if (false != NT_SUCCESS(RtlAppendUnicodeToString(&tempPath, L"\\")))
				{
					result = NT_SUCCESS(RtlAppendUnicodeToString(&tempPath, pAddPath));
				}
			}
			if (false != result)
			{
				result = NT_SUCCESS(RtlUnicodeStringCopy(pPath, &tempPath));
			}
		}
		if (nullptr != tempPath.Buffer)
		{
			ExFreePoolWithTag(tempPath.Buffer, DerKiTag);
		}
		return result;
	}
}