#ifndef TEST_GLSL_H
#define TEST_GLSL_H
class IGame
{
public:
    /* Input */
    virtual void onKeyDown(int key) = 0;
    virtual void onKeyUp(int key) = 0;
    virtual void onPointerDown(int id, int x, int y, bool right_click) = 0;
    virtual void onPointerUp(int id, int x, int y, bool right_click) = 0;
    virtual void onPointerMove(int id, int x, int y) = 0;

    /* Events */
    virtual void onCreate( int argc, char **argv, void *native_window ) = 0;
    virtual void onDestroy() = 0;
    virtual void onPause() = 0;
    virtual void onResume() = 0;
    virtual void onSaveState() = 0;

    /* Graphic Events */
    virtual void onResize(int w, int h) = 0;
    virtual void onNewFrame() = 0;
    virtual void onMove( int x, int y ) = 0;
    virtual void onSecondPointerMove( int first_x, int first_y, int other_x, int other_y ) = 0;
    virtual void onSecondPointerUp( int x, int y ) = 0;
    virtual void onSecondPointerDown( int first_x, int first_y, int other_x, int other_y ) = 0;

    virtual ~IGame() {}
};

#if defined( PLATFORM_ARM_ANDROID )
#include <android/input.h>
#include <android/log.h>
#else
#include <stdio.h>
#include <stdarg.h>
#endif

class Base
{
public:

    /* Logging */
    static void log(const char* format, ...) {
        va_list args;
        va_start (args, format);
#if defined( PLATFORM_ARM_ANDROID )
	if ( format )
	{
	        __android_log_vprint(ANDROID_LOG_DEBUG, "ETChess3D", format, args);
	}
#else
	vprintf(format,args);
#endif
        va_end (args);
    };

};

class TestGLSL : IGame
{
	public:
		TestGLSL() { animating = false; };

		/* Input */
		void onKeyDown(int key);
		void onKeyUp(int key);
		void onPointerDown(int id, int x, int y, bool right_click);
		void onPointerUp(int id, int x, int y, bool right_click);
		void onPointerMove(int id, int x, int y);

		/* Events */
		void onCreate( int argc, char **argv, void *native_window );
		void onDestroy();
		void onPause();
		void onResume();
		void onSaveState();

		/* Graphic Events */
		void onResize(int w, int h);
		void onNewFrame();
		void onMove(int x, int y);
		void onSecondPointerMove( int first_x, int first_y, int other_x, int other_y );
		void onSecondPointerUp( int x, int y );
		void onSecondPointerDown( int first_x, int first_y, int other_x, int other_y );

	public:
		bool isAnimating() { return this->animating; }

	private:

		bool animating;
		float angle;
};

#endif

