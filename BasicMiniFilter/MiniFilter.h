#pragma once

namespace MiniFilter
{
	enum class operation
		{
			read,
			write,
			create,
            dir_control,
			unknow
		};
	typedef struct _CONTEXT_FILE_OBJECT
		{
			PFLT_FILE_NAME_INFORMATION FileNameInformation;
			operation Operation;
			HANDLE ProcessId;
			HANDLE ThreadId;
		}CONTEXT_FILE_OBJECT,*PCONTEXT_FILE_OBJECT;
    typedef struct _FLT_REGISTRATION_WIN7 {
            USHORT Size;
            USHORT Version;
            FLT_REGISTRATION_FLAGS Flags;
            CONST FLT_CONTEXT_REGISTRATION* ContextRegistration;
            CONST FLT_OPERATION_REGISTRATION* OperationRegistration;
            PFLT_FILTER_UNLOAD_CALLBACK FilterUnloadCallback;
            PFLT_INSTANCE_SETUP_CALLBACK InstanceSetupCallback;
            PFLT_INSTANCE_QUERY_TEARDOWN_CALLBACK InstanceQueryTeardownCallback;
            PFLT_INSTANCE_TEARDOWN_CALLBACK InstanceTeardownStartCallback;
            PFLT_INSTANCE_TEARDOWN_CALLBACK InstanceTeardownCompleteCallback;
            PFLT_GENERATE_FILE_NAME GenerateFileNameCallback;
            PFLT_NORMALIZE_NAME_COMPONENT NormalizeNameComponentCallback;
            PFLT_NORMALIZE_CONTEXT_CLEANUP NormalizeContextCleanupCallback;
            PFLT_TRANSACTION_NOTIFICATION_CALLBACK TransactionNotificationCallback;
            PFLT_NORMALIZE_NAME_COMPONENT_EX NormalizeNameComponentExCallback;
        } FLT_REGISTRATION_WIN7, * PFLT_REGISTRATION_WIN7;
    typedef struct _FLT_REGISTRATION_WIN8 {
            USHORT Size;
            USHORT Version;
            FLT_REGISTRATION_FLAGS Flags;
            CONST FLT_CONTEXT_REGISTRATION* ContextRegistration;
            CONST FLT_OPERATION_REGISTRATION* OperationRegistration;
            PFLT_FILTER_UNLOAD_CALLBACK FilterUnloadCallback;
            PFLT_INSTANCE_SETUP_CALLBACK InstanceSetupCallback;
            PFLT_INSTANCE_QUERY_TEARDOWN_CALLBACK InstanceQueryTeardownCallback;
            PFLT_INSTANCE_TEARDOWN_CALLBACK InstanceTeardownStartCallback;
            PFLT_INSTANCE_TEARDOWN_CALLBACK InstanceTeardownCompleteCallback;
            PFLT_GENERATE_FILE_NAME GenerateFileNameCallback;
            PFLT_NORMALIZE_NAME_COMPONENT NormalizeNameComponentCallback;
            PFLT_NORMALIZE_CONTEXT_CLEANUP NormalizeContextCleanupCallback;
            PFLT_TRANSACTION_NOTIFICATION_CALLBACK TransactionNotificationCallback;
            PFLT_NORMALIZE_NAME_COMPONENT_EX NormalizeNameComponentExCallback;
            PFLT_SECTION_CONFLICT_NOTIFICATION_CALLBACK SectionNotificationCallback;
        } FLT_REGISTRATION_WIN8, * PFLT_REGISTRATION_WIN8;

	NTSTATUS Register(PDRIVER_OBJECT pDrvObj);
	bool InstanceInstall(PUNICODE_STRING pDrvRegPath, unsigned long pAltitude);
	NTSTATUS Start();
	void Stop();
}
