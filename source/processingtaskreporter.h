#ifndef PROCESSINGTASKREPORTER_H
#define PROCESSINGTASKREPORTER_H

#include <atomic>
#include <string>

/** \brief this is a virtual baseclass for Reporters used by ProcessingTask to display status information */
class ProcessingTaskReporter
{
public:
    ProcessingTaskReporter();
    virtual ~ProcessingTaskReporter();

    /** \brief set the frame processing progress, if \a updateMessage is \c true, this also updates the message displayed in the progress (if the widget supports this!)
     *
     * \note The implementation needs to be \b thread-safe!!!
     */
    virtual void reportFrameProgress(int currentFrame, int maxFrames, bool updateMessage=true)=0;
    /** \brief report an arbitrary message
     *
     * \note The implementation needs to be \b thread-safe!!!
     */
    virtual void reportMessage(const std::string& message)=0;
    /** \brief report that the \a n -th file was stored to the given \a filename
     *
     * \note The implementation needs to be \b thread-safe!!!
     */
    virtual void reportMessageSavedResult(const std::string& filename, int n)=0;
    /** \brief report that the the video is currently beeing opened
     *
     * \note The implementation needs to be \b thread-safe!!!
     */
    virtual void reportMessageOpeningVideo()=0;
    /** \brief report an error message
     *
     * \note The implementation needs to be \b thread-safe!!!
     */
    virtual void reportErrorMessage(const std::string& message)=0;
    /** \brief report canceled by user
     *
     * \note The implementation needs to be \b thread-safe!!!
     */
    virtual void reportCanceledByUser()=0;
    /** \brief report that the processing finished sucessfully
     *
     * \note The implementation needs to be \b thread-safe!!!
     */
    virtual void reportProcessingFinished()=0;


    /** \brief store internally that the processing should be canceled, \b thread-safe */
    void setReporterWasCanceled();

    /** \brief indicates that the task has been canceled \b (thread-safe) */
    bool getReporterWasCanceled() const;

private:
    std::atomic<bool> m_wasCanceled;

};

#endif // PROCESSINGTASKREPORTER_H
