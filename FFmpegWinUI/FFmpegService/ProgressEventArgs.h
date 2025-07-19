#pragma once
#include "ProgressEventArgs.g.h"

namespace winrt::FFmpegWinUI::implementation
{
    struct ProgressEventArgs : ProgressEventArgsT<ProgressEventArgs>
    {
        ProgressEventArgs() = default;

        uint64_t ReceivedBytes();
        void ReceivedBytes(uint64_t value);
        uint64_t TotalBytes();
        void TotalBytes(uint64_t value);
        double Percentage();
        void Percentage(double value);
        double Speed();
        void Speed(double value);

    private:
        uint64_t m_ReceivedBytes;
        uint64_t m_TotalBytes;
        double m_Percentage;
        double m_Speed;
    };
}
namespace winrt::FFmpegWinUI::factory_implementation
{
    struct ProgressEventArgs : ProgressEventArgsT<ProgressEventArgs, implementation::ProgressEventArgs>
    {
    };
}
