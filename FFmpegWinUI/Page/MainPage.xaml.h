#pragma once

#include "MainPage.g.h"

namespace winrt::FFmpegWinUI::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
    private:
        void InitializeNavigation();

    public:
        MainPage()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
            InitializeComponent();
            InitializeNavigation();
        }
  
        void onMainNaviSelectionChanged(winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs const& args);
    };
}

namespace winrt::FFmpegWinUI::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
