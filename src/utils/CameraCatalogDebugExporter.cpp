#include "thetaexplorer/CameraCatalogDebugExporter.h"
#include "thetaexplorer/Logger.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

namespace {

struct NormalizedNameInfo {
    QString extensionLower;
    QString baseName;
    QString stem;
    QString stemUpper;
    QString prefixBeforeTrailingNumber;
    int trailingNumber = -1;
    int trailingNumberWidth = 0;
};

NormalizedNameInfo normalizeName(const QString& fileName)
{
    NormalizedNameInfo info;
    const QFileInfo fi(fileName);
    info.extensionLower = fi.suffix().toLower();
    info.baseName = fi.completeBaseName();
    info.stem = info.baseName;
    info.stemUpper = info.stem.toUpper();

    static const QRegularExpression trailingNumberRegex("^(.*?)(\\d+)$");
    const QRegularExpressionMatch match = trailingNumberRegex.match(info.stem);
    if (match.hasMatch()) {
        info.prefixBeforeTrailingNumber = match.captured(1);
        const QString digits = match.captured(2);
        info.trailingNumber = digits.toInt();
        info.trailingNumberWidth = digits.size();
    } else {
        info.prefixBeforeTrailingNumber = info.stem;
    }

    return info;
}

QJsonObject makeFileObject(const CameraFileInfo& file, int originalIndex)
{
    const NormalizedNameInfo normalized = normalizeName(file.name);

    QJsonObject obj;
    obj["originalIndex"] = originalIndex;
    obj["name"] = file.name;
    obj["uti"] = file.uti;
    obj["devicePath"] = file.devicePath;
    obj["sizeBytes"] = QString::number(file.sizeBytes);
    obj["creationDateIso"] = file.creationDate.isValid()
        ? file.creationDate.toString(Qt::ISODateWithMs)
        : QString();
    obj["creationDateValid"] = file.creationDate.isValid();
    obj["width"] = file.width;
    obj["height"] = file.height;
    obj["isVideo"] = file.isVideo;
    obj["isRaw"] = file.isRaw;

    QJsonObject normalizedObj;
    normalizedObj["extensionLower"] = normalized.extensionLower;
    normalizedObj["baseName"] = normalized.baseName;
    normalizedObj["stem"] = normalized.stem;
    normalizedObj["stemUpper"] = normalized.stemUpper;
    normalizedObj["prefixBeforeTrailingNumber"] = normalized.prefixBeforeTrailingNumber;
    normalizedObj["hasTrailingNumber"] = normalized.trailingNumber >= 0;
    if (normalized.trailingNumber >= 0) {
        normalizedObj["trailingNumber"] = normalized.trailingNumber;
        normalizedObj["trailingNumberWidth"] = normalized.trailingNumberWidth;
    } else {
        normalizedObj["trailingNumber"] = QJsonValue();
        normalizedObj["trailingNumberWidth"] = 0;
    }
    obj["normalizedName"] = normalizedObj;

    return obj;
}

QJsonObject makeAdjacencyObject(const CameraFileInfo& current,
                                const CameraFileInfo& next,
                                const QString& streamName)
{
    const NormalizedNameInfo a = normalizeName(current.name);
    const NormalizedNameInfo b = normalizeName(next.name);

    QJsonObject obj;
    obj["stream"] = streamName;
    obj["currentName"] = current.name;
    obj["nextName"] = next.name;
    obj["currentDevicePath"] = current.devicePath;
    obj["nextDevicePath"] = next.devicePath;
    obj["samePrefixBeforeTrailingNumber"] =
        !a.prefixBeforeTrailingNumber.isEmpty() &&
        a.prefixBeforeTrailingNumber == b.prefixBeforeTrailingNumber;
    obj["hasConsecutiveTrailingNumbers"] =
        a.trailingNumber >= 0 &&
        b.trailingNumber >= 0 &&
        b.trailingNumber == a.trailingNumber + 1;

    if (current.creationDate.isValid() && next.creationDate.isValid()) {
        obj["gapMs"] = QString::number(current.creationDate.msecsTo(next.creationDate));
    } else {
        obj["gapMs"] = QJsonValue();
    }

    return obj;
}

QJsonObject makeCandidateRunObject(const QList<CameraFileInfo>& run,
                                   const QString& streamName,
                                   qint64 maxGapMs)
{
    QJsonObject obj;
    obj["stream"] = streamName;
    obj["count"] = run.size();
    obj["maxNeighborGapMsThreshold"] = QString::number(maxGapMs);

    QJsonArray names;
    for (const CameraFileInfo& file : run) {
        names.append(file.name);
    }
    obj["names"] = names;

    if (!run.isEmpty()) {
        obj["firstDevicePath"] = run.first().devicePath;
        obj["lastDevicePath"] = run.last().devicePath;
    }

    return obj;
}

QList<CameraFileInfo> sortedFilesForStream(const QList<CameraFileInfo>& files,
                                           const std::function<bool(const CameraFileInfo&)>& predicate)
{
    QList<CameraFileInfo> stream;
    for (const CameraFileInfo& file : files) {
        if (predicate(file)) {
            stream.append(file);
        }
    }

    std::sort(stream.begin(), stream.end(), [](const CameraFileInfo& a, const CameraFileInfo& b) {
        if (a.creationDate.isValid() && b.creationDate.isValid() && a.creationDate != b.creationDate) {
            return a.creationDate < b.creationDate;
        }
        if (a.creationDate.isValid() != b.creationDate.isValid()) {
            return a.creationDate.isValid();
        }
        return a.name < b.name;
    });

    return stream;
}

QJsonArray buildAdjacencyHints(const QList<CameraFileInfo>& stream, const QString& streamName)
{
    QJsonArray arr;
    for (int i = 0; i + 1 < stream.size(); ++i) {
        arr.append(makeAdjacencyObject(stream[i], stream[i + 1], streamName));
    }
    return arr;
}

QJsonArray buildCandidateRuns(const QList<CameraFileInfo>& stream,
                              const QString& streamName,
                              qint64 maxGapMs)
{
    QJsonArray arr;
    if (stream.isEmpty()) {
        return arr;
    }

    QList<CameraFileInfo> currentRun;
    currentRun.append(stream.first());

    for (int i = 1; i < stream.size(); ++i) {
        const CameraFileInfo& previous = stream[i - 1];
        const CameraFileInfo& current = stream[i];

        bool staysInRun = false;
        if (previous.creationDate.isValid() && current.creationDate.isValid()) {
            const qint64 gapMs = previous.creationDate.msecsTo(current.creationDate);
            staysInRun = gapMs >= 0 && gapMs <= maxGapMs;
        }

        if (staysInRun) {
            currentRun.append(current);
        } else {
            if (currentRun.size() >= 2) {
                arr.append(makeCandidateRunObject(currentRun, streamName, maxGapMs));
            }
            currentRun.clear();
            currentRun.append(current);
        }
    }

    if (currentRun.size() >= 2) {
        arr.append(makeCandidateRunObject(currentRun, streamName, maxGapMs));
    }

    return arr;
}

} // namespace

