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

    winrt::Windows::Foundation::IAsyncOperation<hstring> FFmpegService::GetFFmpegVersionAsync(winrt::hstring filePath)
    {   
        // �����ܵ������ڲ����ӽ������
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = nullptr;

        HANDLE hRead, hWrite;
        if (!CreatePipe(&hRead, &hWrite, &sa, 0))
            co_return L"(CreatePipe)-Failed to get the version!";

        // ȷ��д��˿ɱ��ӽ��̼̳�
        SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

        PROCESS_INFORMATION pi = {};
        STARTUPINFOW si = {};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        si.hStdOutput = hWrite;
        si.hStdError = hWrite;
        si.wShowWindow = SW_HIDE; // ���ش���

        // ����������
        std::wstring cmd = filePath.c_str();
        cmd.append(L"\\ffmpeg.exe -version");
        if (!CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
        {
            CloseHandle(hRead);
            CloseHandle(hWrite);
            co_return L"(CreateProcess)-Failed to get the version!";
        }
        CloseHandle(hWrite);
        CloseHandle(pi.hThread);
        // ��ȡ���
        std::string result;
        std::array<char, 256> buffer;
        DWORD bytesRead = 0;
        while (ReadFile(hRead, buffer.data(), buffer.size()-1, &bytesRead, nullptr) && bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            result += buffer.data();
        }

        CloseHandle(hRead);
        CloseHandle(pi.hProcess);

        auto pos = result.find('\n');
        if (pos != std::string::npos)
        {
            result.erase(result.begin() + pos - 1, result.end());
            co_return to_hstring(result);
        }
        co_return L"The version number of ffmpeg was not found!";
    }

    winrt::Windows::Foundation::IAsyncOperation<bool> FFmpegService::CheckFFmpegExistsAsync(hstring filePath)
    {
        try
        {
            // ���Ի�ȡ�ļ�
            hstring ffmpegFile = filePath + L"\\ffmpeg.exe";
            auto file = co_await StorageFile::GetFileFromPathAsync(ffmpegFile);
            co_return true;
        }
        catch (hresult_error const& ex)
        {
            // �ļ������ڻ��׳��쳣
            if (ex.code() == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                OutputDebugString(L"The file does not exist!");
            }
            co_return false;
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
