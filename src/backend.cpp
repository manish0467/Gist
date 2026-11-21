#include "backend.h"

#include <QDateTime>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QStandardPaths>
#include <QTimer>

static Backend *s_backendInstance = nullptr;

static void coreEventCallback(const char *event_json) {
    if (s_backendInstance && event_json) {
        const QString jsonStr = QString::fromUtf8(event_json);
        QMetaObject::invokeMethod(s_backendInstance, [jsonStr]() {
            s_backendInstance->onCoreEvent(jsonStr);
        }, Qt::QueuedConnection);
    }
}

Backend::Backend(QObject *parent) : QObject(parent) {
    s_backendInstance = this;

    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    const QString dbPath = dataDir + "/gist.db";

    m_coreEngine = gist_core_new(dbPath.toUtf8().constData());
    if (m_coreEngine) {
        gist_core_set_callback(m_coreEngine, coreEventCallback);
    }

    m_accounts = {
        QVariantMap{{"id", "personal"}, {"name", "Personal"}, {"email", "user@omarchy.org"}, {"color", "#FFD60A"}},
        QVariantMap{{"id", "work"}, {"name", "Work"}, {"email", "alen@work.io"}, {"color", "#64D2FF"}}
    };

    initSampleData();
    loadFromCore();
    updateCurrentThread();
}

Backend::~Backend() {
    if (m_coreEngine) {
        gist_core_free(m_coreEngine);
        m_coreEngine = nullptr;
    }
    if (s_backendInstance == this) {
        s_backendInstance = nullptr;
    }
}

void Backend::addAccount(const QString &id, const QString &name, const QString &email,
                         const QString &imapHost, int imapPort,
                         const QString &username, const QString &password) {
    if (!m_coreEngine) return;

    gist_core_add_account(
        m_coreEngine,
        id.toUtf8().constData(),
        name.toUtf8().constData(),
        email.toUtf8().constData(),
        imapHost.toUtf8().constData(),
        static_cast<uint16_t>(imapPort),
        username.toUtf8().constData(),
        password.toUtf8().constData()
    );

    QVariantMap acc;
    acc["id"] = id;
    acc["name"] = name;
    acc["email"] = email;
    acc["color"] = "#34C759";
    m_accounts.append(acc);
    emit accountsChanged();

    m_statusMessage = QString("Account %1 added. Starting sync...").arg(email);
    emit statusMessageChanged();
    syncAccount(id);
}

void Backend::syncAccount(const QString &accountId) {
    if (!m_coreEngine) return;
    m_isSyncing = true;
    emit isSyncingChanged();

    m_statusMessage = "Syncing mail with IMAP server...";
    emit statusMessageChanged();

    gist_core_sync_account(m_coreEngine, accountId.toUtf8().constData());
}

void Backend::onCoreEvent(const QString &eventJson) {
    const auto doc = QJsonDocument::fromJson(eventJson.toUtf8());
    const auto obj = doc.object();
    const QString event = obj.value("event").toString();

    m_isSyncing = false;
    emit isSyncingChanged();

    if (event == "sync_finished") {
        const int synced = obj.value("synced").toInt();
        m_statusMessage = QString("Sync complete. %1 new messages.").arg(synced);
        emit statusMessageChanged();
        loadFromCore();
    } else if (event == "sync_error") {
        m_statusMessage = QString("Sync failed: %1").arg(obj.value("error").toString());
        emit statusMessageChanged();
    }
}

void Backend::loadFromCore() {
    if (!m_coreEngine) return;

    char *jsonStr = gist_core_get_messages_json(m_coreEngine, m_activeFolder.toUtf8().constData());
    if (jsonStr) {
        const auto doc = QJsonDocument::fromJson(QByteArray(jsonStr));
        gist_core_free_string(jsonStr);

        if (doc.isArray() && !doc.array().isEmpty()) {
            m_emails.clear();
            const auto arr = doc.array();
            for (const auto &v : arr) {
                const auto o = v.toObject();
                EmailItem item;
                item.id = o.value("id").toString();
                item.senderName = o.value("sender_name").toString();
                item.senderEmail = o.value("sender_email").toString();
                item.subject = o.value("subject").toString();
                item.snippet = o.value("snippet").toString();
                item.body = o.value("body").toString();
                item.date = o.value("date").toString();
                item.folder = o.value("folder").toString();
                item.isUnread = o.value("is_unread").toBool();
                item.isStarred = o.value("is_starred").toBool();
                item.summary = o.value("summary").toString();
                m_emails.append(item);
            }
            emit threadsChanged();
            updateCurrentThread();
        }
    }
}

