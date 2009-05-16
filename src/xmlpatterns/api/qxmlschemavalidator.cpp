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

#include "qxmlschemavalidator.h"
#include "qxmlschemavalidator_p.h"

#include "qacceltreeresourceloader_p.h"
#include "qxmlschema.h"
#include "qxmlschema_p.h"
#include "qxsdvalidatinginstancereader_p.h"

#include <QtCore/QBuffer>
#include <QtCore/QIODevice>
#include <QtCore/QUrl>

/*!
  \class QXmlSchemaValidator

  \brief The QXmlSchemaValidator class validates XML instance documents against a W3C XML Schema.

  \reentrant
  \since 4.X
  \ingroup xml-tools

  The QXmlSchemaValidator class loads, parses an XML instance document and validates it
  against a W3C XML Schema that has been compiled with \l{QXmlSchema}.
*/

/*!
  Constructs a schema validator that will use \a schema for validation.
 */
QXmlSchemaValidator::QXmlSchemaValidator(const QXmlSchema &schema)
    : d(new QXmlSchemaValidatorPrivate(schema))
{
}

/*!
  Destroys this QXmlSchemaValidator.
 */
QXmlSchemaValidator::~QXmlSchemaValidator()
{
    delete d;
}

/*!
  Sets the \a schema that shall be used for further validation.
 */
void QXmlSchemaValidator::setSchema(const QXmlSchema &schema)
{
    d->setSchema(schema);
}

/*!
  Validates the XML instance document read from \a data with the
  given \a documentUri against the schema.

  Returns \c true if the XML instance document is valid according the
  schema, \c false otherwise.
 */
bool QXmlSchemaValidator::validate(const QByteArray &data, const QUrl &documentUri)
{
    QByteArray localData(data);

    QBuffer buffer(&localData);
    buffer.open(QIODevice::ReadOnly);

    return validate(&buffer, documentUri);
}

/*!
  Validates the XML instance document read from \a source against the schema.

  Returns \c true if the XML instance document is valid according the
  schema, \c false otherwise.
 */
bool QXmlSchemaValidator::validate(const QUrl &source)
{
    d->m_context->setMessageHandler(messageHandler());
    d->m_context->setUriResolver(uriResolver());
    d->m_context->setNetworkAccessManager(networkAccessManager());

    const QPatternist::AutoPtr<QNetworkReply> reply(QPatternist::AccelTreeResourceLoader::load(source, d->m_context->networkAccessManager(),
                                                                                               d->m_context, QPatternist::AccelTreeResourceLoader::ContinueOnError));
    if (reply)
        return validate(reply.data(), source);
    else
        return false;
}

/*!
  Validates the XML instance document read from \a source with the
  given \a documentUri against the schema.

  Returns \c true if the XML instance document is valid according the
  schema, \c false otherwise.
 */
bool QXmlSchemaValidator::validate(QIODevice *source, const QUrl &documentUri)
{
    if (!source) {
        qWarning("A null QIODevice pointer cannot be passed.");
        return false;
    }

    if (!source->isReadable()) {
        qWarning("The device must be readable.");
        return false;
    }

    d->m_context->setMessageHandler(messageHandler());
    d->m_context->setUriResolver(uriResolver());
    d->m_context->setNetworkAccessManager(networkAccessManager());

    QPatternist::NetworkAccessDelegator::Ptr delegator(new QPatternist::NetworkAccessDelegator(d->m_context->networkAccessManager(),
                                                                                               d->m_context->networkAccessManager()));

    QPatternist::AccelTreeResourceLoader loader(d->m_context->namePool(), delegator, QPatternist::AccelTreeBuilder<true>::SourceLocationsFeature);

    QPatternist::Item item;
    try {
        item = loader.openDocument(source, documentUri, d->m_context);
    } catch (QPatternist::Exception exception) {
        return false;
    }

    QXmlNodeModelIndex index = item.asNode();
    const QAbstractXmlNodeModel *model = item.asNode().model();

    QPatternist::XsdValidatedXmlNodeModel *validatedModel = new QPatternist::XsdValidatedXmlNodeModel(model);

    QPatternist::XsdValidatingInstanceReader reader(validatedModel, documentUri, d->m_context);
    if (d->m_schema)
        reader.addSchema(d->m_schema, d->m_schemaDocumentUri);
    try {
        reader.read();
    } catch (QPatternist::Exception exception) {
        return false;
    }

    return true;
}

