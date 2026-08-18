#include "OpenVibeUpdateThread.hpp"

#include <OBSApp.hpp>
#include <qt-wrappers.hpp>
#include <util/curl/curl-helper.h>

#include <nlohmann/json.hpp>

#include <QDesktopServices>
#include <QMessageBox>
#include <QPushButton>
#include <QUrl>

#include <algorithm>
#include <cctype>
#include <ctime>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "moc_OpenVibeUpdateThread.cpp"

namespace {
constexpr const char *UPDATE_MANIFEST_URL =
	"https://github.com/OpenVibe/ovbs-studio/releases/latest/download/OpenVibe-update.json";
constexpr const char *EXPECTED_PRODUCT = "OpenVibe Studio";
constexpr int EXPECTED_SCHEMA_VERSION = 1;

size_t CurlWrite(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	auto *output = static_cast<std::string *>(userdata);
	const size_t total = size * nmemb;
	output->append(ptr, total);
	return total;
}

std::string FetchUpdateManifest()
{
	CURL *curl = curl_easy_init();
	if (!curl) {
		throw std::runtime_error("Could not initialize HTTP client");
	}

	std::string output;
	char errorBuffer[CURL_ERROR_SIZE] = {};

	curl_easy_setopt(curl, CURLOPT_URL, UPDATE_MANIFEST_URL);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
	curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "OpenVibe-Studio-Updater/1");
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);
	curl_obs_set_revoke_setting(curl);

	const CURLcode result = curl_easy_perform(curl);
	long responseCode = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
	curl_easy_cleanup(curl);

	if (result != CURLE_OK) {
		const std::string detail = errorBuffer[0] ? errorBuffer : curl_easy_strerror(result);
		throw std::runtime_error("Unable to fetch OpenVibe update manifest: " + detail);
	}
	if (responseCode < 200 || responseCode >= 300) {
		throw std::runtime_error("OpenVibe update manifest returned HTTP " + std::to_string(responseCode));
	}
	if (output.empty()) {
		throw std::runtime_error("OpenVibe update manifest was empty");
	}

	return output;
}

struct SemVersion {
	uint64_t major = 0;
	uint64_t minor = 0;
	uint64_t patch = 0;
	std::vector<std::string> prerelease;
};

std::vector<std::string> Split(const std::string &value, char delimiter)
{
	std::vector<std::string> result;
	std::stringstream stream(value);
	std::string part;
	while (std::getline(stream, part, delimiter)) {
		result.emplace_back(std::move(part));
	}
	return result;
}

bool IsDigits(const std::string &value)
{
	return !value.empty() && std::all_of(value.begin(), value.end(), [](unsigned char ch) { return std::isdigit(ch); });
}

SemVersion ParseVersion(std::string value)
{
	while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
		value.erase(value.begin());
	}
	while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
		value.pop_back();
	}
	if (!value.empty() && (value.front() == 'v' || value.front() == 'V')) {
		value.erase(value.begin());
	}

	static const std::regex pattern(
		R"(^([0-9]+)\.([0-9]+)\.([0-9]+)(?:-([0-9A-Za-z.-]+))?(?:\+[0-9A-Za-z.-]+)?$)");
	std::smatch match;
	if (!std::regex_match(value, match, pattern)) {
		throw std::runtime_error("Invalid semantic version: " + value);
	}

	SemVersion version;
	version.major = std::stoull(match[1].str());
	version.minor = std::stoull(match[2].str());
	version.patch = std::stoull(match[3].str());
	if (match[4].matched) {
		version.prerelease = Split(match[4].str(), '.');
	}
	return version;
}

int ComparePrereleaseIdentifier(const std::string &left, const std::string &right)
{
	const bool leftNumeric = IsDigits(left);
	const bool rightNumeric = IsDigits(right);
	if (leftNumeric && rightNumeric) {
		const uint64_t l = std::stoull(left);
		const uint64_t r = std::stoull(right);
		return l < r ? -1 : l > r ? 1 : 0;
	}
	if (leftNumeric != rightNumeric) {
		return leftNumeric ? -1 : 1;
	}
	return left < right ? -1 : left > right ? 1 : 0;
}

int CompareVersions(const SemVersion &left, const SemVersion &right)
{
	if (left.major != right.major) {
		return left.major < right.major ? -1 : 1;
	}
	if (left.minor != right.minor) {
		return left.minor < right.minor ? -1 : 1;
	}
	if (left.patch != right.patch) {
		return left.patch < right.patch ? -1 : 1;
	}

	if (left.prerelease.empty() != right.prerelease.empty()) {
		return left.prerelease.empty() ? 1 : -1;
	}

	const size_t count = std::min(left.prerelease.size(), right.prerelease.size());
	for (size_t i = 0; i < count; ++i) {
		const int comparison = ComparePrereleaseIdentifier(left.prerelease[i], right.prerelease[i]);
		if (comparison != 0) {
			return comparison;
		}
	}
	return left.prerelease.size() < right.prerelease.size()
		       ? -1
		       : left.prerelease.size() > right.prerelease.size() ? 1 : 0;
}

