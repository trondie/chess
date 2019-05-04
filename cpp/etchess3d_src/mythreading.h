#ifndef __MY_THREADING_H__
#define __MY_THREADING_H__

#define THREADING_INVALID_HANDLE (thread_handle)-1

typedef void * thread_handle;
typedef void * mutex_handle;
typedef void * (*thread_function)(thread_handle);

/* Thread features */
thread_handle create_thread( thread_function function, void *data );
void *get_thread_data( thread_handle handle );
void set_thread_data( thread_handle handle, void* data );
int join_thread( thread_handle handle );
void destroy_thread( thread_handle handle );
bool exit_is_signalled( thread_handle handle );
void signal_exit( thread_handle handle );
void set_jni( thread_handle handle, void *jvm, int activity);

/* Mutex features */
mutex_handle create_mutex( const char *name );
void destroy_mutex( mutex_handle handle );
void release_mutex( mutex_handle handle );
void grab_mutex( mutex_handle handle );

#endif /* __MY_THREADING_H__ */