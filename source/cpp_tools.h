#ifndef CPP_TOOLS_H
#define CPP_TOOLS_H

#include <string>
#include <chrono>
#include <memory>
#include <atomic>

template <class F>
class final_act
{
public:
    explicit final_act(F f) noexcept
      : f_(std::move(f)), invoke_(true) {}

    final_act(final_act&& other) noexcept
     : f_(std::move(other.f_)),
       invoke_(other.invoke_)
    {
        other.invoke_ = false;
    }

    final_act(const final_act&) = delete;
    final_act& operator=(const final_act&) = delete;

    ~final_act() noexcept
    {
        if (invoke_) f_();
    }

private:
    F f_;
    bool invoke_;
};

template <class F>
inline final_act<F> finally(const F& f) noexcept
{
    return final_act<F>(f);
}

template <class F>
inline final_act<F> finally(F&& f) noexcept
{
    return final_act<F>(std::forward<F>(f));
}



class BlockTimer {
public:
    BlockTimer(const std::string& message);
    ~BlockTimer();
private:
    std::chrono::high_resolution_clock::time_point m_tstart;
    std::string m_message;
    static std::atomic<int> m_indent;
};

#define TIME_BLOCK(objname, message) BlockTimer objname(message);
#define TIME_BLOCK_A(objname) BlockTimer objname(std::string(__FUNCTION__));
#ifdef ANALYZE_TIMING
    #define TIME_BLOCK_SW(objname, message) BlockTimer objname(message);
    #define TIME_BLOCK_A_SW(objname) BlockTimer objname(std::string(__FUNCTION__));
#else
#define TIME_BLOCK_SW(objname, message)
#define TIME_BLOCK_A_SW(objname)
#endif

template <class T>
T qMinMin(T x,T y,T z) {
    return qMin<T>(qMin<T>(x,y),z);
}

template <class T>
T qMaxMax(T x,T y,T z) {
    return qMax<T>(qMax<T>(x,y),z);
}

#endif // CPP_TOOLS_H
