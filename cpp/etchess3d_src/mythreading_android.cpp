#include "mythreading.h"
#include <jni.h>
#include <pthread.h>
#include <cstdlib>
#include "../native-lib.h"

struct thread_internal
{
	thread_handle hThread;
	thread_function function;
	mutex_handle hMutexThreadState;
	int exit_signal;
	void *data;
	JavaVM* jvm;
	jobject activity;
};

void *get_thread_data( thread_handle handle )
{
	struct thread_internal *thread = (struct thread_internal*)handle;
	void *data;
	grab_mutex( thread->hMutexThreadState );
	data = thread->data;
	release_mutex( thread->hMutexThreadState );
	return data;
}

void set_thread_data( thread_handle handle, void* data )
{
	struct thread_internal *thread = (struct thread_internal*)handle;
	grab_mutex( thread->hMutexThreadState );
	thread->data = data;
	release_mutex( thread->hMutexThreadState );
}

void signal_exit( thread_handle handle )
{
	struct thread_internal *thread = (struct thread_internal*)handle;
	grab_mutex( thread->hMutexThreadState );
	thread->exit_signal = 1;
	release_mutex( thread->hMutexThreadState );
}

bool exit_is_signalled( thread_handle handle )
{
	struct thread_internal *thread = (struct thread_internal*)handle;
	grab_mutex( thread->hMutexThreadState );
	int exit = thread->exit_signal;
	release_mutex( thread->hMutexThreadState );
	return !!exit;
}

thread_handle create_thread( thread_function function, void *data )
{
	struct thread_internal * thread = new thread_internal();
	if (thread)
	{
		pthread_t *new_thread;
		thread->function = function;
		thread->hMutexThreadState = create_mutex( "MyThreadingMutexThreadState" );
		thread->exit_signal = 0;
		thread->data = data;
		new_thread = (pthread_t*)malloc(sizeof(*new_thread));
		pthread_create( new_thread, NULL, thread->function, thread );
		thread->hThread = (thread_handle)new_thread;
		thread->jvm = (JavaVM*)get_jvm();
		thread->activity = (jobject)get_activity();
	}
	return (thread_handle)thread;
}

int join_thread( thread_handle handle )
{
	struct thread_internal *thread = (struct thread_internal*)handle;
	void *thread_result = 0;
	signal_exit(handle);
	pthread_join( *((pthread_t*)thread->hThread), &thread_result );
	return 0;
}

void destroy_thread( thread_handle handle )
{
	struct thread_internal *thread = (struct thread_internal*)handle;
	join_thread(handle);
	free( (pthread_t*)thread->hThread );
	destroy_mutex( thread->hMutexThreadState );
	thread->jvm->DetachCurrentThread();
}

mutex_handle create_mutex( const char *name )
{
	pthread_mutex_t *mutex;
	mutex = (pthread_mutex_t *)malloc(sizeof(*mutex));
	pthread_mutex_init( mutex, NULL );
	return (mutex_handle)mutex;
}

void grab_mutex( mutex_handle handle )
{
	pthread_mutex_t *hMutex = (pthread_mutex_t*)handle;
	pthread_mutex_lock( hMutex );
}

void release_mutex( mutex_handle handle )
{
	pthread_mutex_t *hMutex = (pthread_mutex_t*)handle;
	pthread_mutex_unlock( hMutex );
}

void destroy_mutex( mutex_handle handle )
{
	pthread_mutex_t *hMutex = (pthread_mutex_t*)handle;
	pthread_mutex_destroy( hMutex );
	free( hMutex );
}
