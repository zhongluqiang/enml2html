#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
using namespace std;

const char *HEADER =
    "<!DOCTYPE html> \n"
    "<html>\n"
    "<head>\n"
    "<meta charset=\"utf-8\">\n"
    "<title>这是一篇印象笔记导出的笔记</title>\n"
    "</head>\n"
    "<body style=\"width:1000px; margin-left:auto; margin-right:auto;\">\n";
const char *TAIL = "\n</body>\n</html>";

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace)))
                .base(),
            s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

void replaceMediaTags(string &body) {
    regex pattern("(<en-media[^>]*?>)");
    vector<string> enmdia_array;

    /* 找到所有的en-media标签 */
    for (sregex_iterator it(body.begin(), body.end(), pattern), end_it;
         it != end_it; ++it) {
        cout << it->str() << endl;
        enmdia_array.push_back(it->str());
    }

    /* 正则表达式对象的赋值非常消耗时间，不应放在循环体内部(by C++ Primier 5th)
     */
    pattern = "hash=\"([^\"]*)\"";

    /* 将所有的en-media标签转化成img标签 */
    for (auto i = enmdia_array.begin(); i != enmdia_array.end(); ++i) {
        /* 获取hash值，暂时使用hash值作为img的url地址 */
        smatch hash_matches;
        if (regex_search(*i, hash_matches, pattern)) {
            cout << hash_matches.str(1) << endl;

            /* 将en-media标签整体替换成img标签 */
            // todo 转换成正解的资源名称和图片属性
            string url = "<img src=\"";
            url += hash_matches.str(1);
            url += ".jpg\"/>";
            cout << url << endl;
            size_t start_pos = body.find(*i);
            if (start_pos != std::string::npos) {
                body.replace(start_pos, i->length(), url);
            }
        }
    }
}

