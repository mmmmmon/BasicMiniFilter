#include "Public.h"

#ifdef __cplusplus
extern "C"
{
#endif 
	NTSTATUS DriverEntry(PDRIVER_OBJECT pDrvObj, PUNICODE_STRING pDrvRegPath)
	{
		NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
		do
		{
			if (false == MiniFilter::InstanceInstall(pDrvRegPath, 360000ul))
			{
				break;
			}
			ntStatus = MiniFilter::Register(pDrvObj);
			if (false == NT_SUCCESS(ntStatus))
			{
				break;
			}
			ntStatus = MiniFilter::Start();
			if (false == NT_SUCCESS(ntStatus))
			{
				break;
			}
		} while (false);
		return ntStatus;
	}
#ifdef __cplusplus
}
#endif