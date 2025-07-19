#pragma once

#include "FFmpegService.g.h"

namespace winrt::FFmpegWinUI::implementation
{
    struct FFmpegService : FFmpegServiceT<FFmpegService>
    {
        FFmpegService() = default;
        FFmpegService(winrt::Windows::System::DispatcherQueue dispatcher);

        winrt::Windows::Foundation::IAsyncAction StartDownloadAsync(winrt::hstring url, winrt::hstring fileName);

        void UpdateProgress(winrt::FFmpegWinUI::ProgressEventArgs& args);
        void UpdateStatus(winrt::hstring status);

        winrt::event_token ProgressChanged(winrt::Windows::Foundation::EventHandler<winrt::FFmpegWinUI::ProgressEventArgs> const& handler);
        void ProgressChanged(winrt::event_token const& token) noexcept;

        winrt::event_token StatusChanged(winrt::Windows::Foundation::EventHandler<hstring> const& handler);
        void StatusChanged(winrt::event_token const& token) noexcept;

        winrt::event<Windows::Foundation::EventHandler<winrt::FFmpegWinUI::ProgressEventArgs>> m_progressChanged;
        winrt::event<Windows::Foundation::EventHandler<hstring>> m_statusChanged;
        winrt::Windows::System::DispatcherQueue m_dispatcher{ nullptr };
        std::atomic<bool> m_isCanceled;
    };
}
namespace winrt::FFmpegWinUI::factory_implementation
{
    struct FFmpegService : FFmpegServiceT<FFmpegService, implementation::FFmpegService>
    {
    };
}
