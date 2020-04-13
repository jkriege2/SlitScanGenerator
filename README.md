# SlitScanGenerator
Program that generates [slit scan photography](https://en.wikipedia.org/wiki/Slit-scan_photography) images from videos. It generates a synthetic image by only taking one line of image pixels from each frame of a video. These lines are fused (and subsequently filtered) to generate images like these:

​          [![rose](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/DSC_5973_stack010_thumb.jpg)](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/DSC_5973_stack010.jpg)         [![rose](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/DSC_6003_stack001_thumb.jpg)](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/DSC_6003_stack001.jpg) 

For the images above, flowers (roses on the left, a lily on the right) were put on a rotating plate and a video was recorded using a DSLR camera. Then the program SlitScanGenerator was used to create these images.

You can find more example image here: https://jkrieger.de/photography/slitscan/index.html

A detailed description of the different functions of this package can be found below:

[TOC]



## Download:

Installers for Windows and Source-Code Archives can be found here:

  https://github.com/jkriege2/SlitScanGenerator/releases/



## Basic Usage:

1. Open Video file ![img](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/resources/folder_video.png) (It will be scaled down and only every N'th frame will be loaded to save memory during the initial stage)
2. Click on the top-left image (1) and select positions for cuts.  The cuts generated from the current settings are shown at the top-right (2) and bottom-left (3).
3. If you like a cut, add it with the ![add](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/resources/list_add.png) "add ..." buttons. All selected cuts will be shown in the list (4) on the bottom right.
4. You can also set other filtering and processing options in the additional tabs at the bottom-right (5).
5. Finally click on ![img](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/resources/wizard2.png) "Process All" to generate the output images (in the same folder as the video). Note that processing is done with the full resolution video, not the size-reduced video used for display. When the video is processed, all you settings are stored as an INI-file and you can reload (and alter) them by simply reloading the video or the INI-file with the "File" menu.
6. After clicking on "Process All", the processing is performed in the background (see list at the bottom) and you can load and edit the next video.

![screenshot](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/MainWindow.png)



## Detailed Description/Manual

### Steps during Processing

SlitScanGenerator processes videos (X-Y-T) in the following steps:

1. for each frame:
   1. OPTIONAL: normalize the colors in the frame at the specified position, i.e. scale all colors so the normalizing position has the same color in each frame of the whole video. This can be used to remove intensity variations during the video acquisition.
   2. calculate the line to copy, based on the chosen settings for the cut position, mode, angle (pitch/roll) etc.
   3. copy the line from the frame and put it into the output image
   4. OPTIONAL: If the current frame shall be used for a still(strip), store it for later
   5. load next frame
2. apply optional filters to the output image:
   1. a notch filter can be used to eliminate fast flickering of the images
   2. apply a white-point correction 
3. save the output image:
     [![rose](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/DSC_6003_stack001_thumb.jpg)](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/DSC_6003_stack001.jpg) 
4. generate the still image strip and save it:
    [![DSC_6003_stack009_stillstrip](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/DSC_6003_stack009_stillstrip_tumb.jpg)](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/DSC_6003_stack009_stillstrip.jpg)





### Cut Modes

SlitScanGenerator supports different cut modes. These are explained in the next sections. 

You can configure the parameters and which cuts to generate in the main window in the tab "XZ/YZ-Cuts" (at the bottom-right). There you can set the angle for roll/pith and the roll/pitch-mode itself. Also the tab shows a table with all the tabs already created for the current video (as a table). 

Whenever this tab is activated, you can click on different positions in the preview frame at the top-left. And a new center for the cut-cross is selected. The current position and cross is shown in red in the preview image. In the two preview images, you see currently configured cuts and can choose to add them to the processing list for the current video by clicking on "add XZ" or "add ZY".

#### XZ-Cuts

![XZ-Cuts](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/cuts_XZ.png)

The output is a series of horizontal lines from the video frames.

#### YZ-Cuts

![YZ-Cuts](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/cuts_YZ.png)

The output is a series of vertical lines (columns) from the video frames.

#### Rotated XZ/YZ-Cuts

![XZ-Cuts, rotated](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/cuts_XZ_rot.png)

The output is a series of rotated horizontal/vertical lines from the video frames. the rotation is given as an angle &alpha; from the horizontal.

#### Pitched XZ/YZ-Cuts

![XZ-Cuts, pitched](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/cuts_XZ_pitch.png)

The output is a series of horizontal or vertical lines from the video frames. Other than in the simple case (above), the line is moved through the frame along the time/z-axis of the video.





### Normalization

Sometimes the illumination varies during the video. You can accomodate for this, if you have a small patch that shows the same non-moving object in every frame. Then you can activate the "normalization" option in the tab "normalization" at the bottom-right. Then you can select a pixel to use for the normalization (click on any pixel in the preview image, top-left). the current pixel used for normalization is shown as a small blue square in the preview image.

![Normalization GUI](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/settingsStripesOn.png)

**Note that the wavelength filter is applied to EVERY cut in the list!**





### Wavelength Filter Options

![Notch Filter example](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/notchfilter.png)

The Notch filter can be used, when fast intensity variations exist in the image. You simply give the wavelength of the pattern as a parameter plus a small allowed wavelength variation. The exactly waves of this wavelength are removed from the output image. The filter settings can be found in the tab "filter" at the bottom-right:

![GUI](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/settingsFilterOn.png)

**Note that the wavelength filter is applied to EVERY cut in the list!**





### Color Options

SlitScanGenerator allows to perform a white-balancing (e.g. when the white balancing of the camera was off during the acquisition). This option is available from the tab "Filter" at the bottom-right:

![color options](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/settingsColor.png)

When this tab is active, you can click on any location in the image that should contain a neutral (white or gray, the brighter the better) color. The program then samples this color and tries to make it neutral while preserving its brightness. Here is an example (the white-balance was not properly set while recoding the video):

![white balance example](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/whitepoint_correction.png)

**Note that the wavelength filter is applied to EVERY cut in the list!**





### Still Stripe

In addition to the actual output image, SlitScanGenerator also generates a strip with a few stills that demonstrates how the output image is generated. 

[![still strip](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/DSC_6003_stack009_stillstrip_tumb.jpg)](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/DSC_6003_stack009_stillstrip.jpg)

You can set the options for this strip in the tab "Stills" at the bottom-right:

![Stills GUI](https://raw.githubusercontent.com/jkriege2/SlitScanGenerator/master/screenshots/settingsStills.png)