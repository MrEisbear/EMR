#include <algorithm>
#include <charconv>
#include <compare>
#include <cctype>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

struct SemVer {
    int major{ 0 };
    int minor{ 0 };
    int patch{ 0 };
    std::string prerelease;
    std::string build;

    [[nodiscard]] std::string to_string() const {
        std::string result = std::to_string(major) + "." +
            std::to_string(minor) + "." + std::to_string(patch);

        if (!prerelease.empty())
            result += "-" + prerelease;

        if (!build.empty())
            result += "+" + build;

        return result;
    }

    [[nodiscard]] std::strong_ordering operator<=>(
        const SemVer& other) const noexcept {
        if (auto c = major <=> other.major; c != 0) return c;
        if (auto c = minor <=> other.minor; c != 0) return c;
        if (auto c = patch <=> other.patch; c != 0) return c;

        // A prerelease is lower than the corresponding release.
        if (prerelease.empty() != other.prerelease.empty()) {
            return prerelease.empty()
                ? std::strong_ordering::greater
                : std::strong_ordering::less;
        }

        if (!prerelease.empty()) {
            auto a = split(prerelease, '.');
            auto b = split(other.prerelease, '.');

            for (std::size_t i = 0;
                i < std::min(a.size(), b.size()); ++i) {
                const bool an = numeric(a[i]);
                const bool bn = numeric(b[i]);

                if (an && bn) {
                    if (auto c = compare_numeric(a[i], b[i]); c != 0)
                        return c;
                }
                else if (an != bn) {
                    return an ? std::strong_ordering::less
                        : std::strong_ordering::greater;
                }
                else if (auto c = a[i] <=> b[i]; c != 0) {
                    return c;
                }
            }

            if (a.size() != b.size())
                return a.size() <=> b.size();
        }

        // Custom rule: build metadata DOES affect precedence.
        // No metadata < any nonempty metadata.
        return build <=> other.build;
    }

    [[nodiscard]] bool operator==(const SemVer& other) const noexcept {
        return (*this <=> other) == 0;
    }

private:
    static std::vector<std::string_view> split(
        std::string_view text, char delimiter) {
        std::vector<std::string_view> parts;

        while (true) {
            auto pos = text.find(delimiter);
            parts.push_back(text.substr(0, pos));

            if (pos == std::string_view::npos)
                break;

            text.remove_prefix(pos + 1);
        }

        return parts;
    }

    static bool numeric(std::string_view s) noexcept {
        return !s.empty() &&
            std::all_of(s.begin(), s.end(),
                [](unsigned char c) { return c >= '0' && c <= '9'; });
    }

    static std::strong_ordering compare_numeric(
        std::string_view a, std::string_view b) noexcept {
        // Compare arbitrary-length numeric identifiers without overflow.
        while (a.size() > 1 && a.front() == '0') a.remove_prefix(1);
        while (b.size() > 1 && b.front() == '0') b.remove_prefix(1);

        if (a.size() != b.size())
            return a.size() <=> b.size();

        return a <=> b;
    }

    friend std::optional<SemVer> parse_semver(std::string_view);
};

namespace TextUtils {
    [[nodiscard]] constexpr std::string_view trim(
        std::string_view s) noexcept {
        constexpr std::string_view ws = " \t\r\n";
        auto first = s.find_first_not_of(ws);

        if (first == std::string_view::npos)
            return {};

        return s.substr(first, s.find_last_not_of(ws) - first + 1);
    }

    [[nodiscard]] bool parse_integer(
        std::string_view s, int& result) noexcept {
        if (s.empty() || s.front() == '-')
            return false;

        auto [ptr, ec] = std::from_chars(
            s.data(), s.data() + s.size(), result);

        return ec == std::errc{} && ptr == s.data() + s.size();
    }
}

[[nodiscard]] std::optional<SemVer> parse_semver(
    std::string_view text) {
    using TextUtils::trim;
    text = trim(text);

    if (text.empty())
        return std::nullopt;

    SemVer result;

    auto plus = text.find('+');

    if (plus != std::string_view::npos) {
        result.build = std::string(text.substr(plus + 1));
        text = text.substr(0, plus);

        if (result.build.empty())
            return std::nullopt;
    }

    auto dash = text.find('-');

    if (dash != std::string_view::npos) {
        result.prerelease = std::string(text.substr(dash + 1));
        text = text.substr(0, dash);

        if (result.prerelease.empty())
            return std::nullopt;
    }

    auto valid_identifiers = [](std::string_view s,
        bool is_prerelease) {
            if (s.empty())
                return false;

            while (true) {
                auto dot = s.find('.');
                auto id = s.substr(0, dot);

                if (id.empty())
                    return false;

                for (unsigned char c : id) {
                    if (!((c >= 'a' && c <= 'z') ||
                        (c >= 'A' && c <= 'Z') ||
                        (c >= '0' && c <= '9') || c == '-'))
                        return false;
                }

                bool digits = std::all_of(id.begin(), id.end(),
                    [](unsigned char c) {
                        return c >= '0' && c <= '9';
                    });

                if (is_prerelease && digits &&
                    id.size() > 1 && id.front() == '0')
                    return false;

                if (dot == std::string_view::npos)
                    return true;

                s.remove_prefix(dot + 1);
            }
        };

    if (!result.prerelease.empty() &&
        !valid_identifiers(result.prerelease, true))
        return std::nullopt;

    if (!result.build.empty() &&
        !valid_identifiers(result.build, false))
        return std::nullopt;

    auto core = text;
    int* fields[] = { &result.major, &result.minor, &result.patch };

    for (int i = 0; i < 3; ++i) {
        auto dot = core.find('.');
        auto part = core.substr(0, dot);

        if (part.empty() ||
            (part.size() > 1 && part.front() == '0') ||
            !TextUtils::parse_integer(part, *fields[i]))
            return std::nullopt;

        if (i < 2) {
            if (dot == std::string_view::npos)
                return std::nullopt;

            core.remove_prefix(dot + 1);
        }
        else if (dot != std::string_view::npos) {
            return std::nullopt;
        }
    }

    return result;
}

