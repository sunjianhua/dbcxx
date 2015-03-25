/*
 *  使用介绍和应用范围看：https://github.com/sunjianhua/dbcxx
 *  因为字符对齐的问题，所以用dbcxx管理的数据结构，不要出现指针，因为32位和64位指针长度是不同的
 *  因为字符对齐的问题，所以用dbcxx管理的数据结构，不要出现long和short关键字，原因同上
 *  在mac的64位操作系统，int、unsigned int、float的长度为4，和32位操作系统下相同，手机平台没测试过
 */

#ifndef DBCXX_H
#define DBCXX_H

#include <stdio.h>
#include <string.h>

template<class T>
class dbcxx
{
protected:
    T* _pEntries;
    unsigned int _rowNum;
    unsigned int _colNum;
    char* _pStringData;
    unsigned int _textSize;

public:
    dbcxx() : _pEntries(NULL), _rowNum(0), _colNum(0), _pStringData(NULL), _textSize(0)
    {}

    ~dbcxx()
    {
        cleanup();
    }

    bool load(const char* pFilename, const char* pFormat)
    {
        //static void* ptr;
        //static int ptrLen = sizeof(void*);// 32位下4，64位下8
        //static char cc;
        static int cLen = sizeof(char);
        //static unsigned int uInt;
        static int uIntLen = sizeof(unsigned int);
        //static int ii;
        static int iLen = sizeof(int);
        //static int ff;
        //static int fLen = sizeof(float);
        static const char curVer[] = "1.0";

        if(_pEntries)
        {
            ;// 设置错误码
            return false;
        }

        //
        if(!pFilename || !pFormat)
        {
            ;// 设置错误码
            return false;
        }

        // 打开文件
        FILE *pFile = fopen(pFilename, "rb");
        if(0 == pFile)
        {
            ;// 设置错误码
            return false;
        }

        // 读头
        char head[6] = ""; // dbcxx";
        char ver[4] = ""; // 1.0";
        fread(head, cLen, 5, pFile);
        fread(ver, cLen, 3, pFile);
        if(0 != strcmp(head, "dbcxx") || 0 != strcmp(ver, curVer))
        {
            fclose(pFile);
            ;// 设置错误码
            return false;
        }

        fread(&_rowNum, cLen, iLen, pFile);
        fread(&_colNum, cLen, iLen, pFile);
        fread(&_textSize, cLen, uIntLen, pFile);

        // 判断格式是否相同
        char *pColType = new char[_colNum + 1];
        memset(pColType, 0, _colNum + 1);
        fread(pColType, cLen, _colNum, pFile);
        if(0 != strcmp(pColType, pFormat))
        {
            delete []pColType;
            fclose(pFile);
            ;// 设置错误码
            return false;
        }

        // 读数据
        _pEntries = new T[_rowNum];
        fread(_pEntries, cLen, sizeof(T) * _rowNum, pFile);
        // 读文本
        //        // 判断文字区域大小是否和读取的文字大小相同
        //        unsigned int tOffset = ftell(pFile);
        //        fseek(pFile, 0, SEEK_END);
        //        unsigned int tSize = ftell(pFile);
        //        tSize -= tOffset;
        //        fseek(pFile, tOffset, SEEK_SET);
        //        if(tSize != _textSize)
        //        {
        //            delete []_pEntries;
        //            _pEntries = NULL;
        //            delete []pColType;
        //            fclose(pFile);
        //            ;// 设置错误码
        //            return false;
        //        }

        _pStringData = new char[_textSize];
        fread(_pStringData, cLen, _textSize, pFile);

        // 清理
        delete []pColType;
        fclose(pFile);

        //
        return true;
    }

    T* LookupEntry(unsigned int i)
    {
        if(!_pEntries || i >= _rowNum)
            return NULL;

        return _pEntries + i;
    }

    unsigned int getNumRows()
    {
        return _rowNum;
    }

protected:
    void cleanup()
    {
        if(_pEntries)
        {
            delete []_pEntries;
            _pEntries = NULL;
        }
        if(_pStringData)
        {
            delete []_pStringData;
            _pStringData = NULL;
        }
    }
};

#endif // DBCXX_H
