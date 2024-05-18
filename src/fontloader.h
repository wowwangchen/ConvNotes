#ifndef FONTLOADER_H
#define FONTLOADER_H

#include <QFont>
#include <QString>
#include <QFontDatabase>



//颜色加载类
class FontLoader
{
public:
//根据QT版本
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QFontDatabase m_fontDatabase;
    FontLoader();
#else
    FontLoader() = default;
#endif

     ~FontLoader() = default;

    //单例模式
    static FontLoader &getInstance()
    {
        static FontLoader instance;
        return instance;
    }
    QFont loadFont(const QString &family, const QString &style, int pointSize);
};

#endif // FONTLOADER_H
