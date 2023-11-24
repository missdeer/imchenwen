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

#ifndef MPVRENDERER_H_
#define MPVRENDERER_H_

#include <Danmaku2ASS/AssBuilder.h>

#include <QObject>
#include <QQuickWindow>
#include <QtQml/qqmlregistration.h>
#include <QtQuick/QQuickFramebufferObject>

#include "mpv.hpp"

class MpvRenderer;

class MpvObject : public QQuickFramebufferObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qint64 time READ time NOTIFY timeChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(QSize videoSize READ videoSize NOTIFY videoSizeChanged)
    Q_PROPERTY(bool subVisible READ subVisible WRITE setSubVisible NOTIFY subVisibleChanged)
    Q_PROPERTY(QStringList audioTracks READ audioTracks NOTIFY audioTracksChanged)
    Q_PROPERTY(QStringList subtitles READ subtitles NOTIFY subtitlesChanged)

    friend class MpvRenderer;

public:
    enum State
    {
        STOPPED,
        VIDEO_PLAYING,
        VIDEO_PAUSED,
        TV_PLAYING
    };
    enum Hwdec
    {
        AUTO,
        VAAPI,
        VDPAU,
        NVDEC
    };
    Q_ENUM(State)

    static MpvObject *instance()
    {
        return s_instance;
    }

    explicit MpvObject(QQuickItem *parent = nullptr);
    [[nodiscard]] Renderer *createRenderer() const override;

    // Access properties
    [[nodiscard]] QSize videoSize() const
    {
        return QSize(m_videoWidth, m_videoHeight) / window()->effectiveDevicePixelRatio();
    }
    [[nodiscard]] State state() const
    {
        return m_state;
    }
    [[nodiscard]] qint64 duration() const
    {
        return m_duration;
    }
    [[nodiscard]] qint64 time() const
    {
        return m_time;
    }
    [[nodiscard]] bool subVisible() const
    {
        return m_subVisible;
    }
    [[nodiscard]] int volume() const
    {
        return m_volume;
    }
    [[nodiscard]] QStringList audioTracks() const
    {
        return m_audioTracks;
    }
    [[nodiscard]] QStringList subtitles() const
    {
        return m_subtitles;
    }

    void setVolume(int volume);
    void setSubVisible(bool subVisible);

public slots:
    void open(const QUrl &fileUrl, const QUrl &danmakuUrl = QUrl(), const QUrl &audioTrackUrl = QUrl());
    void play();
    void pause();
    void stop();
    void seek(qint64 offset, bool absolute = true);
    void screenshot();
    void addAudioTrack(const QUrl &url);
    void addDanmaku(const Danmaku2ASS::AssBuilder::Ptr &danmakuAss);
    void addSubtitle(const QUrl &url);
    void reloadDanmaku(bool top, bool bottom, bool scrolling, double reservedArea, const QStringList &blockWords);
    void setProperty(const QString &name, const QVariant &value);
    void showText(const QByteArray &text);

signals:
    void audioTracksChanged();
    void stateChanged();
    void subtitlesChanged();
    void subVisibleChanged();
    void durationChanged();
    void timeChanged();
    void volumeChanged();
    void videoSizeChanged();

private:
    static void      on_update(void *ctx);
    Q_INVOKABLE void onMpvEvent();
    void             handleMpvError(int code);

    Mpv::Handle m_mpv;

    State                        m_state         = STOPPED;
    mpv_end_file_reason          m_endFileReason = MPV_END_FILE_REASON_STOP;
    bool                         m_subVisible    = true;
    int                          m_volume;
    int                          m_danmakuDisallowMode = 0;
    int64_t                      m_time;
    int64_t                      m_duration;
    int64_t                      m_videoWidth   = 0;
    int64_t                      m_videoHeight  = 0;
    double                       m_reservedArea = 0;
    QUrl                         m_danmakuUrl;
    QUrl                         m_audioToBeAdded;
    QStringList                  m_audioTracks;
    QStringList                  m_subtitles;
    Danmaku2ASS::AssBuilder::Ptr m_danmakuAss;
    std::vector<std::string>     m_blockWords;

    static MpvObject *s_instance;
};

#endif
