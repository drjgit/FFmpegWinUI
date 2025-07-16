#pragma once

#include "Transcoder.g.h"

namespace winrt::FFmpegWinUI::implementation
{
    struct Transcoder : TranscoderT<Transcoder>
    {
        Transcoder()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }
    };
}

namespace winrt::FFmpegWinUI::factory_implementation
{
    struct Transcoder : TranscoderT<Transcoder, implementation::Transcoder>
    {
    };
}
