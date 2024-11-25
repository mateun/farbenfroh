call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

cl src\unity.cpp /Fd.\out\  /Fo.\out\ /DEBUG /Zi /I src /EHsc /link user32.lib gdi32.lib opengl32.lib dinput8.lib dxguid.lib /out:.\out\game.exe /pdb:.\out\