std::string AssetKey()
{
#ifdef _WIN32
	return App()->IsPortableMode() ? "windows_x64_portable" : "windows_x64_installer";
#elif defined(__APPLE__) && defined(__aarch64__)
	return "macos_arm64";
#elif defined(__APPLE__)
	return "macos_x64";
#elif defined(__linux__) && defined(__x86_64__)
	return "linux_deb_x64";
#else
	return {};
#endif
}

void ValidateDownloadUrl(const std::string &url)
{
	const QUrl parsed(QString::fromStdString(url));
	if (!parsed.isValid() || parsed.scheme() != QStringLiteral("https") || parsed.host() != QStringLiteral("github.com") ||
	    !parsed.path().startsWith(QStringLiteral("/OpenVibe/ovbs-studio/releases/download/"))) {
		throw std::runtime_error("Update manifest contains an untrusted download URL");
	}
}
} // namespace

void OpenVibeUpdateThread::infoSlot(const QString &title, const QString &text)
{
	OBSMessageBox::information(App()->GetMainWindow(), title, text);
}

void OpenVibeUpdateThread::info(const QString &title, const QString &text)
{
	QMetaObject::invokeMethod(this, &OpenVibeUpdateThread::infoSlot, Qt::BlockingQueuedConnection, title, text);
}

bool OpenVibeUpdateThread::queryUpdateSlot(const QString &currentVersion, const QString &latestVersion)
{
	QMessageBox box(App()->GetMainWindow());
	box.setIcon(QMessageBox::Information);
	box.setWindowTitle(QStringLiteral("OpenVibe Studio Update"));
	box.setText(QStringLiteral("OpenVibe Studio %1 is available.").arg(latestVersion));
	box.setInformativeText(
		QStringLiteral("You are running %1. Download the verified package for this computer from the OpenVibe GitHub release?")
			.arg(currentVersion));

	QPushButton *download = box.addButton(QStringLiteral("Download Update"), QMessageBox::AcceptRole);
	box.addButton(QStringLiteral("Later"), QMessageBox::RejectRole);
	box.exec();
	return box.clickedButton() == download;
}

bool OpenVibeUpdateThread::queryUpdate(const QString &currentVersion, const QString &latestVersion)
{
	bool result = false;
	QMetaObject::invokeMethod(this, &OpenVibeUpdateThread::queryUpdateSlot, Qt::BlockingQueuedConnection,
				  qReturnArg(result), currentVersion, latestVersion);
	return result;
}

void OpenVibeUpdateThread::openUrlSlot(const QUrl &url)
{
	QDesktopServices::openUrl(url);
}

void OpenVibeUpdateThread::openUrl(const QUrl &url)
{
	QMetaObject::invokeMethod(this, &OpenVibeUpdateThread::openUrlSlot, Qt::BlockingQueuedConnection, url);
}

void OpenVibeUpdateThread::run()
try {
	const std::string text = FetchUpdateManifest();
	const nlohmann::json manifest = nlohmann::json::parse(text);
	if (manifest.at("schema_version").get<int>() != EXPECTED_SCHEMA_VERSION) {
		throw std::runtime_error("Unsupported OpenVibe update manifest schema");
	}
	if (manifest.at("product").get<std::string>() != EXPECTED_PRODUCT) {
		throw std::runtime_error("Unexpected update manifest product");
	}
	if (manifest.at("channel").get<std::string>() != "stable") {
		throw std::runtime_error("Latest update manifest is not a stable release");
	}

	const std::string currentString = App()->GetVersionString(false);
	const std::string latestString = manifest.at("version").get<std::string>();
	const SemVersion current = ParseVersion(currentString);
	const SemVersion latest = ParseVersion(latestString);

	config_set_int(App()->GetAppConfig(), "General", "openvibeLastUpdateCheck", static_cast<long long>(time(nullptr)));
	config_save_safe(App()->GetAppConfig(), "tmp", nullptr);

	if (CompareVersions(latest, current) <= 0) {
		if (manualUpdate) {
			info(QStringLiteral("OpenVibe Studio Update"),
			     QStringLiteral("You are already running the latest stable OpenVibe Studio release (%1).")
				     .arg(QString::fromStdString(currentString)));
		}
		return;
	}

	const std::string assetKey = AssetKey();
	if (assetKey.empty()) {
		throw std::runtime_error("No OpenVibe update package is defined for this platform/architecture");
	}

	const auto &asset = manifest.at("assets").at(assetKey);
	const std::string downloadUrl = asset.at("url").get<std::string>();
	ValidateDownloadUrl(downloadUrl);

	if (!queryUpdate(QString::fromStdString(currentString), QString::fromStdString(latestString))) {
		return;
	}

	openUrl(QUrl(QString::fromStdString(downloadUrl)));

} catch (const std::exception &exception) {
	blog(LOG_WARNING, "OpenVibe update check failed: %s", exception.what());
	if (manualUpdate) {
		info(QStringLiteral("OpenVibe Studio Update"),
		     QStringLiteral("Could not check for OpenVibe Studio updates.\n\n%1")
			     .arg(QString::fromUtf8(exception.what())));
	}
}
