/*****************************************************************************
 * infopanels.cpp : Panels for the information dialogs
 ****************************************************************************
 * Copyright (C) 2006-2007 the VideoLAN team
 * $Id$
 *
 * Authors: Clément Stenac <zorglub@videolan.org>
 *          Jean-Baptiste Kempf <jb@videolan.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include "qt4.hpp"
#include "components/infopanels.hpp"

#include <QTreeWidget>
#include <QListView>
#include <QPushButton>
#include <QHeaderView>
#include <QList>
#include <QStringList>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>
#include <QTabWidget>

/************************************************************************
 * Single panels
 ************************************************************************/

/**
 * First Panel - Meta Info
 * All the usual MetaData are displayed and can be changed.
 **/
MetaPanel::MetaPanel( QWidget *parent,
                      intf_thread_t *_p_intf )
                      : QWidget( parent ), p_intf( _p_intf )
{
    QGridLayout *l = new QGridLayout( this );
    int line = 0; /* Counter for GridLayout */
    p_input = NULL;

#define ADD_META( string, widget ) {                             \
    l->addWidget( new QLabel( qtr( string ) + " :" ), line, 0 ); \
    widget = new QLineEdit;                                      \
    l->addWidget( widget, line, 1, 1, 5 );                       \
    line++;            }

    /* ART_URL */
    art_cover = new QLabel( "" );
    art_cover->setMinimumHeight( 128 );
    art_cover->setMinimumWidth( 128 );
    art_cover->setMaximumHeight( 128 );
    art_cover->setMaximumWidth( 128 );
    art_cover->setScaledContents( true );
    art_cover->setPixmap( QPixmap( ":/noart.png" ) );
    l->addWidget( art_cover, line, 6, 4, 2 );

    /* Title, artist and album*/
    ADD_META( VLC_META_TITLE, title_text ); /* OK */
    ADD_META( VLC_META_ARTIST, artist_text ); /* OK */
    ADD_META( VLC_META_COLLECTION, collection_text ); /* OK */

    /* Genre Name */
    /* FIXME List id3genres.h is not includable yet ? */
    genre_text = new QLineEdit;
    l->addWidget( new QLabel( qtr( VLC_META_GENRE ) + " :" ), line, 0 );
    l->addWidget( genre_text, line, 1, 1, 2 );

    /* Number */
    l->addWidget( new QLabel( qtr( "Track Number" )  + " :" ),
                  line, 3 );
    seqnum_text = new QLineEdit;
    seqnum_text->setInputMask("0000");
    seqnum_text->setAlignment( Qt::AlignRight );
    l->addWidget( seqnum_text, line, 4, 1, 2 );
    line++;

    /* Date (Should be in years) */
    date_text = new QLineEdit;
    date_text->setInputMask("0000");
    date_text->setAlignment( Qt::AlignRight );
    l->addWidget( new QLabel( qtr( VLC_META_DATE ) + " :" ), line, 0 );
    l->addWidget( date_text, line, 1, 1, 1 );

    /* Rating */

    l->addWidget( new QLabel( qtr( VLC_META_RATING ) + " :" ), line, 2 );
    rating_text = new QSpinBox; setSpinBounds( rating_text) ;
    l->addWidget( rating_text, line, 3, 1, 1 );

    /* Now Playing ? */
//    ADD_META( VLC_META_NOW_PLAYING, nowplaying_text );

    /* Language and settings */
    l->addWidget( new QLabel( qfu( VLC_META_LANGUAGE ) + " :" ), line, 4 );
    language_text = new QLineEdit;
    l->addWidget( language_text, line, 5, 1, 1 );
/*    l->addWidget( new QLabel( qtr( VLC_META_SETTING ) + " :" ), line, 5 );
    setting_text = new QLineEdit;
    l->addWidget( setting_text, line, 6, 1, 4 );
    line++;*/

