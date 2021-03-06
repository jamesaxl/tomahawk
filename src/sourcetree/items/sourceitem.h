/*
 *    Copyright 2010-2011, Leo Franchi <lfranchi@kde.org>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef SOURCE_ITEM_H
#define SOURCE_ITEM_H

#include "sourcetreeitem.h"

class TemporaryPageItem;
class GenericPageItem;
class CategoryItem;

namespace Tomahawk
{
    class ViewPage;
}

class SourceItem : public SourceTreeItem
{
    Q_OBJECT
public:
    SourceItem( SourcesModel* model, SourceTreeItem* parent, const Tomahawk::source_ptr& source );

    virtual QString text() const;
    virtual QIcon icon() const;
    virtual int peerSortValue() const;
    virtual int IDValue() const;

    virtual bool localLatchedOn() const;

    Tomahawk::source_ptr source() const;

    CategoryItem* stationsCategory() const { return m_stations; }
    CategoryItem* playlistsCategory() const { return m_playlists; }
    void setStationsCategory( CategoryItem* item ) { m_stations = item; }
    void setPlaylistsCategory( CategoryItem* item ) { m_playlists = item; }

public slots:
    virtual void activate();

private slots:
    void onPlaylistsAdded( const QList<Tomahawk::playlist_ptr>& playlists );
    void onPlaylistDeleted( const Tomahawk::playlist_ptr& playlists );
    void onAutoPlaylistsAdded( const QList<Tomahawk::dynplaylist_ptr>& playlists );
    void onAutoPlaylistDeleted( const Tomahawk::dynplaylist_ptr& playlists );
    void onStationsAdded( const QList<Tomahawk::dynplaylist_ptr>& stations );
    void onStationDeleted( const Tomahawk::dynplaylist_ptr& stations );

    void latchedOn( const Tomahawk::source_ptr&, const Tomahawk::source_ptr& );
    void latchedOff( const Tomahawk::source_ptr&, const Tomahawk::source_ptr& );

    void requestExpanding();

    Tomahawk::ViewPage* sourceInfoClicked();
    Tomahawk::ViewPage* getSourceInfoPage() const;

    Tomahawk::ViewPage* collectionClicked();
    Tomahawk::ViewPage* getCollectionPage() const;

    Tomahawk::ViewPage* coolPlaylistsClicked();
    Tomahawk::ViewPage* getCoolPlaylistsPage() const;

    Tomahawk::ViewPage* lovedTracksClicked();
    Tomahawk::ViewPage* getLovedTracksPage() const;

private:
    void playlistsAddedInternal( SourceTreeItem* parent, const QList< Tomahawk::dynplaylist_ptr >& playlists );
    template< typename T >
    void playlistDeletedInternal( SourceTreeItem* parent, const T& playlists );

    Tomahawk::source_ptr m_source;
    QPixmap m_superCol, m_defaultAvatar;
    CategoryItem* m_playlists;
    CategoryItem* m_stations;

    bool m_latchedOn;
    Tomahawk::source_ptr m_latchedOnTo;

    GenericPageItem* m_collectionItem;
    GenericPageItem* m_sourceInfoItem;
    GenericPageItem* m_coolPlaylistsItem;
    GenericPageItem* m_lovedTracksItem;

    Tomahawk::ViewPage* m_collectionPage;
    Tomahawk::ViewPage* m_sourceInfoPage;
    Tomahawk::ViewPage* m_coolPlaylistsPage;
    Tomahawk::ViewPage* m_lovedTracksPage;
    Tomahawk::ViewPage* m_whatsHotPage;
};


#endif