void Backend::initSampleData() {
    m_emails = {
        EmailItem{
            "1",
            "GitHub Releases",
            "notifications@github.com",
            "Omarchy 2.1 has been released!",
            "Hey everyone! Omarchy 2.1 is officially out with major performance improvements...",
            "Hey everyone!\n\nOmarchy 2.1 is officially out! Here is what is new:\n\n"
            "• Wayland compositing latency reduced by 35%\n"
            "• Seamless live theme syncing with custom accent palette support\n"
            "• Upgraded Hyprland plugin manager & instant scratchpads\n"
            "• Built-in AI assistant integration layer\n\n"
            "Check out full release notes at https://github.com/omacom/omarchy/releases/tag/v2.1\n\n"
            "Thanks to all 48 contributors who submitted patches for this milestone!",
            "10:42 AM",
            "inbox",
            true,
            true,
            "• Omarchy 2.1 released with 35% reduced Wayland latency.\n• Live theme syncing & Hyprland plugin updates.\n• 48 contributors acknowledged in the changelog."
        },
        EmailItem{
            "2",
            "Elena Rostova",
            "elena.r@designcraft.studio",
            "Feedback on Feather / Gist UX Prototype",
            "I reviewed the new 3-pane email design for Omarchy. The contrast ratios look great...",
            "Hi Alen,\n\nI reviewed the new 3-pane email design for Omarchy. The contrast ratios look great on OLED and dark displays.\n\n"
            "A few observations:\n"
            "1. The AI drawer sliding from the right feels super natural.\n"
            "2. Make sure keyboard shortcuts like 'j' and 'k' allow seamless thread jumping.\n"
            "3. The accent color border around unread badges really pops.\n\n"
            "Let me know if you want to hop on a call this afternoon to review the interaction specs.\n\nBest,\nElena",
            "09:15 AM",
            "inbox",
            true,
            false,
            "• Elena reviewed the 3-pane UX prototype and praised OLED contrast.\n• Recommended testing j/k navigation and accent styling.\n• Open to a design call this afternoon."
        },
        EmailItem{
            "3",
            "Infrastructure Alert",
            "alerts@pagerduty.internal",
            "[Resolved] Incident #4928: High memory watermark on build-node-04",
            "The cluster auto-scaler successfully migrated non-critical workloads...",
            "INCIDENT REPORT #4928\nStatus: RESOLVED\nSeverity: Medium\n\n"
            "Summary:\nAt 04:12 UTC, build-node-04 reached 92% RAM usage during continuous integration runs.\n"
            "Action taken: Daemon evicted dormant cgroups and scheduled overflow jobs to build-node-05.\n\n"
            "No customer jobs were disrupted.",
            "Yesterday",
            "inbox",
            false,
            false,
            "• Incident #4928 on build-node-04 resolved automatically.\n• Workloads rescheduled to build-node-05; zero disruption."
        },
        EmailItem{
            "4",
            "Marcus Vance",
            "marcus@archlinux.org",
            "Gist PKGBUILD review & repository submission",
            "The package looks clean. Let's make sure we bundle the SVG icon in hicolor...",
            "Hi team,\n\nI reviewed the initial PKGBUILD for Gist. Everything conforms nicely to the Arch packaging guidelines.\n"
            "Just double-check that the scalable icon goes into /usr/share/icons/hicolor/scalable/apps/gist.svg so launchers like Rofi / Omarchy menu pick it up instantly.\n\n"
            "Cheers,\nMarcus",
            "Nov 19",
            "inbox",
            false,
            true,
            "• Marcus reviewed Gist PKGBUILD and approved standard Arch conformance.\n• Reminder to verify hicolor scalable SVG path."
        },
        EmailItem{
            "5",
            "Stripe Billing",
            "invoices@stripe.com",
            "Your receipt for Server Hosting - Nov 2026",
            "Thank you for your payment of $24.00 USD. Your invoice is attached...",
            "Hi Alen,\n\nThis confirms your payment of $24.00 for cloud instance hosting.\nPayment Method: •••• 4242\nDate: Nov 18, 2026\n\nDownload your full PDF receipt from your account billing portal.",
            "Nov 18",
            "archive",
            false,
            false,
            "• Receipt for $24.00 server hosting paid via card ending 4242."
        }
    };
}

