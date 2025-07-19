#include "pch.h"
#include "FFmpegService.h"
#include "FFmpegService.g.cpp"

#include "winrt/Windows.Web.Http.Headers.h"
#include "winrt/Windows.Storage.Pickers.h"
#include "winrt/Windows.Storage.Streams.h"
#include "winrt/Windows.Storage.h"
#include "winrt/Windows.Web.Http.h"
#include "winrt/Windows.System.h"
#include "ProgressEventArgs.h"

using namespace winrt;
using namespace Windows::System;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage;
using namespace Windows::Web::Http;
using namespace Windows::Foundation;

namespace winrt::FFmpegWinUI::implementation
{
    FFmpegService::FFmpegService(DispatcherQueue dispatcher)
        : m_dispatcher(dispatcher), m_isCanceled(false)
    {

    }

    winrt::Windows::Foundation::IAsyncAction FFmpegService::StartDownloadAsync(hstring url, hstring fileName)
    {
        double speed = 0;
        uint64_t lastBytes = 0;
        uint64_t receivedBytes = 0;
        std::chrono::steady_clock::time_point lastUpdate;
        try
        {
            UpdateStatus(L"Start downloading");
            HttpClient httpClient;
            Windows::Foundation::Uri uri{ url };
            HttpResponseMessage response = co_await httpClient.GetAsync(uri, HttpCompletionOption::ResponseHeadersRead);
            if (!response.IsSuccessStatusCode())
            {
                UpdateStatus(L"HTTP Error: " + to_hstring(static_cast<int>(response.StatusCode())));
                co_return;
            }

            uint64_t totalBytes = response.Content().Headers().ContentLength().GetUInt64();

            // ����Ŀ���ļ�
            StorageFolder downloadsFolder = co_await KnownFolders::GetFolderForUserAsync(nullptr, KnownFolderId::DownloadsFolder);
            StorageFile file = co_await downloadsFolder.CreateFileAsync(fileName, CreationCollisionOption::GenerateUniqueName);

            // ���ļ�д����
            IOutputStream outputStream = co_await file.OpenAsync(FileAccessMode::ReadWrite);

            // ��ȡ��Ӧ����
            IInputStream inputStream = co_await response.Content().ReadAsInputStreamAsync();
            Buffer buffer(1024 * 1024); // 1MB������

            while (true)
            {
                // ��ȡ���ݿ�
                IBuffer bytesRead = co_await inputStream.ReadAsync(buffer, buffer.Capacity(), InputStreamOptions::None);
                if (bytesRead.Length() == 0)
                    break;

                // д���ļ�
                co_await outputStream.WriteAsync(bytesRead);
                receivedBytes += bytesRead.Length();

                double progress = (double)receivedBytes / totalBytes * 100.0;

                // ���������ٶ�
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count();
                
                // ÿ�����һ���ٶ�
                if (elapsed > 1000)
                {
                    speed = (receivedBytes - lastBytes) / (elapsed / 1000.0);
                    lastBytes = receivedBytes;
                    lastUpdate = now;
                }
                auto ptr = winrt::make<ProgressEventArgs>();
                ptr.ReceivedBytes(receivedBytes);
                ptr.TotalBytes(totalBytes);
                ptr.Percentage(progress);
                ptr.Speed(speed);
                UpdateProgress(ptr);
            }

            co_await outputStream.FlushAsync();
            UpdateStatus(L"Download complete");

            if (outputStream)
            {
                outputStream.Close();
                outputStream = nullptr;
            }
        }
        catch (const std::exception& e)
        {
            UpdateStatus(winrt::to_hstring(e.what()));
        }
    }

    void FFmpegService::UpdateProgress(winrt::FFmpegWinUI::ProgressEventArgs& args)
    {
        if (m_dispatcher.HasThreadAccess())
        {
            m_progressChanged(*this, args);
        }
        else
        {
            m_dispatcher.TryEnqueue([=]()
                {
                    m_progressChanged(*this, args);
                });
        }
    }

    void FFmpegService::UpdateStatus(hstring status)
    {
        if (m_dispatcher.HasThreadAccess())
        {
            m_statusChanged(*this, status);
        }
        else
        {
            m_dispatcher.TryEnqueue([=]()
                {
                    m_statusChanged(*this, status);
                });
        }
    }
    winrt::event_token FFmpegService::ProgressChanged(winrt::Windows::Foundation::EventHandler<winrt::FFmpegWinUI::ProgressEventArgs> const& handler)
    {
        return m_progressChanged.add(handler);
    }

    void FFmpegService::ProgressChanged(winrt::event_token const& token) noexcept
    {
        m_progressChanged.remove(token);
    }

    winrt::event_token FFmpegService::StatusChanged(winrt::Windows::Foundation::EventHandler<hstring> const& handler)
    {
        return m_statusChanged.add(handler);
    }
    void FFmpegService::StatusChanged(winrt::event_token const& token) noexcept
    {
        m_statusChanged.remove(token);
    }
}
