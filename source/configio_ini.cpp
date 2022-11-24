#include "configio_ini.h"
#include <QString>
#include <QSettings>

ConfigIO_INI::ConfigIO_INI():
    m_set(nullptr)
{

}

ConfigIO_INI::~ConfigIO_INI()
{
    if (m_set) {
        m_set->sync();
        m_set.reset();
    }
}

void ConfigIO_INI::open(const std::string &filename)
{
    m_set=std::make_shared<QSettings>(QString::fromStdString(filename), QSettings::IniFormat);
}

void ConfigIO_INI::close()
{
    m_set->sync();
    m_set.reset();
}

std::string ConfigIO_INI::filename() const
{
    if (!m_set) return std::string();
    return m_set->fileName().toStdString();
}

bool ConfigIO_INI::value(const std::string &key, bool defaultValue) const
{
    if (!m_set) return defaultValue;
    return  m_set->value(QString::fromStdString(key), defaultValue).toBool();
}

double ConfigIO_INI::value(const std::string &key, double defaultValue) const
{
    if (!m_set) return defaultValue;
    return  m_set->value(QString::fromStdString(key), defaultValue).toDouble();
}

unsigned int ConfigIO_INI::value(const std::string &key, unsigned int defaultValue) const
{
    if (!m_set) return defaultValue;
    return  m_set->value(QString::fromStdString(key), defaultValue).toUInt();
}

int ConfigIO_INI::value(const std::string &key, int defaultValue) const
{
    if (!m_set) return defaultValue;
    return  m_set->value(QString::fromStdString(key), defaultValue).toInt();
}

std::string ConfigIO_INI::value(const std::string &key, const std::string &defaultValue) const
{
    if (!m_set) return defaultValue;
    return  m_set->value(QString::fromStdString(key), QString::fromStdString(defaultValue)).toString().toStdString();
}

void ConfigIO_INI::setValue(const std::string &key, bool value)
{
    if (m_set) m_set->setValue(QString::fromStdString(key), value);
}

void ConfigIO_INI::setValue(const std::string &key, double value)
{
    if (m_set) m_set->setValue(QString::fromStdString(key), value);
}

void ConfigIO_INI::setValue(const std::string &key, unsigned int value)
{
    if (m_set) m_set->setValue(QString::fromStdString(key), value);
}

void ConfigIO_INI::setValue(const std::string &key, size_t value)
{
    if (m_set) m_set->setValue(QString::fromStdString(key), value);
}

void ConfigIO_INI::setValue(const std::string &key, int value)
{
    if (m_set) m_set->setValue(QString::fromStdString(key), value);
}

void ConfigIO_INI::setValue(const std::string &key, const std::string& value)
{
    if (m_set) m_set->setValue(QString::fromStdString(key), QString::fromStdString(value));
}
