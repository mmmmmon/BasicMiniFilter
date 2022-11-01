#pragma once
#define DerKiTag 'fuck'

typedef NTSTATUS
(FLTAPI* PFLT_SECTION_CONFLICT_NOTIFICATION_CALLBACK) (
    _In_ PFLT_INSTANCE Instance,
    _In_ PFLT_CONTEXT SectionContext,
    _In_ PFLT_CALLBACK_DATA Data
    );
#define PROTECT_DIRECTORY_PATH_EXPRESSION L"*\\test*"