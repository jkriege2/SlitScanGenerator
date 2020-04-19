#include "configio_dummy.h"

void ConfigIO_Dummy::open(const std::string &/*filename*/)
{

}

void ConfigIO_Dummy::close()
{

}

std::string ConfigIO_Dummy::filename() const
{
    return std::string();
}

bool ConfigIO_Dummy::value(const std::string & /*key*/, bool defaultValue) const
{
    return defaultValue;
}

double ConfigIO_Dummy::value(const std::string & /*key*/, double defaultValue) const
{
    return defaultValue;
}

unsigned int ConfigIO_Dummy::value(const std::string & /*key*/, unsigned int defaultValue) const
{
    return defaultValue;
}

int ConfigIO_Dummy::value(const std::string & /*key*/, int defaultValue) const
{
    return defaultValue;
}

std::string ConfigIO_Dummy::value(const std::string & /*key*/, const std::string &defaultValue) const
{
    return defaultValue;
}

void ConfigIO_Dummy::setValue(const std::string & /*key*/, bool /*value*/)
{

}

void ConfigIO_Dummy::setValue(const std::string & /*key*/, double /*value*/)
{

}

void ConfigIO_Dummy::setValue(const std::string & /*key*/, unsigned int /*value*/)
{

}

void ConfigIO_Dummy::setValue(const std::string & /*key*/, int /*value*/)
{

}

void ConfigIO_Dummy::setValue(const std::string & /*key*/, const std::string &/*value*/)
{

}
