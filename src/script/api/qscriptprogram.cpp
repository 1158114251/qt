/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtScript module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qscriptprogram.h"
#include "qscriptprogram_p.h"
#include "qscriptengine.h"
#include "qscriptengine_p.h"

#include "Executable.h"

QT_BEGIN_NAMESPACE

/*!
  \internal

  \since 4.6
  \class QScriptProgram

  \ingroup script

*/

QScriptProgramPrivate::QScriptProgramPrivate(const QString &src,
                                             const QString &fn,
                                             int ln)
    : sourceCode(src), fileName(fn), lineNumber(ln),
      engine(0), _executable(0), sourceId(-1), isCompiled(false)
{
    ref = 0;
}

QScriptProgramPrivate::~QScriptProgramPrivate()
{
    delete _executable;
}

QScriptProgramPrivate *QScriptProgramPrivate::get(const QScriptProgram &q)
{
    return const_cast<QScriptProgramPrivate*>(q.d_func());
}

JSC::EvalExecutable *QScriptProgramPrivate::executable(JSC::ExecState *exec,
                                                       QScriptEnginePrivate *eng)
{
    if (_executable) {
        if (eng == engine)
            return _executable;
        delete _executable;
    }
    WTF::PassRefPtr<QScript::UStringSourceProviderWithFeedback> provider
        = QScript::UStringSourceProviderWithFeedback::create(sourceCode, fileName, lineNumber, eng);
    sourceId = provider->asID();
    JSC::SourceCode source(provider, lineNumber); //after construction of SourceCode provider variable will be null.
    _executable = new JSC::EvalExecutable(exec, source);
    engine = eng;
    isCompiled = false;
    return _executable;
}

/*!
  Constructs a null QScriptProgram.
*/
QScriptProgram::QScriptProgram()
    : d_ptr(0)
{
}

/*!
  Constructs a new QScriptProgram with the given \a sourceCode, \a
  fileName and \a lineNumber.
*/
QScriptProgram::QScriptProgram(const QString &sourceCode,
                               const QString fileName,
                               int lineNumber)
    : d_ptr(new QScriptProgramPrivate(sourceCode, fileName, lineNumber))
{
}

/*!
  Constructs a new QScriptProgram that is a copy of \a other.
*/
QScriptProgram::QScriptProgram(const QScriptProgram &other)
    : d_ptr(other.d_ptr)
{
}

/*!
  Destroys this QScriptProgram.
*/
QScriptProgram::~QScriptProgram()
{
    Q_D(QScriptProgram);
    //    if (d->engine && (d->ref == 1))
    //      d->engine->unregisterScriptProgram(d);
}

/*!
  Assigns the \a other value to this QScriptProgram.
*/
QScriptProgram &QScriptProgram::operator=(const QScriptProgram &other)
{
  //    if (d_func() && d_func()->engine && (d_func()->ref == 1))
      //        d_func()->engine->unregisterScriptProgram(d_func());
  //    }
    d_ptr = other.d_ptr;
    return *this;
}

/*!
  Returns true if this QScriptProgram is null; otherwise
  returns false.
*/
bool QScriptProgram::isNull() const
{
    Q_D(const QScriptProgram);
    return (d == 0);
}

/*!
  Returns the source code of this program.
*/
QString QScriptProgram::sourceCode() const
{
    Q_D(const QScriptProgram);
    if (!d)
        return QString();
    return d->sourceCode;
}

/*!
  Returns the filename associated with this program.
*/
QString QScriptProgram::fileName() const
{
    Q_D(const QScriptProgram);
    if (!d)
        return QString();
    return d->fileName;
}

/*!
  Returns the line number associated with this program.
*/
int QScriptProgram::lineNumber() const
{
    Q_D(const QScriptProgram);
    if (!d)
        return -1;
    return d->lineNumber;
}

/*!
  Returns true if this QScriptProgram is equal to \a other;
  otherwise returns false.
*/
bool QScriptProgram::operator==(const QScriptProgram &other) const
{
    Q_D(const QScriptProgram);
    if (d == other.d_func())
        return true;
    return (sourceCode() == other.sourceCode())
        && (fileName() == other.fileName())
        && (lineNumber() == other.lineNumber());
}

/*!
  Returns true if this QScriptProgram is not equal to \a other;
  otherwise returns false.
*/
bool QScriptProgram::operator!=(const QScriptProgram &other) const
{
    return !operator==(other);
}

QT_END_NAMESPACE
