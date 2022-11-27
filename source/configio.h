#ifndef ConfigIO_H
#define ConfigIO_H

#include <string>

/** \brief virtual base class for writer that generate Configuration files */
class ConfigIO {
public:
    ConfigIO();
    virtual ~ConfigIO();

    /** \brief open a config file */
    virtual void open(const std::string& filename)=0;
    /** \brief close the opened config file */
    virtual void close()=0;

    /** \brief returns the curent filename */
    virtual std::string filename() const=0;

    /** \brief read a bool value */
    virtual bool value(const std::string& key, bool defaultValue) const=0;
    /** \brief read a double value */
    virtual double value(const std::string& key, double defaultValue) const=0;
    /** \brief read a unsigned int value */
    virtual unsigned int value(const std::string& key, unsigned int defaultValue) const=0;
    /** \brief read a int value */
    virtual int value(const std::string& key, int defaultValue) const=0;
    /** \brief read a std::string value */
    virtual std::string value(const std::string& key, const std::string &defaultValue) const=0;
    /** \brief read a std::string value */
    virtual std::string value(const std::string& key, const char* defaultValue) const=0;

    /** \brief write a bool value */
    virtual void setValue(const std::string& key, bool value)=0;
    /** \brief write a double value */
    virtual void setValue(const std::string& key, double value)=0;
    /** \brief write a unsigned int value */
    virtual void setValue(const std::string& key, unsigned int value)=0;
    /** \brief write a unsigned int value */
    virtual void setValue(const std::string& key, size_t value)=0;
    /** \brief write a int value */
    virtual void setValue(const std::string& key, int value)=0;
    /** \brief write a std::string value */
    virtual void setValue(const std::string& key, const std::string &value)=0;
    /** \brief write a std::string value */
    virtual void setValue(const std::string& key, const char *value)=0;
};

#endif // ConfigIO_H
