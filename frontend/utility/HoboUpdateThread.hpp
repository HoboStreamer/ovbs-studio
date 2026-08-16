#pragma once

#include <QThread>
#include <QUrl>

class HoboUpdateThread : public QThread {
	Q_OBJECT

	bool manualUpdate = false;

	void run() override;
	void info(const QString &title, const QString &text);
	bool queryUpdate(const QString &currentVersion, const QString &latestVersion);
	void openUrl(const QUrl &url);

private slots:
	void infoSlot(const QString &title, const QString &text);
	bool queryUpdateSlot(const QString &currentVersion, const QString &latestVersion);
	void openUrlSlot(const QUrl &url);

public:
	explicit HoboUpdateThread(bool manualUpdate_) : manualUpdate(manualUpdate_) {}
};