QList<EmailItem*> Backend::filteredItems() const {
    QList<EmailItem*> list;
    for (int i = 0; i < m_emails.size(); ++i) {
        EmailItem *item = const_cast<EmailItem*>(&m_emails[i]);
        if (m_activeFolder == "starred") {
            if (!item->isStarred) continue;
        } else if (m_activeFolder != "all" && item->folder != m_activeFolder) {
            continue;
        }
        if (!m_searchQuery.isEmpty()) {
            const QString q = m_searchQuery.toLower();
            if (!item->subject.toLower().contains(q) &&
                !item->senderName.toLower().contains(q) &&
                !item->snippet.toLower().contains(q) &&
                !item->body.toLower().contains(q)) {
                continue;
            }
        }
        list.append(item);
    }
    return list;
}

QVariantList Backend::threads() const {
    QVariantList list;
    const auto items = filteredItems();
    for (int i = 0; i < items.size(); ++i) {
        const EmailItem *item = items[i];
        QVariantMap map;
        map["id"] = item->id;
        map["senderName"] = item->senderName;
        map["senderEmail"] = item->senderEmail;
        map["subject"] = item->subject;
        map["snippet"] = item->snippet;
        map["body"] = item->body;
        map["date"] = item->date;
        map["folder"] = item->folder;
        map["isUnread"] = item->isUnread;
        map["isStarred"] = item->isStarred;
        map["summary"] = item->summary;
        list.append(map);
    }
    return list;
}

int Backend::unreadCount() const {
    int count = 0;
    for (const auto &item : m_emails) {
        if (item.folder == "inbox" && item.isUnread) count++;
    }
    return count;
}

int Backend::starredCount() const {
    int count = 0;
    for (const auto &item : m_emails) {
        if (item.isStarred) count++;
    }
    return count;
}

QVariantMap Backend::currentThread() const {
    const auto items = filteredItems();
    if (m_selectedThreadIndex >= 0 && m_selectedThreadIndex < items.size()) {
        const EmailItem *item = items[m_selectedThreadIndex];
        QVariantMap map;
        map["id"] = item->id;
        map["senderName"] = item->senderName;
        map["senderEmail"] = item->senderEmail;
        map["subject"] = item->subject;
        map["snippet"] = item->snippet;
        map["body"] = item->body;
        map["date"] = item->date;
        map["folder"] = item->folder;
        map["isUnread"] = item->isUnread;
        map["isStarred"] = item->isStarred;
        map["summary"] = item->summary;
        return map;
    }
    return {};
}

void Backend::setSearchQuery(const QString &query) {
    if (m_searchQuery == query) return;
    m_searchQuery = query;
    m_selectedThreadIndex = 0;
    emit searchQueryChanged();
    emit threadsChanged();
    updateCurrentThread();
}

void Backend::setAiDrawerOpen(bool open) {
    if (m_aiDrawerOpen == open) return;
    m_aiDrawerOpen = open;
    emit aiDrawerOpenChanged();
}

void Backend::selectFolder(const QString &folder) {
    if (m_activeFolder == folder) return;
    m_activeFolder = folder;
    m_selectedThreadIndex = 0;
    emit activeFolderChanged();
    loadFromCore();
    emit threadsChanged();
    updateCurrentThread();
}

void Backend::selectThread(int index) {
    const auto items = filteredItems();
    if (index >= 0 && index < items.size()) {
        m_selectedThreadIndex = index;
        items[index]->isUnread = false;
        emit selectedThreadIndexChanged();
        emit threadsChanged();
        updateCurrentThread();
    }
}

void Backend::nextThread() {
    const auto items = filteredItems();
    if (m_selectedThreadIndex + 1 < items.size()) {
        selectThread(m_selectedThreadIndex + 1);
    }
}

void Backend::previousThread() {
    if (m_selectedThreadIndex - 1 >= 0) {
        selectThread(m_selectedThreadIndex - 1);
    }
}

void Backend::toggleStarCurrent() {
    const auto items = filteredItems();
    if (m_selectedThreadIndex >= 0 && m_selectedThreadIndex < items.size()) {
        items[m_selectedThreadIndex]->isStarred = !items[m_selectedThreadIndex]->isStarred;
        emit threadsChanged();
        updateCurrentThread();
    }
}

void Backend::toggleReadCurrent() {
    const auto items = filteredItems();
    if (m_selectedThreadIndex >= 0 && m_selectedThreadIndex < items.size()) {
        items[m_selectedThreadIndex]->isUnread = !items[m_selectedThreadIndex]->isUnread;
        emit threadsChanged();
        updateCurrentThread();
    }
}

