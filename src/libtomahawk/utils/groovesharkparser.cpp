/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2010-2011, Leo Franchi <lfranchi@kde.org>
 *   Copyright 2010-2011, Hugo Lindström <hugolm84@gmail.com>
 *   Copyright 2010-2011, Stefan Derkits <stefan@derkits.at>
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

#include "groovesharkparser.h"

#include "utils/logger.h"
#include "utils/tomahawkutils.h"
#include "query.h"
#include "sourcelist.h"
#include "dropjob.h"
#include "jobview/JobStatusView.h"
#include "jobview/JobStatusModel.h"
#include "dropjobnotifier.h"
#include "viewmanager.h"

#include <qjson/parser.h>

#include <QtCrypto>

#include <QCoreApplication>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

using namespace Tomahawk;

QPixmap* GroovesharkParser::s_pixmap = 0;

const char* enApiSecret = "erCj5s0Vebyqtc9Aduyotc1CLListJ9HfO2os5hBeew=";

GroovesharkParser::GroovesharkParser( const QStringList& trackUrls, bool createNewPlaylist, QObject* parent )
    : QObject ( parent )
    , m_limit ( 40 )
    , m_trackMode( true )
    , m_createNewPlaylist( createNewPlaylist )
    , m_browseJob( 0 )
{
    QByteArray magic = QByteArray::fromBase64( enApiSecret );

    QByteArray wand = QByteArray::fromBase64( QCoreApplication::applicationName().toLatin1() );
    int length = magic.length(), n2 = wand.length();
    for ( int i=0; i<length; i++ ) magic[i] = magic[i] ^ wand[i%n2];

    m_apiKey = QCA::SymmetricKey( magic );

    foreach ( const QString& url, trackUrls )
        lookupUrl( url );
}

GroovesharkParser::~GroovesharkParser()
{

}

void
GroovesharkParser::lookupUrl( const QString& link )
{
    if( link.contains( "playlist" ) )
    {
        if( !m_createNewPlaylist )
            m_trackMode = true;
        else
            m_trackMode = false;

        lookupGroovesharkPlaylist( link );
    }
    else
        return;

}


void
GroovesharkParser::lookupGroovesharkPlaylist( const QString& linkRaw )
{
    tLog() << "Parsing Grooveshark Playlist URI:" << linkRaw;

    QStringList urlParts = linkRaw.split( "/" );
    bool ok;
    QString playlistStr = urlParts.last();
    playlistStr.truncate(playlistStr.indexOf("?"));
    int playlistID = playlistStr.toInt( &ok, 10 );
    if (!ok)
    {
        tDebug() << "incorrect grooveshark url";
        return;
    }
    
    m_title = urlParts.at( urlParts.size()-2 );
    
    tDebug() << "should get playlist " << playlistID;
    
    DropJob::DropType type;

    if ( linkRaw.contains( "playlist" ) )
        type = DropJob::Playlist;

    QString base_url( "http://api.grooveshark.com/ws3.php?sig=" );

    QByteArray data = QString( "{\"method\":\"getPlaylistSongs\",\"parameters\":{\"playlistID\":\"%1\"},\"header\":{\"wsKey\":\"tomahawkplayer\"}}" ).arg( playlistID ).toLocal8Bit();
    
    
    
    
    QCA::MessageAuthenticationCode hmac( "hmac(md5)", m_apiKey );

    QCA::SecureArray secdata( data );
    hmac.update(secdata);
    QCA::SecureArray resultArray = hmac.final();
    
    QString hash = QCA::arrayToHex( resultArray.toByteArray() );
    QUrl url = QUrl( base_url + hash );
    
    tDebug() << "Looking up URL..." << url.toString();

    QNetworkReply* reply = TomahawkUtils::nam()->post( QNetworkRequest( url ), data );
    connect( reply, SIGNAL( finished() ), this, SLOT( groovesharkLookupFinished() ) );

    m_browseJob = new DropJobNotifier( pixmap(), "Grooveshark", type, reply );
    JobStatusView::instance()->model()->addJob( m_browseJob );

    m_queries.insert( reply );
}

void
GroovesharkParser::groovesharkLookupFinished()
{
    QNetworkReply* r = qobject_cast< QNetworkReply* >( sender() );
    Q_ASSERT( r );

    m_queries.remove( r );
    r->deleteLater();

    if ( r->error() == QNetworkReply::NoError )
    {
        QJson::Parser p;
        bool ok;
        QVariantMap res = p.parse( r, &ok ).toMap();

        if ( !ok )
        {
            tLog() << "Failed to parse json from Grooveshark browse item :" << p.errorString() << "On line" << p.errorLine();
            checkTrackFinished();
            return;
        }

        QVariantList list = res.value( "result" ).toMap().value( "songs" ).toList();
        foreach (const QVariant& var, list)
        {
            QVariantMap trackResult = var.toMap();
            
            QString title, artist, album;

            title = trackResult.value( "SongName", QString() ).toString();
            artist = trackResult.value( "ArtistName", QString() ).toString();
            album = trackResult.value( "AlbumName", QString() ).toString();

            if ( title.isEmpty() && artist.isEmpty() ) // don't have enough...
            {
                tLog() << "Didn't get an artist and track name from grooveshark, not enough to build a query on. Aborting" << title << artist << album;
                return;
            }

            Tomahawk::query_ptr q = Tomahawk::Query::get( artist, title, album, uuid(), m_trackMode );
            m_tracks << q;
        }
        

    } else
    {
        tLog() << "Error in network request to grooveshark for track decoding:" << r->errorString();
    }

    if ( m_trackMode )
        checkTrackFinished();
    else
        checkPlaylistFinished();
}

void
GroovesharkParser::checkPlaylistFinished()
{
    tDebug() << "Checking for grooveshark batch playlist job finished" << m_queries.isEmpty() << m_createNewPlaylist;
    if ( m_queries.isEmpty() ) // we're done
    {
        if ( m_browseJob )
            m_browseJob->setFinished();

        if( m_createNewPlaylist && !m_tracks.isEmpty() )
        {
            m_playlist = Playlist::create( SourceList::instance()->getLocal(),
                                       uuid(),
                                       m_title,
                                       m_info,
                                       m_creator,
                                       false,
                                       m_tracks );
            connect( m_playlist.data(), SIGNAL( revisionLoaded( Tomahawk::PlaylistRevision ) ), this, SLOT( playlistCreated() ) );
            return;
        }

        
        emit tracks( m_tracks );

        deleteLater();
    }
}

void
GroovesharkParser::checkTrackFinished()
{
    tDebug() << "Checking for grooveshark batch track job finished" << m_queries.isEmpty();
    if ( m_queries.isEmpty() ) // we're done
    {
        if ( m_browseJob )
            m_browseJob->setFinished();

        emit tracks( m_tracks );

        deleteLater();
    }

}

void
GroovesharkParser::playlistCreated()
{

    ViewManager::instance()->show( m_playlist );

    deleteLater();
}


QPixmap
GroovesharkParser::pixmap() const
{
    if ( !s_pixmap )
        s_pixmap = new QPixmap( RESPATH "images/grooveshark.png" );

    return *s_pixmap;
}
