#pragma once

#include "Settings.g.h"

namespace winrt::FFmpegWinUI::implementation
{
    struct Settings : SettingsT<Settings>
    {
    private:
        void loadSavedPath();
        winrt::fire_and_forget openFFmpegFolder();

    public:
        winrt::Windows::Foundation::IAsyncAction StartDownloadAsync(winrt::hstring& uri, winrt::hstring& fileName);

        void Pause();
        void Resume();
        void Cancel();
        void UpdateProgress();
        void UpdateStatus(winrt::hstring status);

    private:
        winrt::Microsoft::UI::Dispatching::DispatcherQueue m_dispatcherQueue{ nullptr };
        std::chrono::steady_clock::time_point m_lastUpdate;
        std::atomic<bool> m_cancelRequested{ false };
        std::atomic<bool> m_paused{ false };
        uint64_t m_totalBytes = 0;
        uint64_t m_receivedBytes = 0;
        uint64_t m_lastBytes = 0;
        bool m_hasTotalSize{ false };

    public:
        Settings()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
            InitializeComponent();
            loadSavedPath();
        }
        
        void OnDownloadFFmpeg(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void OnSelectFFmpegPath(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void OnWingetInstall(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void OnOpenFFmpegFolder(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::FFmpegWinUI::factory_implementation
{
    struct Settings : SettingsT<Settings, implementation::Settings>
    {
    };
}
