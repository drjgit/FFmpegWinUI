#include "pch.h"
#include "ProgressEventArgs.h"
#include "ProgressEventArgs.g.cpp"

namespace winrt::FFmpegWinUI::implementation
{
    uint64_t ProgressEventArgs::ReceivedBytes()
    {
        return m_ReceivedBytes;
    }
    void ProgressEventArgs::ReceivedBytes(uint64_t value)
    {
        m_ReceivedBytes = value;
    }
    uint64_t ProgressEventArgs::TotalBytes()
    {
        return m_TotalBytes;
    }
    void ProgressEventArgs::TotalBytes(uint64_t value)
    {
        m_TotalBytes = value;
    }
    double ProgressEventArgs::Percentage()
    {
        return m_Percentage;
    }
    void ProgressEventArgs::Percentage(double value)
    {
        m_Percentage = value;
    }
    double ProgressEventArgs::Speed()
    {
        return m_Speed;
    }
    void ProgressEventArgs::Speed(double value)
    {
        m_Speed = value;
    }
}