void Backend::archiveCurrent() {
    const auto items = filteredItems();
    if (m_selectedThreadIndex >= 0 && m_selectedThreadIndex < items.size()) {
        items[m_selectedThreadIndex]->folder = "archive";
        m_statusMessage = "Thread moved to Archive";
        emit statusMessageChanged();
        emit threadsChanged();
        if (m_selectedThreadIndex >= filteredItems().size())
            m_selectedThreadIndex = qMax(0, filteredItems().size() - 1);
        emit selectedThreadIndexChanged();
        updateCurrentThread();
    }
}

void Backend::deleteCurrent() {
    const auto items = filteredItems();
    if (m_selectedThreadIndex >= 0 && m_selectedThreadIndex < items.size()) {
        items[m_selectedThreadIndex]->folder = "trash";
        m_statusMessage = "Thread moved to Trash";
        emit statusMessageChanged();
        emit threadsChanged();
        if (m_selectedThreadIndex >= filteredItems().size())
            m_selectedThreadIndex = qMax(0, filteredItems().size() - 1);
        emit selectedThreadIndexChanged();
        updateCurrentThread();
    }
}

void Backend::primaryAction() {
    const auto items = filteredItems();
    if (items.isEmpty() || m_selectedThreadIndex < 0 || m_selectedThreadIndex >= items.size())
        return;

    m_isAiBusy = true;
    m_aiDrawerOpen = true;
    emit isAiBusyChanged();
    emit aiDrawerOpenChanged();

    QTimer::singleShot(250, this, [this]() {
        const auto items = filteredItems();
        if (m_selectedThreadIndex >= 0 && m_selectedThreadIndex < items.size()) {
            const EmailItem *item = items[m_selectedThreadIndex];
            m_aiSummary = item->summary.isEmpty() ?
                ("• Key point: " + item->subject + "\n• Sender: " + item->senderName + "\n• Quick action needed: Review thread content.") :
                item->summary;
            m_aiDraft.clear();
        }
        m_isAiBusy = false;
        emit isAiBusyChanged();
        emit aiSummaryChanged();
        emit aiDraftChanged();
    });
}

void Backend::generateReply(const QString &tone) {
    const auto items = filteredItems();
    if (items.isEmpty() || m_selectedThreadIndex < 0 || m_selectedThreadIndex >= items.size())
        return;

    m_isAiBusy = true;
    m_aiDrawerOpen = true;
    emit isAiBusyChanged();
    emit aiDrawerOpenChanged();

    QTimer::singleShot(300, this, [this, tone]() {
        const auto items = filteredItems();
        if (m_selectedThreadIndex >= 0 && m_selectedThreadIndex < items.size()) {
            const EmailItem *item = items[m_selectedThreadIndex];
            if (tone == "casual") {
                m_aiDraft = QString("Hey %1,\n\nThanks for sending this over! Looks great to me, let's proceed.\n\nCheers,\nAlen")
                                .arg(item->senderName.section(' ', 0, 0));
            } else if (tone == "brief") {
                m_aiDraft = "Acknowledged. Thanks for the update!\n\nBest,\nAlen";
            } else {
                m_aiDraft = QString("Hi %1,\n\nThank you for the update regarding \"%2\". I have reviewed the details and will follow up with any required actions shortly.\n\nSincerely,\nAlen")
                                .arg(item->senderName.section(' ', 0, 0), item->subject);
            }
        }
        m_isAiBusy = false;
        emit isAiBusyChanged();
        emit aiDraftChanged();
    });
}

void Backend::askAi(const QString &prompt) {
    if (prompt.trimmed().isEmpty()) return;
    m_isAiBusy = true;
    m_aiDrawerOpen = true;
    emit isAiBusyChanged();
    emit aiDrawerOpenChanged();

    QTimer::singleShot(350, this, [this, prompt]() {
        m_aiSummary = QString("Q: \"%1\"\n\nBased on your email thread:\nAll related information confirms the task is on track. No pending blockers found.")
                          .arg(prompt);
        m_isAiBusy = false;
        emit isAiBusyChanged();
        emit aiSummaryChanged();
    });
}

void Backend::refresh() {
    m_statusMessage = "Syncing mail with IMAP engine...";
    emit statusMessageChanged();
    if (!m_accounts.isEmpty()) {
        const QString firstAccId = m_accounts.first().toMap().value("id").toString();
        syncAccount(firstAccId);
    }
}

void Backend::updateCurrentThread() {
    const auto items = filteredItems();
    if (m_selectedThreadIndex >= 0 && m_selectedThreadIndex < items.size()) {
        m_aiSummary = items[m_selectedThreadIndex]->summary;
    } else {
        m_aiSummary.clear();
    }
    m_aiDraft.clear();
    emit currentThreadChanged();
    emit aiSummaryChanged();
    emit aiDraftChanged();
}
