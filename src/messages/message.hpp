#pragma once

#include "messages/word.hpp"
#include "widgets/helper/scrollbarhighlight.hpp"

#include <chrono>
#include <memory>
#include <vector>

namespace chatterino {

class Channel;

namespace messages {
class Message;

typedef std::shared_ptr<Message> SharedMessage;
typedef char MessageFlagsType;

class Message
{
public:
    enum MessageFlags : MessageFlagsType {
        None = 0,
        System = (1 << 1),
        Timeout = (1 << 2),
        Highlighted = (1 << 3),
    };

    bool containsHighlightedPhrase() const;
    void setHighlight(bool value);
    const QString &getTimeoutUser() const;
    int getTimeoutCount() const;
    const QString &getContent() const;
    const std::chrono::time_point<std::chrono::system_clock> &getParseTime() const;
    std::vector<Word> &getWords();
    MessageFlags getFlags() const;
    void setFlags(MessageFlags flags);
    void addFlags(MessageFlags flags);
    void removeFlags(MessageFlags flags);
    bool isDisabled() const;
    void setDisabled(bool value);
    const QString &getId() const;
    bool getCollapsedDefault() const;
    void setCollapsedDefault(bool value);
    bool getDisableCompactEmotes() const;
    void setDisableCompactEmotes(bool value);
    void updateContent() const;
    widgets::ScrollbarHighlight getScrollBarHighlight() const;

    QString loginName;
    QString displayName;
    QString localizedName;
    QString timeoutUser;

    const QString text;
    bool centered = false;

    static Message *createSystemMessage(const QString &text);

    static Message *createTimeoutMessage(const QString &username, const QString &durationInSeconds,
                                         const QString &reason, bool multipleTimes);

private:
    static LazyLoadedImage *badgeStaff;
    static LazyLoadedImage *badgeAdmin;
    static LazyLoadedImage *badgeGlobalmod;
    static LazyLoadedImage *badgeModerator;
    static LazyLoadedImage *badgeTurbo;
    static LazyLoadedImage *badgeBroadcaster;
    static LazyLoadedImage *badgePremium;

    static QRegularExpression *cheerRegex;

    MessageFlags flags = MessageFlags::None;

    // what is highlightTab?
    bool highlightTab = false;

    int timeoutCount = 0;
    bool disabled = false;

    bool collapsedDefault = false;
    bool disableCompactEmotes = false;

    std::chrono::time_point<std::chrono::system_clock> parseTime;

    mutable QString content;
    QString id = "";

    std::vector<Word> words;
};

}  // namespace messages
}  // namespace chatterino
