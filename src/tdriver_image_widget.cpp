/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (testabilitydriver@nokia.com)
**
** This file is part of Testability Driver.
**
** If you have questions regarding the use of this file, please contact
** Nokia at testabilitydriver@nokia.com .
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/


#include <QPoint>

#include <tdriver_tabbededitor.h>

#include "tdriver_main_window.h"

void MainWindow::connectImageWidgetSignals() {

    connect( imageWidget, SIGNAL( forceRefresh()), this, SLOT(forceRefreshData()));

    connect( imageWidget, SIGNAL( imageTapCoords() ), this, SLOT( clickedImage() ) );
    connect( imageWidget, SIGNAL( imageInspectCoords( ) ), this, SLOT( imageInspectFindItem() ) );
    connect( imageWidget, SIGNAL( imageInsertCoordsAtClick()), this, SLOT( imageInsertCoords() ) );
    connect( imageWidget, SIGNAL( imageInsertObjectAtClick()), this, SLOT( imageInsertFindItem() ) );

    connect( imageWidget, SIGNAL( imageInsertObjectById(int)), this, SLOT(imageInsertObjectFromId(int)));
    connect( imageWidget, SIGNAL( imageInspectById(int)), this, SLOT(imageInspectFromId(int)));
    connect( imageWidget, SIGNAL( imageTapById(int)), this, SLOT(imageTapFromId(int)));
}


// This function is called always when main Window is resize. The image size is
// controlled by setting a minimum allowed size to it: properties/method widget
// adjusts itself according to it
void  MainWindow::resizeEvent( QResizeEvent *event ) {

    Q_UNUSED( event );

    imageWidget->setMinimumSize( QSize( width() / 5, height() / 4 ) );
}



// This is triggered from ImageWidget when hoovering on image - gets x,y from imagewidget and searches for item.
// If an item is found it is higlighted
 void MainWindow::imageInspectFindItem()
 {
    QRect area(0, 0, imageWidget->imageWidth()-1, imageWidget->imageHeight()-1);
    QPoint pos(imageWidget->getPressedPos());

    if (area.isValid() && area.contains(pos)) {

        highlightAtCoords( pos, true );
        if ( activeDevice.value( "type" ).toLower() == "s60" ) imageWidget->convertS60Pos(pos);
        statusbar( "x: " + QString::number(pos.x()) + ", y: " + QString::number(pos.y()), 2000 );
    }
}


void MainWindow::imageInsertFindItem()
{
    QRect area(0, 0, imageWidget->imageWidth()-1, imageWidget->imageHeight()-1);
    QPoint pos(imageWidget->getPressedPos());

    if (area.isValid() && area.contains(pos)) {

        highlightAtCoords( pos, true, "tap" );
        if ( activeDevice.value( "type" ).toLower() == "s60" ) imageWidget->convertS60Pos(pos);
        statusbar( "x: " + QString::number(pos.x()) + ", y: " + QString::number(pos.y()), 2000 );
    }
}


void MainWindow::imageInsertCoords()
{
    QPoint pos = imageWidget->getPressedPos();
    //emit insertToCodeEditor(QString("@sut.tap_screen(%1, %2)\n").arg(pos.x()).arg(pos.y()), false, false);
    emit insertToCodeEditor(QString(" %1, %2 ").arg(pos.x()).arg(pos.y()), false, false);
}


void MainWindow::imageInsertObjectFromId(int id)
{
    qDebug() << __FUNCTION__ << id;
    highlightById(id, true, "");
}


void MainWindow::imageInspectFromId(int id)
{
    qDebug() << __FUNCTION__ << id;
    highlightById(id, true);
}


void MainWindow::imageTapFromId(int id)
{
    qDebug() << __FUNCTION__ << id;

    if ( highlightById( id, false ) && lastHighlightedObjectPtr != 0 ) {
        QHash<QString, QString> treeItemData = objectTreeData.value( lastHighlightedObjectPtr );
        tapScreen( "tap " + treeItemData.value( "type" ) + "(:id=>" + treeItemData.value( "id" ) + ") " + currentApplication.value("id") );
        highlightById( id, true );
    }
    else {
        QMessageBox::critical( 0, "Tap to screen", "No object found with id " + QString::number( id));
    }
}


void MainWindow::tapScreen( QString target )
{
    //qDebug() << "tapScreen";
    statusbar( "Tapping...", 0, 1 );
    typedef QList<QByteArray> QByteArrayList;

    if ( !execute_command( commandTapScreen, QString( activeDevice.value( "name" ) + " " + target ))) {
        statusbar( "Error: Failed to tap the screen", 1000 );
    }

    else {
        statusbar( "Tapping...", 1000 );
        refreshData();
    }
}

// Image has been clicked - fetch X, Y from imageWidget, and
// if S60 without Qt, send tap_screen command to coordinates,
// else try to tap object that was clicked
void MainWindow::clickedImage()
{
    QPoint pos(imageWidget->getPressedPos());

    if ( activeDevice.value( "type" ).toLower() == "s60" ) {
        imageWidget->convertS60Pos(pos);
        tapScreen( "tap_screen " + QString::number( pos.x() ) + " " + QString::number( pos.y() ) );
    }

    else {
        if ( highlightAtCoords( pos, false ) && lastHighlightedObjectPtr != 0 ) {
            QHash<QString, QString> treeItemData = objectTreeData.value( lastHighlightedObjectPtr );
            tapScreen( "tap " + treeItemData.value( "type" ) + "(:id=>" + treeItemData.value( "id" ) + ") " + currentApplication.value("id") );
            highlightAtCoords( pos, true );
        }

        else {
            QMessageBox::critical( 0, "Tap to screen", "No object found at coordinates x: " + QString::number( pos.x() ) + ", y: " + QString::number( pos.y() ) );
        }
    }
}


