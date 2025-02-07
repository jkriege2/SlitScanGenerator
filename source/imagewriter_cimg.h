#ifndef IMAGEWRITER_CIMG_H
#define IMAGEWRITER_CIMG_H

#include <string>
#include "imagewriter.h"

/** \brief this concrete implementation of ImageWriter simply copies the image
 *         into an output CImg object provided to the constructor
 *
 * In addition this may check a condition to the saved filename:
 * It copies to the output CImg if and only if the provided image
 * is of a certain type which is specified in the constructor.
 */

class ImageWriter_CImg : public ImageWriter
{
public:
    ImageWriter_CImg(cimg_library::CImg<uint8_t>& outframe);
    ImageWriter_CImg(cimg_library::CImg<uint8_t>& outframe, ImageType type_match);
    /** \copydoc ImageWriter::saveImage() */
    virtual std::string saveImage(const std::string& filename, ImageType type, const cimg_library::CImg<uint8_t>& frame, int quality=-1) const override;
    virtual bool writesToDisk() const override;


private:
    cimg_library::CImg<uint8_t>& m_outframe;
    ImageType m_type_match;
    bool m_do_filter;
};

#endif // IMAGEWRITER_CIMG_H
