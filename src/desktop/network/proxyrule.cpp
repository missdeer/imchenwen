#include <QtCore>
#include "proxyrule.h"

ProxyRule::ProxyRule( QObject *parent /*= 0*/ ) 
: QObject(parent)
, m_proxyRuleMatcher(nullptr)
{
    setPattern("");
    setEnabled(true);
    setDeleted(false);
    setHits(0);
    setRuleType(PRT_GFWLIST);
}

ProxyRule::ProxyRule( const QString& patt,QObject *parent /*= 0*/ ) 
: QObject(parent)
, m_proxyRuleMatcher(nullptr)
{
    setPattern(patt);
    setEnabled(true);
    setDeleted(false);
    setHits(0);
    setRuleType(PRT_GFWLIST);
}

QString ProxyRule::pattern() const
{
    return m_pattern;
}

void ProxyRule::setPattern( const QString& val )
{
    m_pattern = val;
    m_proxyRuleMatcher = ProxyRuleMatcherFactory::create(val);
}

bool ProxyRule::enabled() const
{
    return m_enbled;
}

void ProxyRule::setEnabled( bool b )
{
    m_enbled = b;
}

int ProxyRule::hits() const
{
    return m_hits;
}

void ProxyRule::setHits( int h )
{
    m_hits = h;
}

int ProxyRule::ruleType() const
{
    return static_cast<int>( m_type);
}

void ProxyRule::setRuleType( int type )
{
    m_type = static_cast<ProxyRuleType>(type);
}

bool ProxyRule::deleted() const
{
    return m_deleted;
}

void ProxyRule::setDeleted(int deleted)
{
    m_deleted = deleted;
    emit deletedChanged();
}

bool ProxyRule::isMatched(const QUrl &url )
{
    if (m_proxyRuleMatcher) {
        if (m_proxyRuleMatcher->isMatched(url)) {
            m_hits++;
            return true;
        }
    }

    return false;
}

void ProxyRule::increaseHits()
{
    m_hits++;
}

ProxyRuleRegexMatcher::ProxyRuleRegexMatcher( const QString& pattern ) 
: ProxyRuleMatcher(pattern)
, m_reg(pattern)
{

}

bool ProxyRuleRegexMatcher::isMatched( QUrl url )
{
    // match domain name only
    return m_reg.exactMatch(url.host());
}

ProxyRuleExcludeMatcher::ProxyRuleExcludeMatcher( const QString& pattern ) 
: ProxyRuleMatcher(pattern)
{
    m_reverseMatcher = ProxyRuleMatcherFactory::create(pattern);
    Q_ASSERT(m_reverseMatcher);
}

bool ProxyRuleExcludeMatcher::isMatched( QUrl url )
{
    // match the last part only, high level user would act according to different rule type
    return m_reverseMatcher->isMatched(url);
}

bool ProxyRuleStartWithMatcher::isMatched( QUrl url )
{
    QString checkUrl = url.toEncoded(QUrl::RemoveScheme );
    // remove the double slash at the beginning
    checkUrl.remove(0, 2);

    return checkUrl.startsWith(m_pattern);
}

bool ProxyRuleKeywordMatcher::isMatched( QUrl url )
{
    QString checkUrl = url.toEncoded(QUrl::RemoveScheme );
    // remove the double slash at the beginning
    checkUrl.remove(0, 2);
    return checkUrl.contains(m_pattern);
}

bool ProxyRuleHostContainMatcher::isMatched( QUrl url )
{
    QString host = url.host();
    return host.contains(m_pattern);
}

ProxyRuleMatcher* ProxyRuleMatcherFactory::create( const QString& pattern )
{
    if (pattern.startsWith("@@")) {
        return new ProxyRuleExcludeMatcher(pattern.right(pattern.length() - 2));
    }

    if (pattern.startsWith("||")) {
        return new ProxyRuleHostContainMatcher(pattern.right(pattern.length() - 2));
    }

    if (pattern.startsWith("|")) {
        return new ProxyRuleStartWithMatcher(pattern.right(pattern.length() - 1));
    }

    QString regexReservedChars("*?\\^[]()$");
    for (int index = 0; index < regexReservedChars.length(); index++) {
        if (pattern.contains(regexReservedChars[index])) {
            return new ProxyRuleRegexMatcher(pattern);
        }
    }

    return new ProxyRuleKeywordMatcher(pattern);
}
