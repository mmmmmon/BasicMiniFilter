#include "Public.h"

namespace MiniFilter
{
	PFLT_FILTER	 fltHandle = nullptr;
	namespace CallBack
	{
		NTSTATUS fltUnloadCallBack(
			FLT_FILTER_UNLOAD_FLAGS pFlags
		)
			{
				UNREFERENCED_PARAMETER(pFlags);
				if (nullptr != fltHandle)
				{
					FltUnregisterFilter(fltHandle);
				}
				return STATUS_SUCCESS;
			}
		void fltContextCleanUp(
			PFLT_CONTEXT pContext,
			FLT_CONTEXT_TYPE pContextType
		)
			{
				if (FLT_STREAMHANDLE_CONTEXT == pContextType && nullptr != pContext)
				{
					const auto fltContext = reinterpret_cast<PCONTEXT_FILE_OBJECT>(pContext);
					if (nullptr != fltContext->FileNameInformation)
					{
						FltReleaseFileNameInformation(fltContext->FileNameInformation);
						fltContext->FileNameInformation = nullptr;
					}
				}
				return;
			}
		FLT_PREOP_CALLBACK_STATUS fltPreCallBack(
			PFLT_CALLBACK_DATA pData,
			PCFLT_RELATED_OBJECTS pFltObjects,
			PVOID* pCompletionContext
		)
			{
				UNREFERENCED_PARAMETER(pFltObjects);
				FLT_PREOP_CALLBACK_STATUS fltStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
				PCONTEXT_FILE_OBJECT fltContext = nullptr;
				do
				{
					PFLT_FILE_NAME_INFORMATION fileNameInformation = nullptr;
					auto operation = MiniFilter::operation::unknow;
					bool enableMonitor = false;
					if (false == NT_SUCCESS(pData->IoStatus.Status) || nullptr == pData->Thread || APC_LEVEL < KeGetCurrentIrql())
					{
						break;
					}
					switch (pData->Iopb->MajorFunction)
					{
						case IRP_MJ_READ:
						{
							operation = MiniFilter::operation::read;
							enableMonitor = true;
						}
						break;
						case IRP_MJ_WRITE:
						{
							operation = MiniFilter::operation::write;
						}
						break;
						case IRP_MJ_CREATE:
						{
							operation = MiniFilter::operation::create;
						}
						break;
						case IRP_MJ_DIRECTORY_CONTROL:
						{
							operation = MiniFilter::operation::dir_control;
							do
							{
								if (FileIdBothDirectoryInformation != pData->Iopb->Parameters.DirectoryControl.QueryDirectory.FileInformationClass && 0 >= pData->Iopb->Parameters.DirectoryControl.QueryDirectory.Length)
								{
									break;
								}
								if (false == Tools::FilterGetFileName(pData, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP, &fileNameInformation))
								{
									break;
								}
								UNICODE_STRING expressionPath = { 0 };
								RtlInitUnicodeString(&expressionPath, PROTECT_DIRECTORY_PATH_EXPRESSION);
								if (TRUE == FsRtlIsNameInExpression(&expressionPath, &fileNameInformation->Name, false, nullptr))
								{
									pData->IoStatus.Information = 0;
									pData->IoStatus.Status = STATUS_ACCESS_DENIED;
									fltStatus = FLT_PREOP_COMPLETE;
								}
							} while (false);
							if (nullptr != fileNameInformation)
							{
								FltReleaseFileNameInformation(fileNameInformation);
							}
						}
						break;
						default:
						{
							operation = MiniFilter::operation::unknow;
						}
						break;
					}
					if (false != enableMonitor)
					{
						if (false == NT_SUCCESS(FltAllocateContext(fltHandle, FLT_STREAMHANDLE_CONTEXT, sizeof(CONTEXT_FILE_OBJECT), NonPagedPool, reinterpret_cast<PFLT_CONTEXT*>(&fltContext))))
						{
							break;
						}
						RtlSecureZeroMemory(fltContext, sizeof(CONTEXT_FILE_OBJECT));
						if (false == Tools::FilterGetFileName(pData, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP, &fileNameInformation))
						{
							break;
						}
						const auto process = IoThreadToProcess(pData->Thread);
						if (nullptr == process)
						{
							break;
						}
						const auto threadId = PsGetThreadId(pData->Thread);
						const auto processId = PsGetProcessId(process);
						fltContext->FileNameInformation = fileNameInformation;
						fltContext->Operation = operation;
						fltContext->ProcessId = processId;
						fltContext->ThreadId = threadId;
						DbgPrint(
							"minifilter capture pre operation : file: %wZ ,operation: %d ,processid=%p ,threadid=%p \n",
							&fltContext->FileNameInformation->Name,
							fltContext->Operation,
							fltContext->ProcessId,
							fltContext->ThreadId);
						*pCompletionContext = fltContext;
						fltContext = nullptr;
					}
				} while (false);
				if (nullptr != fltContext)
				{
					FltReleaseContext(fltContext);
				}
				return fltStatus;
			}
		FLT_POSTOP_CALLBACK_STATUS fltPostCallBack(
			PFLT_CALLBACK_DATA pData,
			PCFLT_RELATED_OBJECTS pFltObjects,
			PVOID pCompletionContext,
			FLT_POST_OPERATION_FLAGS pFlags
		)
			{
				UNREFERENCED_PARAMETER(pFltObjects);
				FLT_POSTOP_CALLBACK_STATUS status = FLT_POSTOP_FINISHED_PROCESSING;
				if (true != FlagOn(pFlags, FLTFL_POST_OPERATION_DRAINING) && APC_LEVEL > KeGetCurrentIrql())
				{
					if (nullptr != pCompletionContext && false != NT_SUCCESS(pData->IoStatus.Status))
					{
						const auto fltContext = reinterpret_cast<PCONTEXT_FILE_OBJECT>(pCompletionContext);
						DbgPrint("minifilter capture post operation : file:%wZ,operation:%d,processid=%p,threadid=%p \n",
							&fltContext->FileNameInformation->Name,
							fltContext->Operation,
							fltContext->ProcessId,
							fltContext->ThreadId);
					}
				}
				if (nullptr != pCompletionContext)
				{
					FltReleaseContext(pCompletionContext);
				}
				return status;
			}
	}
	namespace RegistrationData
		{
			const FLT_OPERATION_REGISTRATION fltOperationRegistration[] =
			{
				{
					IRP_MJ_CREATE,
					0,
					CallBack::fltPreCallBack,
					CallBack::fltPostCallBack,
				},
				{
					IRP_MJ_READ,
					0,
					CallBack::fltPreCallBack,
					CallBack::fltPostCallBack,
				},
				{
					IRP_MJ_WRITE,
					0,
					CallBack::fltPreCallBack,
					CallBack::fltPostCallBack,
				},
				{
					IRP_MJ_DIRECTORY_CONTROL,
					0,
					CallBack::fltPreCallBack,
					CallBack::fltPostCallBack,
				},
				{
					IRP_MJ_OPERATION_END
				}
			};
			const FLT_CONTEXT_REGISTRATION fltContextRegistration[] =
			{
				{
					FLT_STREAMHANDLE_CONTEXT,
					0,
					CallBack::fltContextCleanUp,
					sizeof(CONTEXT_FILE_OBJECT),
					DerKiTag
				},
				{
					FLT_CONTEXT_END
				}
			};
			MiniFilter::FLT_REGISTRATION_WIN8 fltRegistrationWin8 =
			{
				sizeof(MiniFilter::FLT_REGISTRATION_WIN8),
				FLT_REGISTRATION_VERSION_0203,
				FLTFL_REGISTRATION_SUPPORT_NPFS_MSFS,
				fltContextRegistration,
				fltOperationRegistration,
				CallBack::fltUnloadCallBack,
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				nullptr
			};
			MiniFilter::FLT_REGISTRATION_WIN7 fltRegistrationWin7 =
			{
				sizeof(MiniFilter::FLT_REGISTRATION_WIN7),
				FLT_REGISTRATION_VERSION_0202,
				0,
				fltContextRegistration,
				fltOperationRegistration,
				CallBack::fltUnloadCallBack,
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				nullptr
			};
		}

