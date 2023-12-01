#include "LogManager.h"


#include <QFile>
#include <iostream>
#include <string>
#include <vector>
#include <QTextStream>
LogManager::LogManager()
{

}

void LogManager::Write(std::string className, std::string funcName, std::string comment, std::string data)
{
    std::string mergedName = className + "," + funcName + "," + comment + "," + data + "\n";

    QFile file("sample.log");
        file.open(QIODevice::Append);
        file.write(mergedName.c_str());
        file.close();
}

void LogManager::trace()
{
    std::string str;
    int i = 0;
    QFile file("sample.log");
    std::string className;
    std::string funcName;
    std::string comment;
    if (!file.open(QIODevice::ReadOnly))//読込のみでオープンできたかチェック
    {
//        qDebug() << "データ読み込み失敗" ;
    }else{
//        qDebug() << "データ読み込み成功" ;
    }

    QTextStream in(&file);
    char del = ',';

    while(!in.atEnd())
    {
        str = in.readLine().toStdString();
        for(const auto subStr : split(str,del))
        {
            switch (i) {
            case 0:
                className = subStr;
                i++;
                break;
            case 1:
                funcName = subStr;
                i++;
                break;
            case 2:
                comment = subStr;
                i++;
                break;
            default:
                break;
            }

        }
        i=0;
//        std::cout << className << std::endl;
//        std::cout << funcName << std::endl;
//        std::cout << str << std::endl;
        std::string len = className + "," + funcName + "," + comment + ",";
//        std::cout << len << std::endl;
//        std::cout << str.substr(len.length(),str.length()) << std::endl;
        check( className, funcName, str.substr(len.length(),str.length()) );
    }

}

void LogManager::check(std::string className, std::string funcName, std::string data)
{

//    if(className.compare("renderarea.cpp") == 0)
//    {
//        std::cout << "YES" << std::endl;
//        if(funcName.compare("mouseReleaseEvent") == 0)
//        {
//            std::cout << "YES" << std::endl;
//            char del = ',';
//            float element[16];
//            int i = 0;

//            for(const auto subStr : split(data,del))
//            {
//                element[i]=stof(subStr);
//                i++;
//                if(i == 16)
//                {
//                    i = 0;
//                }

//            }
//            m_screen->logPlay(element);
//        }
//    }

    double d;
//    double *heap;
    std::string str;
    std::string *heap;
    int i = 0;
    char del = ',';

    int len = std::count(data.begin(),data.end(),del);
    if(len == 0)
    {
        heap = new std::string[1];
        heap[0] = data;
    }else{
        heap = new std::string[len + 1];
        for(const auto subStr : split(data,del))
        {
            heap[i] = subStr;
            std::cout << heap[i] << std::endl;
            i++;
        }
    }

    panelClass panel;
    if(className.compare("renderarea.cpp")            == 0)//0
        panel = RenderAreaP;
    if(className.compare("animationcontrols.cpp")     == 0)//1
        panel = AnimationControlsP;
    if(className.compare("transferfunceditor_CS.cpp") == 0)//2
        panel = TransferFunctionEditorP;
    if(className.compare("coordinatepanel.cpp")       == 0)//3
        panel = CoordinateP;
    if(className.compare("filterinfopanel.cpp")       == 0)//4
        panel = FilterinfoP;
    if(className.compare("legendpanel.cpp")           == 0)//5
        panel = LegendP;
    if(className.compare("particlepanel.cpp")         == 0)//6
        panel = ParticleP;
    if(className.compare("renderoptionpanel.cpp")     == 0)//7
        panel = RenderoptionP;
    if(className.compare("systemstatuspanel.cpp")     == 0)//8
        panel = SystemstatusP;
    if(className.compare("timecontrolpanel.cpp")      == 0)//9
        panel = TimecontralP;
    if(className.compare("transformpanel.cpp")        == 0)//10
        panel = TransformP;
    if(className.compare("viewercontrols.cpp")        == 0)//11
        panel = ViewerControlsP;

    switch(panel)
    {
    case 0:
        m_screen->logPlay(funcName,heap);
        break;
    case 1:
        m_animationControls->logPlay(funcName,heap);
        break;
    case 2:
        m_tf_editor->logPlay(funcName,heap);
        break;
    case 3:
        m_coordinatePanel->logPlay(funcName,heap);
        break;
    case 4:
        break;
    case 5:
        m_legendPanel->logPlay(funcName,heap);
        break;
    case 6:
        m_particlePanel->logPlay(funcName, heap);
        break;
    case 7:
        m_renderoptionPanel->logPlay(funcName,heap);
        break;
    case 8:
//        m_systemstatusPanel->logPlay(funcName,heap);
        break;
    case 9:
//        m_timecontrolPanel->logPlay(funcName, heap);
        break;
    case 10:

        m_transformPanel->logPlay(funcName,heap);
        break;

    case 11:        
        m_vc->logPlay(funcName, heap);
        break;

    default:
        break;
    }
    delete[] heap;
}

std::vector<std::string> LogManager::split(std::string str, char del) {
    int first = 0;
    int last = str.find_first_of(del);

    std::vector<std::string> result;

    while (first < str.size())
    {
        std::string subStr(str, first, last - first);

        result.push_back(subStr);

        first = last + 1;
        last = str.find_first_of(del, first);

        if (last == std::string::npos)
        {
            last = str.size();
        }
    }

    return result;
}
