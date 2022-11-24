#ifndef CONFIGIO_INI_H
#define CONFIGIO_INI_H

#include "configio.h"
#include <memory>

class QSettings; //forward

/** \brief concrete ConfigIO implementation that reads/write to/from an INI-file */
class ConfigIO_INI : public ConfigIO
{
public:
    ConfigIO_INI();
    virtual ~ConfigIO_INI();

    /** \copydoc ConfigValue::open() */
    virtual void open(const std::string& filename) override;
    /** \copydoc ConfigValue::close() */
    virtual void close() override;

    /** \copydoc ConfigValue::filename() */
    virtual std::string filename() const override;

    /** \copydoc ConfigValue::value() */
    virtual bool value(const std::string& key, bool defaultValue) const override;
    /** \copydoc ConfigValue::value() */
    virtual double value(const std::string& key, double defaultValue) const override;
    /** \copydoc ConfigValue::value() */
    virtual unsigned int value(const std::string& key, unsigned int defaultValue) const override;
    /** \copydoc ConfigValue::value() */
    virtual int value(const std::string& key, int defaultValue) const override;
    /** \copydoc ConfigValue::value() */
    virtual std::string value(const std::string& key, const std::string &defaultValue) const override;

    /** \copydoc ConfigValue::setValue() */
    virtual void setValue(const std::string& key, bool value) override;
    /** \copydoc ConfigValue::setValue() */
    virtual void setValue(const std::string& key, double value) override;
    /** \copydoc ConfigValue::setValue() */
    virtual void setValue(const std::string& key, unsigned int value) override;
    /** \copydoc ConfigValue::setValue() */
    virtual void setValue(const std::string& key, size_t value) override;
    /** \copydoc ConfigValue::setValue() */
    virtual void setValue(const std::string& key, int value) override;
    /** \copydoc ConfigValue::setValue() */
    virtual void setValue(const std::string& key, const std::string &value) override;

private:
    std::shared_ptr<QSettings> m_set;
};

#endif // CONFIGIO_INI_H