	bool InstanceInstall(PUNICODE_STRING pDrvRegPath, unsigned long pAltitude)
		{
			bool result = false;
			HANDLE regInstanceHandle = nullptr;
			HANDLE regInstanceFolderHandle = nullptr;
			UNICODE_STRING regPath = { 0 };
			do
			{
				wchar_t instanceItem[] = L"BasicMiniFilter First Instance";
				wchar_t instance[] = L"Instances";
				unsigned long flags = 0ul;

				DECLARE_UNICODE_STRING_SIZE(altitude, 8);
				auto status = RtlIntegerToUnicodeString(pAltitude, 10, &altitude);
				if (false == NT_SUCCESS(status))
				{
					break;
				}
				const auto regPathFinallyLength = pDrvRegPath->MaximumLength + sizeof(instance) + sizeof(instanceItem) + sizeof(L"\\")*2;
				regPath.Length = pDrvRegPath->Length;
				regPath.MaximumLength = static_cast<unsigned short>(regPathFinallyLength);
				regPath.Buffer = reinterpret_cast<wchar_t*>(ExAllocatePoolWithTag(NonPagedPool, regPathFinallyLength, DerKiTag));
				status = RtlUnicodeStringCopy(&regPath, pDrvRegPath);
				if (false == NT_SUCCESS(status))
				{
					break;
				}
				if (false == Tools::RtlAppendPath(&regPath, instance))
				{
					break;
				}
				regInstanceFolderHandle = Tools::CreateKey(&regPath);
				if (nullptr == regInstanceFolderHandle)
				{
					break;
				}
				if (false == Tools::SetValueKey(regInstanceFolderHandle, L"DefaultInstance", instanceItem, sizeof(instanceItem), REG_SZ))
				{
					break;
				}
				if (false == Tools::RtlAppendPath(&regPath, instanceItem))
				{
					break;
				}
				regInstanceHandle = Tools::CreateKey(&regPath);
				if (nullptr == regInstanceHandle)
				{
					break;
				}
				if (false == Tools::SetValueKey(regInstanceHandle, L"Altitude", altitude.Buffer, altitude.Length, REG_SZ))
				{
					break;
				}
				if (false == Tools::SetValueKey(regInstanceHandle, L"Flags", &flags, sizeof(flags), REG_DWORD))
				{
					break;
				}
				result = true;
			} while (false);
			if (nullptr != regInstanceFolderHandle)
			{
				Tools::CloseHandle(regInstanceFolderHandle);
			}
			if (nullptr != regInstanceHandle)
			{
				Tools::CloseHandle(regInstanceHandle);
			}
			if (nullptr != regPath.Buffer)
			{
				ExFreePoolWithTag(regPath.Buffer, DerKiTag);
			}
			return result;
		}
	NTSTATUS Register(PDRIVER_OBJECT pDrvObj)
		{
			const auto osVersion = Tools::GetOsVersion();
			if (Tools::os_version::unknow == osVersion)
			{
				return false;
			}
			const auto fltRegistration = osVersion >= Tools::os_version::win8 ? reinterpret_cast<PFLT_REGISTRATION>(&RegistrationData::fltRegistrationWin8) : reinterpret_cast<PFLT_REGISTRATION>(&RegistrationData::fltRegistrationWin7);
			return FltRegisterFilter(pDrvObj, fltRegistration, &fltHandle);
		}
	void Stop()
		{
			if (nullptr != fltHandle)
			{
				FltUnregisterFilter(fltHandle);
			}
			return;
		}
	NTSTATUS Start()
		{
			const auto status = FltStartFiltering(fltHandle);
			if (false == NT_SUCCESS(status))
			{
				Stop();
			}
			return status;
		}
}
