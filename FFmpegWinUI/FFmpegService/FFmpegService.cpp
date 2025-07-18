#include "pch.h"
#include "FFmpegService.h"
#include "FFmpegService.g.cpp"

#include "winrt/Windows.Web.Http.Headers.h"
#include "winrt/Windows.Storage.Pickers.h"
#include "winrt/Windows.Storage.Streams.h"
#include "winrt/Windows.Storage.h"
#include "winrt/Windows.Web.Http.h"
#include "winrt/Windows.System.h"

using namespace winrt;
using namespace Windows::System;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage;
using namespace Windows::Web::Http;
using namespace Windows::Foundation;

#include <winrt/Windows.System.Threading.h>

namespace winrt::FFmpegWinUI::implementation
{
    FFmpegService::FFmpegService(DispatcherQueue dispatcher)
        : m_dispatcher(dispatcher), m_isCanceled(false)
    {

    }

    winrt::Windows::Foundation::IAsyncAction FFmpegService::StartDownloadAsync(hstring url, hstring fileName)
    {
        m_isCanceled = false;
        uint64_t lastBytes = 0;
        uint64_t receivedBytes = 0;
        std::chrono::steady_clock::time_point lastUpdate;
        try
        {
            RaiseCompleted(L"��ʼ����...");
            HttpClient httpClient;
            Windows::Foundation::Uri uri{ url };
            HttpResponseMessage response = co_await httpClient.GetAsync(uri, HttpCompletionOption::ResponseHeadersRead);
            if (!response.IsSuccessStatusCode())
            {
                RaiseCompleted(L"HTTP Error: " + to_hstring(static_cast<int>(response.StatusCode())));
                co_return;
            }

            uint64_t totalBytes = response.Content().Headers().ContentLength().GetUInt64();

            // ����Ŀ���ļ�
            StorageFolder downloadsFolder = co_await KnownFolders::GetFolderForUserAsync(nullptr, KnownFolderId::DownloadsFolder);
            StorageFile file = co_await downloadsFolder.CreateFileAsync(fileName, CreationCollisionOption::ReplaceExisting);

            // ���ļ�д����
            IOutputStream outputStream = co_await file.OpenAsync(FileAccessMode::ReadWrite);

            // ��ȡ��Ӧ����
            IInputStream inputStream = co_await response.Content().ReadAsInputStreamAsync();
            Buffer buffer(1024 * 1024); // 1MB������

            while (true)
            {
                if (m_isCanceled)
                {
                    RaiseCompleted(L"Download canceled");
                    co_return;
                }

                // ��ȡ���ݿ�
                IBuffer bytesRead = co_await inputStream.ReadAsync(buffer, buffer.Capacity(), InputStreamOptions::None);
                if (bytesRead.Length() == 0)
                    break;

                // д���ļ�
                co_await outputStream.WriteAsync(bytesRead);
                receivedBytes += bytesRead.Length();

                uint32_t progress = (uint32_t)receivedBytes / totalBytes * 100.0;

                // ���������ٶ�
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count();
                // ÿ�����һ���ٶ�
                if (elapsed > 1000)
                {
                    double speed = (receivedBytes - lastBytes) / (elapsed / 1000.0);
                    lastBytes = receivedBytes;
                    lastUpdate = now;
                }

                RaiseProgress(progress);
            }

            co_await outputStream.FlushAsync();
            RaiseCompleted(L"Download complete");

            if (outputStream)
            {
                outputStream.Close();
                outputStream = nullptr;
            }
        }
        catch (const std::exception& e)
        {
            RaiseCompleted(winrt::to_hstring(e.what()));
        }
    }

    void FFmpegService::RaiseProgress(uint32_t progress)
    {
        if (m_dispatcher.HasThreadAccess())
        {
            m_progressChanged(*this, progress);
        }
        else
        {
            m_dispatcher.TryEnqueue([=]()
                {
                    m_progressChanged(*this, progress);
                });
        }
    }

    void FFmpegService::RaiseCompleted(hstring message)
    {
        if (m_dispatcher.HasThreadAccess())
        {
            m_completed(*this, message);
        }
        else
        {
            m_dispatcher.TryEnqueue([=]()
                {
                    m_completed(*this, message);
                });
        }
    }

    winrt::event_token FFmpegService::ProgressChanged(winrt::Windows::Foundation::EventHandler<uint32_t> const& handler)
    {
        return m_progressChanged.add(handler);
    }

    void FFmpegService::ProgressChanged(winrt::event_token const& token) noexcept
    {
        m_progressChanged.remove(token);
    }

    winrt::event_token FFmpegService::Completed(winrt::Windows::Foundation::EventHandler<hstring> const& handler)
    {
        return m_completed.add(handler);
    }
    void FFmpegService::Completed(winrt::event_token const& token) noexcept
    {
        m_completed.remove(token);
    }
}
