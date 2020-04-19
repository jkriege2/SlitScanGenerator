#ifndef VIDEOREADER_H
#define VIDEOREADER_H

#define _USE_MATH_DEFINES
#include "CImg.h"
#include <cstdint>
#include <string>

/** \brief base class for video readers that can read a video frame by frame
 *
 * usage:
 * \code
 *  std::unique_ptr<VideoReader> reader { new ConcreteReader(...) };
 *  if (reader->open()) {
 *    while(reader->readNext(frame)) {
 *      process(frame);
 *    }
 *    reader->close();
 *  } else {
 *    std::cout<<reader->getLastError();
 *  }
 * \endcode
 *
 * \note This class is moveable only, not copyable!
 */
class VideoReader {
public:
    VideoReader();
    virtual ~VideoReader() ;

    VideoReader(const VideoReader& )=delete;
    VideoReader(VideoReader&& )=default;
    VideoReader& operator=(const VideoReader& )=delete;
    VideoReader& operator=(VideoReader&& )=default;

    /** \brief open the video stream */
    virtual bool open(const std::string& filename)=0;
    /** \brief read the next frame into \a frame, resize \a frame if necessary! */
    virtual bool readNext(cimg_library::CImg<uint8_t>& frame)=0;
    /** \brief close the video stream */
    virtual void close()=0;
    /** \brief returns the number of frames in the current video */
    virtual int getFrameCount() const=0;

    /** \brief get the last error message */
    std::string getLastError() const;
    /** \brief have there been errors? */
    bool hadError() const;

    /** \brief return the name of the currently opened file */
    std::string getFilename() const;
protected:
    /** \brief set the error state to \c true and set the given \a error_message
     *         Use getLastError() and hadError() to access the data! */
    void setError(const std::string &error_message);
    /** \brief reset the error state */
    void resetError();
    /** \brief store a new filename */
    void setFilename(const std::string& filename);
    /** \brief reset the stored filename */
    void resetFilename();
private:
    std::string m_lastError;
    bool m_wasError;
    std::string m_filename;
};

#endif // VIDEOREADER_H
