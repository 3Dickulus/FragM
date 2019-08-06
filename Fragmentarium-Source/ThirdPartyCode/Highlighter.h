/***************************************************************************
**                                                                        **
**  Highlighter.h, for handling GLSL syntax highlighting in Fragmentarium **
**  Copyright (C) 2013 Richard Paquette                                   **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Richard Paquette                                     **
**  Website/Contact: http://www.digilanti.org/ 3dickulus@gmail.com        **
**             Date: 12.16.13                                             **
**                                                                        **
** See these links for more information...                                **
** 3Dickulus http://www.fractalforums.com/index.php?topic=16405.0         **
** Syntopia http://www.fractalforums.com/index.php?topic=17178.0          **
****************************************************************************/
#pragma once

#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QMessageBox>
#include <QRegExp>
#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QTextStream>
#include <QXmlStreamReader>

#define GLSL_MIN_VERSION 110

namespace Fragmentarium
{
namespace GUI
{

struct ParenthesisInfo {
    QChar character;
    int position;
};

class TextBlockData : public QTextBlockUserData
{
public:
    TextBlockData();

    QVector<ParenthesisInfo *> parentheses();
    void insert ( ParenthesisInfo *info );
    void append ( ParenthesisInfo *info );

private:
    QVector<ParenthesisInfo *> m_parentheses;
};

class FragmentHighlighter : public QSyntaxHighlighter
{
public:
    FragmentHighlighter ( QTextDocument *parent ) : QSyntaxHighlighter ( parent )
    {
        setupFragmentariumDefaults();
        QString fileName ( "./Misc/glsl.xml" );
        QFile file ( fileName );
        if ( !file.open ( QFile::ReadOnly | QFile::Text ) ) {
            qDebug() << "Cannot read file " << fileName << "\n" << file.errorString();
            return;
        }

        if ( readXML ( &file ) ) {
            QDomElement element = domDocument.documentElement().firstChildElement ( "highlighting" );
            setupFormats ( element );
            setupKeywords ( element );
            setupContexts ( element );
        }
        /* Removes any device() or data from the reader
         * and resets its internal state to the initial state. */
        domDocument.clear();
    }

    bool glslVersionChanged = false;
    bool changeGLSLVersion()
    {
        return glslVersionChanged;
    };

protected:
    virtual void highlightBlock ( const QString &text )
    {
        enum { NormalState = -1, InsideMultiLineComment, InsideSingleLineComment };

        static int myversion = GLSL_MIN_VERSION;
        int lastversion = myversion;
        int state = previousBlockState();
        int start = 0;

        /// version check
        if ( text.left ( 8 ).endsWith ( "#version" ) ) {
            if ( text.length() > 11 ) {
                myversion = text.split ( " " ).at ( 1 ).toInt();
            } else {
                myversion = GLSL_MIN_VERSION;
            }
        } else {
            myversion = lastversion;
        }

        QString verString = contextRules.value ( "version" ).value ( QString ( "%1" ).arg ( myversion ) );
        if ( verString == "#pop" )
            verString = QString ( "v%1%2" ).arg ( myversion ).arg (( myversion == 100 || myversion == 300 ) ? "ES" : "" );
        else if ( verString.isEmpty() ) {
            myversion = GLSL_MIN_VERSION;
        }

        foreach ( const HighlightingRule &rule, highlightingRules ) {
            QRegExp expression ( rule.pattern );
            int index = text.indexOf ( expression );

            while ( index != -1 ) { // found a word
                int length = expression.matchedLength();
                // lookup format for this words group based on desired version
                QString formatKey = contextRules.value ( verString ).value ( rule.group );

                if ( formatKey.isEmpty() ) {
                    formatKey = rule.group;    // probably not found glsl.xml?
                }

                QTextCharFormat myformat = textFormat.value ( formatKey );
                setFormat ( index, length, myformat );
                index = expression.indexIn ( text, index + length );
            }

            if ( text.length() == text.indexOf ( expression ) + expression.matchedLength() ) {
                setCurrentBlockState ( 0 );
                if ( rule.group == "Preprocessor" ) {
                    break;
                }
            }
        }

        if ( myversion != lastversion ) {
            glslVersionChanged = true;
        } else {
            glslVersionChanged = false;
        }

        TextBlockData *data = new TextBlockData;

        for ( int i = 0; i < text.length(); ++i ) {
            if ( state == InsideMultiLineComment ) {
                if ( text.mid ( i, 2 ) == "*/" ) {
                    setFormat ( start, text.length() - start, textFormat.value ( "MultiComment" ) );
                    state = NormalState;
                }
            } else {
                if ( text.mid ( i, 2 ) == "//" ) {
                    setFormat ( i, text.length() - i, textFormat.value ( "Comment" ) );
                    break;
                } else if ( text.mid ( i, 2 ) == "/*" ) {
                    start = i;
                    state = InsideMultiLineComment;
                }
            }
            if ( state != InsideMultiLineComment && currentBlockState() != 0 ) {
                if ( text.at ( i ) == '{' || text.at ( i ) == '}' ) {
                    setFormat ( i, 1, textFormat.value ( "Preprocessor" ) );
                } else if ( text.at ( i ) == '(' || text.at ( i ) == ')' ) {
                    setFormat ( i, 1, textFormat.value ( "UserFunction" ) );
                    ParenthesisInfo *info = new ParenthesisInfo;
                    info->character = text.at ( i );
                    info->position = i;
                    data->insert ( info );
                }
            }
        }

        setCurrentBlockUserData ( data );

        if ( state == InsideMultiLineComment ) {
            setFormat ( start, text.length() - start, textFormat.value ( "MultiComment" ) );
        }

        setCurrentBlockState ( state );
    }

