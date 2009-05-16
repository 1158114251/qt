/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef Patternist_XsdDocumentation_H
#define Patternist_XsdDocumentation_H

#include "qanytype_p.h"
#include "qanyuri_p.h"
#include "qderivedstring_p.h"
#include "qnamedschemacomponent_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Represents a XSD documentation object.
     *
     * This class represents the <em>documentation</em> component of an <em>annotation</em> object
     * of a XML schema as described <a href="http://www.w3.org/TR/xmlschema11-1/#cAnnotations">here</a>.
     *
     * @ingroup Patternist_schema
     * @author Tobias Koenig <tobias.koenig@trolltech.com>
     */
    class XsdDocumentation : public NamedSchemaComponent
    {
        public:
            typedef QExplicitlySharedDataPointer<XsdDocumentation> Ptr;
            typedef QList<XsdDocumentation::Ptr> List;

            /**
             * Creates a new documentation object.
             */
            XsdDocumentation();

            /**
             * Destroys the documentation object.
             */
            ~XsdDocumentation();

            /**
             * Sets the @p source of the documentation.
             *
             * The source points to an URL that contains more
             * information.
             */
            void setSource(const AnyURI::Ptr &source);

            /**
             * Returns the source of the documentation.
             */
            AnyURI::Ptr source() const;

            /**
             * Sets the @p language of the documentation.
             */
            void setLanguage(const DerivedString<TypeLanguage>::Ptr &language);

            /**
             * Returns the language of the documentation.
             */
            DerivedString<TypeLanguage>::Ptr language() const;

            /**
             * Sets the @p content of the documentation.
             *
             * The content can be of abritrary type.
             */
            void setContent(const QString &content);

            /**
             * Returns the content of the documentation.
             */
            QString content() const;

        private:
            AnyURI::Ptr                      m_source;
            DerivedString<TypeLanguage>::Ptr m_language;
            QString                          m_content;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
