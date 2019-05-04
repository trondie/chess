@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat"      
set compilerflags=/Fo.\build\ /Gm /Gi /Od /Zi /MDd /EHsc /DPLATFORM_WINDOWS /D_DEBUG /DLIB3DS_STATIC /DSUPPORT_3D_TEXTURE=0 /DTROND_ENGINE_EXTERNAL_GUI=1 /Itrond_engine_src /Ilib3ds_src /Ietchess3d_src /Istb_image_src /Itinyxml2_src /I"c:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Include"
set linkerflags=/OUT:etchess3d.exe /LIBPATH:"c:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\lib"
set sourcefiles=etchess3d_src/3ds_helpers.cpp etchess3d_src/chess_helpers.cpp etchess3d_src/ee_camera.cpp etchess3d_src/ee_scene_gl.cpp etchess3d_src/geometry_helpers.cpp etchess3d_src/main.cpp etchess3d_src/mymath.cpp etchess3d_src/mythreading_win32.cpp etchess3d_src/opengl_helpers.cpp etchess3d_src/os_helpers.cpp etchess3d_src/picking.cpp etchess3d_src/quatmath.cpp etchess3d_src/texture_helper.cpp etchess3d_src/vertex_declaration_helpers.cpp lib3ds_src/*.c stb_image_src/*.c tinyxml2_src/tinyxml2.cpp trond_engine_src/Board.cpp trond_engine_src/engine.cpp trond_engine_src/Node.cpp trond_engine_src/Piece.cpp trond_engine_src/Rules.cpp trond_engine_src/Vec2.cpp
set staticlibs=opengl32.lib freeglut.lib
        
cl.exe %compilerflags% %sourcefiles% %staticlibs% /link %linkerflags%

echo "Builed ended for etchess3d.exe"