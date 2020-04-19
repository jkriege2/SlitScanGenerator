#include "processingtaskreporter_dummy.h"

ProcessingTaskReporter_Dummy::ProcessingTaskReporter_Dummy():
    ProcessingTaskReporter()
{

}

void ProcessingTaskReporter_Dummy::reportFrameProgress(int currentFrame, int maxFrames, bool updateMessage)
{

}

void ProcessingTaskReporter_Dummy::reportMessage(const std::string &message)
{

}

void ProcessingTaskReporter_Dummy::reportMessageSavedResult(const std::string &filename, int n)
{

}

void ProcessingTaskReporter_Dummy::reportMessageOpeningVideo()
{

}

void ProcessingTaskReporter_Dummy::reportErrorMessage(const std::string &message)
{

}

void ProcessingTaskReporter_Dummy::reportCanceledByUser()
{

}

void ProcessingTaskReporter_Dummy::reportProcessingFinished()
{

}
