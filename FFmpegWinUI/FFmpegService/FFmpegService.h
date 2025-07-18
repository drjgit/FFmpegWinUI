#pragma once

#include "FFmpegService.g.h"

namespace winrt::FFmpegWinUI::implementation
{
    struct FFmpegService : FFmpegServiceT<FFmpegService>
    {
        FFmpegService() = default;
        FFmpegService(winrt::Windows::System::DispatcherQueue dispatcher);

        winrt::Windows::Foundation::IAsyncAction StartDownloadAsync(winrt::hstring url, winrt::hstring fileName);

        void RaiseProgress(uint32_t progress);
        void RaiseCompleted(winrt::hstring message);

        winrt::event_token ProgressChanged(winrt::Windows::Foundation::EventHandler<uint32_t> const& handler);
        void ProgressChanged(winrt::event_token const& token) noexcept;

        winrt::event_token Completed(winrt::Windows::Foundation::EventHandler<hstring> const& handler);
        void Completed(winrt::event_token const& token) noexcept;

        winrt::event<Windows::Foundation::EventHandler<uint32_t>> m_progressChanged;
        winrt::event<Windows::Foundation::EventHandler<hstring>> m_completed;
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
