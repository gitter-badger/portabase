TEMPLATE        = app
CONFIG         += qt warn_on thread
QT             += xml
DESTDIR         = build
OBJECTS_DIR     = build
MOC_DIR         = build
RCC_DIR         = build
VERSION         = 2.0.0
TRANSLATIONS    = resources/translations/portabase_cs.ts \
                  resources/translations/portabase_de.ts \
                  resources/translations/portabase_en.ts \
                  resources/translations/portabase_es.ts \
                  resources/translations/portabase_fr.ts \
                  resources/translations/portabase_ja.ts \
                  resources/translations/portabase_zh_TW.ts

# Files common between all versions
HEADERS         = calc/calcdateeditor.h \
                  calc/calceditor.h \
                  calc/calcnode.h \
                  calc/calcnodeeditor.h \
                  calc/calctimeeditor.h \
                  calc/calcwidget.h \
                  calculator.h \
                  color_picker/qtcolorpicker.h \
                  columneditor.h \
                  columninfo.h \
                  commandline.h \
                  condition.h \
                  conditioneditor.h \
                  csverror.h \
                  csvutils.h \
                  database.h \
                  datamodel.h \
                  datatypes.h \
                  datedialog.h \
                  datewidget.h \
                  dbeditor.h \
                  dynamicedit.h \
                  encryption/blowfish.h \
                  encryption/bytestream.h \
                  encryption/crypto.h \
                  encryption/randomkit/randomkit.h \
                  encryption/randomkit/rk_isaac.h \
                  encryption/randomkit/rk_mt.h \
                  enumeditor.h \
                  enummanager.h \
                  eventfilter.h \
                  factory.h \
                  filter.h \
                  filtereditor.h \
                  formatting.h \
                  image/imageeditor.h \
                  image/imageselector.h \
                  image/imageutils.h \
                  image/imageviewer.h \
                  image/imagewidget.h \
                  image/slideshowdialog.h \
                  importdialog.h \
                  importutils.h \
                  menuactions.h \
                  metakitfuncs.h \
                  mobiledb.h \
                  notebutton.h \
                  noteeditor.h \
                  numberwidget.h \
                  oldconfig.h \
                  passdialog.h \
                  pbdialog.h \
                  pbmaemo5style.h \
                  pdbfile.h \
                  portabase.h \
                  preferences.h \
                  qqutil/qqdialog.h \
                  qqutil/qqhelpbrowser.h \
                  qqutil/qqmainwindow.h \
                  qqutil/qqmenuhelper.h \
                  qqutil/qqtoolbarstretch.h \
                  roweditor.h \
                  rowviewer.h \
                  sorteditor.h \
                  timewidget.h \
                  view.h \
                  viewdisplay.h \
                  vieweditor.h \
                  vsfmanager.h \
                  xmlexport.h \
                  xmlimport.h
SOURCES         = calc/calcdateeditor.cpp \
                  calc/calceditor.cpp \
                  calc/calcnode.cpp \
                  calc/calcnodeeditor.cpp \
                  calc/calctimeeditor.cpp \
                  calc/calcwidget.cpp \
                  calculator.cpp \
                  color_picker/qtcolorpicker.cpp \
                  columneditor.cpp \
                  columninfo.cpp \
                  commandline.cpp \
                  condition.cpp \
                  conditioneditor.cpp \
                  csverror.cpp \
                  csvutils.cpp \
                  database.cpp \
                  datamodel.cpp \
                  datedialog.cpp \
                  datewidget.cpp \
                  dbeditor.cpp \
                  dynamicedit.cpp \
                  encryption/blowfish.cpp \
                  encryption/bytestream.cpp \
                  encryption/crypto.cpp \
                  encryption/randomkit/rk_isaac.c \
                  encryption/randomkit/rk_mt.c \
                  enumeditor.cpp \
                  enummanager.cpp \
                  eventfilter.cpp \
                  factory.cpp \
                  filter.cpp \
                  filtereditor.cpp \
                  formatting.cpp \
                  image/imageeditor.cpp \
                  image/imageselector.cpp \
                  image/imageutils.cpp \
                  image/imageviewer.cpp \
                  image/imagewidget.cpp \
                  image/slideshowdialog.cpp \
                  importdialog.cpp \
                  importutils.cpp \
                  main.cpp \
                  menuactions.cpp \
                  metakitfuncs.cpp \
                  mobiledb.cpp \
                  notebutton.cpp \
                  noteeditor.cpp \
                  numberwidget.cpp \
                  oldconfig.cpp \
                  passdialog.cpp \
                  pbdialog.cpp \
                  pbmaemo5style.cpp \
                  pdbfile.cpp \
                  portabase.cpp \
                  preferences.cpp \
                  qqutil/qqdialog.cpp \
                  qqutil/qqhelpbrowser.cpp \
                  qqutil/qqmainwindow.cpp \
                  qqutil/qqmenuhelper.cpp \
                  qqutil/qqtoolbarstretch.cpp \
                  roweditor.cpp \
                  rowviewer.cpp \
                  sorteditor.cpp \
                  timewidget.cpp \
                  view.cpp \
                  viewdisplay.cpp \
                  vieweditor.cpp \
                  vsfmanager.cpp \
                  xmlexport.cpp \
                  xmlimport.cpp

