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

#ifndef PHONON_MMF_ABSTRACTMEDIAPLAYER_H
#define PHONON_MMF_ABSTRACTMEDIAPLAYER_H

#include <QTimer>
#include <QScopedPointer>
#include <e32std.h>
#include "abstractplayer.h"

class RFile;

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace MMF
    {
		class AudioOutput;
    
		/**
		 * Interface via which MMF client APIs for both audio and video can be
		 * accessed.
		 */
        class AbstractMediaPlayer : public AbstractPlayer
        {
			Q_OBJECT
        
        protected:
        	AbstractMediaPlayer();
        	explicit AbstractMediaPlayer(const AbstractPlayer& player);
        	~AbstractMediaPlayer();
        	
        public:
        	// MediaObjectInterface
        	virtual void play();
        	virtual void pause();
        	virtual void stop();
        	virtual void seek(qint64 milliseconds);
        	virtual bool isSeekable() const;
            virtual Phonon::ErrorType errorType() const;
            virtual QString errorString() const;
            virtual Phonon::State state() const;
            virtual MediaSource source() const;
            virtual void setFileSource(const Phonon::MediaSource&, RFile&);
            virtual void setNextSource(const MediaSource &source);
            
        	// VolumeObserver
        	virtual void volumeChanged(qreal volume);

        protected:
        	// AbstractPlayer
        	virtual void doSetTickInterval(qint32 interval);
        	
        protected:
        	virtual void doPlay() = 0;
        	virtual void doPause() = 0;
        	virtual void doStop() = 0;
        	virtual void doSeek(qint64 pos) = 0;
        	virtual int setDeviceVolume(int mmfVolume) = 0;
        	virtual int openFile(RFile& file) = 0;
        	virtual void close() = 0;
        	
        protected:
        	bool tickTimerRunning() const;
            void startTickTimer();
        	void stopTickTimer();
        	void maxVolumeChanged(int maxVolume);
        	
        	/**
			 * Defined private state enumeration in order to add GroundState
			 */
			enum PrivateState
				{
				LoadingState    = Phonon::LoadingState,
				StoppedState    = Phonon::StoppedState,
				PlayingState    = Phonon::PlayingState,
				BufferingState	= Phonon::BufferingState,
				PausedState		= Phonon::PausedState,
				ErrorState		= Phonon::ErrorState,
				GroundState
				};

			/**
			 * Converts PrivateState into the corresponding Phonon::State
			 */
			Phonon::State phononState() const;
			
			/**
			 * Converts PrivateState into the corresponding Phonon::State
			 */
			static Phonon::State phononState(PrivateState state);

			/**
			 * Changes state and emits stateChanged()
			 */
			void changeState(PrivateState newState);
			
			/**
			 * Records error and changes state to ErrorState
			 */
			void setError(Phonon::ErrorType error);
			
            static qint64 toMilliSeconds(const TTimeIntervalMicroSeconds &);
            
        private:
        	void doVolumeChanged();
        	
        Q_SIGNALS:
			void tick(qint64 time);
            void stateChanged(Phonon::State oldState,
                              Phonon::State newState);
        
        private Q_SLOTS:
			/**
			 * Receives signal from m_tickTimer
			 */
			void tick();

        private:
        	PrivateState				m_state;
        	Phonon::ErrorType			m_error;
        	
        	/**
        	 * This flag is set to true if play is called when the object is
        	 * in a Loading state.  Once loading is complete, playback will 
        	 * be started.
        	 */
        	bool						m_playPending;
        	
        	QScopedPointer<QTimer>		m_tickTimer;
        	
        	qreal						m_volume;
            int							m_mmfMaxVolume;
            
            MediaSource					m_source;
            MediaSource					m_nextSource;
  
        };
    }
}

QT_END_NAMESPACE

#endif