/* useless metadata
#define ADD_META_2( string, widget ) {                             \
    l->addWidget( new QLabel( qtr( string ) + " :" ), line, 0 ); \
    widget = new QLineEdit;                                      \
    l->addWidget( widget, line, 1, 1, 7 );                       \
    line++;            }

    ADD_META_2( VLC_META_COPYRIGHT, copyright_text );
    ADD_META_2( VLC_META_PUBLISHER, publisher_text );

    ADD_META_2( VLC_META_ENCODED_BY, encodedby_text );
    ADD_META_2( VLC_META_DESCRIPTION, description_text );
*/
    /*  ADD_META( TRACKID )  Useless ? */
    /*  ADD_URI - DO not show it, done outside */

#undef ADD_META
//#undef ADD_META_2


    CONNECT( title_text, textEdited( QString ), this, editMeta( QString ) );
//    CONNECT( description_text, textEdited( QString ), this, editMeta( QString ) );
    CONNECT( artist_text, textEdited( QString ), this, editMeta( QString ) );
    CONNECT( collection_text, textEdited( QString ), this, editMeta( QString ) );
    CONNECT( genre_text, textEdited( QString ), this, editMeta( QString ) );
    CONNECT( date_text, textEdited( QString ), this, editMeta( QString ) );
    CONNECT( seqnum_text, textEdited( QString ), this, editMeta( QString ) );
/*    CONNECT( rating_text, valueChanged( QString ), this, editMeta( QString ) );*/
    in_edit = false;
}

MetaPanel::~MetaPanel(){}

/**
 * Save the MetaData, triggered by parent->save Button
 **/
void MetaPanel::saveMeta()
{
    playlist_t *p_playlist;
    char psz[5];

    meta_export_t p_export;
    p_export.p_item = p_input;

    if( p_input == NULL )
        return;

    /* we can write meta data only in a file */
    vlc_mutex_lock( &p_input->lock );
    int i_type = p_input->i_type;
    vlc_mutex_unlock( &p_input->lock );
    if( i_type == ITEM_TYPE_FILE )
    {
        char *psz_uri_orig = input_item_GetURI( p_input );
        char *psz_uri = psz_uri_orig;
        if( !strncmp( psz_uri, "file://", 7 ) )
            psz_uri += 7; /* strlen("file://") = 7 */

        p_export.psz_file = strndup( psz_uri, PATH_MAX );
        free( psz_uri_orig );
    }
    else
        return;

    /* now we read the modified meta data */
    input_item_SetArtist( p_input, qtu( artist_text->text() ) );
    input_item_SetAlbum(  p_input, qtu( collection_text->text() ) );
    input_item_SetGenre(  p_input, qtu( genre_text->text() ) );

    input_item_SetDate(  p_input, qtu( date_text->text() ) );

    input_item_SetTrackNum(  p_input, qtu( seqnum_text->text() ) );

    input_item_SetTitle(  p_input, qtu( title_text->text() ) );

    p_playlist = pl_Yield( p_intf );

    PL_LOCK;
    p_playlist->p_private = &p_export;

    module_t *p_mod = module_Need( p_playlist, "meta writer", NULL, 0 );
    if( p_mod )
        module_Unneed( p_playlist, p_mod );
    PL_UNLOCK;
    pl_Release( p_playlist );
    in_edit = false;
}

void MetaPanel::editMeta( QString edit )
{
    in_edit = true;
    emit editing();
}

void MetaPanel::setEdit( bool editing )
{
   in_edit = editing;
}

void MetaPanel::setInput( input_item_t *input )
{
    if( in_edit ) return;

    p_input = input;
}

/**
 * Update all the MetaData and art on an "item-changed" event
 **/
void MetaPanel::update( input_item_t *p_item )
{
    if( in_edit ) return;
    char *psz_meta; 
#define UPDATE_META( meta, widget ) {               \
    psz_meta = input_item_Get##meta( p_item );      \
    if( !EMPTY_STR( psz_meta ) )                    \
        widget->setText( qfu( psz_meta ) );         \
    else                                            \
        widget->setText( "" ); }                    \
    free( psz_meta );

