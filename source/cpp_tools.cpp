#include "cpp_tools.h"
#include <iostream>

std::atomic<int> BlockTimer::m_indent(0);

BlockTimer::BlockTimer(const std::string &message):
    m_tstart(std::chrono::high_resolution_clock::now()),m_message(message)
{
    std::cout<<std::string(m_indent.load()*2, ' ')<<"STARTING "<<message<<" ..."<<std::endl;
    m_indent+=2;
}

BlockTimer::~BlockTimer()
{
    m_indent-=2;
    const auto now=std::chrono::high_resolution_clock::now();
    const double dur_ms=static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(now-m_tstart).count())/1000000.0;
    std::cout<<std::string(m_indent.load()*2, ' ')<<"FINISHING "<<m_message<<" ... duration="<<std::fixed<<dur_ms<<"ms"<<std::endl;
}
