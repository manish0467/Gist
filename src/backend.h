#pragma once

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

struct EmailItem {
    QString id;
    QString senderName;
    QString senderEmail;
    QString subject;
    QString snippet;
    QString body;
    QString date;
    QString folder; // "inbox", "starred", "sent", "archive", "trash"
    bool isUnread = true;
    bool isStarred = false;
    QString summary;
};

class Backend : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString activeFolder READ activeFolder NOTIFY activeFolderChanged)
    Q_PROPERTY(int unreadCount READ unreadCount NOTIFY threadsChanged)
    Q_PROPERTY(int starredCount READ starredCount NOTIFY threadsChanged)
    Q_PROPERTY(QVariantList accounts READ accounts NOTIFY accountsChanged)
    Q_PROPERTY(QVariantList threads READ threads NOTIFY threadsChanged)
    Q_PROPERTY(int selectedThreadIndex READ selectedThreadIndex NOTIFY selectedThreadIndexChanged)
    Q_PROPERTY(QVariantMap currentThread READ currentThread NOTIFY currentThreadChanged)
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)
    Q_PROPERTY(bool isAiBusy READ isAiBusy NOTIFY isAiBusyChanged)
    Q_PROPERTY(QString aiSummary READ aiSummary NOTIFY aiSummaryChanged)
    Q_PROPERTY(QString aiDraft READ aiDraft NOTIFY aiDraftChanged)
    Q_PROPERTY(bool aiDrawerOpen READ aiDrawerOpen WRITE setAiDrawerOpen NOTIFY aiDrawerOpenChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit Backend(QObject *parent = nullptr);

    QString activeFolder() const { return m_activeFolder; }
    int unreadCount() const;
    int starredCount() const;
    QVariantList accounts() const { return m_accounts; }
    QVariantList threads() const;
    int selectedThreadIndex() const { return m_selectedThreadIndex; }
    QVariantMap currentThread() const;
    QString searchQuery() const { return m_searchQuery; }
    void setSearchQuery(const QString &query);
    bool isAiBusy() const { return m_isAiBusy; }
    QString aiSummary() const { return m_aiSummary; }
    QString aiDraft() const { return m_aiDraft; }
    bool aiDrawerOpen() const { return m_aiDrawerOpen; }
    void setAiDrawerOpen(bool open);
    QString statusMessage() const { return m_statusMessage; }

    Q_INVOKABLE void selectFolder(const QString &folder);
    Q_INVOKABLE void selectThread(int index);
    Q_INVOKABLE void nextThread();
    Q_INVOKABLE void previousThread();
    Q_INVOKABLE void toggleStarCurrent();
    Q_INVOKABLE void toggleReadCurrent();
    Q_INVOKABLE void archiveCurrent();
    Q_INVOKABLE void deleteCurrent();
    Q_INVOKABLE void primaryAction(); // Trigger AI Gist summary
    Q_INVOKABLE void generateReply(const QString &tone);
    Q_INVOKABLE void askAi(const QString &prompt);
    Q_INVOKABLE void refresh();

signals:
    void activeFolderChanged();
    void accountsChanged();
    void threadsChanged();
    void selectedThreadIndexChanged();
    void currentThreadChanged();
    void searchQueryChanged();
    void isAiBusyChanged();
    void aiSummaryChanged();
    void aiDraftChanged();
    void aiDrawerOpenChanged();
    void statusMessageChanged();

private:
    void initSampleData();
    QList<EmailItem*> filteredItems() const;
    void updateCurrentThread();

    QString m_activeFolder = "inbox";
    QString m_searchQuery;
    int m_selectedThreadIndex = 0;
    bool m_isAiBusy = false;
    QString m_aiSummary;
    QString m_aiDraft;
    bool m_aiDrawerOpen = false;
    QString m_statusMessage;

    QVariantList m_accounts;
    QList<EmailItem> m_emails;
};