/*!
  Returns the name pool used by this QXmlSchemaValidator for constructing \l
  {QXmlName} {names}. There is no setter for the name pool, because
  mixing name pools causes errors due to name confusion.
 */
QXmlNamePool QXmlSchemaValidator::namePool() const
{
    return d->m_namePool;
}

/*!
  Changes the \l {QAbstractMessageHandler}{message handler} for this
  QXmlSchemaValidator to \a handler. The schema validator sends all parsing and
  validation messages to this message handler. QXmlSchemaValidator does not take
  ownership of \a handler.

  Normally, the default message handler is sufficient. It writes
  compile and validation messages to \e stderr. The default message
  handler includes color codes if \e stderr can render colors.

  When QXmlSchemaValidator calls QAbstractMessageHandler::message(),
  the arguments are as follows:

  \table
  \header
    \o message() argument
    \o Semantics
  \row
    \o QtMsgType type
    \o Only QtWarningMsg and QtFatalMsg are used. The former
       identifies a warning, while the latter identifies an error.
  \row
    \o const QString & description
    \o An XHTML document which is the actual message. It is translated
       into the current language.
  \row
    \o const QUrl &identifier
    \o Identifies the error with a URI, where the fragment is
       the error code, and the rest of the URI is the error namespace.
  \row
    \o const QSourceLocation & sourceLocation
    \o Identifies where the error occurred.
  \endtable

 */
void QXmlSchemaValidator::setMessageHandler(QAbstractMessageHandler *handler)
{
    d->m_userMessageHandler = handler;
}

/*!
    Returns the message handler that handles parsing and validation
    messages for this QXmlSchemaValidator.
 */
QAbstractMessageHandler *QXmlSchemaValidator::messageHandler() const
{
    if (d->m_userMessageHandler)
        return d->m_userMessageHandler;

    return d->m_messageHandler.data()->value;
}

/*!
  Sets the URI resolver to \a resolver. QXmlSchemaValidator does not take
  ownership of \a resolver.

  \sa uriResolver()
 */
void QXmlSchemaValidator::setUriResolver(QAbstractUriResolver *resolver)
{
    d->m_uriResolver = resolver;
}

/*!
  Returns the schema's URI resolver. If no URI resolver has been set,
  QtXmlPatterns will use the URIs in queries as they are.

  The URI resolver provides a level of abstraction, or \e{polymorphic
  URIs}. A resolver can rewrite \e{logical} URIs to physical ones, or
  it can translate obsolete or invalid URIs to valid ones.

  When QtXmlPatterns calls QAbstractUriResolver::resolve() the
  absolute URI is the URI mandated by the schema specification, and the
  relative URI is the URI specified by the user.

  \sa setUriResolver()
 */
QAbstractUriResolver *QXmlSchemaValidator::uriResolver() const
{
    return d->m_uriResolver;
}

/*!
  Sets the network manager to \a manager.
  QXmlSchemaValidator does not take ownership of \a manager.

  \sa networkAccessManager()
 */
void QXmlSchemaValidator::setNetworkAccessManager(QNetworkAccessManager *manager)
{
    d->m_userNetworkAccessManager = manager;
}

/*!
  Returns the network manager, or 0 if it has not been set.

  \sa setNetworkAccessManager()
 */
QNetworkAccessManager *QXmlSchemaValidator::networkAccessManager() const
{
    if (d->m_userNetworkAccessManager)
        return d->m_userNetworkAccessManager;

    return d->m_networkAccessManager.data()->value;
}
