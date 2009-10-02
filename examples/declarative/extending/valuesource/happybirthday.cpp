#include "happybirthday.h"
#include <QTimer>

HappyBirthday::HappyBirthday(QObject *parent)
: QObject(parent), m_line(-1)
{
    setName(QString());
    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(advance()));
    timer->start(1000);
}

void HappyBirthday::setTarget(const QmlMetaProperty &p)
{
    m_target = p;
}

QString HappyBirthday::name() const
{
    return m_name;
}

void HappyBirthday::setName(const QString &name)
{
    m_name = name;

    m_lyrics.clear();
    m_lyrics << "Happy birthday to you,";
    m_lyrics << "Happy birthday to you,";
    m_lyrics << "Happy birthday dear " + m_name + ",";
    m_lyrics << "Happy birthday to you!";
    m_lyrics << "";
}
    
void HappyBirthday::advance()
{
    m_line = (m_line + 1) % m_lyrics.count();

    m_target.write(m_lyrics.at(m_line));
}

QML_DEFINE_TYPE(People, 1, 0, 0, HappyBirthday, HappyBirthday);
