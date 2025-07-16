#include "pch.h"
#include "Settings.xaml.h"
#if __has_include("Settings.g.cpp")
#include "Settings.g.cpp"
#endif

#include "winrt/Windows.Web.Http.Headers.h"
#include "winrt/Windows.Storage.Pickers.h"
#include "winrt/Windows.Storage.Streams.h"
#include "winrt/Windows.Storage.h"
#include "winrt/Windows.Web.Http.h"
#include "App.xaml.h"
#include <ShObjIdl.h>
#include <stdio.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage;
using namespace Windows::Web::Http;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

#define FFMPEG_BIN_PATH L"FFmpegBinPath"

namespace winrt::FFmpegWinUI::implementation
{
    bool IsValidPath(hstring const& path)
    {
        if (path.empty()) return false;

        // ���·���Ƿ����
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

        wchar_t array[64] = {0};
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

    winrt::Windows::Foundation::IAsyncAction Settings::StartDownloadAsync(hstring& uri, hstring& fileName)
    {
        //auto lifetime = get_strong();
        m_cancelRequested = false;
        m_paused = false;
        m_hasTotalSize = false;
        m_receivedBytes = 0;
        m_totalBytes = 0;
        m_lastBytes = 0;
        m_lastUpdate = std::chrono::steady_clock::now();
        IOutputStream outputStream{ nullptr };
        try
        {
            //co_await resume_background(); // �л�����̨�߳�

            // ����GET����
            UpdateStatus(L"��ʼ����...");
            Windows::Web::Http::HttpClient httpClient;
            Windows::Foundation::Uri requestUri{ uri };
            HttpResponseMessage response = co_await httpClient.GetAsync(requestUri, HttpCompletionOption::ResponseHeadersRead);
            response.EnsureSuccessStatusCode();

            // ��ȡ�ļ��ܴ�С(���ܱ���ͨ��)
            m_totalBytes = response.Content().Headers().ContentLength().GetUInt64();
            m_hasTotalSize = m_totalBytes > 0;

            // ����Ŀ���ļ�
            StorageFolder downloadsFolder = co_await KnownFolders::GetFolderForUserAsync(nullptr, KnownFolderId::DownloadsFolder);
            StorageFile file = co_await downloadsFolder.CreateFileAsync(fileName, CreationCollisionOption::GenerateUniqueName);

            // ���ļ�д����
            outputStream = co_await file.OpenAsync(FileAccessMode::ReadWrite);

            // ��ȡ��Ӧ����
            IInputStream  inputStream = co_await response.Content().ReadAsInputStreamAsync();
            Windows::Storage::Streams::Buffer buffer(1024 * 1024); // 1MB������

            while (true)
            {
                if (m_cancelRequested) 
                    throw hresult_canceled();

                while (m_paused)
                {
                    co_await resume_after(std::chrono::seconds(1));
                }

                // ��ȡ���ݿ�
                IBuffer bytesRead = co_await inputStream.ReadAsync(buffer, buffer.Capacity(), InputStreamOptions::None);
                if (bytesRead.Length() == 0) 
                    break;

                // д���ļ�
                co_await outputStream.WriteAsync(bytesRead);
                m_receivedBytes += bytesRead.Length();

                UpdateProgress();
            }

            co_await outputStream.FlushAsync();

            if (!m_cancelRequested)
            {
                UpdateStatus(L"�������");
            }
        }
        catch (hresult_canceled const&)
        {
            UpdateStatus(L"������ȡ��");
        }
        catch (hresult_error const& ex)
        {
            UpdateStatus(L"���ش���: " + ex.message());
        }

        if (outputStream)
        {
            outputStream.Close();
            outputStream = nullptr;
        }
    }

    void Settings::Pause()
    {
        m_paused = true;
        UpdateStatus(L"��������ͣ");
    }

    void Settings::Resume()
    {
        m_paused = false;
        UpdateStatus(L"�����ѻָ�");
    }

    void Settings::Cancel()
    {
        m_cancelRequested = true;
    }

    void Settings::UpdateProgress()
    {
        if (m_hasTotalSize)
        {
            double progress = static_cast<double>(m_receivedBytes) / m_totalBytes;

            DownloadProgressBar().Value(progress * 100);
            InfoTextBlock().Text(to_hstring(static_cast<int>(progress * 100)) + L"%");
        }
        // ���������ٶ�
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastUpdate).count();
        // ÿ�����һ���ٶ�
        if (elapsed > 1000) 
        {
            double speed = (m_receivedBytes - m_lastBytes) / (elapsed / 1000.0);
            SpeedText().Text(FormatSpeed(speed));
            m_lastBytes = m_receivedBytes;
            m_lastUpdate = now;
        }
    }

    void Settings::UpdateStatus(hstring status)
    {
        StatusText().Text(status);
    }

    void Settings::loadSavedPath()
    {
        // ��ȡ����
        auto settings = winrt::Windows::Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(FFMPEG_BIN_PATH);
        if (settings != nullptr)
        {
            auto data = settings.as<hstring>();
            FFmpegPathBox().Text(data);
        }
    }

    winrt::fire_and_forget Settings::openFFmpegFolder()
    {
        FolderPicker folderPicker{};

        // ��ȡ���ھ����HWND��
        auto hWnd = App::GetWindowHandle();

        // ��������
        auto initializeWithWindow = folderPicker.as<IInitializeWithWindow>();
        winrt::check_hresult(initializeWithWindow->Initialize(hWnd));

        // ��ʼ��ѡ����
        folderPicker.SuggestedStartLocation(PickerLocationId::Downloads);

        // �����ļ����͹���������ѡ��
        folderPicker.FileTypeFilter().Append(L"*");

        // ��ʾ�Ի���
        auto folder = co_await folderPicker.PickSingleFolderAsync();
        if (folder != nullptr)
        {
            // ��ȡѡ���·��
            hstring path = folder.Path();
            // ����UI
            FFmpegPathBox().Text(path);
            // ���·��
            if (IsValidPath(path))
            {
                // ��������
                winrt::Windows::Storage::ApplicationData::Current().LocalSettings().Values().Insert(FFMPEG_BIN_PATH,
                    winrt::box_value(path));

                // ���� InfoBar
                StatusInfoBar().Message(L"���ҵ� FFmpeg: {version}");
                StatusInfoBar().Severity(InfoBarSeverity::Success);
                StatusInfoBar().IsOpen(true);
            }
            else
            {
                // ���� InfoBar
                StatusInfoBar().Message(L"δ�ҵ���Ч�� FFmpeg");
                StatusInfoBar().Severity(InfoBarSeverity::Warning);
                StatusInfoBar().IsOpen(true);
            }
        }
    }

	void Settings::OnDownloadFFmpeg(IInspectable const& sender, RoutedEventArgs const& e)
	{
        DownloadFFmpegBtn().IsEnabled(false);

        ProgressGrid().Visibility(Visibility::Visible);
        ErrorInfoBadge().Visibility(Visibility::Collapsed);
        OpenFFmpegFolderBtn().Visibility(Visibility::Collapsed);

        hstring uri(L"https://www.gyan.dev/ffmpeg/builds/packages/ffmpeg-7.0.2-essentials_build.zip");
        hstring fileName(L"ffmpeg-release-essentials.zip");
        StartDownloadAsync(uri, fileName);
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
