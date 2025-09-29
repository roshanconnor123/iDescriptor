#ifndef DEVICEMENUWIDGET_H
#define DEVICEMENUWIDGET_H
#include "iDescriptor.h"
#include <QStackedWidget>
#include <QWidget>
class DeviceMenuWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceMenuWidget(iDescriptorDevice *device,
                              QWidget *parent = nullptr);
    void switchToTab(const QString &tabName);
    // ~DeviceMenuWidget();
private:
    QStackedWidget *stackedWidget; // Pointer to the stacked widget
    iDescriptorDevice *device;     // Pointer to the iDescriptor device
signals:
};

#endif // DEVICEMENUWIDGET_H
