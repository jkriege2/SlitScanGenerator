#include "videoreader_ffmpeg.h"
#include "ffmpeg_tools.h"

VideoReader_FFMPEG::VideoReader_FFMPEG():
    vid(nullptr)
{

}

VideoReader_FFMPEG::~VideoReader_FFMPEG()
{
    close();
}

bool VideoReader_FFMPEG::open(const std::string &filename)
{
    resetError();
    std::string err;
    vid=openFFMPEGVideo(filename, &err);
    if (vid) {
        setFilename(filename);
        return true;
    } else {
        setError(std::string("Unable to open video '")+getFilename()+std::string("'. Reason: ")+err);
        return false;
    }
}

bool VideoReader_FFMPEG::readNext(cimg_library::CImg<uint8_t> &frame)
{
    if (hadError()) return false;
    if (!vid) {
        setError(std::string("Video File not opened yet!"));
        return false;
    }
    return readFFMPEGFrame(frame, vid);
}

void VideoReader_FFMPEG::close()
{
    if (vid) {
        closeFFMPEGVideo(vid);
        vid=nullptr;
    }
}

int VideoReader_FFMPEG::getFrameCount() const
{
    if (!vid) return 0;
    return ::getFrameCount(vid);
}