void replaceCheckboxes(string &body) {
    regex pattern("<en-todo[^>]*?>");
    vector<string> entodo_array;

    /* 找到所有的en-todo标签 */
    for (sregex_iterator it(body.begin(), body.end(), pattern), end_it;
         it != end_it; ++it) {
        cout << it->str() << endl;
        entodo_array.push_back(it->str());
    }

    pattern = "true";
    /* 将en-todo标签转化成input标签 */
    for (auto i = entodo_array.begin(); i != entodo_array.end(); ++i) {
        string input_tag = "<input type=\"checkbox\" name=\"unused\" "
                           "value=\"unused\" disabled ";
        /* 获取entodo的checked属性是否为true */
        smatch checked_maches;
        if (regex_search(*i, checked_maches, pattern)) {
            cout << checked_maches.str() << endl;

            if (string("true") == checked_maches.str()) {
                input_tag += "checked>";
            }
        } else {
            input_tag += ">";
        }

        cout << input_tag << endl;
        /* 将en-todo标签整体替换成input标签 */
        size_t start_pos = body.find(*i);
        if (start_pos != std::string::npos) {
            body.replace(start_pos, i->length(), input_tag);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: ./enml2html input output\n");
        return -1;
    }

    ifstream in;
    ofstream out;

    in.open(argv[1]);
    out.open(argv[2]);
    if (!in || !out) {
        cerr << "open file failed" << endl;
        return -1;
    }

    stringstream buffer;
    buffer << in.rdbuf();
    string body = buffer.str();

    /* 通过正则表达式对笔记的内容进行格式转换 */

    /* 删除xml版本声明 */
    regex reg("(<\\?xml[^>]*?>)");
    body = regex_replace(body, reg, "");

    /* 删除DOCTYPE声明 */
    reg = "(<!DOCTYPE[^>]*?>)";
    body = regex_replace(body, reg, "");

    /* 删除<en-note>标签对 */
    reg = "(<en-note[^>]*?>)";
    body = regex_replace(body, reg, "");
    reg = "(</en-note>)";
    body = regex_replace(body, reg, "");

    /* 删除<en-crypt>标签对及其内容 */
    reg = "<en-crypt[^>]*?>[\\s\\S]*?<\\/en-crypt>";
    body = regex_replace(body, reg, "");

    /* 删除印象笔记的<del>标签 */
    reg = "(<del[^>]*?>[\\s\\S]*?<\\/del>)";
    body = regex_replace(body, reg, "");

    /* 删除第1个div标签的属性，这项主要是针对马克飞象编辑的笔记。印象笔记全部的
     * 笔记内容都处在第一个div内，这个div的属性会影响整篇内容。如果是用印象笔记
     * 客户端编辑的笔记，那这个div的内容一般为空，而用马克飞象编辑的笔记，这个
     * div的属性会不为空。将其属性删除之后，马克飞象编辑过的文本字会更清晰一些，
     * 因为马克飞象默认的属性会对字体进行一度的模糊处理 */
    reg = "(<div)(\\s*[^>]*?)(>)";
    /* 使用format_first_only指定只替换子表达式的第1次出现 */
    body = regex_replace(body, reg, "$1$3", regex_constants::format_first_only);

    /* 删除印象笔记的隐藏标签<center> */
    reg = "(<center[^>]*?display[^>]*?>[\\s\\S]*?<\\/center>)";
    body = regex_replace(body, reg, "");

    /* 转换<en-media>中的图片标签，非图片标签转换成下载链接 */
    replaceMediaTags(body);

    /* 清除空的<div>标签 */
    reg = "<div[^>]*?><\\/div>";
    body = regex_replace(body, reg, "");

    /* 清除前后空格 */
    trim(body);

    /* 清除不需要的标签，ENML基于XHTML，增加的元素有些不被HTML支持，需要删除 */
    // todo...

    /* 删除印象笔记自带的高亮style属性的-evernote-highlight:true字段
     * <xxx style="...;-evernote-highlight:true;"> */
    reg = "(<\\w+\\s+style=\"[^>]*?;)(-evernote-highlight:true;)(\">)";
    body = regex_replace(body, reg, "$1$3");

    /* 删除印象笔记自带的代码格式中的sytle属性的-en-codeblock:true字段
     * <div style="...;-en-codeblock:true;"> */
    reg = "(<div\\s+style=\"[^>]*?;)(-en-codeblock:true;)(\">)";
    body = regex_replace(body, reg, "$1$3");

    /* 将印象笔记自带的en-todo复选框转换成HTML的input标签，使用type="checkbox"
     * 属性使其转变成复选框形式，使用checked属性选中复选框，使用disabled使复选框
     * 不可编辑 */
    replaceCheckboxes(body);

    /* 备注：evernote-sync插件中对印象笔记自带的代码格式进行了转换，将div标签
     * 转换成br标签，实际测试发现不需要转换就可以正常显示；同样，pre标签下的马克
     * 飞象笔记代码也不需要转换，否则由于pre的预格式定义功能，代码格式会出现错乱
     */

    /* 备注：evernote-sync插件会对印象笔记中单独的行进行字体大小判断，根据字体
     * 大小将其重新格式化成HTML的h1~h6标题，这一部并不是必须的，可跳过 */

    /* 清除标签属性，实际测试发现，删除div p pre的属性对马克飞象的笔记显示效果有
     * 一点点提升，主要是字体辨识度提高了一些，所以这三个标签的属性可以删除，但大
     * 部分标签的属性是不可以删除的，例如h1~h6, code, span等，这些标签的属性一旦
     * 删除会影响显著影响原有的格式，所以不宜删除 */
    // reg = "(<div|p|pre)(\\s+[^>]*?)(>)";
    // body = regex_replace(body, reg, "$1$3");

    /* 剩下的是一些收尾工作，目的是使html文档的格式更整洁一些 */

    /* 格式化br标签，删除属性，全部变成<br/> */
    reg = "<br[^>]*?>";
    body = regex_replace(body, reg, "<br/>");

    /* 清除重复的<br/>标签 */
    reg = "(<br\\/>){2,10}";
    body = regex_replace(body, reg, "$1");

    /* 清除<div.../>、<p.../>、<span.../>，这些标签是只有标签但没有内容的 */
    reg = "<(div|p|span)((\\s[^>]*)?)\\/>";
    body = regex_replace(body, reg, "");

    /* 清除空标签 <xxx>   </xxx>*/
    /* 这串表达式的原始内容是：<([0-9a-zA-Z]+)[^>\/]*?>[\s]*?<\/\1> 最后一个
    \1表示的是第一个表达式子串所匹配到的值 */
    reg = "<([0-9a-zA-Z]+)[^>\\/]*?>[\\s]*?<\\/\\1>";
    body = regex_replace(body, reg, "");

    /* 清除空标签 <xxx> <br/> </xxx> */
    /* 印象笔记原生笔记使用<div><br/></div>来表示空白行，所这条不能删除 */
    // reg = "<([0-9a-zA-Z]+)[^>\\/]*?>[\\s]*?<br/>[\\s]*?<\\/\\1>";
    // body = regex_replace(body, reg, "");

    /* 清理无用的<br/> */
    /* 使用马克飞象编辑的笔记中的有序列表使用<br/>来进行换行，这一条不可以删除
     */
    // reg = "<br\\/>";
    // body = regex_replace(body, reg, "");

    /* 压缩空白行，暂时还没想到行好的办法，todo...*/
    /* 下面的压缩语句有bug，处理pre标签时如果将内部的span标签之间的空格删除掉的话
     * 会导致马克飞象代码块中的空格全部消失 */
    // reg = ">(\\s+?)<";
    // body = regex_replace(body, reg, "><");

    out << HEADER;
    out << body;
    out << TAIL;
    in.close();
    out.close();
    return 0;
}