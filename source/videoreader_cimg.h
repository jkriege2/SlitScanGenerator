#ifndef VIDEOREADER_CIMG_H
#define VIDEOREADER_CIMG_H

#include "videoreader.h"
#include "CImg.h"

/** \brief Concrete VideoReader implementation, that reads a video from a xyt-imagestack
 *         (as a CImg object!), provided as a reference to the constructor
 *
 * \note You have to ensure that the imagestack lives at least as long as a reader that
 *       references it!
 */
class VideoReader_CImg : public VideoReader
{
public:
    VideoReader_CImg(cimg_library::CImg<uint8_t>& video_stack);
    virtual ~VideoReader_CImg();

    /** \copydoc VideoReader::open() */
    virtual bool open(const std::string &filename) override;
    /** \copydoc VideoReader::readNext() */
    virtual bool readNext(cimg_library::CImg<uint8_t>& frame) override;
    /** \copydoc VideoReader::close() */
    virtual void close() override;
    /** \copydoc VideoReader::getFrameCount() */
    virtual int getFrameCount() const override;
private:
    bool isOK() const;
    cimg_library::CImg<uint8_t>& m_video_stack;
    int m_index;
};

#endif // VIDEOREADER_CIMG_H
