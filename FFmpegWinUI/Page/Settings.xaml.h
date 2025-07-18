#pragma once

#include "Settings.g.h"
#include "FFmpegService/FFmpegService.h"


namespace winrt::FFmpegWinUI::implementation
{
    struct Settings : SettingsT<Settings>
    {
    private:
        IFFmpegService m_downloader{ nullptr };

    private:
        void loadSavedPath();
        void initializeDownloader();
        winrt::fire_and_forget openFFmpegFolder();

    public:
        Settings()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
            InitializeComponent();
            initializeDownloader();
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
