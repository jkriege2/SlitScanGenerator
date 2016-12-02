# SlitScanGenerator
C++ program that generates slit scan photography 

Usage:

1. Open Video file (It will be scaled down and only every N'th frame will be loaded to save memory during the initial stage)

2. click on the top-left image and select positions for cuts. If you like a cut, add it with the "add ..." buttons. All selected cuts will be shown in the list on the bottom right

3. Finally click on "Process All" to generate the output images (in the same folder as the video). Note that ürpcessing is done with the full resolution video, not the size-reduced video used for display.
