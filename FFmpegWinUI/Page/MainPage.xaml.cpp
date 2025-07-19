#include "pch.h"
#include "MainPage.xaml.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

#include "winrt/Windows.UI.Xaml.Interop.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;


// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::FFmpegWinUI::implementation
{
    void MainPage::InitializeNavigation()
    {
        // ������Ĭ��ҳ��
        contentFrame().Navigate(xaml_typename<Home>());
    }

    // �����˵���ѡ��仯���¼�
    void MainPage::onMainNaviSelectionChanged(NavigationView const& sender, NavigationViewSelectionChangedEventArgs const& args)
    {
        if (auto selectedItem = args.SelectedItem().try_as<NavigationViewItem>())
        {
            auto tag = selectedItem.Tag().try_as<winrt::hstring>();
            if (tag == L"Home")
            {
                // ������ҳѡ��
                auto page = xaml_typename<Home>();
                contentFrame().Navigate(page);
            }
            else if (tag == L"Settings")
            {
                contentFrame().Navigate(xaml_typename<Settings>());
            }
            else if (tag == L"Transcoder")
            {
                contentFrame().Navigate(xaml_typename<Transcoder>());
            }
        }
    }
}


