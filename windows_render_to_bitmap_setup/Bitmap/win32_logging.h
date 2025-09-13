#ifndef WIN32_LOGGING_H
#define WIN32_LOGGING_H

#include <Windows.h>
#include <stdio.h>

extern CRITICAL_SECTION _win32_log_critical_section;
extern wchar_t _win32_log_unicode_buffer[1024];
extern char _win32_log_ascii_buffer[1024];


void win32LogInitialize();
void win32LogDeInitialize();
void _win32LogFormatLastError(DWORD error);


#define WIN32_LOG(message, ...)                                                                           \
{                                                                                                         \
    EnterCriticalSection(&_win32_log_critical_section);                                                   \
	OutputDebugStringA(__FILE__);                                                                         \
    swprintf(                                                                                             \
        _win32_log_unicode_buffer,                                                                        \
		sizeof(_win32_log_unicode_buffer) / sizeof(*_win32_log_unicode_buffer),                           \
		L"::%d ", __LINE__                                                                                \
	);                                                                                                    \
    OutputDebugStringW(_win32_log_unicode_buffer);                                                        \
    sprintf_s(                                                                                            \
        _win32_log_ascii_buffer,                                                                          \
		sizeof(_win32_log_ascii_buffer) / sizeof(*_win32_log_ascii_buffer),                               \
		message, ##__VA_ARGS__                                                                            \
	);                                                                                                    \
    MultiByteToWideChar(                                                                                  \
        CP_UTF8, 0,                                                                                       \
		_win32_log_ascii_buffer, -1,                                                                      \
        _win32_log_unicode_buffer, sizeof(_win32_log_unicode_buffer) / sizeof(*_win32_log_unicode_buffer) \
    );                                                                                                    \
    OutputDebugStringW(_win32_log_unicode_buffer);                                                        \
    OutputDebugStringW(L"\n");                                                                            \
    LeaveCriticalSection(&_win32_log_critical_section);                                                   \
}

#define WIN32_LOG_LAST_ERROR(error, message, ...)                                                         \
{                                                                                                         \
    DWORD _error = error;                                                                                 \
	EnterCriticalSection(&_win32_log_critical_section);                                                   \
	OutputDebugStringA(__FILE__);                                                                         \
	swprintf(                                                                                             \
		_win32_log_unicode_buffer,                                                                        \
		sizeof(_win32_log_unicode_buffer) / sizeof(*_win32_log_unicode_buffer),                           \
		L"::%d ", __LINE__                                                                                \
	);                                                                                                    \
	OutputDebugStringW(_win32_log_unicode_buffer);                                                        \
	sprintf_s(                                                                                            \
		_win32_log_ascii_buffer,                                                                          \
		sizeof(_win32_log_ascii_buffer) / sizeof(*_win32_log_ascii_buffer),                               \
		message, ##__VA_ARGS__                                                                            \
	);                                                                                                    \
	MultiByteToWideChar(                                                                                  \
		CP_UTF8, 0,                                                                                       \
		_win32_log_ascii_buffer, -1,                                                                      \
		_win32_log_unicode_buffer, sizeof(_win32_log_unicode_buffer) / sizeof(*_win32_log_unicode_buffer) \
	);                                                                                                    \
	OutputDebugStringW(_win32_log_unicode_buffer);                                                        \
	OutputDebugStringW(L"\n");                                                                            \
    swprintf(                                                                                             \
        _win32_log_unicode_buffer,                                                                        \
        sizeof(_win32_log_unicode_buffer) / sizeof(*_win32_log_unicode_buffer),                           \
        L"LastError (%d): ", _error                                                                       \
	);                                                                                                    \
    OutputDebugStringW(_win32_log_unicode_buffer);                                                        \
    _win32LogFormatLastError(_error);                                                                     \
    OutputDebugStringW(L"\n");                                                                            \
	LeaveCriticalSection(&_win32_log_critical_section);                                                   \
}


#ifndef PAWEL_DO_NOT_IMPLEMENT

CRITICAL_SECTION _win32_log_critical_section;
wchar_t _win32_log_unicode_buffer[1024];
char _win32_log_ascii_buffer[1024];


void win32LogInitialize()
{
	InitializeCriticalSection(&_win32_log_critical_section);
}


void win32LogDeInitialize()
{
	DeleteCriticalSection(&_win32_log_critical_section);
}


void _win32LogFormatLastError(DWORD error)
{
	DWORD written_characters;
	written_characters = FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error,
		0,
		_win32_log_unicode_buffer,
		sizeof(_win32_log_unicode_buffer) / sizeof(*_win32_log_unicode_buffer) - 1,
		NULL
	);
	_win32_log_unicode_buffer[written_characters] = 0;
	if (written_characters == 0) {
		swprintf(
			_win32_log_unicode_buffer,
			sizeof(_win32_log_unicode_buffer) / sizeof(*_win32_log_unicode_buffer),
			L"Unable to generate message due to error: %d", error
		);
	}
	OutputDebugStringW(_win32_log_unicode_buffer);
}


#endif
#endif