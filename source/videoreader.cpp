#include "videoreader.h"
#include "cimg_tools.h"

VideoReader::VideoReader():
    m_lastError(), m_wasError(false), m_filename()
{

}

VideoReader::~VideoReader()
{

}

std::string VideoReader::getLastError() const
{
    return m_lastError;
}

bool VideoReader::hadError() const
{
    return m_wasError;
}

std::string VideoReader::getFilename() const
{
    return m_filename;
}

void VideoReader::setError(const std::string &error_message)
{
    m_wasError=true;
    m_lastError=error_message;
}

void VideoReader::resetError()
{
    m_wasError=false;
    m_lastError.clear();
}

void VideoReader::setFilename(const std::string &filename)
{
    m_filename=filename;
}

void VideoReader::resetFilename()
{
    m_filename.clear();
}
