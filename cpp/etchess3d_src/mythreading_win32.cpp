#include "mythreading.h"
#include <windows.h>

struct thread_internal
{
	HANDLE hThread;
	thread_function function;
	HANDLE hMutexThreadState;
	int exit_signal;
	void *data;
};

void *get_thread_data( thread_handle handle )
{
	struct thread_internal *thread = (struct thread_internal*)handle;
	void *data;
	WaitForSingleObject( thread->hMutexThreadState, INFINITE );
	data = thread->data;
	ReleaseMutex( thread->hMutexThreadState );
	return data;
}

void set_thread_data( thread_handle handle, void* data )
{
	struct thread_internal *thread = (struct thread_internal*)handle;
	WaitForSingleObject( thread->hMutexThreadState, INFINITE );
	thread->data = data;
	ReleaseMutex( thread->hMutexThreadState );
}

void signal_exit( thread_handle handle )
{
	struct thread_internal *thread = (struct thread_internal*)handle;
	WaitForSingleObject( thread->hMutexThreadState, INFINITE );
	thread->exit_signal = 1;
	ReleaseMutex( thread->hMutexThreadState );
}

bool exit_is_signalled( thread_handle handle )
{
	struct thread_internal *thread = (struct thread_internal*)handle;
	WaitForSingleObject( thread->hMutexThreadState, INFINITE );
	int exit = thread->exit_signal;
	ReleaseMutex( thread->hMutexThreadState );
	return !!exit;
}

thread_handle create_thread( thread_function function, void *data )
{
	struct thread_internal * thread = new thread_internal();
	if (thread)
	{
		thread->function = function;
		thread->hMutexThreadState = CreateMutexW( NULL, FALSE, L"MyThreadingMutexThreadState" );
		thread->exit_signal = 0;
		thread->data = data;
		thread->hThread = ::CreateThread( NULL, 0, (unsigned long (CALLBACK *)(void*))thread->function, static_cast<void *>(thread), CREATE_SUSPENDED, NULL );
		ResumeThread(thread->hThread);
	}
	return (thread_handle)thread;
}

int join_thread( thread_handle handle )
{
	struct thread_internal *thread = (struct thread_internal*)handle;
	signal_exit(handle);
	WaitForSingleObject( thread->hThread, INFINITE );
	return 0;
}

void destroy_thread( thread_handle handle )
{
	struct thread_internal *thread = (struct thread_internal*)handle;
	join_thread(handle);
	CloseHandle( thread->hMutexThreadState);
}

mutex_handle create_mutex( const char *name )
{
	return (mutex_handle)CreateMutexA( NULL, FALSE, name );
}

void grab_mutex( mutex_handle handle )
{
	HANDLE hMutex = (HANDLE)handle;
	WaitForSingleObject( hMutex, INFINITE );
}

void release_mutex( mutex_handle handle )
{
	HANDLE hMutex = (HANDLE)handle;
	ReleaseMutex( hMutex );
}

void destroy_mutex( mutex_handle handle )
{
	HANDLE hMutex = (HANDLE)handle;
	CloseHandle( hMutex );
}