#ifndef IMAGEWRITER_PNG_H
#define IMAGEWRITER_PNG_H

#include "imagewriter.h"

/** \brief this concrete implementation of ImageWriter saves frames to a PNG file, using Qt */
class ImageWriter_PNG: public ImageWriter
{
public:
    /** \copydoc ImageWriter::saveImage() */
    virtual std::string saveImage(const std::string& filename, ImageType type, const cimg_library::CImg<uint8_t>& frame) const override;

};

#endif // IMAGEWRITER_PNG_H
