#include "fontloader.h"




FontLoader::FontLoader()
{
    // add font file
    int fontId = QFontDatabase::addApplicationFont(":/syles/fontawesome-webfont.ttf");
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);

    Q_ASSERT_X(fontFamilies.size()==1,"font","font not exist.");

    this->font.setFamily(fontFamilies.at(0));


}



QChar FontLoader::getIconChar(IconIdentity code)
{
    return QChar((int)code);
}

QFont FontLoader::getFont()
{
    return this->font;
}
