#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qfiledialog.h"
#include "qmessagebox.h"
#include "strtk.hpp"
#include "qtextcodec.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());
    ui->csvPath->setText(tr("/Users/sunjianhua/Desktop/test.csv"));
    ui->outPath->setText(tr("/Users/sunjianhua/Desktop/test.dbcxx"));
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

    // 跳过第三行，处理所有数据
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
    //textOffset = ftell(pFile);

    if(outTextSize > 0)
        fwrite(pTextM, cLen, outTextSize, pFile);

    // 设置实际文字位置信息
    fseek(pFile, textInfoPos, SEEK_SET);
//    fwrite(&textOffset, cLen, uIntLen, pFile);
    fwrite(&outTextSize, cLen, uIntLen, pFile);
    // 清理
    free(pTextM);
    delete []colType;
    fclose(pFile);
}

void MainWindow::on_closeWindow_clicked()
{
    this->close();
}

struct TestX
{
    int pText;
    int i;
    unsigned int u;
    float f;
    int pText1;
};

void MainWindow::on_selCSV_clicked()
{
    QString outPath = ui->outPath->toPlainText();
    if(outPath.isEmpty())
    {
        QMessageBox::warning(this, tr("提示信息"), tr("阿斯顿发送到发生发顺丰"), tr("确  定"));
        return;
    }

    // 打开文件
    FILE *pFile = fopen(outPath.toStdString().c_str(), "rb");
    if(NULL == pFile)
    {
        QMessageBox::warning(this, tr("提示信息"), tr("异常，打开输出文件失败！！！"), tr("确  定"));
        return;
    }

    //
    void* ptr;
    int ptrLen = sizeof(void*);
    char cc;
    int cLen = sizeof(char);
    unsigned int uInt;
    int uIntLen = sizeof(unsigned int);
    int ii;
    int iLen = sizeof(int);
    int ff;
    int fLen = sizeof(float);

    // 读头
    char head[6] = ""; // dbcxx";
    char ver[3] = ""; // 1.0";
    fread(head, cLen, 5, pFile);
    fread(ver, cLen, 3, pFile);
    if(0 != strcmp(head, "dbcxx") || 0 != strcmp(ver, "1.0"))
    {
        QMessageBox::warning(this, tr("提示信息"), tr("异常，错误的dbcxx类型"), tr("确  定"));
        on_closeWindow_clicked();
    }
    int rowNum = 0;
    int colNum = 0;
    unsigned int textSize = 0;
    fread(&rowNum, cLen, iLen, pFile);
    fread(&colNum, cLen, iLen, pFile);
    fread(&textSize, cLen, uIntLen, pFile);

    char *colType = new char[colNum + 1];
    memset(colType, 0, colNum + 1);
    fread(colType, cLen, colNum, pFile);
    // 判断格式是否相同
    ;//
    // 读数据
    TestX *testX = new TestX[rowNum];
    int www = sizeof(TestX);
    int ttt = ptrLen + ptrLen + uIntLen + fLen + iLen;
    fread(testX, cLen, sizeof(TestX) * rowNum, pFile);
    // 读文本
    unsigned int tOffset = ftell(pFile);
    fseek(pFile, 0, SEEK_END);
    unsigned int tSize = ftell(pFile);
    tSize -= tOffset;
    fseek(pFile, tOffset, SEEK_SET);
    if(tSize != textSize)
    {
        QMessageBox::warning(this, tr("提示信息"), tr("异常，文件损坏"), tr("确  定"));
        on_closeWindow_clicked();
    }

    char* textInfo = new char[textSize];
    fread(textInfo, cLen, textSize, pFile);

    QByteArray ba(textInfo + testX[19].pText1);

    QString xxx(ba);// = _pUTF8->toUnicode(textInfo);
    QMessageBox::warning(this, tr("提示信息"), xxx, tr("确  定"));

    // 清理
    delete []testX;
    delete []textInfo;
    delete []colType;
    fclose(pFile);
//    QString filename = QFileDialog::getOpenFileName(this, tr("打开CSV文件"), "", tr("*.csv"));
//    if(!filename.isEmpty())
//        ui->csvPath->setText(filename);
}

void MainWindow::on_selPath_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("保存为DBCXX文件"), "", tr("*.dbcxx"));
    if(!filename.isEmpty())
        ui->outPath->setText(filename);
}
