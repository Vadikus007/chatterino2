#include "messages/lazyloadedimage.hpp"
#include "asyncexec.hpp"
#include "singletons/emotemanager.hpp"
#include "singletons/ircmanager.hpp"
#include "singletons/windowmanager.hpp"
#include "util/networkmanager.hpp"
#include "util/urlfetch.hpp"

#include <QBuffer>
#include <QImageReader>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

#include <functional>
#include <thread>

namespace chatterino {
namespace messages {

LazyLoadedImage::LazyLoadedImage(const QString &url, qreal scale, const QString &name,
                                 const QString &tooltip, const QMargins &margin, bool isHat)
    : currentPixmap(nullptr)
    , url(url)
    , name(name)
    , tooltip(tooltip)
    , margin(margin)
    , ishat(isHat)
    , scale(scale)
    , isLoading(false)
{
}

LazyLoadedImage::LazyLoadedImage(QPixmap *image, qreal scale, const QString &name,
                                 const QString &tooltip, const QMargins &margin, bool isHat)
    : currentPixmap(image)
    , name(name)
    , tooltip(tooltip)
    , margin(margin)
    , ishat(isHat)
    , scale(scale)
    , isLoading(true)
{
}

void LazyLoadedImage::loadImage()
{
    util::NetworkRequest req(this->getUrl());
    req.setCaller(this);
    req.get([lli = this](QNetworkReply * reply) {
        QByteArray array = reply->readAll();
        QBuffer buffer(&array);
        buffer.open(QIODevice::ReadOnly);

        QImage image;
        QImageReader reader(&buffer);

        bool first = true;

        for (int index = 0; index < reader.imageCount(); ++index) {
            if (reader.read(&image)) {
                auto pixmap = new QPixmap(QPixmap::fromImage(image));

                if (first) {
                    first = false;
                    lli->currentPixmap = pixmap;
                }

                chatterino::messages::LazyLoadedImage::FrameData data;
                data.duration = std::max(20, reader.nextImageDelay());
                data.image = pixmap;

                lli->allFrames.push_back(data);
            }
        }

        if (lli->allFrames.size() > 1) {
            lli->animated = true;
        }

        singletons::EmoteManager::getInstance().incGeneration();

        singletons::WindowManager::getInstance().layoutVisibleChatWidgets();
    });

    singletons::EmoteManager::getInstance().getGifUpdateSignal().connect([=]() {
        this->gifUpdateTimout();
    });  // For some reason when Boost signal is in thread scope and thread deletes the signal
         // doesn't work, so this is the fix.
}

void LazyLoadedImage::gifUpdateTimout()
{
    if (animated) {
        this->currentFrameOffset += GIF_FRAME_LENGTH;

        while (true) {
            if (this->currentFrameOffset > this->allFrames.at(this->currentFrame).duration) {
                this->currentFrameOffset -= this->allFrames.at(this->currentFrame).duration;
                this->currentFrame = (this->currentFrame + 1) % this->allFrames.size();
            } else {
                break;
            }
        }

        this->currentPixmap = this->allFrames[this->currentFrame].image;
    }
}

const QPixmap *LazyLoadedImage::getPixmap()
{
    if (!this->isLoading) {
        this->isLoading = true;

        loadImage();
    }
    return this->currentPixmap;
}

qreal LazyLoadedImage::getScale() const
{
    return this->scale;
}

const QString &LazyLoadedImage::getUrl() const
{
    return this->url;
}

const QString &LazyLoadedImage::getName() const
{
    return this->name;
}

const QString &LazyLoadedImage::getTooltip() const
{
    return this->tooltip;
}

const QMargins &LazyLoadedImage::getMargin() const
{
    return this->margin;
}

bool LazyLoadedImage::getAnimated() const
{
    return this->animated;
}

bool LazyLoadedImage::isHat() const
{
    return this->ishat;
}

int LazyLoadedImage::getWidth() const
{
    if (this->currentPixmap == nullptr) {
        return 16;
    }
    return this->currentPixmap->width();
}

int LazyLoadedImage::getScaledWidth() const
{
    return static_cast<int>(getWidth() * this->scale);
}

int LazyLoadedImage::getHeight() const
{
    if (this->currentPixmap == nullptr) {
        return 16;
    }
    return this->currentPixmap->height();
}

int LazyLoadedImage::getScaledHeight() const
{
    return static_cast<int>(getHeight() * this->scale);
}

}  // namespace messages
}  // namespace chatterino
