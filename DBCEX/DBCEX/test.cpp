/*
 *  使用介绍和应用范围看：https://github.com/sunjianhua/dbcxx
 *  因为字符对齐的问题，所以用dbcxx管理的数据结构，不要出现指针，因为32位和64位指针长度是不同的
 *  因为字符对齐的问题，所以用dbcxx管理的数据结构，不要出现long和short关键字，原因同上
 *  在mac的64位操作系统，int、unsigned int、float的长度为4，和32位操作系统下相同，手机平台没测试过
 */
#include "test.h"

// 注意，出现字符指针的地方，都用unsigned int替代，原因见最上注释
struct Spell
{
    unsigned int id;        // 技能的id，和它在dbcxx里的索引位置相同（数组下标）
    unsigned int name;      // 技能显示名字，用unsigned int保存的是文字信息在dbcxx字符组里的偏移量
    int level;              // 技能等级
    float damage;           // 伤害
    unsigned int image;     // 技能用到图片，用unsigned int保存的是文字信息在dbcxx字符组里的偏移量
    int type;               // 类型
};

dbcxx<Spell> dbcxxSpell;

// 注意，在指定格式的时候，字符用'c'，int用'i'，unsigned int用'u', flaot用'f'
// 这个主要是要和策划文档里的对应，用来检测数据格式是否相同，保证程序和策划之间的同步
const char* SpellFormat =
        "u" // Id
        "c" // name
        "i" // level
        "f" // damage
        "c" // image
        "i" // type
        ;

template<class T>
bool loader_stub(const char* filename, const char* format, T& l)
{
    ;// 输出提示信息
    return l.load(filename, format);
}

#define LOAD_DBC(filename, format, stor) if(!loader_stub(filename, format, stor)) { return false; }

bool LoadDBCs()
{
    LOAD_DBC("/Users/sunjianhua/Desktop/spell.dbcxx", SpellFormat, dbcxxSpell);

    for(int i = 0; i < dbcxxSpell.getNumRows(); i++)
    {
        Spell* pSpell = dbcxxSpell.LookupEntry(i);
        if(!pSpell)
        {
            ;// 错误处理
            return false;
        }
    }

    return true;
}
