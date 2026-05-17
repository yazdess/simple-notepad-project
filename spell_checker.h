#ifndef SPELL_CHECKER_H
#define SPELL_CHECKER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

#include <set>
#include <string>
#include <vector>

class spell_checker : public QSyntaxHighlighter {
public:
    explicit spell_checker(QTextDocument* parent = nullptr);

    void load_dictionary(const std::string& path);

    [[nodiscard]] bool is_correct(const std::string& word) const;

    [[nodiscard]] std::vector<std::string>
    suggestions(const std::string& word, std::size_t max_count = 5) const;

protected:
    void highlightBlock(const QString& text) override;

private:
    static std::string normalize(const std::string& word);

    static int levenshtein_distance(
        const std::string& a,
        const std::string& b
    );

    std::set<std::string> dictionary;

    QTextCharFormat misspelled_format;
};

#endif // SPELL_CHECKER_H