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

#ifndef QUEUEPROXYMODEL_H
#define QUEUEPROXYMODEL_H

#include "playlistproxymodel.h"

#include "dllmacro.h"

class QMetaData;
class TrackView;

class DLLEXPORT QueueProxyModel : public PlaylistProxyModel
{
Q_OBJECT

public:
    explicit QueueProxyModel( TrackView* parent = 0 );
    virtual ~QueueProxyModel();

    virtual Tomahawk::playlistinterface_ptr getPlaylistInterface();

private slots:
    void onIndexActivated( const QModelIndex& index );
    void onTrackCountChanged( unsigned int count );
};

#endif // QUEUEPROXYMODEL_H
