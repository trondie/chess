/*
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <jni.h>
#include <errno.h>

#include <android/sensor.h>
#include <android/input.h>

#include <android_native_app_glue.h>
#include <TestGLSL.h>
#include <android/asset_manager.h>
#include <cstring>


void assets_init(struct android_app* state_param)
{
    JNIEnv* env = state_param->activity->env;
    JavaVM* vm = state_param->activity->vm;
    vm->AttachCurrentThread(&env, NULL);
    jclass activityClass = env->GetObjectClass(state_param->activity->clazz);
    // Get path to cache dir (/data/data/org.wikibooks.OpenGL/cache)
    jmethodID getCacheDir = env->GetMethodID(activityClass, "getCacheDir", "()Ljava/io/File;");
    jobject file = env->CallObjectMethod(state_param->activity->clazz, getCacheDir);
    jclass fileClass = env->FindClass("java/io/File");
    jmethodID getAbsolutePath = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
    jstring jpath = (jstring)env->CallObjectMethod(file, getAbsolutePath);
    const char* app_dir = env->GetStringUTFChars(jpath, NULL);
 
    // chdir in the application cache directory
    Base::log("app_dir: %s", app_dir);
    chdir(app_dir);
    env->ReleaseStringUTFChars(jpath, app_dir);

    AAssetManager* mgr = state_param->activity->assetManager;

    AAssetDir* assetDir = AAssetManager_openDir(mgr, "");
    const char* filename = (const char*)NULL;
    while ((filename = AAssetDir_getNextFileName(assetDir)) != NULL) {
	AAsset* asset = AAssetManager_open(mgr, filename, AASSET_MODE_STREAMING);
	char buf[BUFSIZ];
	int nb_read = 0;
        Base::log("writing filename: %s", filename);

	FILE* out = fopen(filename, "w");
	while ((nb_read = AAsset_read(asset, buf, BUFSIZ)) > 0)
	    fwrite(buf, nb_read, 1, out);
	fclose(out);
	AAsset_close(asset);
    }
    AAssetDir_close(assetDir);

}



struct engine 
{
	struct android_app* app;

	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	ASensorEventQueue* sensorEventQueue;

	TestGLSL* testglsl;
};


static int other_x = 0;
static int other_y = 0;
static int onclick_x = 0;
static int onclick_y = 0;
static int onclick_state = 10000;
static int lmdown = 0;
static int other_pointer_down = false;
static int first_pointer_wait = false;
static int other_pointer_id = -1;

static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) 
{
	struct engine* engine = (struct engine*)app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) 
	{		
		float f = AMotionEvent_getPressure( event, 0 );
		
		if( AMotionEvent_getPointerCount(event) > 1 )
		{
			int action = AMotionEvent_getAction( event );
			int pointer_idx = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK ) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

			if ( !other_pointer_down )
			{
				other_pointer_down = true;
				other_pointer_id = pointer_idx; 
			}

			other_x = AMotionEvent_getX(event, other_pointer_id);
			other_y = AMotionEvent_getY(event, other_pointer_id);
			onclick_x = AMotionEvent_getX(event, pointer_idx);
			onclick_y =  AMotionEvent_getY(event, pointer_idx);
			//Base::log("onSecondMove id:%d (%d,%d,%d,%d)", pointer_idx, onclick_x, onclick_y, other_x, other_y );
			engine->testglsl->onSecondPointerMove(onclick_x, onclick_y, other_x, other_y);
		}
		else 
		{
			int tooltype = AMotionEvent_getToolType( event, 0 );
			int state = AMotionEvent_getButtonState( event );
			int primary_button = state & AMOTION_EVENT_BUTTON_PRIMARY;
			int clicked = AMotionEvent_getAction( event );

			if (other_pointer_down )
			{
				onclick_x = AMotionEvent_getX(event, 0);
				onclick_y =  AMotionEvent_getY(event, 0);

				engine->testglsl->onSecondPointerUp( other_x, other_y );
				other_pointer_down = false;
				other_pointer_id = -1;
				first_pointer_wait = true;
			}
			else if (first_pointer_wait)
			{
				if( clicked == AMOTION_EVENT_ACTION_UP )
				{
					first_pointer_wait = false;
					engine->testglsl->onPointerUp( 0, onclick_x, onclick_y, primary_button ? true : false );
				}
			} 
			else
			{
				onclick_x = AMotionEvent_getX(event, 0);
				onclick_y =  AMotionEvent_getY(event, 0);

				if ( clicked == AMOTION_EVENT_ACTION_DOWN )
				{
					//Base::log("testglsl->onMove(%d,%d) lmdown:%d f:%f clicked: %d tooltype:%d primary: %d", onclick_x, onclick_y, lmdown, f, clicked, tooltype, primary_button );
					engine->testglsl->onPointerDown( 0, onclick_x, onclick_y, primary_button ? true : false );
				}
				else if( clicked == AMOTION_EVENT_ACTION_UP )
				{
					//Base::log("testglsl->onMove(%d,%d) lmdown:%d f:%f clicked: %d tooltype:%d primary: %d", onclick_x, onclick_y, lmdown, f, clicked, tooltype, primary_button );
					engine->testglsl->onPointerUp( 0, onclick_x, onclick_y, primary_button ? true : false );
				}
				//Base::log("onMove");
				engine->testglsl->onMove( onclick_x, onclick_y );

			}

		}
		return 1;
	}

#if 0
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) 
	{
		int new_state = AKeyEvent_getAction( event );
		Base::log("testglsl->onClick %d", new_state);
		if ( onclick_state == new_state ) return 1;
		onclick_state = new_state;
		if ( onclick_state == 1 ) engine->testglsl->onPointerUp( 0, onclick_x, onclick_y );
		else if ( onclick_state == 0 ) engine->testglsl->onPointerDown( 0, onclick_x, onclick_y );
		return 1;
	}
#endif
	return 0;
}

static void engine_handle_cmd(struct android_app* app, int32_t cmd) 
{
	struct engine* engine = (struct engine*)app->userData;
	static int window_width = 0;
	static int window_height = 0;
	static int orientation = -1;

	switch (cmd) 
	{
		case APP_CMD_SAVE_STATE:
			// The system has asked us to save our current state.  Do so.
			if (engine->app->window != NULL)
			{
				Base::log("OnSaveState");
				engine->testglsl->onSaveState();
			}
			break;
		case APP_CMD_INIT_WINDOW:
			if (engine->app->window != NULL) 
			{
				Base::log("Oncreate=n");
				assets_init(app);
				engine->testglsl->onCreate( 0, NULL, (void*)app->window );
				window_width = ANativeWindow_getWidth(app->window);
				window_height = ANativeWindow_getHeight(app->window);
				engine->testglsl->onResize(window_width, window_height);
				orientation = AConfiguration_getOrientation(app->config);
			}
			break;
		case APP_CMD_WINDOW_RESIZED:
			if (engine->app->window != NULL)
			{
				int width = ANativeWindow_getWidth(app->window);
				int height = ANativeWindow_getHeight(app->window);
				Base::log("OnResize(%d,%d)", width, height);
				if (!(width == window_width && height == window_height)) {				
					engine->testglsl->onResize(width, height);
					window_width = width;
					window_height = height;
				}
			}
			break;
		case APP_CMD_CONFIG_CHANGED:
			// Work around for buggy Android Native orientation changes.
			// Need to recreate window for assuring right dimensions.
			// Sometimes this will be followed by another destroy + init :(
			// Proper fix is from java side to listen to onsurfacechanged
			// and call into JNI to resize. THis changes everything tho :(
			
			Base::log("OnConfigChanged");
			if (AConfiguration_getOrientation(app->config) != orientation) {
				Base::log("OnConfigChanged - recreate");
				engine->testglsl->onDestroy();
				assets_init(app);
				engine->testglsl->onCreate( 0, NULL, (void*)app->window );
				window_width = ANativeWindow_getWidth(app->window);
				window_height = ANativeWindow_getHeight(app->window);
				engine->testglsl->onResize(window_width, window_height);
				orientation = AConfiguration_getOrientation(app->config);
			}
			break;
		case APP_CMD_TERM_WINDOW:
			Base::log("OnDestroy");
			engine->testglsl->onDestroy();
			break;
		case APP_CMD_GAINED_FOCUS:
			Base::log("OnResume");
			if (engine->accelerometerSensor != NULL) 
			{
				// We'd like to get 60 events per second (in us).
				ASensorEventQueue_enableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
				ASensorEventQueue_setEventRate(engine->sensorEventQueue, engine->accelerometerSensor, (1000L/60)*1000);
			}
			if (engine->app->window != NULL)
			{
				// Check if resize has happened while being paused
				int width = ANativeWindow_getWidth(app->window);
				int height = ANativeWindow_getHeight(app->window);
				Base::log("OnResume - resize(%d,%d)", width, height);
				if (!(width == window_width && height == window_height)) {				
					engine->testglsl->onResize(width, height);
					window_width = width;
					window_height = height;
				}
			}
			engine->testglsl->onResume();
			break;
		case APP_CMD_LOST_FOCUS:
			Base::log("OnPause");
			if (engine->accelerometerSensor != NULL) 
			{
				ASensorEventQueue_disableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
			}
			engine->testglsl->onPause();
			break;
	}
}

void android_main(struct android_app* state) {
	struct engine engine;

	// Make sure glue isn't stripped.
	app_dummy();

	memset(&engine, 0, sizeof(engine));
	state->userData = &engine;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
	engine.testglsl = new TestGLSL();
	engine.app = state;

	// Prepare to monitor accelerometer
	engine.sensorManager = ASensorManager_getInstance();
	engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager, state->looper, LOOPER_ID_USER, NULL, NULL);

	Base::log("NOT IMPLEMENTED: %s:%d\n\t load the state", __FILE__, __LINE__);
	/*
	EXAMPLE OF STATE LOADING
	if (state->savedState != NULL) 
	{
		// We are starting with a previous saved state; restore from it.
		engine.state = *(struct saved_state*)state->savedState;
	}
	*/

	while (1) 
	{
		int ident;
		int events;
		struct android_poll_source* source;

		while ((ident=ALooper_pollAll(engine.testglsl->isAnimating() ? 0 : -1, NULL, &events, (void**)&source)) >= 0) 
		{
			if (source != NULL) source->process(state, source); 

			// If a sensor has data, process it now.
			if (ident == LOOPER_ID_USER) 
			{
				if (engine.accelerometerSensor != NULL) 
				{
					ASensorEvent event;
					while (ASensorEventQueue_getEvents( engine.sensorEventQueue, &event, 1) > 0) 
					{
						/* Base::log("accelerometer: x=%f y=%f z=%f",
								event.acceleration.x, event.acceleration.y,
								event.acceleration.z); */
					}
				}
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0) 
			{
				engine.testglsl->onPause();
				return;
			}
		}
		engine.testglsl->onNewFrame();
	}
}

