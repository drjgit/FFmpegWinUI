#pragma once

#include "Home.g.h"

namespace winrt::FFmpegWinUI::implementation
{
    struct Home : HomeT<Home>
    {
        Home()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }
    };
}

namespace winrt::FFmpegWinUI::factory_implementation
{
    struct Home : HomeT<Home, implementation::Home>
    {
    };
}