# Stuff for all Linux/UNIX versions
unix:LIBS       += -lm -lmk4

# Stuff for Mac OS X
macx {
    CONFIG             += release x86 ppc
    TARGET              = PortaBase
    RESOURCES           = resources/mac.qrc
    ICON                = packaging/mac/PortaBase.icns
    DOCUMENT_ICON.files = packaging/mac/PortaBaseFile.icns
    DOCUMENT_ICON.path  = Contents/Resources
    CS_LPROJ.files      = packaging/mac/cs.lproj/InfoPlist.strings \
                          packaging/mac/cs.lproj/locversion.plist
    CS_LPROJ.path       = Contents/Resources/cs.lproj
    DE_LPROJ.files      = packaging/mac/de.lproj/InfoPlist.strings \
                          packaging/mac/de.lproj/locversion.plist
    DE_LPROJ.path       = Contents/Resources/de.lproj
    EN_LPROJ.files      = packaging/mac/en.lproj/InfoPlist.strings \
                          packaging/mac/en.lproj/locversion.plist
    EN_LPROJ.path       = Contents/Resources/en.lproj
    ES_LPROJ.files      = packaging/mac/es.lproj/InfoPlist.strings \
                          packaging/mac/es.lproj/locversion.plist
    ES_LPROJ.path       = Contents/Resources/es.lproj
    FR_LPROJ.files      = packaging/mac/fr.lproj/InfoPlist.strings \
                          packaging/mac/fr.lproj/locversion.plist
    FR_LPROJ.path       = Contents/Resources/fr.lproj
    JA_LPROJ.files      = packaging/mac/ja.lproj/InfoPlist.strings \
                          packaging/mac/ja.lproj/locversion.plist
    JA_LPROJ.path       = Contents/Resources/ja.lproj
    ZH_HANT_LPROJ.files = packaging/mac/zh-Hant.lproj/InfoPlist.strings \
                          packaging/mac/zh-Hant.lproj/locversion.plist
    ZH_HANT_LPROJ.path  = Contents/Resources/zh-Hant.lproj
    QMAKE_BUNDLE_DATA  += DOCUMENT_ICON \
                          CS_LPROJ \
                          DE_LPROJ \
                          EN_LPROJ \
                          ES_LPROJ \
                          FR_LPROJ \
                          JA_LPROJ \
                          ZH_HANT_LPROJ
    QMAKE_INFO_PLIST    = packaging/mac/Info.plist
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
}

# Stuff for Maemo
maemo5|contains(QT_CONFIG, hildon): {
    CONFIG             += debug qdbus
    LIBS               += -L../../src/metakit/builds
    isEmpty(PREFIX) {
        PREFIX          = /usr/local
    }
    DATADIR             = $$PREFIX/share
    DEFINES            += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"
    TARGET              = portabase
    INSTALLS           += target desktop service icon16 icon22 icon32 icon48 icon64 mime
    desktop.path        = $$DATADIR/applications/hildon
    desktop.files       = packaging/maemo/portabase.desktop
    service.path        = $$DATADIR/dbus-1/services
    service.files       = packaging/maemo/net.sourceforge.portabase.service
    icon16.path         = $$DATADIR/icons/hicolor/16x16/apps
    icon16.files        = resources/icons/linux/16x16/portabase.png
    icon22.path         = $$DATADIR/icons/hicolor/22x22/apps
    icon22.files        = resources/icons/linux/22x22/portabase.png
    icon32.path         = $$DATADIR/icons/hicolor/32x32/apps
    icon32.files        = resources/icons/linux/32x32/portabase.png
    icon48.path         = $$DATADIR/icons/hicolor/48x48/apps
    icon48.files        = resources/icons/linux/48x48/portabase.png
    icon64.path         = $$DATADIR/icons/hicolor/64x64/apps
    icon64.files        = resources/icons/linux/64x64/portabase.png
    mime.path           = $$DATADIR/mime/packages
    mime.files          = packaging/maemo/portabase-mime.xml
}
contains(QT_CONFIG, hildon) {
    target.path         = $$PREFIX/bin
    RESOURCES           = resources/diablo.qrc
} else:maemo5 {
    QT                 += dbus maemo5
    target.path         = /opt/maemo
    RESOURCES           = resources/fremantle.qrc
}

# Stuff for other Linux/UNIX platforms
unix:!macx:!maemo5:!contains(QT_CONFIG, hildon): {
    #QMAKE_CXXFLAGS       += -O0 # for valgrind
    CONFIG               += debug
    LIBS                 += -Lmetakit/builds
    RESOURCES             = resources/linux.qrc
}

# Stuff for Windows
win32 {
    CONFIG                 += release
    TARGET                  = PortaBase
    RESOURCES               = resources/windows.qrc
    LIBS                   += c:/portabase/metakit/builds/libmk4.a
    RC_FILE                 = portabase.rc
    INCLUDEPATH            += c:/portabase/metakit/include
}

# Stuff for static builds
static {
    QTPLUGIN           += qtaccessiblewidgets qjpeg
    DEFINES            += STATIC_QT
    QMAKE_CXXFLAGS     += -fvisibility=hidden
}
