#ifndef IMAGEWRITERFACTORY_H
#define IMAGEWRITERFACTORY_H
#include "imagewriter.h"
#include "processingtask.h"
#include <memory>

ImageWriter* makeImageWriter(ProcessingTask::FileFormat fileformat);
std::shared_ptr<ImageWriter> makeSharedImageWriter(ProcessingTask::FileFormat fileformat);

#endif // IMAGEWRITERFACTORY_H
