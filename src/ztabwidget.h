#ifndef ZTABWIDGET_H
#define ZTABWIDGET_H

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

class ZTab : public QPushButton
{
    Q_OBJECT

public:
    explicit ZTab(const QString &text, QWidget *parent = nullptr);
    void setIcon(const QIcon &icon);
};

class ZTabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ZTabWidget(QWidget *parent = nullptr);
    void finalizeStyles();
    ZTab *addTab(QWidget *widget, const QString &label);
    void setCurrentIndex(int index);
    int currentIndex() const;
    QWidget *widget(int index) const;

signals:
    void currentChanged(int index);

private slots:
    void onTabClicked();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QHBoxLayout *m_tabLayout;
    QVBoxLayout *m_mainLayout;
    QWidget *m_tabBar;
    QStackedWidget *m_stackedWidget;
    QButtonGroup *m_buttonGroup;
    QWidget *m_glider;
    QPropertyAnimation *m_gliderAnimation;
    QList<ZTab *> m_tabs;
    QList<QWidget *> m_widgets;
    int m_currentIndex;

    void setupGlider();
    void animateGlider(int index);
    void updateTabStyles();
};

#endif // ZTABWIDGET_H