enum class ComparisonOperator {
    Equal, GreaterThan, LessThan, GreaterThanOrEqual, LessThanOrEqual
};

struct VersionConstraint {
    ComparisonOperator op;
    SemVer target;

    [[nodiscard]] bool is_satisfied_by(const SemVer& version) const noexcept {
        switch (op) {
        case ComparisonOperator::Equal:              return version == target;
        case ComparisonOperator::GreaterThan:        return version > target;
        case ComparisonOperator::LessThan:           return version < target;
        case ComparisonOperator::GreaterThanOrEqual: return version >= target;
        case ComparisonOperator::LessThanOrEqual:    return version <= target;
        }

        return false;
    }
};

struct DependencyRequirement {
    std::string mod_slug;
    std::vector<VersionConstraint> constraints;

    [[nodiscard]] bool is_satisfied_by(const SemVer& installed) const noexcept {
        return !constraints.empty() &&
            std::all_of(constraints.begin(), constraints.end(),
                [&](const auto& c) {
                    return c.is_satisfied_by(installed);
                });
    }
};

[[nodiscard]] std::optional<ComparisonOperator> parse_operator(
    std::string_view s) noexcept {
    if (s == "==") return ComparisonOperator::Equal;
    if (s == ">")  return ComparisonOperator::GreaterThan;
    if (s == "<")  return ComparisonOperator::LessThan;
    if (s == ">=") return ComparisonOperator::GreaterThanOrEqual;
    if (s == "<=") return ComparisonOperator::LessThanOrEqual;
    return std::nullopt;
}

[[nodiscard]] std::optional<DependencyRequirement> parse_dependency_entry(
    std::string_view entry) {
    auto next_token = [](std::string_view& s) {
        s = TextUtils::trim(s);
        auto pos = s.find_first_of(" \t");

        auto token = s.substr(0, pos);

        s = pos == std::string_view::npos
            ? std::string_view{}
        : TextUtils::trim(s.substr(pos));

        return token;
        };

    DependencyRequirement result;

    auto first = entry.find('&');
    auto initial = TextUtils::trim(entry.substr(0, first));
    auto rest = first == std::string_view::npos
        ? std::string_view{}
    : entry.substr(first + 1);

    result.mod_slug = std::string(next_token(initial));

    if (result.mod_slug.empty())
        return std::nullopt;

    // First condition is optional: "ecore" means any version.
    if (!initial.empty()) {
        auto op = parse_operator(next_token(initial));
        auto version_text = next_token(initial);

        if (!op || version_text.empty() || !initial.empty())
            return std::nullopt;

        auto version = parse_semver(version_text);

        if (!version)
            return std::nullopt;

        result.constraints.push_back({ *op, *version });
    }

    while (!TextUtils::trim(rest).empty()) {
        auto amp = rest.find('&');
        auto condition = TextUtils::trim(rest.substr(0, amp));

        rest = amp == std::string_view::npos
            ? std::string_view{}
        : rest.substr(amp + 1);

        auto op = parse_operator(next_token(condition));
        auto version_text = next_token(condition);

        if (!op || version_text.empty() || !condition.empty())
            return std::nullopt;

        auto version = parse_semver(version_text);

        if (!version)
            return std::nullopt;

        result.constraints.push_back({ *op, *version });
    }

    return result;
}

[[nodiscard]] std::optional<std::vector<DependencyRequirement>>
parse_manifest_file(const std::string& path) {
    std::ifstream file(path);

    if (!file) {
        std::cerr << "Failed to open manifest: " << path << '\n';
        return std::nullopt;
    }

    std::vector<DependencyRequirement> result;
    std::string line;
    bool in_dependencies = false;
    std::size_t line_number = 0;

    while (std::getline(file, line)) {
        ++line_number;

        auto text = TextUtils::trim(line);

        if (text.empty() || text.starts_with('#'))
            continue;

        if (text == "dependencies:") {
            in_dependencies = true;
            continue;
        }

        if (!in_dependencies)
            continue;

        if (line.starts_with(' ') || line.starts_with('\t')) {
            if (text.starts_with('-')) {
                auto entry = TextUtils::trim(text.substr(1));
                auto dependency = parse_dependency_entry(entry);

                if (!dependency) {
                    std::cerr << "Invalid dependency at line "
                        << line_number << '\n';
                    return std::nullopt;
                }

                result.push_back(std::move(*dependency));
            }
        }
        else {
            in_dependencies = false;
        }
    }

    return result;
}