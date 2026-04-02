#include "thetaexplorer/HdrGroupingService.h"
#include "thetaexplorer/Logger.h"

#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <algorithm>

namespace {

struct ParsedName {
    QString extension;
    QString prefix;
    int number = -1;
    bool hasNumber = false;
};

ParsedName parseName(const QString& fileName)
{
    ParsedName parsed;
    const QFileInfo info(fileName);
    parsed.extension = info.suffix().toLower();

    static const QRegularExpression regex("^(.*?)(\\d+)$");
    const QString stem = info.completeBaseName();
    const QRegularExpressionMatch match = regex.match(stem);
    if (match.hasMatch()) {
        parsed.prefix = match.captured(1).toUpper();
        parsed.number = match.captured(2).toInt();
        parsed.hasNumber = true;
    } else {
        parsed.prefix = stem.toUpper();
    }
    return parsed;
}

bool isBrowsable(const CameraFileInfo& file)
{
    if (file.isVideo || file.isRaw) {
        return true;
    }

    const QString ext = QFileInfo(file.name).suffix().toLower();
    return ext == "jpg" || ext == "jpeg";
}

bool sameFamily(const CameraFileInfo& a, const CameraFileInfo& b)
{
    if (a.isVideo || b.isVideo) {
        return a.isVideo == b.isVideo;
    }
    if (a.isRaw != b.isRaw) {
        return false;
    }
    const QString extA = QFileInfo(a.name).suffix().toLower();
    const QString extB = QFileInfo(b.name).suffix().toLower();
    return extA == extB;
}

bool areAdjacentHdrCandidates(const CameraFileInfo& previous, const CameraFileInfo& current)
{
    if (!sameFamily(previous, current)) {
        return false;
    }
    if (previous.isVideo || current.isVideo) {
        return false;
    }

    const ParsedName prevName = parseName(previous.name);
    const ParsedName currName = parseName(current.name);

    if (prevName.prefix != currName.prefix) {
        return false;
    }

    if (previous.creationDate.isValid() && current.creationDate.isValid()) {
        const qint64 gapMs = previous.creationDate.msecsTo(current.creationDate);
        if (gapMs < 0 || gapMs > 2500) {
            return false;
        }
    }

    if (prevName.hasNumber && currName.hasNumber) {
        const int delta = currName.number - prevName.number;
        return delta >= 1 && delta <= 2;
    }

    return true;
}

QString kindBadge(const MediaAssetGroup& group)
{
    switch (group.kind) {
        case MediaAssetKind::HdrJpegSet:
            return QString("HDR JPG · %1").arg(group.imageCount());
        case MediaAssetKind::HdrRawSet:
            return QString("HDR DNG · %1").arg(group.imageCount());
        case MediaAssetKind::Video:
            return "VIDEO";
        case MediaAssetKind::SingleRaw:
            return "DNG";
        case MediaAssetKind::SinglePhoto:
        default:
            return "JPG";
    }
}

QString buildRangeTitle(const QList<CameraFileInfo>& files)
{
    if (files.isEmpty()) {
        return QString();
    }
    if (files.size() == 1) {
        return files.first().name;
    }

    const QFileInfo firstInfo(files.first().name);
    const QFileInfo lastInfo(files.last().name);
    const ParsedName firstParsed = parseName(files.first().name);
    const ParsedName lastParsed = parseName(files.last().name);

    if (firstParsed.hasNumber && lastParsed.hasNumber && firstParsed.prefix == lastParsed.prefix) {
        return QString("%1%2-%3")
            .arg(firstParsed.prefix)
            .arg(firstParsed.number)
            .arg(lastParsed.number);
    }

    return QString("%1 +%2")
        .arg(firstInfo.fileName())
        .arg(files.size() - 1);
}

MediaAssetGroup makeGroup(const QList<CameraFileInfo>& files, MediaAssetKind kind)
{
    MediaAssetGroup group;
    group.files = files;
    group.kind = kind;
    group.isHdrSet = (kind == MediaAssetKind::HdrJpegSet || kind == MediaAssetKind::HdrRawSet);
    group.isRaw = (kind == MediaAssetKind::HdrRawSet || kind == MediaAssetKind::SingleRaw);
    group.isVideo = (kind == MediaAssetKind::Video);
    group.representativeIndex = files.isEmpty() ? 0 : files.size() / 2;
    group.representative = files[group.representativeIndex];
    group.captureTime = group.representative.creationDate;
    group.id = group.representative.devicePath;
    group.displayTitle = group.isHdrSet ? buildRangeTitle(files) : group.representative.name;
    group.subtitle = kindBadge(group);
    return group;
}

QList<MediaAssetGroup> groupImageStream(const QList<CameraFileInfo>& input, bool raw)
{
    QList<CameraFileInfo> files;
    for (const CameraFileInfo& file : input) {
        if (!file.isVideo && file.isRaw == raw && isBrowsable(file)) {
            files.append(file);
        }
    }

    std::sort(files.begin(), files.end(), [](const CameraFileInfo& a, const CameraFileInfo& b) {
        const ParsedName aName = parseName(a.name);
        const ParsedName bName = parseName(b.name);
        if (aName.prefix == bName.prefix && aName.hasNumber && bName.hasNumber && aName.number != bName.number) {
            return aName.number < bName.number;
        }
        if (a.creationDate.isValid() && b.creationDate.isValid() && a.creationDate != b.creationDate) {
            return a.creationDate < b.creationDate;
        }
        return a.name < b.name;
    });

    QList<MediaAssetGroup> groups;
    QList<CameraFileInfo> current;

    const auto flushRun = [&groups, raw](QList<CameraFileInfo>& run) {
        if (run.isEmpty()) {
            return;
        }
        if (run.size() >= 3) {
            groups.append(makeGroup(run, raw ? MediaAssetKind::HdrRawSet : MediaAssetKind::HdrJpegSet));
        } else {
            for (const CameraFileInfo& file : run) {
                groups.append(makeGroup({file}, raw ? MediaAssetKind::SingleRaw : MediaAssetKind::SinglePhoto));
            }
        }
        run.clear();
    };

    for (const CameraFileInfo& file : files) {
        if (current.isEmpty()) {
            current.append(file);
            continue;
        }
        if (areAdjacentHdrCandidates(current.last(), file)) {
            current.append(file);
            continue;
        }
        flushRun(current);
        current.append(file);
    }
    flushRun(current);

    return groups;
}

QList<MediaAssetGroup> groupVideos(const QList<CameraFileInfo>& input)
{
    QList<MediaAssetGroup> groups;
    for (const CameraFileInfo& file : input) {
        if (file.isVideo) {
            groups.append(makeGroup({file}, MediaAssetKind::Video));
        }
    }
    return groups;
}

} // namespace

