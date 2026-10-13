#pragma once

#include <compare>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct SemVer {
    int major{ 0 };
    int minor{ 0 };
    int patch{ 0 };
    std::string prerelease;
    std::string build;
    [[nodiscard]] std::string to_string() const;

    [[nodiscard]] std::strong_ordering operator<=>(
        const SemVer& other) const noexcept;

    [[nodiscard]] bool operator==(
        const SemVer& other) const noexcept;
};

[[nodiscard]] std::optional<SemVer> parse_semver(
    std::string_view text);

enum class ComparisonOperator {
    Equal,
    GreaterThan,
    LessThan,
    GreaterThanOrEqual,
    LessThanOrEqual
};

struct VersionConstraint {
    ComparisonOperator op;
    SemVer target;

    [[nodiscard]] bool is_satisfied_by(
        const SemVer& version) const noexcept;

};

struct DependencyRequirement {
    std::string mod_slug;
    std::vector<VersionConstraint> constraints;

    [[nodiscard]] bool is_satisfied_by(
        const SemVer& installed) const noexcept;


};

[[nodiscard]] std::optional<ComparisonOperator> parse_operator(
    std::string_view text) noexcept;

[[nodiscard]] std::optional<DependencyRequirement>
parse_dependency_entry(std::string_view entry);

[[nodiscard]] std::optional<std::vector<DependencyRequirement>>
parse_manifest_file(const std::string& path);