#define UPDATE_META_INT( meta, widget ) {           \
    psz_meta = input_item_Get##meta( p_item );      \
    if( !EMPTY_STR( psz_meta ) )                    \
        widget->setValue( atoi( psz_meta ) ); }     \
    free( psz_meta );


    /* Name / Title */
    psz_meta = input_item_GetTitle( p_item );
    char *psz_name = input_item_GetName( p_item );
    if( !EMPTY_STR( psz_meta ) )
        title_text->setText( qfu( psz_meta ) );
    else if( !EMPTY_STR( psz_name ) )
        title_text->setText( qfu( psz_name ) );
    else title_text->setText( "" );
    free( psz_meta );
    free( psz_name );

    /* URL / URI */
    psz_meta = input_item_GetURL( p_item );
    if( !EMPTY_STR( psz_meta ) )
    {
        emit uriSet( QString( psz_meta ) );
        free( psz_meta );
    }
    else
    {
        free( psz_meta );
        psz_meta = input_item_GetURI( p_item );
        if( !EMPTY_STR( psz_meta ) )
            emit uriSet( QString( psz_meta ) );
    }

    /* Other classic though */
    UPDATE_META( Artist, artist_text );
    UPDATE_META( Genre, genre_text );
//    UPDATE_META( Copyright, copyright_text );
    UPDATE_META( Album, collection_text );
//    UPDATE_META( Description, description_text );
    UPDATE_META( Language, language_text );
//    UPDATE_META( NowPlaying, nowplaying_text );
//    UPDATE_META( Publisher, publisher_text );
//    UPDATE_META( Setting, setting_text );
//    UPDATE_META( EncodedBy, encodedby_text );

    UPDATE_META( Date, date_text );
    UPDATE_META( TrackNum, seqnum_text );
    UPDATE_META_INT( Rating, rating_text );

#undef UPDATE_META_INT
#undef UPDATE_META

    /* Art Urls */
    psz_meta = input_item_GetArtURL( p_item );
    if( psz_meta && !strncmp( psz_meta, "file://", 7 ) )
    {
        QString artUrl = qfu( psz_meta ).replace( "file://",QString("" ) );
        art_cover->setPixmap( QPixmap( artUrl ) );
    }
    else
        art_cover->setPixmap( QPixmap( ":/noart.png" ) );
    free( psz_meta );
}

/*
 * Clear all the metadata widgets
 * Unused yet
 */
void MetaPanel::clear(){}

/**
 * Second Panel - Shows the extra metadata in a tree, non editable.
 **/
ExtraMetaPanel::ExtraMetaPanel( QWidget *parent,
                                intf_thread_t *_p_intf )
                                : QWidget( parent ), p_intf( _p_intf )
{
     QGridLayout *layout = new QGridLayout(this);

     QLabel *topLabel = new QLabel( qtr( "Extra metadata and other information"
                 " are shown in this list.\n" ) );
     topLabel->setWordWrap( true );
     layout->addWidget( topLabel, 0, 0 );

     extraMetaTree = new QTreeWidget( this );
     extraMetaTree->setAlternatingRowColors( true );
     extraMetaTree->setColumnCount( 2 );
     extraMetaTree->header()->hide();
/*     QStringList headerList = ( QStringList() << qtr( "Type" )
 *                                             << qtr( "Value" ) );
 * Useless, add this header if you think it would help the user          **
 */

     layout->addWidget( extraMetaTree, 1, 0 );
}

/**
 * Update the Extra Metadata from p_meta->i_extras 
 **/
void ExtraMetaPanel::update( input_item_t *p_item )
{
    vlc_mutex_lock( &p_item->lock );
    vlc_meta_t *p_meta = p_item->p_meta;
    if( !p_meta )
        return;
    QStringList tempItem;

    QList<QTreeWidgetItem *> items;
    vlc_dictionary_t * p_dict = &p_meta->extra_tags;
    char ** ppsz_allkey = vlc_dictionary_all_keys( p_dict );
    for (int i = 0; ppsz_allkey[i] ; i++ )
    {
        const char * psz_value = (const char *)vlc_dictionary_value_for_key(
                p_dict, ppsz_allkey[i] );
        tempItem.append( qfu( ppsz_allkey[i] ) + " : ");
        tempItem.append( qfu( psz_value ) );
        items.append( new QTreeWidgetItem ( extraMetaTree, tempItem ) );
        free( ppsz_allkey[i] );
    }
    vlc_mutex_unlock( &p_item->lock );
    free( ppsz_allkey );
    extraMetaTree->addTopLevelItems( items );
}

