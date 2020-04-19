#ifndef CONFIGIO_DUMMY_H
#define CONFIGIO_DUMMY_H

#include "configio.h"

/** \brief concrete ConfigIO implementation that does nothing and on read returns the default values */
class ConfigIO_Dummy : public ConfigIO {
public:

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
    virtual std::string value(const std::string&, const std::string& defaultValue) const override;

    /** \copydoc ConfigValue::setValue() */
    virtual void setValue(const std::string& key, bool value) override;
    /** \copydoc ConfigValue::setValue() */
    virtual void setValue(const std::string& key, double value) override;
    /** \copydoc ConfigValue::setValue() */
    virtual void setValue(const std::string& key, unsigned int value) override;
    /** \copydoc ConfigValue::setValue() */
    virtual void setValue(const std::string& key, int value) override;
    /** \copydoc ConfigValue::setValue() */
    virtual void setValue(const std::string& key, const std::string &value) override;

};

#endif // CONFIGIO_DUMMY_H
