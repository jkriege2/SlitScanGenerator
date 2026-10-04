

.\7zip\7za.exe -o.\ffmpeg\win64 -y x .\ffmpeg\ffmpeg-n9.0-latest-win64-gpl-shared-9.0.zip
xcopy .\ffmpeg\win64\ffmpeg-n9.0-latest-win64-gpl-shared-9.0\* .\ffmpeg\win64 /S /E /R /Y
rmdir /S /Q .\ffmpeg\win64\ffmpeg-n9.0-latest-win64-gpl-shared-9.0


del .\CImg\CImg.zip /f /q
.\7zip\7za.exe a -y -r .\CImg\CImg.zip .\CImg\*