/**
 * Clear the ExtraMetaData Tree
 **/
void ExtraMetaPanel::clear()
{
    extraMetaTree->clear();
}

/**
 * Third panel - Stream info
 * Display all codecs and muxers info that we could gather.
 **/
InfoPanel::InfoPanel( QWidget *parent,
                      intf_thread_t *_p_intf )
                      : QWidget( parent ), p_intf( _p_intf )
{
     QGridLayout *layout = new QGridLayout(this);

     QList<QTreeWidgetItem *> items;

     QLabel *topLabel = new QLabel( qtr( "Information about what your media or"
              " stream is made of.\n Muxer, Audio and Video Codecs, Subtitles "
              "are shown." ) );
     topLabel->setWordWrap( true );
     layout->addWidget( topLabel, 0, 0 );

     InfoTree = new QTreeWidget(this);
     InfoTree->setColumnCount( 1 );
     InfoTree->header()->hide();
     layout->addWidget(InfoTree, 1, 0 );
}

InfoPanel::~InfoPanel()
{
}

/**
 * Update the Codecs information on parent->update()
 **/
void InfoPanel::update( input_item_t *p_item)
{
    InfoTree->clear();
    QTreeWidgetItem *current_item = NULL;
    QTreeWidgetItem *child_item = NULL;

    for( int i = 0; i< p_item->i_categories ; i++)
    {
        current_item = new QTreeWidgetItem();
        current_item->setText( 0, qfu(p_item->pp_categories[i]->psz_name) );
        InfoTree->addTopLevelItem( current_item );

        for( int j = 0 ; j < p_item->pp_categories[i]->i_infos ; j++ )
        {
            child_item = new QTreeWidgetItem ();
            child_item->setText( 0,
                    qfu(p_item->pp_categories[i]->pp_infos[j]->psz_name)
                    + ": "
                    + qfu(p_item->pp_categories[i]->pp_infos[j]->psz_value));

            current_item->addChild(child_item);
        }
         InfoTree->setItemExpanded( current_item, true);
    }
}

/**
 * Clear the tree
 **/
void InfoPanel::clear()
{
    InfoTree->clear();
}

/**
 * Save all the information to a file
 * Not yet implemented.
 **/
/*
void InfoPanel::saveCodecsInfo()
{

}
*/

/**
 * Fourth Panel - Stats
 * Displays the Statistics for reading/streaming/encoding/displaying in a tree
 */
InputStatsPanel::InputStatsPanel( QWidget *parent,
                                  intf_thread_t *_p_intf )
                                  : QWidget( parent ), p_intf( _p_intf )
{
     QGridLayout *layout = new QGridLayout(this);

     QList<QTreeWidgetItem *> items;

     QLabel *topLabel = new QLabel( qtr( "Various statistics about the current"
                 " media or stream.\n Played and streamed info are shown." ) );
     topLabel->setWordWrap( true );
     layout->addWidget( topLabel, 0, 0 );

     StatsTree = new QTreeWidget(this);
     StatsTree->setColumnCount( 3 );
     StatsTree->header()->hide();

#define CREATE_TREE_ITEM( itemName, itemText, itemValue, unit ) {              \
    itemName =                                                                 \
      new QTreeWidgetItem((QStringList () << itemText << itemValue << unit )); \
    itemName->setTextAlignment( 1 , Qt::AlignRight ) ; }

#define CREATE_CATEGORY( catName, itemText ) {                           \
    CREATE_TREE_ITEM( catName, itemText , "", "" );                      \
    catName->setExpanded( true );                                        \
    StatsTree->addTopLevelItem( catName );    }

