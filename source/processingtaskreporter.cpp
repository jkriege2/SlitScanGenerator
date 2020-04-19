#include "processingtaskreporter.h"

ProcessingTaskReporter::ProcessingTaskReporter():
    m_wasCanceled(false)
{

}

ProcessingTaskReporter::~ProcessingTaskReporter()
{

}

void ProcessingTaskReporter::setReporterWasCanceled()
{
    m_wasCanceled=true;
}

bool ProcessingTaskReporter::getReporterWasCanceled() const
{
    return m_wasCanceled;
}
