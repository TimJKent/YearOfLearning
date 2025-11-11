#define NOMINMAX

#include <iostream>
#include <SOGLR/SOGLR.hpp>
#include <SOGLR/etc/ImageConversions.hpp>
#include <CPCam/CPCam.hpp>
#include <iostream>
#include <algorithm>

SOGLR::Renderer renderer;

int width, height;

int main()
{
    std::unique_ptr<CPCam::Camera> camera = CPCam::Camera::Create();

    width = 1920;
    height = 1080;
    renderer.GetWindow()->SetTitle("Webcam Rendering");
    renderer.GetWindow()->Resize(width, height);

    std::shared_ptr<SOGLR::Framebuffer> framebuffer = std::make_shared<SOGLR::Framebuffer>(width, height);
    std::shared_ptr<SOGLR::Shader> framebuffer_shader = std::make_shared<SOGLR::Shader>(
        "S:\\Users\\Timber\\Documents\\GitHub\\YearOfLearning\\1-WebcamRendering\\src\\framebuffer.vert",
        "S:\\Users\\Timber\\Documents\\GitHub\\YearOfLearning\\1-WebcamRendering\\src\\framebuffer.frag");

    while (renderer.IsRunning())
    {
        renderer.BeginFrame();
        Vector2Int window_size = renderer.GetWindow()->GetSize();

        uint8_t *rgb_data = nullptr;
        uint8_t *nv12_data = camera->CaptureFrame();
        if (nv12_data != nullptr)
        {

            rgb_data = new uint8_t[width * height * 3]; // 3 bytes per pixel for RGB
            SOGLR::ConvertNv12ToRgb(nv12_data, width, height, rgb_data);

            framebuffer->BindTexture();

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // NV12 format: first width*height bytes are Y (luminance) data
            // NV12 data: upload as single channel R8 texture with height * 1.5
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, camera->GetWidth(), camera->GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, rgb_data);
            delete[] rgb_data;
        }

        renderer.DrawFramebufferToScreen(framebuffer, framebuffer_shader);
        renderer.EndFrame();
    }

    // Convert camera output to OpenGL texture
    // Assume SOGLR is initialized and OpenGL context is active

    // Get the video frame as a buffer

    return 0;
}
