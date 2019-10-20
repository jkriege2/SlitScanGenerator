.\7zip\7za.exe -o.\ffmpeg\win32 -y x .\ffmpeg\ffmpeg-4.2.1-win32-dev.zip 
xcopy .\ffmpeg\win32\ffmpeg-4.2.1-win32-dev\* .\ffmpeg\win32 /S /E /R /Y
rmdir /S /Q .\ffmpeg\win32\ffmpeg-4.2.1-win32-dev

.\7zip\7za.exe -o.\ffmpeg\win32 -y x .\ffmpeg\ffmpeg-4.2.1-win32-shared.zip 
xcopy .\ffmpeg\win32\ffmpeg-4.2.1-win32-shared\* .\ffmpeg\win32 /S /E /R /Y
rmdir /S /Q .\ffmpeg\win32\ffmpeg-4.2.1-win32-shared



.\7zip\7za.exe -o.\ffmpeg\win64 -y x .\ffmpeg\ffmpeg-4.2.1-win64-dev.zip 
xcopy .\ffmpeg\win64\ffmpeg-4.2.1-win64-dev\* .\ffmpeg\win64 /S /E /R /Y
rmdir /S /Q .\ffmpeg\win64\ffmpeg-4.2.1-win64-dev

.\7zip\7za.exe -o.\ffmpeg\win64 -y x .\ffmpeg\ffmpeg-4.2.1-win64-shared.zip 
xcopy .\ffmpeg\win64\ffmpeg-4.2.1-win64-shared\* .\ffmpeg\win64 /S /E /R /Y
rmdir /S /Q .\ffmpeg\win64\ffmpeg-4.2.1-win64-shared


.\7zip\7za.exe a -y -r .\CImg\CImg.zip .\CImg\*