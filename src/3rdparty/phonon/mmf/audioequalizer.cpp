/*  This file is part of the KDE project.

Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).

This library is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 or 3 of the License.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <AudioEqualizerBase.h>
#include "audioequalizer.h"

QT_BEGIN_NAMESPACE

using namespace Phonon;
using namespace Phonon::MMF;

/*! \class MMF::AudioEqualizer
  \internal
*/

// Define functions which depend on concrete native effect class name
PHONON_MMF_DEFINE_EFFECT_FUNCTIONS(AudioEqualizer)

AudioEqualizer::AudioEqualizer(QObject *parent, const QList<EffectParameter>& parameters)
    :   AbstractAudioEffect::AbstractAudioEffect(parent, parameters)
{

}

void AudioEqualizer::parameterChanged(const int pid,
                                      const QVariant &value)
{
    if (m_effect.data()) {
        const int band = pid;
        const qreal level = value.toReal();
        setBandLevel(band, level);
    }
}

void AudioEqualizer::applyParameters()
{
    if (m_effect.data()) {
	Phonon::EffectParameter param;
        foreach (param, parameters()) {
            const int band = param.id();
            const qreal level = parameterValue(param).toReal();
            setBandLevel(band, level);
        }
    }
}

void AudioEqualizer::setBandLevel(int band, qreal externalLevel)
{
    const EffectParameter &param = m_params[band-1]; // Band IDs are 1-based
    const int internalLevel = param.toInternalValue(externalLevel);

    // TODO: handle audio effect errors
    TRAP_IGNORE(concreteEffect()->SetBandLevelL(band, internalLevel));
}

//-----------------------------------------------------------------------------
// Static functions
//-----------------------------------------------------------------------------

const char* AudioEqualizer::description()
{
    return "Audio equalizer";
}

bool AudioEqualizer::getParameters(CMdaAudioOutputStream *stream,
    QList<EffectParameter>& parameters)
{
    bool supported = false;

    QScopedPointer<CAudioEqualizer> effect;
    TRAPD(err, effect.reset(CAudioEqualizer::NewL(*stream)));

    if (KErrNone == err) {
        supported = true;

        TInt32 dbMin;
        TInt32 dbMax;
        effect->DbLevelLimits(dbMin, dbMax);

        const int bandCount = effect->NumberOfBands();

        // For some reason, band IDs are 1-based, as opposed to the
        // 0-based indices used in just about other Symbian API...!
        for (int i = 1; i <= bandCount; ++i) {
            const qint32 hz = effect->CenterFrequency(i);

            // We pass a floating-point parameter range of -1.0 to +1.0 for
            // each band in order to work around a limitation in
            // Phonon::EffectWidget.  See documentation of EffectParameter
            // for more details.
            EffectParameter param(
                 /* parameterId */        i,
                 /* name */               tr("%1 Hz").arg(hz),
                 /* hints */              EffectParameter::LogarithmicHint,
                 /* defaultValue */       QVariant(qreal(0.0)),
                 /* minimumValue */       QVariant(qreal(-1.0)),
                 /* maximumValue */       QVariant(qreal(+1.0)));

            param.setInternalRange(dbMin, dbMax);
            parameters.append(param);
        }
    }

    return supported;
}

QT_END_NAMESPACE
