

.\7zip\7za.exe -o.\ffmpeg\win64 -y x .\ffmpeg\ffmpeg-7.1-full_build-shared.7z
xcopy .\ffmpeg\win64\ffmpeg-7.1-full_build-shared\* .\ffmpeg\win64 /S /E /R /Y
rmdir /S /Q .\ffmpeg\win64\ffmpeg-7.1-full_build-shared


del .\CImg\CImg.zip /f /q
.\7zip\7za.exe a -y -r .\CImg\CImg.zip .\CImg\*