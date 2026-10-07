#include "TransformSettings.h"
#include "TransformDefaults.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

namespace
{
    const TransformDefaults::Entry* FindDefault(const std::string& id)
    {
        for (const auto& entry : TransformDefaults::Entries)
            if (id == entry.id) return &entry;
        return nullptr;
    }

    bool IsFinite(const Vec3& value)
    {
        return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
    }

    std::string FloatLiteral(float value)
    {
        std::ostringstream stream;
        stream.imbue(std::locale::classic());
        stream << std::setprecision(std::numeric_limits<float>::max_digits10) << value;
        std::string literal = stream.str();
        if (literal.find_first_of(".eE") == std::string::npos) literal += ".0";
        return literal + "f";
    }

    std::string VectorLiteral(const Vec3& value)
    {
        return "{ " + FloatLiteral(value.x) + ", " + FloatLiteral(value.y) + ", " + FloatLiteral(value.z) + " }";
    }

    void ReplaceDefault(std::string& source, const TransformDefaults::Entry& entry,
        const ObjectTransform& value)
    {
        const std::string begin = "// BEGIN_TRANSFORM(" + std::string(entry.id) + ")";
        const std::string end = "// END_TRANSFORM(" + std::string(entry.id) + ")";
        const auto beginPosition = source.find(begin);
        const auto endPosition = source.find(end);
        if (beginPosition == std::string::npos || endPosition == std::string::npos ||
            beginPosition != source.rfind(begin) || endPosition != source.rfind(end) ||
            beginPosition >= endPosition)
            throw std::runtime_error("Missing or duplicate source markers: " + std::string(entry.id));

        const auto firstLineEnd = source.find('\n', beginPosition + begin.size());
        const auto lastLineEnd = source.rfind('\n', endPosition);
        if (firstLineEnd == std::string::npos || lastLineEnd == std::string::npos ||
            firstLineEnd > lastLineEnd)
            throw std::runtime_error("Invalid source block: " + std::string(entry.id));

        // 目印の間にある初期値だけを変更する
        const std::string newline = source.find("\r\n") != std::string::npos ? "\r\n" : "\n";
        const std::string replacement =
            "    inline constexpr ObjectTransform " + std::string(entry.name) + "{" + newline +
            "        " + VectorLiteral(value.scale) + "," + newline +
            "        " + VectorLiteral(value.position) + "," + newline +
            "        " + VectorLiteral(value.rotation) + newline +
            "    };" + newline;
        source.replace(firstLineEnd + 1, lastLineEnd - firstLineEnd, replacement);
    }
}

const std::string& TransformSettings::SourceFilePath()
{
    // /FCで、このソースの場所をビルド時に記録する
    static const std::string path =
        (std::filesystem::path(__FILE__).parent_path() / "TransformDefaults.h").string();
    return path;
}

bool TransformSettings::IsValid(const ObjectTransform& transform)
{
    return IsFinite(transform.scale) && IsFinite(transform.position) && IsFinite(transform.rotation)
        && transform.scale.x > 0.0f && transform.scale.y > 0.0f && transform.scale.z > 0.0f;
}

bool TransformSettings::Save(const std::vector<DebugTransformTarget>& targets, std::string& status,
    const std::string& sourcePath)
{
    std::filesystem::path temporaryPath;
    bool temporaryOwned = false;
    try
    {
        if (targets.empty()) throw std::runtime_error("No objects selected.");
        std::ifstream file(sourcePath, std::ios::binary);
        if (!file) throw std::runtime_error("Cannot open TransformDefaults.h.");
        std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        if (file.bad()) throw std::runtime_error("Cannot read source file.");
        file.close();

        for (const auto& target : targets)
        {
            const auto* entry = FindDefault(target.id);
            if (!entry) throw std::runtime_error("Unknown object ID: " + target.id);
            const auto value = target.get();
            if (!IsValid(value)) throw std::runtime_error("Invalid transform: " + target.id);
            ReplaceDefault(source, *entry, value);
        }

        // 書き込みが完了してからソースを置き換える
        const std::filesystem::path path(sourcePath);
        temporaryPath = path;
        temporaryPath += ".tmp";
        {
            std::ofstream output(temporaryPath, std::ios::binary | std::ios::trunc);
            if (!output) throw std::runtime_error("Cannot write source file.");
            temporaryOwned = true;
            output.write(source.data(), static_cast<std::streamsize>(source.size()));
            output.close();
            if (!output) throw std::runtime_error("Source write failed.");
        }
#ifdef _WIN32
        if (!MoveFileExW(temporaryPath.c_str(), path.c_str(),
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            throw std::runtime_error("Cannot replace source file.");
#else
        std::filesystem::rename(temporaryPath, path);
#endif
        status = u8"ソースに保存しました。次回起動には再ビルドしてください。";
        return true;
    }
    catch (const std::exception& error)
    {
        if (temporaryOwned)
        {
            std::error_code ignored;
            std::filesystem::remove(temporaryPath, ignored);
        }
        status = std::string(u8"保存失敗: ") + error.what();
        return false;
    }
}

bool TransformSettings::ResetToDefaults(const std::vector<DebugTransformTarget>& targets, std::string& status)
{
    for (const auto& target : targets)
    {
        const auto* entry = FindDefault(target.id);
        if (!entry || !IsValid(*entry->value))
        {
            status = u8"初期値を確認してください。";
            return false;
        }
    }
    // 今のビルドに含まれている初期値へ戻す
    for (const auto& target : targets) target.set(*FindDefault(target.id)->value);
    status = u8"このビルドの初期値に戻しました。";
    return !targets.empty();
}