    bool readXML ( QIODevice *device )
    {
        QString errorStr;
        int errorLine;
        int errorColumn;
        if ( !domDocument.setContent ( device, true, &errorStr, &errorLine, &errorColumn ) ) {
            qDebug() << QString ( "Parse error at line %1, column %2:\n%3" )
                     .arg ( errorLine )
                     .arg ( errorColumn )
                     .arg ( errorStr );
            return false;
        }

        QDomElement root = domDocument.documentElement();
        if ( root.tagName() != "language" ) {
            qDebug() << "./Misc/glsl.xml is not a language highlighting file.";
            return false;
        }
        if ( root.attribute ( "name" ) != "GLSL" || root.attribute ( "section" ) != "Sources" ) {
            qDebug() << "./Misc/glsl.xml is not for GLSL sources.";
            return false;
        }
        if ( !root.attribute ( "author" ).contains ( "Robert Menzel (mail@renderingpipeline.com)" ) ) {
            qDebug() << "This may be the wrong glsl.xml file!";
        }

        return true;
    }

    void setupFragmentariumDefaults()
    {
        HighlightingRule rule;
        QTextCharFormat myformat;

        myformat.setForeground ( QBrush ( Qt::black ) );
        myformat.setFontWeight ( QFont::DemiBold );
        myformat.setFontItalic ( false );
        textFormat.insert ( "UserFunction", myformat );

        myformat.setForeground ( QBrush ( Qt::darkRed ) );
        myformat.setFontWeight ( QFont::Bold );
        textFormat.insert ( "Fragmentarium", myformat );

        myformat.setForeground ( QBrush ( Qt::red ) );
        myformat.setFontWeight ( QFont::Normal );
        textFormat.insert ( "Quotes", myformat );

        myformat.setForeground ( Qt::darkYellow );
        myformat.setFontWeight ( QFont::Normal );
        textFormat.insert ( "Numbers", myformat );

        myformat.setForeground ( Qt::blue );
        myformat.setFontWeight ( QFont::Bold );
        textFormat.insert ( "Preprocessor", myformat );

        myformat.setForeground ( Qt::darkGreen );
        myformat.setFontWeight ( QFont::Normal );
        myformat.setFontItalic ( true );
        textFormat.insert ( "Comment", myformat );

        myformat.setForeground ( Qt::darkBlue );
        myformat.setFontWeight ( QFont::Normal );
        myformat.setFontItalic ( true );
        textFormat.insert ( "MultiComment", myformat );

        /// comments functions numbers and quotes
        rule.group = "Comment";
        rule.pattern = QRegExp ( "//.*$" );
        rule.pattern.setCaseSensitivity ( Qt::CaseInsensitive );
        rule.format = textFormat.value ( "Comment" );
        highlightingRules.append ( rule );

        rule.group = "UserFunction";
        rule.pattern = QRegExp ( "\\b[A-Za-z0-9_]+(?=\\()\\b" );
        rule.pattern.setCaseSensitivity ( Qt::CaseInsensitive );
        rule.format = textFormat.value ( "UserFunction" );
        highlightingRules.append ( rule );

        rule.group = "Numbers"; // generic
//         rule.pattern = QRegExp("\\b[-+]?\\d*\\.?\\d+([eE][-+]?\\d+)?\\b");
        rule.pattern = QRegExp ( "\\b[+-]?\\d{1,}[\\.]?\\d*(?:([eE][+-]\\d{1,}|[fF]))?\\b" );
        rule.pattern.setCaseSensitivity ( Qt::CaseInsensitive );
        rule.format = textFormat.value ( "Numbers" );
        highlightingRules.append ( rule );

        rule.group = "Quotes";
        rule.pattern = QRegExp ( "[\"|\'].*[\'|\"]" );
        rule.pattern.setCaseSensitivity ( Qt::CaseSensitive );
        rule.format = textFormat.value ( "Quotes" );
        highlightingRules.append ( rule );

        /// Fragmentarium preprocessor words
        QStringList patterns;
        patterns << "\\bdonotrun\\b"
                 << "\\bvertex\\b"
                 << "\\bendvertex\\b"
                 << "\\bbuffer\\b"
                 << "\\binfo\\b"
                 << "\\bcamera\\b"
                 << "\\bgroup\\b"
                 << "\\bif\\b"
                 << "\\bifdef\\b"
                 << "\\belse\\b"
                 << "\\bendif\\b"
                 << "\\bdefine\\b"
                 << "\\binclude\\b"
                 << "\\bpreset\\b"
                 << "\\bendpreset\\b"
                 << "\\bTexParameter\\b"
                 << "\\bbuffershader\\b"
                 << "\\bversion\\b"
                 << "\\bundef\\b"
                 << "\\bifndef\\b"
                 << "\\belseif\\b"
                 << "\\berror\\b"
                 << "\\bpragma\\b"
                 << "\\bline\\b";

        foreach ( const QString &pattern, patterns ) {
            rule.group = "Preprocessor";
            rule.pattern = QRegExp ( "#" + pattern );
            rule.pattern.setCaseSensitivity ( Qt::CaseSensitive );
            rule.format = textFormat.value ( "Preprocessor" );
            highlightingRules.append ( rule );
        }

        QString NUMf = QString ( "[ ]?[-+]?\\d{0,}(?:[\\.]\\d{1,})?(?:[eE]{1}[+-]{1}\\d{1,})?" );
        QString NUMi = QString ( "[ ]?[-+]?\\d{1,}" );

        patterns.clear();
        /// Fragmentarium keywords
        patterns << "^\\s*uniform\\s+([d]{0,1}vec4)\\s+(\\S+);\\s*slider\\[\\((" + NUMf + "),(" + NUMf + "),(" + NUMf + "),(" + NUMf + ")\\),\\((" + NUMf + "),(" + NUMf + "),(" + NUMf + "),(" + NUMf + ")\\),\\((" + NUMf + "),(" + NUMf + "),(" + NUMf + "),(" + NUMf + ")\\)\\]"
                << "^\\s*uniform\\s+([d]{0,1}vec3)\\s+(\\S+);\\s*slider\\[\\((" + NUMf + "),(" + NUMf + "),(" + NUMf + ")\\),\\((" + NUMf + "),(" + NUMf + "),(" + NUMf + ")\\),\\((" + NUMf + "),(" + NUMf + "),(" + NUMf + ")\\)\\]"
                << "^\\s*uniform\\s+([d]{0,1}vec2)\\s+(\\S+);\\s*slider\\[\\((" + NUMf + "),(" + NUMf + ")\\),\\((" + NUMf + "),(" + NUMf + ")\\),\\((" + NUMf + "),(" + NUMf + ")\\)\\]"
                << "^\\s*uniform\\s+([d]{0,1}vec3)\\s+(\\S+);\\s*color\\[(" + NUMf + "),(" + NUMf + "),(" + NUMf + ")\\]"
                << "^\\s*uniform\\s+([d]{0,1}vec4)\\s+(\\S+);\\s*color\\[(" + NUMf + "),(" + NUMf + "),(" + NUMf + "),(" + NUMf + "),(" + NUMf + "),(" + NUMf + ")\\]"

                << "^\\s*uniform\\s+([float|double]{1,6})\\s+(\\S+);\\s*slider\\[(" + NUMf + "),(" + NUMf + "),(" + NUMf + ")\\]"
                << "^\\s*uniform\\s+int\\s+(\\S+);\\s*slider\\[(" + NUMi + "),(" + NUMi + "),(" + NUMi + ")\\]"
                << "^\\s*uniform\\s+bool\\s+(\\S+);\\s*checkbox\\[([true|false]{1,5})\\]"
                << "^\\s*uniform\\s+sampler([2D|Cube]{1,4})\\s+(\\S+);\\s*file\\[(\\S+)\\]"
                << "(random\\[" + NUMf + "," + NUMf + "\\])"
                << "^\\s*uniform\\s+float\\s+(time);"
                << "^\\s*uniform\\s+int\\s+(subframe);"
                << "^\\s*uniform\\s+([d]{0,1}vec2)\\s+(pixelSize);";

        foreach ( const QString &pattern, patterns ) {
            rule.group = "Fragmentarium";
            rule.pattern = QRegExp ( pattern );
            rule.pattern.setCaseSensitivity ( Qt::CaseSensitive );
            rule.format = textFormat.value ( "Fragmentarium" );
            highlightingRules.append ( rule );
        }
    }

