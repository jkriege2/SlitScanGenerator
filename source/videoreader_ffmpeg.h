#ifndef VIDEOREADER_FFMPEG_H
#define VIDEOREADER_FFMPEG_H

#include "videoreader.h"

struct FFMPEGVideo; // forward

/** \brief Concrete VideoReader implementation, that reads a video file, using FFMPEG */
class VideoReader_FFMPEG : public VideoReader
{
public:
    VideoReader_FFMPEG();
    ~VideoReader_FFMPEG();

    /** \copydoc VideoReader::open() */
    virtual bool open(const std::string &filename) override;
    /** \copydoc VideoReader::readNext() */
    virtual bool readNext(cimg_library::CImg<uint8_t>& frame) override;
    /** \copydoc VideoReader::close() */
    virtual void close() override;
    /** \copydoc VideoReader::getFrameCount() */
    virtual int getFrameCount() const override;

private:
    FFMPEGVideo* vid;
};

#endif // VIDEOREADER_FFMPEG_H
