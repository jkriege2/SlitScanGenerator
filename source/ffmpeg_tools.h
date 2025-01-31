#ifndef FFMPEG_TOOLS_H
#define FFMPEG_TOOLS_H

#include "cimg_tools.h"
#include <string>
#include <functional>

/** \brief list all supported hwacell options */
QStringList listFFMPEGHWAccelOptions();

/** \brief use ffmpeg to load a video into a CImg-stack, while only loading every \a everyNthFrame-th frame
 *         and reducing the frame-size by a factor \a xyscale
 *
 * \return returns \c true on success and \c false else, error messages are returned in \a error.
 */
bool readFFMPEGAsImageStack(cimg_library::CImg<uint8_t> &video, const std::string& filename, int everyNthFrame=1, double xyscale=1, std::string* error=nullptr, std::function<bool(int,int)> frameCallback=std::function<bool(int,int)>(), int maxFrame=-1, QString hw_accel_opt="auto", int numFFMPEGThreads=-1);

struct FFMPEGVideo;

FFMPEGVideo* openFFMPEGVideo(const std::string& filename, std::string* error=nullptr);
bool readFFMPEGFrame(cimg_library::CImg<uint8_t>& frame, FFMPEGVideo* video);
void closeFFMPEGVideo(FFMPEGVideo* video);
int getFrameCount(const FFMPEGVideo* video);

void initFFMPEG();
#endif // CIMG_TOOLS_H
