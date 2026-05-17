#include "spell_checker.h"

#include <QBrush>
#include <QColor>
#include <QRegularExpression>

#include <algorithm>
#include <cctype>
#include <fstream>

spell_checker::spell_checker(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    misspelled_format.setUnderlineColor(Qt::red);

    misspelled_format.setUnderlineStyle(
        QTextCharFormat::SpellCheckUnderline
    );
}

void spell_checker::load_dictionary(const std::string& path)
{
    std::ifstream file(path);

    std::string word;

    while (file >> word) {
        dictionary.insert(normalize(word));
    }
}

std::string spell_checker::normalize(const std::string& word)
{
    std::string result;

    for (char c : word) {
        unsigned char uc = static_cast<unsigned char>(c);

        if (std::isalpha(uc)) {
            result += static_cast<char>(std::tolower(uc));
        }
    }

    return result;
}

bool spell_checker::is_correct(const std::string& word) const
{
    const std::string normalized = normalize(word);

    if (normalized.empty()) {
        return true;
    }

    return dictionary.contains(normalized);
}

void spell_checker::highlightBlock(const QString& text)
{
    QRegularExpression re("[A-Za-z']+");

    auto it = re.globalMatch(text);

    while (it.hasNext()) {
        auto match = it.next();

        const QString word = match.captured();

        if (!is_correct(word.toStdString())) {
            setFormat(
                match.capturedStart(),
                match.capturedLength(),
                misspelled_format
            );
        }
    }
}

int spell_checker::levenshtein_distance(
    const std::string& a,
    const std::string& b
)
{
    std::vector<std::vector<int>> dp(
        a.size() + 1,
        std::vector<int>(b.size() + 1)
    );

    for (std::size_t i = 0; i <= a.size(); ++i) {
        dp[i][0] = static_cast<int>(i);
    }

    for (std::size_t j = 0; j <= b.size(); ++j) {
        dp[0][j] = static_cast<int>(j);
    }

    for (std::size_t i = 1; i <= a.size(); ++i) {
        for (std::size_t j = 1; j <= b.size(); ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;

            dp[i][j] = std::min({
                dp[i - 1][j] + 1,
                dp[i][j - 1] + 1,
                dp[i - 1][j - 1] + cost
            });
        }
    }

    return dp[a.size()][b.size()];
}

std::vector<std::string>
spell_checker::suggestions(
    const std::string& word,
    std::size_t max_count
) const
{
    std::vector<std::pair<int, std::string>> candidates;

    const std::string normalized = normalize(word);

    for (const auto& entry : dictionary) {
        int distance =
            levenshtein_distance(normalized, entry);

        if (distance <= 2) {
            candidates.push_back({distance, entry});
        }
    }

    std::sort(
        candidates.begin(),
        candidates.end()
    );

    std::vector<std::string> result;

    for (std::size_t i = 0;
         i < candidates.size() && i < max_count;
         ++i) {
        result.push_back(candidates[i].second);
    }

    return result;
}