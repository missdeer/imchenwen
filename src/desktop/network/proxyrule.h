#ifndef PROXYRULE_H
#define PROXYRULE_H

#include <QUrl>
#include <QObject>
#include <QRegExp>

enum ProxyRuleType
{
    PRT_GFWLIST = 1,
    PRT_CUSTOM
};

class ProxyRuleMatcher
{
protected:
    QString m_pattern;
public:
    ProxyRuleMatcher(const QString& pattern)
        : m_pattern(pattern)
    {}
    virtual ~ProxyRuleMatcher() {}
    virtual bool isMatched(QUrl url) = 0;
};

class ProxyRuleRegexMatcher : public ProxyRuleMatcher
{
public:
    ProxyRuleRegexMatcher(const QString& pattern);
    bool isMatched(QUrl url);
protected:
private:
    QRegExp m_reg;
};

class ProxyRuleExcludeMatcher : public ProxyRuleMatcher
{
public:
    ProxyRuleExcludeMatcher(const QString& pattern);
    bool isMatched(QUrl url);
protected:
private:
    ProxyRuleMatcher* m_reverseMatcher;
};

class ProxyRuleStartWithMatcher : public ProxyRuleMatcher
{
public:
    ProxyRuleStartWithMatcher(const QString& pattern)
        : ProxyRuleMatcher(pattern)
    { }

    bool isMatched(QUrl url);
protected:
private:
};

class ProxyRuleKeywordMatcher : public ProxyRuleMatcher
{
public:
    ProxyRuleKeywordMatcher(const QString& pattern)
        : ProxyRuleMatcher(pattern)
    { }

    bool isMatched(QUrl url);
protected:
private:
};

class ProxyRuleHostContainMatcher : public ProxyRuleMatcher
{
public:
    ProxyRuleHostContainMatcher(const QString& pattern)
        : ProxyRuleMatcher(pattern)
    { }

    bool isMatched(QUrl url);
protected:
private:
};

class ProxyRuleMatcherFactory
{
public:
    static ProxyRuleMatcher* create(const QString& pattern);
protected:
private:
};

class ProxyRule
    : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString pattern READ pattern WRITE setPattern NOTIFY patternChanged)
    Q_PROPERTY(int hits READ hits WRITE setHits NOTIFY hitsChanged)
    Q_PROPERTY(int ruleType READ ruleType WRITE setRuleType NOTIFY ruleTypeChanged)
    Q_PROPERTY(bool deleted READ deleted WRITE setDeleted NOTIFY deletedChanged)
public:

    explicit ProxyRule(QObject *parent = nullptr);
    explicit ProxyRule(const QString& patt, QObject *parent = nullptr);
    ~ProxyRule() { delete m_proxyRuleMatcher; }
    bool enabled() const;
    void setEnabled(bool b );
    QString pattern() const;
    void setPattern(const QString& val);
    int hits() const;
    void increaseHits();
    void setHits(int h);
    int ruleType() const;
    void setRuleType(int type);
    bool deleted() const;
    void setDeleted(int deleted);

    bool isMatched(const QUrl &url);
signals:
    void enabledChanged();
    void patternChanged();
    void hitsChanged();
    void ruleTypeChanged();
    void deletedChanged();
public slots:

private:
    bool m_enbled;
    bool m_deleted;
    int m_hits;
    ProxyRuleType m_type;
    ProxyRuleMatcher* m_proxyRuleMatcher;
    QString m_pattern;
};

#endif // PROXYRULE_H