    void setupFormats ( const QDomElement &element )
    {
        QDomElement child = element.firstChildElement ( "itemDatas" ).firstChildElement ( "itemData" );
        QTextCharFormat myformat;

        while ( !child.isNull() ) {

            myformat.setFontStrikeOut ( false );
            myformat.setFontWeight ( QFont::Normal );
            myformat.setFontItalic ( false );
            myformat.setFontUnderline ( false );
            myformat.setForeground ( QBrush ( Qt::color1 ) );

            if ( child.tagName() == "itemData" ) {

                QString name = child.attribute ( "name" );

                if ( ! ( child.attribute ( "strikeout" ).isEmpty() ) )
                    myformat.setFontStrikeOut (child.attribute ( "strikeout" ) == "1" ? true : false );
                if ( ! ( child.attribute ( "bold" ).isEmpty() ) )
                    myformat.setFontWeight ( ( child.attribute ( "bold" ) == "1" ) ? ( QFont::Bold ) : ( QFont::Normal ) );
                if ( ! ( child.attribute ( "italic" ).isEmpty() ) )
                    myformat.setFontItalic ( child.attribute ( "italic" ) == "1" ? true : false );
                if ( ! ( child.attribute ( "underline" ).isEmpty() ) )
                    myformat.setFontUnderline ( child.attribute ( "underline" ) == "1" ? true : false );
                if ( ! ( child.attribute ( "color" ).isEmpty() ) ) {
                    myformat.setForeground ( QBrush ( QColor ( child.attribute ( "color" ) ) ) );
                } else { /// add some default colors if none are set in the xml file
                    if ( name == "Normaltext" ) {
                        myformat.setForeground ( QBrush ( Qt::black ) );
                    } else if ( name == "Keywordstypes" ) {
                        myformat.setForeground ( QBrush ( Qt::darkBlue ) );
                    } else if ( name == "Keyword" ) {
                        myformat.setForeground ( QBrush ( Qt::darkMagenta ) );
                    } else if ( name == "Reserved" ) {
                        myformat.setForeground ( QBrush ( Qt::black ) );
                    } else if ( name == "Buildinvariables" ) {
                        myformat.setForeground ( QBrush ( Qt::darkMagenta ) );
                    } else if ( name == "Buildconstants" ) {
                        myformat.setForeground ( QBrush ( Qt::darkMagenta ) );
                    } else if ( name == "Buildinfunctions" ) {
                        myformat.setForeground ( Qt::darkYellow );
                    } else if ( ( name == "Float" ) || ( name == "Octal" ) || ( name == "Hex" ) || ( name == "Decimal" ) ) {
                        myformat.setForeground ( QBrush ( Qt::darkYellow ) );
                    } else if ( name == "Preprocessor" ) {
                        myformat.setForeground ( Qt::blue );
                    } else if ( name == "Comment" ) {
                        myformat.setForeground ( Qt::darkGreen );
                    }
                }
                textFormat.insert ( name, myformat );
            }
            child = child.nextSiblingElement();
        }
    }

