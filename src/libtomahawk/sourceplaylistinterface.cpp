/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2010-2011, Christian Muehlhaeuser <muesli@tomahawk-player.org>
 *
 *   Tomahawk is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Tomahawk is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Tomahawk. If not, see <http://www.gnu.org/licenses/>.
 */

#include "sourceplaylistinterface.h"

#include "source.h"
#include "pipeline.h"

#include "utils/logger.h"

using namespace Tomahawk;


SourcePlaylistInterface::SourcePlaylistInterface( Tomahawk::Source *source )
    : PlaylistInterface()
    , m_source( source )
    , m_currentItem( 0 )
    , m_gotNextItem( false )
{
    if ( !m_source.isNull() )
        connect( m_source.data(), SIGNAL( playbackStarted( const Tomahawk::query_ptr& ) ), SLOT( onSourcePlaybackStarted( const Tomahawk::query_ptr& ) ) );
}


SourcePlaylistInterface::~SourcePlaylistInterface()
{
    m_source.clear();
}


Tomahawk::result_ptr
SourcePlaylistInterface::siblingItem( int itemsAway )
{
    Q_UNUSED( itemsAway );
    return nextItem();
}


Tomahawk::result_ptr
SourcePlaylistInterface::nextItem()
{
    tDebug( LOGEXTRA ) << Q_FUNC_INFO;
    if ( m_source.isNull() || m_source.data()->currentTrack().isNull() || m_source.data()->currentTrack()->results().isEmpty() )
    {
        tDebug( LOGEXTRA ) << Q_FUNC_INFO << " Results were empty for current track or source no longer valid";
        m_currentItem = Tomahawk::result_ptr();
        return m_currentItem;
    }
    else if ( !m_gotNextItem )
    {
        tDebug( LOGEXTRA ) << Q_FUNC_INFO << " This song was already fetched";
        return Tomahawk::result_ptr();
    }

    m_gotNextItem = false;
    m_currentItem = m_source.data()->currentTrack()->results().first();
    return m_currentItem;
}


result_ptr
SourcePlaylistInterface::currentItem() const
{
    return m_currentItem;
}


bool
SourcePlaylistInterface::hasNextItem()
{
    tDebug( LOGEXTRA ) << Q_FUNC_INFO;
    if ( m_source.isNull() || m_source.data()->currentTrack().isNull() || m_source.data()->currentTrack()->results().isEmpty() )
        return false;

    return m_gotNextItem;
}


QList<Tomahawk::query_ptr>
SourcePlaylistInterface::tracks()
{
    QList<Tomahawk::query_ptr> tracks;
    return tracks; // FIXME
}


QWeakPointer< Tomahawk::Source >
SourcePlaylistInterface::source() const
{
    return m_source;
}


void
SourcePlaylistInterface::reset()
{
    if ( !m_currentItem.isNull() )
        m_gotNextItem = true;
    else
        m_gotNextItem = false;
}


void
SourcePlaylistInterface::onSourcePlaybackStarted( const Tomahawk::query_ptr& query )
{
    tDebug( LOGEXTRA ) << Q_FUNC_INFO;
    connect( query.data(), SIGNAL( resolvingFinished( bool ) ), SLOT( resolvingFinished( bool ) ) );
    Pipeline::instance()->resolve( query, true );
    m_gotNextItem = true;
}


void
SourcePlaylistInterface::resolvingFinished( bool hasResults )
{
    tDebug( LOGEXTRA ) << Q_FUNC_INFO << " and has results? : " << (hasResults ? "true" : "false");
    if ( hasResults )
        emit nextTrackReady();
}
