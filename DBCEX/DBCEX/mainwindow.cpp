#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qfiledialog.h"
#include "qmessagebox.h"
#include "strtk.hpp"
#include "qtextcodec.h"

#include "test.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());
    //
    _pGBK = QTextCodec::codecForName("gb18030");// 语言设置为简体中文的excel，默认的csv编码为gbk？？？
    _pUTF8 = QTextCodec::codecForName("utf8");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_saveFile_clicked()
{
    QString csvFile = ui->csvPath->toPlainText();
    if(csvFile.isEmpty())
    {
        QMessageBox::warning(this, tr("提示信息"), tr("选择一个.CSV文件"), tr("确  定"));
        return;
    }

    QString outPath = ui->outPath->toPlainText();
    if(outPath.isEmpty())
    {
        QMessageBox::warning(this, tr("提示信息"), tr("设置输出文件路径"), tr("确  定"));
        return;
    }

    ui->closeWindow->setEnabled(false);
    ui->saveFile->setText("请稍等......");
    ui->saveFile->setEnabled(false);

    // 加载解析csv
    // 输出dbcxx
    strtk::token_grid::options options;
    options.column_delimiters = ";";
    //        options.support_dquotes = true;

    strtk::token_grid grid(csvFile.toStdString(), options);
    // 简单判断
    int rowNum = grid.row_count();
    if(rowNum < 3)
    {
        QMessageBox::warning(this, tr("提示信息"), tr("不是有效的DBCXX文件"), tr("确  定"));
        return;
    }
    if(3 == rowNum)
    {
        QMessageBox::warning(this, tr("提示信息"), tr("没有要输出内容"), tr("确  定"));
        return;
    }
    // 打开文件
    FILE *pFile = fopen(outPath.toStdString().c_str(), "wb");
    if(NULL == pFile)
    {
        QMessageBox::warning(this, tr("提示信息"), tr("异常，创建输出文件失败！！！"), tr("确  定"));
        return;
    }
    // 输出数据
    //
    std::string tempText;
    QByteArray ba;
    void* ptr;
    int ptrLen = sizeof(void*);
    char cc;
    int cLen = sizeof(char);
    unsigned int uInt;
    int uIntLen = sizeof(unsigned int);
    int ii;
    int iLen = sizeof(int);
    float ff;
    int fLen = sizeof(float);
    //
    enum formatType {typeU = 1, typeI, typeC, typeF};
    char format[126] = {};
    format['u'] = typeU;
    format['U'] = typeU;
    format['i'] = typeI;
    format['I'] = typeI;
    format['c'] = typeC;
    format['C'] = typeC;
    format['f'] = typeF;
    format['F'] = typeF;
    // 判断第一行信息
    // 处理第二行类型
    // 写头信息
    int outRowNum = rowNum - 3;
    int colNum = grid.max_column_count();
    //unsigned int textOffset = 0;
    unsigned int textInfoPos = 0;

    int textLen;
    unsigned int outTextSize = 0;
    unsigned int mTextSize = cLen * 1024 * 1024 * 10;// 10M
    char* pTextM = (char*)malloc(mTextSize);

    char head[] = "dbcxx";
    char ver[] = "1.0";

    fwrite(head, cLen, strlen(head), pFile);
    fwrite(ver, cLen, strlen(ver), pFile);
    fwrite(&outRowNum, cLen, iLen, pFile);
    fwrite(&colNum, cLen, iLen, pFile);
    textInfoPos = ftell(pFile);
    fwrite(&outTextSize, cLen, uIntLen, pFile);

    char *colType = new char[colNum];
    strtk::token_grid::row_type row = grid.row(1);
    for(int i = 0; i < colNum; i++)
    {
        cc = row.get<char>(i);
        colType[i] = format[cc];
        fwrite(&cc, cLen, cLen, pFile);
    }

    // 跳过前三行，处理所有数据
    for(int curRow = 3; curRow < rowNum; curRow++)
    {
        row = grid.row(curRow);
        for(int curCol = 0; curCol < colNum; curCol++)
        {
            switch(colType[curCol])
            {
            case typeC:
            {
                tempText = row.get<std::string>(curCol);
                ba = _pGBK->toUnicode(tempText.c_str()).toUtf8();
                textLen = ba.length();
                //
                if(outTextSize + textLen > mTextSize)
                {
                    QMessageBox::warning(this, tr("提示信息"), tr("异常，文字超过默认分配内存大小，找程序修改！！！"), tr("确  定"));

                    on_closeWindow_clicked();
                }
                //
                fwrite(&outTextSize, cLen, uIntLen, pFile);
                memcpy(pTextM + outTextSize, ba.data(), textLen);
                outTextSize += textLen;
                // 输出分割符
                memset(pTextM + outTextSize, '\0', cLen);
                outTextSize += cLen;
            }
                break;
            case typeU:
            {
                uInt = row.get<unsigned int>(curCol);
                fwrite(&uInt, cLen, uIntLen, pFile);
            }
                break;
            case typeI:
            {
                ii = row.get<int>(curCol);
                fwrite(&ii, cLen, iLen, pFile);
            }
                break;
            case typeF:
            {
                ff = row.get<float>(curCol);
                fwrite(&ff, cLen, fLen, pFile);
            }
                break;
            default:
            {
                QMessageBox::warning(this, tr("提示信息"), tr("异常，csv不确定的列类型"), tr("确  定"));

                on_closeWindow_clicked();
            }
            }
        }
    }

    // 输出文字
    if(outTextSize > 0)
        fwrite(pTextM, cLen, outTextSize, pFile);

    // 设置实际文字尺寸信息
    fseek(pFile, textInfoPos, SEEK_SET);
    fwrite(&outTextSize, cLen, uIntLen, pFile);

    // 清理
    free(pTextM);
    delete []colType;
    fclose(pFile);

    //
    QMessageBox::warning(this, tr("提示信息"), tr("转换完成"), tr("确  定"));

    // 激活按钮
    ui->closeWindow->setEnabled(true);
    ui->saveFile->setText("转换CSV到DBCXX");
    ui->saveFile->setEnabled(true);
}

void MainWindow::on_closeWindow_clicked()
{
    this->close();
}

void MainWindow::on_selCSV_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("打开CSV文件"), "", tr("*.csv"));
    if(!filename.isEmpty())
        ui->csvPath->setText(filename);
}

void MainWindow::on_selPath_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("保存为DBCXX文件"), "", tr("*.dbcxx"));
    if(!filename.isEmpty())
        ui->outPath->setText(filename);
}
