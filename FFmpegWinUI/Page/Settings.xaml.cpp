#include "pch.h"
#include "Settings.xaml.h"
#if __has_include("Settings.g.cpp")
#include "Settings.g.cpp"
#endif

#include "winrt/Windows.Storage.Pickers.h"
#include "winrt/Windows.Storage.h"
#include "App.xaml.h"
#include <ShObjIdl.h>
#include <stdio.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

#define FFMPEG_BIN_PATH L"FFmpegBinPath"
#define FFMPEG_DOWNLOAD_LINK  L"https://www.gyan.dev/ffmpeg/builds/packages/ffmpeg-7.0.2-essentials_build.zip"
#define TEST_LINK L"https://dl.todesk.com/irrigation/ToDesk_4.8.0.1.exe"

namespace winrt::FFmpegWinUI::implementation
{
    bool IsValidPath(hstring const& path)
    {
        if (path.empty()) return false;

        // 检查路径是否存在
        auto attr = GetFileAttributes(path.c_str());
        if (attr == INVALID_FILE_ATTRIBUTES)
            return false;

        return true;
    }

    hstring FormatSpeed(double bytesPerSecond)
    {
        constexpr double KB = 1024;
        constexpr double MB = KB * 1024;
        constexpr double GB = MB * 1024;

        wchar_t array[64] = { 0 };
        if (bytesPerSecond >= GB)
            swprintf_s(array, 64, L"%.2f GB/s", bytesPerSecond / GB);
        else if (bytesPerSecond >= MB)
            swprintf_s(array, 64, L"%.2f MB/s", bytesPerSecond / MB);
        else if (bytesPerSecond >= KB)
            swprintf_s(array, 64, L"%.2f KB/s", bytesPerSecond / KB);
        else
            swprintf_s(array, 64, L"%.0f B/s", bytesPerSecond);
        return hstring(array);
    }

    void Settings::loadSavedPath()
    {
        // 读取数据
        auto settings = winrt::Windows::Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(FFMPEG_BIN_PATH);
        if (settings != nullptr)
        {
            auto data = settings.as<hstring>();
            FFmpegPathBox().Text(data);
        }
    }

    void Settings::initializeDownloader()
    {
        Windows::System::DispatcherQueue dispatcher = Windows::System::DispatcherQueue::GetForCurrentThread();
        m_downloader = winrt::make<FFmpegService>(dispatcher);
        // 注册事件
        m_downloader.ProgressChanged([weakThis = get_weak()](auto&&, uint32_t progress)
            {
                if (auto self = weakThis.get())
                {
                    self->DownloadProgressBar().Value(progress);
                    self->InfoTextBlock().Text(to_hstring(progress) + L"%");
                }

            });

        m_downloader.Completed([weakThis = get_weak()](auto&&, hstring message)
            {
                if (auto self = weakThis.get())
                {
                    self->StatusText().Text(message);
                }
            });
    }

    winrt::fire_and_forget Settings::openFFmpegFolder()
    {
        FolderPicker folderPicker{};

        // 获取窗口句柄（HWND）
        auto hWnd = App::GetWindowHandle();

        // 关联窗口
        auto initializeWithWindow = folderPicker.as<IInitializeWithWindow>();
        winrt::check_hresult(initializeWithWindow->Initialize(hWnd));

        // 初始化选择器
        folderPicker.SuggestedStartLocation(PickerLocationId::Downloads);

        // 设置文件类型过滤器（可选）
        folderPicker.FileTypeFilter().Append(L"*");

        // 显示对话框
        auto folder = co_await folderPicker.PickSingleFolderAsync();
        if (folder != nullptr)
        {
            // 获取选择的路径
            hstring path = folder.Path();
            // 更新UI
            FFmpegPathBox().Text(path);
            // 检查路径
            if (IsValidPath(path))
            {
                // 保存数据
                winrt::Windows::Storage::ApplicationData::Current().LocalSettings().Values().Insert(FFMPEG_BIN_PATH,
                    winrt::box_value(path));

                // 更新 InfoBar
                StatusInfoBar().Message(L"已找到 FFmpeg: {version}");
                StatusInfoBar().Severity(InfoBarSeverity::Success);
                StatusInfoBar().IsOpen(true);
            }
            else
            {
                // 更新 InfoBar
                StatusInfoBar().Message(L"未找到有效的 FFmpeg");
                StatusInfoBar().Severity(InfoBarSeverity::Warning);
                StatusInfoBar().IsOpen(true);
            }
        }
    }

	void Settings::OnDownloadFFmpeg(IInspectable const& sender, RoutedEventArgs const& e)
	{
        hstring uri(TEST_LINK);
        hstring fileName(L"ffmpeg-release-essentials.zip");

        FileNameText().Text(fileName);
        DownloadFFmpegBtn().IsEnabled(false);
        ProgressGrid().Visibility(Visibility::Visible);
        ErrorInfoBadge().Visibility(Visibility::Collapsed);
        OpenFFmpegFolderBtn().Visibility(Visibility::Collapsed);

        m_downloader.StartDownloadAsync(uri, fileName);
	}

	void Settings::OnSelectFFmpegPath(IInspectable const& sender, RoutedEventArgs const& e)
	{
        openFFmpegFolder();
	}

	void Settings::OnWingetInstall(IInspectable const& sender, RoutedEventArgs const& e)
	{

	}

	void Settings::OnOpenFFmpegFolder(IInspectable const& sender, RoutedEventArgs const& e)
	{

	}
}
