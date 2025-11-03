#ifndef KEYCHAIN_DIALOG_H
#define KEYCHAIN_DIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QMediaPlayer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVideoWidget>

class KeychainDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KeychainDialog(QWidget *parent = nullptr);
    ~KeychainDialog();

private slots:
    void onOkClicked();
    void onSkipSigningInClicked();

private:
    void setupUI();
    void setupVideo();

    QMediaPlayer *m_player;
    QVideoWidget *m_videoWidget;
    QVBoxLayout *m_mainLayout;
    QPushButton *m_okButton;
    QPushButton *m_skipSigningInButton;
    QLabel *m_titleLabel;
    QLabel *m_descriptionLabel;
    QCheckBox *m_dontShowAgainCheckbox;
};

#endif // KEYCHAIN_DIALOG_H