bool CameraCatalogDebugExporter::exportCatalog(const QList<CameraFileInfo>& files,
                                               const QString& outputPath,
                                               QString* errorMessage)
{
    LOGI("export") << "Starting export. files=" << files.size()
                   << "outputPath=" << outputPath;

    const QFileInfo outInfo(outputPath);
    QDir outDir = outInfo.dir();
    if (!outDir.exists() && !outDir.mkpath(".")) {
        LOGW("export") << "Failed to create directory:" << outDir.absolutePath();
        if (errorMessage) {
            *errorMessage = QString("Could not create directory: %1").arg(outDir.absolutePath());
        }
        return false;
    }

    QJsonObject root;
    root["generatedAtIso"] = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    root["fileCount"] = files.size();

    int imageCount = 0;
    int rawCount = 0;
    int videoCount = 0;
    for (const CameraFileInfo& file : files) {
        if (file.isVideo) {
            ++videoCount;
        } else if (file.isRaw) {
            ++rawCount;
        } else {
            ++imageCount;
        }
    }

    QJsonObject summary;
    summary["photoCount"] = imageCount;
    summary["rawCount"] = rawCount;
    summary["videoCount"] = videoCount;
    root["summary"] = summary;

    QJsonArray fileArray;
    for (int i = 0; i < files.size(); ++i) {
        fileArray.append(makeFileObject(files[i], i));
    }
    root["files"] = fileArray;

    const QList<CameraFileInfo> jpegStream = sortedFilesForStream(files, [](const CameraFileInfo& file) {
        return !file.isVideo && !file.isRaw;
    });
    const QList<CameraFileInfo> rawStream = sortedFilesForStream(files, [](const CameraFileInfo& file) {
        return file.isRaw;
    });

    QJsonObject analysis;
    analysis["jpegAdjacencyHints"] = buildAdjacencyHints(jpegStream, "jpeg");
    analysis["rawAdjacencyHints"] = buildAdjacencyHints(rawStream, "raw");
    analysis["jpegTemporalCandidateRuns"] = buildCandidateRuns(jpegStream, "jpeg", 2000);
    analysis["rawTemporalCandidateRuns"] = buildCandidateRuns(rawStream, "raw", 3000);
    root["analysis"] = analysis;

    QFile outFile(outputPath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        LOGW("export") << "Failed to open output file:" << outputPath;
        if (errorMessage) {
            *errorMessage = QString("Could not open file for writing: %1").arg(outputPath);
        }
        return false;
    }

    outFile.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    outFile.close();
    LOGI("export") << "Export finished successfully:" << outputPath;
    return true;
}