#define CREATE_AND_ADD_TO_CAT( itemName, itemText, itemValue, catName, unit ){ \
    CREATE_TREE_ITEM( itemName, itemText, itemValue, unit );                   \
    catName->addChild( itemName ); }

    /* Create the main categories */
    CREATE_CATEGORY( input, qtr("Input") );
    CREATE_CATEGORY( video, qtr("Video") );
    CREATE_CATEGORY( streaming, qtr("Streaming") );
    CREATE_CATEGORY( audio, qtr("Audio") );

    CREATE_AND_ADD_TO_CAT( read_media_stat, qtr("Read at media"),
                           "0", input , "kB" );
    CREATE_AND_ADD_TO_CAT( input_bitrate_stat, qtr("Input bitrate"),
                           "0", input, "kb/s" );
    CREATE_AND_ADD_TO_CAT( demuxed_stat, qtr("Demuxed"), "0", input, "kB") ;
    CREATE_AND_ADD_TO_CAT( stream_bitrate_stat, qtr("Stream bitrate"),
                           "0", input, "kb/s" );

    CREATE_AND_ADD_TO_CAT( vdecoded_stat, qtr("Decoded blocks"),
                           "0", video, "" );
    CREATE_AND_ADD_TO_CAT( vdisplayed_stat, qtr("Displayed frames"),
                           "0", video, "" );
    CREATE_AND_ADD_TO_CAT( vlost_frames_stat, qtr("Lost frames"),
                           "0", video, "" );

    CREATE_AND_ADD_TO_CAT( send_stat, qtr("Sent packets"), "0", streaming, "" );
    CREATE_AND_ADD_TO_CAT( send_bytes_stat, qtr("Sent bytes"),
                           "0", streaming, "kB" );
    CREATE_AND_ADD_TO_CAT( send_bitrate_stat, qtr("Sent bitrates"),
                           "0", streaming, "kb/s" );

    CREATE_AND_ADD_TO_CAT( adecoded_stat, qtr("Decoded blocks"),
                           "0", audio, "" );
    CREATE_AND_ADD_TO_CAT( aplayed_stat, qtr("Played buffers"),
                           "0", audio, "" );
    CREATE_AND_ADD_TO_CAT( alost_stat, qtr("Lost buffers"), "0", audio, "" );

    input->setExpanded( true );
    video->setExpanded( true );
    streaming->setExpanded( true );
    audio->setExpanded( true );

    StatsTree->resizeColumnToContents( 0 );
    StatsTree->setColumnWidth( 1 , 100 );

    layout->addWidget(StatsTree, 1, 0 );
}

InputStatsPanel::~InputStatsPanel()
{
}

/**
 * Update the Statistics
 **/
void InputStatsPanel::update( input_item_t *p_item )
{
    vlc_mutex_lock( &p_item->p_stats->lock );

#define UPDATE( widget, format, calc... ) \
    { QString str; widget->setText( 1 , str.sprintf( format, ## calc ) );  }

    UPDATE( read_media_stat, "%8.0f",
            (float)(p_item->p_stats->i_read_bytes)/1000);
    UPDATE( input_bitrate_stat, "%6.0f",
                    (float)(p_item->p_stats->f_input_bitrate * 8000 ));
    UPDATE( demuxed_stat, "%8.0f",
                    (float)(p_item->p_stats->i_demux_read_bytes)/1000 );
    UPDATE( stream_bitrate_stat, "%6.0f",
                    (float)(p_item->p_stats->f_demux_bitrate * 8000 ));

    /* Video */
    UPDATE( vdecoded_stat, "%5i", p_item->p_stats->i_decoded_video );
    UPDATE( vdisplayed_stat, "%5i", p_item->p_stats->i_displayed_pictures );
    UPDATE( vlost_frames_stat, "%5i", p_item->p_stats->i_lost_pictures );

    /* Sout */
    UPDATE( send_stat, "%5i", p_item->p_stats->i_sent_packets );
    UPDATE( send_bytes_stat, "%8.0f",
            (float)(p_item->p_stats->i_sent_bytes)/1000 );
    UPDATE( send_bitrate_stat, "%6.0f",
            (float)(p_item->p_stats->f_send_bitrate*8)*1000 );

    /* Audio*/
    UPDATE( adecoded_stat, "%5i", p_item->p_stats->i_decoded_audio );
    UPDATE( aplayed_stat, "%5i", p_item->p_stats->i_played_abuffers );
    UPDATE( alost_stat, "%5i", p_item->p_stats->i_lost_abuffers );

    vlc_mutex_unlock(& p_item->p_stats->lock );
}

void InputStatsPanel::clear()
{
}


