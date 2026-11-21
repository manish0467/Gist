#include <QtTest>
#include "backend.h"
#include "theme.h"
#include "translator.h"

class GistTests : public QObject {
    Q_OBJECT

private slots:
    void testInitialDataLoaded() {
        Backend backend;
        QVERIFY(!backend.threads().isEmpty());
        QCOMPARE(backend.activeFolder(), QString("inbox"));
    }

    void testThreadSelection() {
        Backend backend;
        backend.selectThread(1);
        QCOMPARE(backend.selectedThreadIndex(), 1);
        QCOMPARE(backend.currentThread().value("id").toString(), QString("2"));
    }

    void testFolderFiltering() {
        Backend backend;
        backend.selectFolder("starred");
        QCOMPARE(backend.activeFolder(), QString("starred"));
    }

    void testThemeDefaultAccent() {
        Theme theme;
        QVERIFY(!theme.accent().isEmpty());
    }
};

QTEST_MAIN(GistTests)
#include "gist_tests.moc"
