#include "videoreader_cimg.h"

VideoReader_CImg::VideoReader_CImg(cimg_library::CImg<uint8_t> &video_stack):
    m_video_stack(video_stack), m_index(0)
{

}

VideoReader_CImg::~VideoReader_CImg()
{

}

bool VideoReader_CImg::open(const std::string &filename)
{
    setFilename(filename);
    resetError();
    return isOK();
}

bool VideoReader_CImg::readNext(cimg_library::CImg<uint8_t> &frame)
{
    if (hadError()) return false;
    if (!isOK()) {
        setError("image stack does not contain any data");
        return false;
    }
    if (m_index<m_video_stack.depth()) {

        // check for necessity to reallocate of frame
        if (frame.width()!=m_video_stack.width()
            || frame.height()!=m_video_stack.height()
            || frame.spectrum()!=m_video_stack.spectrum()
            || frame.depth()!=1)
        {
            // resize frame if necessary
            frame.assign(m_video_stack.width(), m_video_stack.height(), 1, m_video_stack.spectrum());
        }

        // copy image
        cimg_forXYC( frame, x, y, c )
        {
            frame( x, y, 0, c )=m_video_stack( x, y, m_index, c );
        }

        // advance frame pointer
        m_index++;
        return true;
    }
    return false;
}

void VideoReader_CImg::close()
{

}

int VideoReader_CImg::getFrameCount() const
{
    return m_video_stack.depth();
}

bool VideoReader_CImg::isOK() const
{
    return m_video_stack.width()*m_video_stack.height()*m_video_stack.spectrum()*m_video_stack.depth()>0;
}
