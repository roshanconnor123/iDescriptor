#include <Cocoa/Cocoa.h>
#include <QDebug>
#include <QMainWindow>

void setupMacOSWindow(QMainWindow *window)
{

    if (!window) {
        qWarning() << "setupMacOSWindow: window is null";
        return;
    }

    NSView *nativeView = reinterpret_cast<NSView *>(window->winId());
    NSWindow *nativeWindow = [nativeView window];

    if (!nativeWindow) {
        qWarning() << "setupMacOSWindow: native window is null";
        return;
    }

    qDebug() << "Setting up macOS window styles";

    window->setUnifiedTitleAndToolBarOnMac(true);

    [nativeWindow setStyleMask:[nativeWindow styleMask] |
                               NSWindowStyleMaskFullSizeContentView |
                               NSWindowTitleHidden];
    [nativeWindow setTitleVisibility:NSWindowTitleHidden];
    [nativeWindow setTitlebarAppearsTransparent:YES];

    NSToolbar *toolbar =
        [[NSToolbar alloc] initWithIdentifier:@"HiddenInsetToolbar"];
    toolbar.showsBaselineSeparator =
        NO; // equivalent to HideToolbarSeparator: true
    [nativeWindow setToolbar:toolbar];
    // [toolbar setVisible:NO];
    // todo : is it ok ?
    [toolbar release];
    // [nativeWindow setContentBorderThickness:0.0 forEdge:NSMinYEdge];

    [nativeWindow center];
}