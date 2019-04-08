# enml2html
C++实现的Evernote/印象笔记ENML标记语言转HTML工具，使用方法 ./enml2html input.enml output.html

------

格式转化过程参考了Wordpress上的[evernote-sync](https://cn.wordpress.org/plugins/evernote-sync/)插件，在其基础上将PHP语法的正则运算转为C++11 regex类支持的正则运算，并修改了部分流程，以适合非Wordpress环境。

这是一个目前还相当不成熟的项目，只适合作为入门印象笔记标记语言ENML及练习C++11 regex正则表达式操作的人使用。后续的计划是并入[印象笔记C++ SDK快速入门项目](https://github.com/zhongluqiang/evernote-sdk-cpp-quickstart)，作为其中的一个子功能来完成导出html文件功能。

目录仍有以下问题待解决：

1. 笔记中的资源文件（图片，附件等）的超链接未实现；
2. 当未设置`ulimit -s unlimited`时，程序运行过程有可能出现段错误，猜测是正则表达式太消耗栈内存导致的；