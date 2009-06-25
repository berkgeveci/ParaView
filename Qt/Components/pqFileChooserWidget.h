/*=========================================================================

   Program: ParaView
   Module:    pqFileChooserWidget.h

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/


#ifndef _pqFileChooserWidget_h
#define _pqFileChooserWidget_h

#include "pqComponentsExport.h"
#include <QWidget>
#include <QString>
#include <QStringList>
class QLineEdit;
class pqServer;

/// file chooser widget which consists of a tool button and a line edit
/// hitting the tool button will bring up a file dialog, and the chosen
/// file will be put in the line edit
class PQCOMPONENTS_EXPORT pqFileChooserWidget : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QStringList filenames READ filenames WRITE setFilenames USER true)
  Q_PROPERTY(QString extension READ extension WRITE setExtension)
  Q_PROPERTY(bool useDirectoryMode
             READ useDirectoryMode WRITE setUseDirectoryMode)
  Q_PROPERTY(bool forceSingleFile
             READ forceSingleFile WRITE setForceSingleFile)

public:
  /// constructor
  pqFileChooserWidget(QWidget* p = NULL);
  /// destructor
  ~pqFileChooserWidget();

  /// get the filename
  QStringList filenames();
  /// set the filename
  void setFilenames(const QStringList&);

  /// convienince functions for when using only a single file (see
  /// forceSingleFile property).
  QString singleFilename();
  void setSingleFilename(const QString &);
  
  /// get the file extension for the file dialog
  QString extension();
  /// set the file extension for the file dialog
  void setExtension(const QString&);

  /// flag specifying whether this widget should accept multiple files
  bool forceSingleFile() { return this->ForceSingleFile; }
  void setForceSingleFile(bool flag) {
    this->ForceSingleFile = flag;
    this->setFilenames(this->filenames());
  }

  /// flag specifying whether this widget should use directory mode
  bool useDirectoryMode() { return this->UseDirectoryMode; }
  void setUseDirectoryMode(bool flag) {
    this->UseDirectoryMode = flag;
    this->setFilenames(this->filenames());
  }

  /// set server to work on.
  /// If server is NULL, a local file dialog is used
  void setServer(pqServer* server);
  pqServer* server();

  /// Converts between a list of file names and delimited string of filenames
  /// (which is shown in the line edit box).
  static QStringList splitFilenames(const QString &filesString) {
    return filesString.split(";", QString::SkipEmptyParts);
  }
  static QString joinFilenames(const QStringList &filesList) {
    return filesList.join(";");
  }

signals:
  /// Signal emitted when the filename changes.  The single string version is a
  /// convenience for when you are only grabbing the first file anyway.
  void filenamesChanged(const QStringList&);
  void filenameChanged(const QString&);

protected slots:
  /// Called when the user hits the choose file button.
  void chooseFile();
  /// Takes a string with delimited files and emits the filenamesChanged
  /// signals.  Generally used to convert the textChanged signal of the line
  /// edit to the filenamesChanged signals.
  void emitFilenamesChanged(const QString &fileString);

protected:
  QString Extension;
  QLineEdit* LineEdit;
  pqServer* Server;
  bool ForceSingleFile;
  bool UseDirectoryMode;
};

#endif // _pqFileChooserWidget_h