void MainWindow::drawHighlight( int itemPtr ) {

    //qDebug() << "drawHighlight";

    if ( itemPtr != 0 && itemPtr != lastHighlightedObjectPtr ) {

        QMap<QString, QHash<QString, QString> > attributes = attributesMap.value( itemPtr );

        // collect geometries for item and its childs
        QStringList geometries = geometriesMap.value( itemPtr );

        if ( !geometries.isEmpty() ) {

            lastHighlightedObjectPtr = itemPtr;

            if ( geometries.size() > 1 ) {

                imageWidget->drawMultipleHighlights( geometries, 0, 0 );

            } else if ( geometries.size() == 1 ) {

                QStringList geometry = geometries[ 0 ].split( "," );

                float x      = geometry[ 0 ].toFloat();
                float y      = geometry[ 1 ].toFloat();
                float width  = geometry[ 2 ].toFloat();
                float height = geometry[ 3 ].toFloat();

                imageWidget->drawHighlight( x, y, width, height );

            } else {

                // no geometries, disable highlighting
                imageWidget->disableDrawHighlight();
                lastHighlightedObjectPtr = 0;

            }

        } else {

            //qDebug() << "no geometry for object: " << itemPtr;
            imageWidget->disableDrawHighlight();
            lastHighlightedObjectPtr = 0;

        }

    }

}


bool MainWindow::splitGeometry( QString geometry, QStringList & geometryList ) {

    bool result = false;

    QRegExp regexp("^(.+),(.+),(.+),(.+)$");

    geometryList.clear();

    if ( regexp.indexIn( geometry ) != -1 )  {

        geometryList << regexp.cap( 1 ) << regexp.cap( 2 ) << regexp.cap( 3 ) << regexp.cap( 4 );
        result = true;

    } else {

        qDebug() << "splitGeometry: Unable to split geometry '" << geometry << "'";

    }

    return result;

}

bool MainWindow::geometryContains( QString geometry, int x, int y ) {

    return convertGeometryToQRect( geometry ).contains( x, y, false );

}

bool MainWindow::collectMatchingVisibleObjects( QPoint pos, QList<int> & matchingObjects ) {

    //qDebug() << "collectMatchingVisibleObjects";

    bool result = false;

    matchingObjects.clear();

    QList<int>::const_iterator visibleObject;

    for ( visibleObject = visibleObjectsList.constBegin(); visibleObject != visibleObjectsList.constEnd(); ++visibleObject ) {

        QStringList geometries = geometriesMap.value( *visibleObject );

        if ( !geometries.isEmpty() ) {

            if ( geometryContains ( geometries.at( 0 ), pos.x(), pos.y() ) ) {
                matchingObjects << int( *visibleObject );
                result = true;
            }

        }

    }

    return result;

}

QRect MainWindow::convertGeometryToQRect( QString geometry ) {

    QRect result( 0, 0, 0, 0 );

    QStringList geometryList;

    if ( splitGeometry( geometry, geometryList ) ) {

        result = QRect( geometryList[ 0 ].toFloat(), geometryList[ 1 ].toFloat(), geometryList[ 2 ].toFloat(), geometryList[ 3 ].toFloat() );

    }

    return result;

}

int MainWindow::geometrySize( QString geometry ) {

    QRect rectangle = convertGeometryToQRect( geometry );

    return rectangle.size().width() * rectangle.size().height();

}

bool MainWindow::getSmallestObjectFromMatches( QList<int> *matchingObjects, int & objectPtr ) {

    //qDebug() << "getSmallestObjectFromMatches";

    bool result = false;
    int smallestObjectSize = -1;
    int smallestObjectPtr = -1;

    QList<int>::const_iterator matchingObject;
    for ( matchingObject = matchingObjects->constBegin(); matchingObject != matchingObjects->constEnd(); ++matchingObject ) {

        QStringList geometries = geometriesMap.value( *matchingObject );

        if ( !geometries.isEmpty() ) {

            // calculate size of object in pixels
            int matchingObjectSize = geometrySize( geometries.at( 0 ) );

            if ( ( smallestObjectPtr == -1 || matchingObjectSize < smallestObjectSize ) && matchingObjectSize > 0 ) {
                smallestObjectPtr = int( *matchingObject );
                smallestObjectSize = matchingObjectSize;
                result = true;
            }
        }
    }

    objectPtr = smallestObjectPtr;
    return result;
}


bool MainWindow::highlightById( int id, bool selectItem, QString insertMethodToEditor )
{
    QTreeWidgetItem *item = objectTreeMap.value( id );
    if (item == NULL) {
        return false;
    }
    else {
        drawHighlight( ( int )( item ) );

        // select item from object tree if selectItem is true
        if ( selectItem ) {
            objectTree->scrollToItem( item );
            objectTree->setCurrentItem( item );
        }

        if (!insertMethodToEditor.isNull()) {
            objectViewItemAction(item, 0, insertAction, insertMethodToEditor);
        }

        return true;
    }
}

bool MainWindow::highlightAtCoords( QPoint pos, bool selectItem, QString insertMethodToEditor ) {

    //qDebug() << __FUNCTION__;

    bool result = false;

    QList<int> matchingObjects;

    if ( collectMatchingVisibleObjects( pos, matchingObjects ) ) {

        qSort( matchingObjects.begin(), matchingObjects.end() );

        int matchingObject = -1;

        if ( getSmallestObjectFromMatches( &matchingObjects, matchingObject ) ) {

            result = highlightById(matchingObject, selectItem, insertMethodToEditor);
        }
    }

    return result;
}


