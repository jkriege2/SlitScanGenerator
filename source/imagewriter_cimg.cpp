#include "imagewriter_cimg.h"

ImageWriter_CImg::ImageWriter_CImg(cimg_library::CImg<uint8_t> &outframe):
    m_outframe(outframe), m_type_match(ImageType::FinalImage), m_do_filter(false)
{

}

ImageWriter_CImg::ImageWriter_CImg(cimg_library::CImg<uint8_t> &outframe, ImageType type_match):
    m_outframe(outframe), m_type_match(type_match), m_do_filter(true)
{

}

std::string ImageWriter_CImg::saveImage(const std::string &filename, ImageType type, const cimg_library::CImg<uint8_t> &frame) const
{
    bool doCopy=false;
    if (!m_do_filter) {
        doCopy=true;
    } else {
        doCopy=(type==m_type_match);
    }
    if (doCopy) {
        m_outframe=frame;
    }
    return filename;
}
