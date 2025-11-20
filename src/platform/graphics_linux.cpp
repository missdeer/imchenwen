/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <mpv/client.h>

#include <QGuiApplication>
#include <QSettings>

#include "graphics.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#    include <QtCore/qnativeinterface.h>
#else
#    include <qpa/qplatformnativeinterface.h>
#endif
#include "../mpvObject.h"

// Attempt to reuse mpv's code for detecting whether we want GLX or EGL (which
// is tricky to do because of hardware decoding concerns). This is not pretty,
// but quite effective and without having to duplicate too much GLX/EGL code.
static std::string probeHwdecInterop()
{
    std::string result;
    Mpv::Handle mpv;

    mpv.set_option("gpu-hwdec-interop", "auto");

    // Actually creating a window is required. There is currently no way to keep
    // this window hidden or invisible.
    mpv.set_option("force-window", true);

    // As a mitigation, put the window in the top/right corner, and make it as
    // small as possible by forcing 1x1 size and removing window borders.
    mpv.set_option("geometry", "1x1+0+0");
    mpv.set_option("border", false);
    if (mpv.initialize() < 0)
    {
        return {};
    }

    result = mpv.get_property_string("hwdec-interop");
    if (!result.empty())
    {
        qInfo("Detected OpenGL backend: %s\n", result.c_str());
    }
    return result;
}

void Graphics::detectOpenGLEarly()
{
    MpvObject::Hwdec hwdec = static_cast<MpvObject::Hwdec>(QSettings().value(QStringLiteral("video/hwdec")).toInt());
    if (hwdec == MpvObject::VAAPI)
    {
        qputenv("QT_XCB_GL_INTEGRATION", QByteArrayLiteral("xcb_egl"));
    }
    else if (hwdec == MpvObject::AUTO)
    {
        if (probeHwdecInterop() == "vaapi-egl")
        {
            qputenv("QT_XCB_GL_INTEGRATION", QByteArrayLiteral("xcb_egl"));
        }
    }
}

void Graphics::detectOpenGLLate() {}

void *Graphics::x11Display()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (QGuiApplication::platformName() == QLatin1String("xcb")) // X11 平台
    {
        if (auto x11App = qGuiApp->nativeInterface<QNativeInterface::QX11Application>())
        {
            return static_cast<void *>(x11App->display());
        }
    }
#else
    if (QGuiApplication::platformName() == QLatin1String("xcb")) // is x11
    {
        Q_ASSERT(QGuiApplication::platformNativeInterface() != nullptr);
        return QGuiApplication::platformNativeInterface()->nativeResourceForWindow(QByteArrayLiteral("display"), nullptr);
    }
#endif
    return nullptr;
}

void *Graphics::waylandDisplay()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (QGuiApplication::platformName() != QLatin1String("xcb")) // is not x11
    {
        if (auto wlApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>())
        {
            return static_cast<void *>(wlApp->display());
        }
    }
#else
    if (QGuiApplication::platformName() != QLatin1String("xcb")) // is not x11
    {
        Q_ASSERT(QGuiApplication::platformNativeInterface() != nullptr);
        return QGuiApplication::platformNativeInterface()->nativeResourceForWindow(QByteArrayLiteral("display"), nullptr);
    }
#endif
    return nullptr;
}
