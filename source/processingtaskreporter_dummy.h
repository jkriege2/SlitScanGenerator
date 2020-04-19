#ifndef PROCESSINGTASKREPORTER_DUMMY_H
#define PROCESSINGTASKREPORTER_DUMMY_H

#include "processingtaskreporter.h"

/** \brief a dummy ProcessingTaskReporter implementation that does nothing */
class ProcessingTaskReporter_Dummy : public ProcessingTaskReporter
{
public:
    ProcessingTaskReporter_Dummy();

    /** \copydoc ProcessingTaskReporter::reportFrameProgress() */
    virtual void reportFrameProgress(int currentFrame, int maxFrames, bool updateMessage=true) override;
    /** \copydoc ProcessingTaskReporter::reportMessage() */
    virtual void reportMessage(const std::string& message) override;
    /** \copydoc ProcessingTaskReporter::reportMessageSavedResult() */
    virtual void reportMessageSavedResult(const std::string& filename, int n) override;
    /** \copydoc ProcessingTaskReporter::reportMessageOpeningVideo() */
    virtual void reportMessageOpeningVideo() override;
    /** \copydoc ProcessingTaskReporter::reportErrorMessage() */
    virtual void reportErrorMessage(const std::string& message) override;
    /** \copydoc ProcessingTaskReporter::reportCanceledByUser() */
    virtual void reportCanceledByUser() override;
    /** \copydoc ProcessingTaskReporter::reportProcessingFinished() */
    virtual void reportProcessingFinished() override;

};

#endif // PROCESSINGTASKREPORTER_DUMMY_H
