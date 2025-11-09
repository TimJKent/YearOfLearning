#define NOMINMAX

#include <iostream>
#include <SOGLR/SOGLR.hpp>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <iostream>
#include <wrl.h>
#include <algorithm>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

using Microsoft::WRL::ComPtr;

HRESULT hr;
ComPtr<IMFSample> sample;
DWORD streamIndex, flags;
LONGLONG timestamp;
SOGLR::Renderer renderer;

int width, height;

void convertNv12ToRgbManual(const unsigned char *nv12_data, int w, int h, unsigned char *rgb_data)
{
    const unsigned char *y_plane = nv12_data;
    const unsigned char *uv_plane = nv12_data + (w * h);

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            int Y = y_plane[y * w + x];

            // UV plane is subsampled by 2 in both dimensions
            int U = uv_plane[(y / 2) * w + (x / 2) * 2];
            int V = uv_plane[(y / 2) * w + (x / 2) * 2 + 1];

            // Adjust Y, U, V values
            Y = Y - 16;
            U = U - 128;
            V = V - 128;

            // YUV to RGB conversion formulas (ITU-R BT.601)
            int R = static_cast<int>(1.164 * Y + 1.596 * V);
            int G = static_cast<int>(1.164 * Y - 0.813 * V - 0.391 * U);
            int B = static_cast<int>(1.164 * Y + 2.018 * U);

            // Clip RGB values to 0-255 range
            R = std::max(0, std::min(255, R));
            G = std::max(0, std::min(255, G));
            B = std::max(0, std::min(255, B));

            // Store RGB data (assuming RGB interleaved format)
            rgb_data[(y * w + x) * 3 + 0] = static_cast<unsigned char>(R);
            rgb_data[(y * w + x) * 3 + 1] = static_cast<unsigned char>(G);
            rgb_data[(y * w + x) * 3 + 2] = static_cast<unsigned char>(B);
        }
    }
}

int main()
{
    width = 1100;
    height = 700;
    renderer.GetWindow()->SetTitle("Webcam Rendering");
    renderer.GetWindow()->Resize(width, height);

    hr = MFStartup(MF_VERSION);
    if (FAILED(hr))
    {
        std::cerr << "MFStartup failed\n";
        return -1;
    }

    ComPtr<IMFAttributes> attr;
    hr = MFCreateAttributes(&attr, 1);
    attr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);

    IMFActivate **devices = nullptr;
    UINT32 count = 0;
    hr = MFEnumDeviceSources(attr.Get(), &devices, &count);
    if (FAILED(hr) || count == 0)
    {
        std::cerr << "No camera found\n";
        MFShutdown();
        return -1;
    }

    // use first camera
    ComPtr<IMFMediaSource> source;
    hr = devices[0]->ActivateObject(IID_PPV_ARGS(&source));
    for (UINT32 i = 0; i < count; i++)
        devices[i]->Release();
    CoTaskMemFree(devices);

    if (FAILED(hr))
    {
        std::cerr << "Failed to activate camera\n";
        MFShutdown();
        return -1;
    }

    // Create reader for frames
    ComPtr<IMFSourceReader> reader;
    hr = MFCreateSourceReaderFromMediaSource(source.Get(), nullptr, &reader);
    if (FAILED(hr))
    {
        std::cerr << "Failed to create reader\n";
        MFShutdown();
        return -1;
    }

    // Check what format the camera is actually providing
    ComPtr<IMFMediaType> currentType;
    hr = reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &currentType);
    if (SUCCEEDED(hr))
    {
        GUID subtype;
        currentType->GetGUID(MF_MT_SUBTYPE, &subtype);

        UINT32 actualWidth, actualHeight;
        MFGetAttributeSize(currentType.Get(), MF_MT_FRAME_SIZE, &actualWidth, &actualHeight);
        std::cout << "Camera actual resolution: " << actualWidth << "x" << actualHeight << std::endl;

        // Update our width/height to match camera
        width = actualWidth;
        height = actualHeight;

        if (subtype == MFVideoFormat_NV12)
            std::cout << "Camera format: NV12 (YUV)" << std::endl;
        else if (subtype == MFVideoFormat_YUY2)
            std::cout << "Camera format: YUY2 (YUV)" << std::endl;
        else if (subtype == MFVideoFormat_RGB24)
            std::cout << "Camera format: RGB24" << std::endl;
        else if (subtype == MFVideoFormat_RGB32)
            std::cout << "Camera format: RGB32" << std::endl;
        else
            std::cout << "Camera format: Unknown/Other YUV" << std::endl;
    }

    std::shared_ptr<SOGLR::Framebuffer> framebuffer = std::make_shared<SOGLR::Framebuffer>(width, height);
    std::shared_ptr<SOGLR::Shader> framebuffer_shader = std::make_shared<SOGLR::Shader>(
        "S:\\Users\\Timber\\Documents\\GitHub\\YearOfLearning\\1-WebcamRendering\\src\\framebuffer.vert",
        "S:\\Users\\Timber\\Documents\\GitHub\\YearOfLearning\\1-WebcamRendering\\src\\framebuffer.frag");

    while (renderer.IsRunning())
    {
        renderer.BeginFrame();
        Vector2Int window_size = renderer.GetWindow()->GetSize();
        framebuffer->Invalidate(window_size.x, window_size.y);
        hr = reader->ReadSample(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0, &streamIndex, &flags, &timestamp, &sample);

        if (FAILED(hr))
        {
            std::cerr << "ReadSample failed\n";
            break;
        }
        if (!sample)
            continue;

        ComPtr<IMFMediaBuffer> buffer;
        if (sample)
        {
            hr = sample->ConvertToContiguousBuffer(&buffer);
            if (SUCCEEDED(hr))
            {
                BYTE *nv12_data = nullptr;
                DWORD maxLen = 0, curLen = 0;
                hr = buffer->Lock(&nv12_data, &maxLen, &curLen);

                if (SUCCEEDED(hr))
                {

                    BYTE *rgb_data = nullptr;
                    rgb_data = new BYTE[width * height * 3]; // 3 bytes per pixel for RGB
                    convertNv12ToRgbManual(nv12_data, width, height, rgb_data);

                    framebuffer->BindTexture();

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                    // NV12 format: first width*height bytes are Y (luminance) data
                    // NV12 data: upload as single channel R8 texture with height * 1.5
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb_data);
                    delete[] rgb_data;
                    buffer->Unlock();
                }
            }
        }

        renderer.DrawFramebufferToScreen(framebuffer, framebuffer_shader);
        renderer.EndFrame();
    }

    // Convert camera output to OpenGL texture
    // Assume SOGLR is initialized and OpenGL context is active

    // Get the video frame as a buffer

    source->Shutdown();
    MFShutdown();
    return 0;
}
