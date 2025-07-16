#pragma once

#include "App.xaml.g.h"

namespace winrt::FFmpegWinUI::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);
        static HWND GetWindowHandle();

    private:
        static winrt::Microsoft::UI::Xaml::Window window;
    };
}