QList<MediaAssetGroup> HdrGroupingService::buildGroups(const QList<CameraFileInfo>& files)
{
    QList<MediaAssetGroup> groups;

    const QList<MediaAssetGroup> jpegGroups = groupImageStream(files, false);
    const QList<MediaAssetGroup> rawGroups = groupImageStream(files, true);
    const QList<MediaAssetGroup> videoGroups = groupVideos(files);

    groups.reserve(jpegGroups.size() + rawGroups.size() + videoGroups.size());
    groups.append(jpegGroups);
    groups.append(rawGroups);
    groups.append(videoGroups);

    std::sort(groups.begin(), groups.end(), [](const MediaAssetGroup& a, const MediaAssetGroup& b) {
        if (a.captureTime.isValid() && b.captureTime.isValid() && a.captureTime != b.captureTime) {
            return a.captureTime > b.captureTime;
        }
        return a.displayTitle > b.displayTitle;
    });

    int hdrJpegCount = 0;
    int hdrRawCount = 0;
    int singleCount = 0;
    for (const MediaAssetGroup& group : groups) {
        if (group.kind == MediaAssetKind::HdrJpegSet) {
            ++hdrJpegCount;
        } else if (group.kind == MediaAssetKind::HdrRawSet) {
            ++hdrRawCount;
        } else {
            ++singleCount;
        }
    }
    LOGI("hdr") << "Grouped catalog into" << groups.size()
                << "items. hdrJpeg=" << hdrJpegCount
                << "hdrRaw=" << hdrRawCount
                << "other=" << singleCount;

    return groups;
}
