#ifndef IMAGEWRITER_H
#define IMAGEWRITER_H
#include <string>
#include "CImg.h"

/** \brief interface for classes that provide image storage capabilities */
class ImageWriter
{
public:
    enum ImageType {
        FinalImage,
        IntermediateImage,
        StillStrip,
        StillImage
    };
    ImageWriter();
    ImageWriter(const ImageWriter&)=delete;
    ImageWriter& operator=(const ImageWriter&)=delete;
    ImageWriter(ImageWriter&&)=default;
    ImageWriter& operator=(ImageWriter&&)=default;
    virtual ~ImageWriter();
    /** \brief saves the given frame to an image, returns the full filename. the parameter \a filename is possibly modified (by e.g. changing the file extension) */
    virtual std::string saveImage(const std::string& filename, ImageType type, const cimg_library::CImg<uint8_t>& frame) const=0;
};

#endif // IMAGEWRITER_H
