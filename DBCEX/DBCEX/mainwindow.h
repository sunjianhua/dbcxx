#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_saveFile_clicked();

    void on_closeWindow_clicked();

    void on_selCSV_clicked();

    void on_selPath_clicked();

private:
    Ui::MainWindow *ui;
    QTextCodec *_pGBK;
    QTextCodec *_pUTF8;
};

#endif // MAINWINDOW_H
