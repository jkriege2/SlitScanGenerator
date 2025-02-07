#include "imagewriterfactory.h"
#include "imagewriter_jpeg.h"
#include "imagewriter_png.h"

ImageWriter *makeImageWriter(ProcessingTask::FileFormat fileformat)
{
    switch(fileformat) {
    case ProcessingTask::FileFormat::JPEG:
        return new ImageWriter_JPEG();
    case ProcessingTask::FileFormat::PNG:
        return new ImageWriter_PNG();
    case ProcessingTask::FileFormat::__COUNT:
        return nullptr;
    }
    return nullptr;
}

std::shared_ptr<ImageWriter> makeSharedImageWriter(ProcessingTask::FileFormat fileformat)
{
    return std::shared_ptr<ImageWriter>(makeImageWriter(fileformat));
}