    void setupKeywords ( const QDomElement &element )
    {
        HighlightingRule rule;
        QDomElement list = element.firstChildElement ( "list" );
        QString name;

        /// generate rules based on the contents of the glsl.xml file
        while ( !list.isNull() ) {
            if ( list.tagName() == "list" ) {
                name = list.attribute ( "name" );
                QDomElement item = list.firstChildElement ( "item" );
                while ( !item.isNull() ) {
                    if ( item.tagName() == "item" ) {
                        rule.group = name;
                        int i = textFormat.keys().count();
                        while ( i-- ) {
                            if ( name.contains ( textFormat.keys().at ( i ), Qt::CaseInsensitive ) ) {
                                rule.format = textFormat.value ( textFormat.keys().at ( i ) );
                                break;
                            }
                        }
                        rule.pattern = QRegExp ( QString ( "\\b%1\\b" ).arg ( item.text() ) );
                        highlightingRules.append ( rule );
                    }
                    item = item.nextSiblingElement();
                }
            }
            list = list.nextSiblingElement();
        }
    }

    void setupContexts ( const QDomElement &element )
    {
        QDomElement contexts = element.firstChildElement ( "contexts" );
        QString versname;
        /// generate rule contexts
        while ( !contexts.isNull() ) {
            if ( contexts.tagName() == "contexts" ) {
                QDomElement context = contexts.firstChildElement ( "context" );
                while ( !context.isNull() ) {
                    if ( context.tagName() == "context" ) {
                        QMap<QString, QString> mymap;
                        versname = context.attribute ( "name" );
                        if ( versname.left ( 1 ) == "v" ) {
                            QDomElement item = context.firstChildElement ( "keyword" );
                            while ( !item.isNull() ) {
                                if ( item.tagName() == "keyword" ) {
                                    mymap.insert ( item.attribute ( "String" ), item.attribute ( "attribute" ) );
                                }
                                item = item.nextSiblingElement();
                            }
                            item = context.firstChildElement ( "StringDetect" );
                            while ( !item.isNull() ) {
                                if ( item.tagName() == "StringDetect" ) {
                                    mymap.insert ( item.attribute ( "String" ).left ( 3 ), item.attribute ( "context" ) );
                                }
                                item = item.nextSiblingElement();
                            }
                        }
                        contextRules.insert ( versname, mymap );
                        mymap.clear();
                    }
                    context = context.nextSiblingElement();
                }
            }
            contexts = contexts.nextSiblingElement();
        }
    }

private:
    struct HighlightingRule {
        QRegExp pattern;
        QTextCharFormat format;
        QString group;
    };

    QVector<HighlightingRule> highlightingRules;
    QMap<QString, QTextCharFormat> textFormat;
    QMap<QString, QMap<QString, QString>> contextRules;
    QDomDocument domDocument;
};

} // namespace GUI
} // namespace Fragmentarium
